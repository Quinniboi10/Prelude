#include "nnue.h"
#include "config.h"
#include "board.h"
#include "util.h"
#include "accumulator.h"

#include <fstream>
#include <format>
#include <immintrin.h>

i16 NNUE::ReLU(const i16 x) {
    if (x < 0)
        return 0;
    return x;
}

i16 NNUE::CReLU(const i16 x) {
    if (x < 0)
        return 0;
    else if (x > QA)
        return QA;
    return x;
}

// Finds the input feature
usize NNUE::feature(Color perspective, Color color, PieceType piece, Square square) {
    int colorIndex  = (perspective == color) ? 0 : 1;
    int squareIndex = (perspective == BLACK) ? flipRank(square) : static_cast<int>(square);

    return colorIndex * 64 * 6 + piece * 64 + squareIndex;
}

void NNUE::loadNetwork(const string& filepath) {
    std::ifstream stream(filepath, std::ios::binary);
    if (!stream.is_open()) {
        cerr << "Failed to open file: " + filepath << endl;
        cerr << "Expect engine to not work as intended with bad evaluation" << endl;
    }

    // Load weightsToHL
    for (usize i = 0; i < weightsToHL.size(); ++i) {
        weightsToHL[i] = readLittleEndian<int16_t>(stream);
    }

    // Load hiddenLayerBias
    for (usize i = 0; i < hiddenLayerBias.size(); ++i) {
        hiddenLayerBias[i] = readLittleEndian<int16_t>(stream);
    }

    // Load weightsToOut
    for (usize i = 0; i < weightsToOut.size(); ++i) {
        for (usize j = 0; j < weightsToOut[i].size(); j++) {
            weightsToOut[i][j] = readLittleEndian<int16_t>(stream);
        }
    }

    // Load outputBias
    for (usize i = 0; i < outputBias.size(); ++i) {
        outputBias[i] = readLittleEndian<int16_t>(stream);
    }
}

// Returns the output of the NN
int NNUE::forwardPass(const Board* board) {
    const usize divisor      = 32 / OUTPUT_BUCKETS;
    const usize outputBucket = (popcount(board->pieces()) - 2) / divisor;

    // Determine the side to move and the opposite side
    Color stm = board->stm;

    const Accumulator& accumulatorSTM = stm ? board->accumulators.white : board->accumulators.black;
    const Accumulator& accumulatorOPP = ~stm ? board->accumulators.white : board->accumulators.black;

    // Accumulate output for STM and OPP using separate weight segments
    i64 eval = 0;

    if constexpr (ACTIVATION != ::SCReLU) {
        for (usize i = 0; i < HL_SIZE; i++) {
            // First HL_SIZE weights are for STM
            if constexpr (ACTIVATION == ::ReLU)
                eval += ReLU(accumulatorSTM[i]) * weightsToOut[outputBucket][i];
            if constexpr (ACTIVATION == ::CReLU)
                eval += CReLU(accumulatorSTM[i]) * weightsToOut[outputBucket][i];

            // Last HL_SIZE weights are for OPP
            if constexpr (ACTIVATION == ::ReLU)
                eval += ReLU(accumulatorOPP[i]) * weightsToOut[outputBucket][HL_SIZE + i];
            if constexpr (ACTIVATION == ::CReLU)
                eval += CReLU(accumulatorOPP[i]) * weightsToOut[outputBucket][HL_SIZE + i];
        }
    }
    else {
        const __m256i vec_zero = _mm256_setzero_si256();
        const __m256i vec_qa   = _mm256_set1_epi16(QA);
        __m256i       sum      = vec_zero;

        for (usize i = 0; i < HL_SIZE / 16; i++) {
            const __m256i us   = _mm256_load_si256(reinterpret_cast<const __m256i*>(&accumulatorSTM[16 * i]));  // Load from accumulator
            const __m256i them = _mm256_load_si256(reinterpret_cast<const __m256i*>(&accumulatorOPP[16 * i]));

            const __m256i us_weights   = _mm256_load_si256(reinterpret_cast<const __m256i*>(&weightsToOut[outputBucket][16 * i]));  // Load from net
            const __m256i them_weights = _mm256_load_si256(reinterpret_cast<const __m256i*>(&weightsToOut[outputBucket][HL_SIZE + 16 * i]));

            const __m256i us_clamped   = _mm256_min_epi16(_mm256_max_epi16(us, vec_zero), vec_qa);
            const __m256i them_clamped = _mm256_min_epi16(_mm256_max_epi16(them, vec_zero), vec_qa);

            const __m256i us_results   = _mm256_madd_epi16(_mm256_mullo_epi16(us_weights, us_clamped), us_clamped);
            const __m256i them_results = _mm256_madd_epi16(_mm256_mullo_epi16(them_weights, them_clamped), them_clamped);

            sum = _mm256_add_epi32(sum, us_results);
            sum = _mm256_add_epi32(sum, them_results);
        }

        __m256i v1 = _mm256_hadd_epi32(sum, sum);
        __m256i v2 = _mm256_hadd_epi32(v1, v1);

        eval = _mm256_extract_epi32(v2, 0) + _mm256_extract_epi32(v2, 4);
    }


    // Dequantization
    if constexpr (ACTIVATION == ::SCReLU)
        eval /= QA;

    eval += outputBias[outputBucket];

    // Apply output bias and scale the result
    return (eval * EVAL_SCALE) / (QA * QB);
}

// Debug feature based on SF
void NNUE::showBuckets(const Board* board) {
    const usize divisor     = 32 / OUTPUT_BUCKETS;
    const usize usingBucket = (popcount(board->pieces()) - 2) / divisor;

    int staticEval = 0;

    cout << "+------------+------------+" << endl;
    cout << "|   Bucket   | Evaluation |" << endl;
    cout << "+------------+------------+" << endl;

    for (usize outputBucket = 0; outputBucket < OUTPUT_BUCKETS; outputBucket++) {
        // Determine the side to move and the opposite side
        Color stm = board->stm;

        const Accumulator& accumulatorSTM = stm ? board->accumulators.white : board->accumulators.black;
        const Accumulator& accumulatorOPP = ~stm ? board->accumulators.white : board->accumulators.black;

        // Accumulate output for STM and OPP using separate weight segments
        i64 eval = 0;

        if constexpr (ACTIVATION != ::SCReLU) {
            for (usize i = 0; i < HL_SIZE; i++) {
                // First HL_SIZE weights are for STM
                if constexpr (ACTIVATION == ::ReLU)
                    eval += ReLU(accumulatorSTM[i]) * weightsToOut[outputBucket][i];
                if constexpr (ACTIVATION == ::CReLU)
                    eval += CReLU(accumulatorSTM[i]) * weightsToOut[outputBucket][i];

                // Last HL_SIZE weights are for OPP
                if constexpr (ACTIVATION == ::ReLU)
                    eval += ReLU(accumulatorOPP[i]) * weightsToOut[outputBucket][HL_SIZE + i];
                if constexpr (ACTIVATION == ::CReLU)
                    eval += CReLU(accumulatorOPP[i]) * weightsToOut[outputBucket][HL_SIZE + i];
            }
        }
        else {
            const __m256i vec_zero = _mm256_setzero_si256();
            const __m256i vec_qa   = _mm256_set1_epi16(QA);
            __m256i       sum      = vec_zero;

            for (usize i = 0; i < HL_SIZE / 16; i++) {
                const __m256i us   = _mm256_load_si256(reinterpret_cast<const __m256i*>(&accumulatorSTM[16 * i]));  // Load from accumulator
                const __m256i them = _mm256_load_si256(reinterpret_cast<const __m256i*>(&accumulatorOPP[16 * i]));

                const __m256i us_weights   = _mm256_load_si256(reinterpret_cast<const __m256i*>(&weightsToOut[outputBucket][16 * i]));  // Load from net
                const __m256i them_weights = _mm256_load_si256(reinterpret_cast<const __m256i*>(&weightsToOut[outputBucket][HL_SIZE + 16 * i]));

                const __m256i us_clamped   = _mm256_min_epi16(_mm256_max_epi16(us, vec_zero), vec_qa);
                const __m256i them_clamped = _mm256_min_epi16(_mm256_max_epi16(them, vec_zero), vec_qa);

                const __m256i us_results   = _mm256_madd_epi16(_mm256_mullo_epi16(us_weights, us_clamped), us_clamped);
                const __m256i them_results = _mm256_madd_epi16(_mm256_mullo_epi16(them_weights, them_clamped), them_clamped);

                sum = _mm256_add_epi32(sum, us_results);
                sum = _mm256_add_epi32(sum, them_results);
            }

            __m256i v1 = _mm256_hadd_epi32(sum, sum);
            __m256i v2 = _mm256_hadd_epi32(v1, v1);

            eval = _mm256_extract_epi32(v2, 0) + _mm256_extract_epi32(v2, 4);
        }


        // Dequantization
        if constexpr (ACTIVATION == ::SCReLU)
            eval /= QA;

        eval += outputBias[outputBucket];

        // Apply output bias and scale the result
        staticEval = (eval * EVAL_SCALE) / (QA * QB);

        cout << "| " << padStr(std::to_string(outputBucket), 11, 0) << "|  " << (staticEval > 0 ? "+" : "-") << " " << padStr(std::format("{:.2f}", std::abs(staticEval / 100.0)), 8, 0) << "|";
        if (outputBucket == usingBucket)
            cout << " <- Current bucket";
        cout << endl;
        if (outputBucket == OUTPUT_BUCKETS - 1)
            cout << "+------------+------------+" << endl;
    }
}