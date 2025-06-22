#include "nnue.h"
#include "config.h"
#include "board.h"
#include "util.h"
#include "thread.h"
#include "accumulator.h"
#include "search.h"

#include "../external/fmt/fmt/format.h"

#include <fstream>
#include <cstring>
#include <algorithm>

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

i32 NNUE::SCReLU(const i16 x) {
    if (x < 0)
        return 0;
    else if (x > QA)
        return QA * QA;
    return x * x;
}

#if defined(__x86_64__) || defined(__amd64__) || (defined(_WIN64) && (defined(_M_X64) || defined(_M_AMD64)) || defined(__ARM_NEON))
    #ifndef __ARM_NEON
        #include <immintrin.h>
    #endif
    #if defined(__AVX512F__)
        #pragma message("Using AVX512 NNUE inference")
using Vectori16 = __m512i;
using Vectori32 = __m512i;
        #define set1_epi16 _mm512_set1_epi16
        #define load_epi16(x) _mm512_load_si512(reinterpret_cast<const Vectori16*>(x))
        #define min_epi16 _mm512_min_epi16
        #define max_epi16 _mm512_max_epi16
        #define madd_epi16 _mm512_madd_epi16
        #define mullo_epi16 _mm512_mullo_epi16
        #define add_epi32 _mm512_add_epi32
        #define reduce_epi32 _mm512_reduce_add_epi32
    #elif defined(__AVX2__)
        #pragma message("Using AVX2 NNUE inference")
using Vectori16 = __m256i;
using Vectori32 = __m256i;
        #define set1_epi16 _mm256_set1_epi16
        #define load_epi16(x) _mm256_load_si256(reinterpret_cast<const Vectori16*>(x))
        #define min_epi16 _mm256_min_epi16
        #define max_epi16 _mm256_max_epi16
        #define madd_epi16 _mm256_madd_epi16
        #define mullo_epi16 _mm256_mullo_epi16
        #define add_epi32 _mm256_add_epi32
        #define reduce_epi32 \
            [](Vectori32 vec) { \
                __m128i xmm1 = _mm256_extracti128_si256(vec, 1); \
                __m128i xmm0 = _mm256_castsi256_si128(vec); \
                xmm0         = _mm_add_epi32(xmm0, xmm1); \
                xmm1         = _mm_shuffle_epi32(xmm0, 238); \
                xmm0         = _mm_add_epi32(xmm0, xmm1); \
                xmm1         = _mm_shuffle_epi32(xmm0, 85); \
                xmm0         = _mm_add_epi32(xmm0, xmm1); \
                return _mm_cvtsi128_si32(xmm0); \
            }
    #elif defined(__ARM_NEON)
        #include <arm_neon.h>
        #pragma message("Using NEON NNUE inference")
using Vectori16 = int16x8_t;
using Vectori32 = int32x4_t;
        #define set1_epi16 vdupq_n_s16
        #define load_epi16(x) vld1q_s16(reinterpret_cast<const i16*>(x))
        #define min_epi16 vminq_s16
        #define max_epi16 vmaxq_s16
        #define madd_epi16 \
            [](Vectori16 a, Vectori16 b) { \
                const Vectori16 low = vmull_s16(vget_low_s16(a), vget_low_s16(b)); \
                const Vectori16 high = vmull_high_s16(a, b); \
                return vpaddq_s32(low, high); \
            }
        #define mullo_epi16 vmulq_s16
        #define add_epi32 vaddq_s32
        #define reduce_epi32 vaddvq_s32
    #else
        #pragma message("Using SSE NNUE inference")
// Assumes SSE support here
using Vectori16 = __m128i;
using Vectori32 = __m128i;
        #define set1_epi16 _mm_set1_epi16
        #define load_epi16(x) _mm_load_si128(reinterpret_cast<const Vectori16*>(x))
        #define min_epi16 _mm_min_epi16
        #define max_epi16 _mm_max_epi16
        #define madd_epi16 _mm_madd_epi16
        #define mullo_epi16 _mm_mullo_epi16
        #define add_epi32 _mm_add_epi32
        #define reduce_epi32 \
            [](Vectori32 vec) { \
                __m128i xmm1 = _mm_shuffle_epi32(vec, 238); \
                vec          = _mm_add_epi32(vec, xmm1); \
                xmm1         = _mm_shuffle_epi32(vec, 85); \
                vec          = _mm_add_epi32(vec, xmm1); \
                return _mm_cvtsi128_si32(vec); \
            }
    #endif
i32 NNUE::vectorizedSCReLU(const Accumulator& stm, const Accumulator& nstm, usize bucket) {
    const usize VECTOR_SIZE = sizeof(Vectori16) / sizeof(i16);
    static_assert(HL_SIZE % VECTOR_SIZE == 0, "HL size must be divisible by the native register size of your CPU for vectorization to work");
    const Vectori16 VEC_QA   = set1_epi16(QA);
    const Vectori16 VEC_ZERO = set1_epi16(0);

    Vectori32 accumulator{};

    #pragma unroll
    for (usize i = 0; i < HL_SIZE; i += VECTOR_SIZE) {
        // Load accumulators
        const Vectori16 stmAccumValues  = load_epi16(&stm[i]);
        const Vectori16 nstmAccumValues = load_epi16(&nstm[i]);

        // Clamp values
        const Vectori16 stmClamped  = min_epi16(VEC_QA, max_epi16(stmAccumValues, VEC_ZERO));
        const Vectori16 nstmClamped = min_epi16(VEC_QA, max_epi16(nstmAccumValues, VEC_ZERO));

        // Load weights
        const Vectori16 stmWeights  = load_epi16(reinterpret_cast<const Vectori16*>(&weightsToOut[bucket][i]));
        const Vectori16 nstmWeights = load_epi16(reinterpret_cast<const Vectori16*>(&weightsToOut[bucket][i + HL_SIZE]));

        // SCReLU it
        const Vectori32 stmActivated  = madd_epi16(stmClamped, mullo_epi16(stmClamped, stmWeights));
        const Vectori32 nstmActivated = madd_epi16(nstmClamped, mullo_epi16(nstmClamped, nstmWeights));

        accumulator = add_epi32(accumulator, stmActivated);
        accumulator = add_epi32(accumulator, nstmActivated);
    }

    return reduce_epi32(accumulator);
}
#else
    #pragma message("Using compiler optimized NNUE inference")
i32 NNUE::vectorizedSCReLU(const Accumulator& stm, const Accumulator& nstm, usize bucket) {
    i32 res = 0;

    #pragma unroll
    for (usize i = 0; i < HL_SIZE; i++) {
        res += (i32) SCReLU(stm[i]) * weightsToOut[bucket][i];
        res += (i32) SCReLU(nstm[i]) * weightsToOut[bucket][i + HL_SIZE];
    }
    return res;
}
#endif

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
        weightsToHL[i] = readLittleEndian<i16>(stream);
    }

    // Load hiddenLayerBias
    for (usize i = 0; i < hiddenLayerBias.size(); ++i) {
        hiddenLayerBias[i] = readLittleEndian<i16>(stream);
    }

    // Load weightsToOut
    for (usize i = 0; i < weightsToOut.size(); ++i) {
        for (usize j = 0; j < weightsToOut[i].size(); j++) {
            weightsToOut[i][j] = readLittleEndian<i16>(stream);
        }
    }

    // Load outputBias
    for (usize i = 0; i < outputBias.size(); ++i) {
        outputBias[i] = readLittleEndian<i16>(stream);
    }
}

// Returns the output of the NN
int NNUE::forwardPass(const Board* board, const AccumulatorPair& accumulators) {
    const usize divisor      = 32 / OUTPUT_BUCKETS;
    const usize outputBucket = (popcount(board->pieces()) - 2) / divisor;

    const Accumulator& accumulatorSTM = board->stm == WHITE ? accumulators.white : accumulators.black;
    const Accumulator& accumulatorOPP = ~board->stm == WHITE ? accumulators.white : accumulators.black;

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
    else
        eval = vectorizedSCReLU(accumulatorSTM, accumulatorOPP, outputBucket);


    // Dequantization
    if constexpr (ACTIVATION == ::SCReLU)
        eval /= QA;

    eval += outputBias[outputBucket];

    // Apply output bias and scale the result
    return (eval * EVAL_SCALE) / (QA * QB);
}

// Debug feature based on SF
void NNUE::showBuckets(const Board* board, const AccumulatorPair& accumulators) {
    const usize divisor     = 32 / OUTPUT_BUCKETS;
    const usize usingBucket = (popcount(board->pieces()) - 2) / divisor;

    int staticEval = 0;

    cout << "+------------+------------+" << endl;
    cout << "|   Bucket   | Evaluation |" << endl;
    cout << "+------------+------------+" << endl;

    const Accumulator& accumulatorSTM = board->stm == WHITE ? accumulators.white : accumulators.black;
    const Accumulator& accumulatorOPP = ~board->stm == WHITE ? accumulators.white : accumulators.black;

    for (usize outputBucket = 0; outputBucket < OUTPUT_BUCKETS; outputBucket++) {
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
        else
            eval = vectorizedSCReLU(accumulatorSTM, accumulatorOPP, outputBucket);


        // Dequantization
        if constexpr (ACTIVATION == ::SCReLU)
            eval /= QA;

        eval += outputBias[outputBucket];

        // Apply output bias and scale the result
        staticEval = (eval * EVAL_SCALE) / (QA * QB);

        cout << "| " << padStr(std::to_string(outputBucket), 11, 0) << "|  " << (staticEval > 0 ? "+" : "-") << " " << padStr(fmt::format("{:.2f}", std::abs(staticEval / 100.0)), 8, 0) << "|";
        if (outputBucket == usingBucket)
            cout << " <- Current bucket";
        cout << endl;
        if (outputBucket == OUTPUT_BUCKETS - 1)
            cout << "+------------+------------+" << endl;
    }
}

i16 NNUE::evaluate(const Board& board, Search::ThreadInfo& thisThread) {
    #ifndef NDEBUG
    AccumulatorPair verifAccumulator;
    verifAccumulator.resetAccumulators(board);
    if (verifAccumulator != thisThread.accumulatorStack.top())
        board.display();
    assert(verifAccumulator == thisThread.accumulatorStack.top());
    #endif
    return std::clamp(forwardPass(&board, thisThread.accumulatorStack.top()), static_cast<int>(Search::TB_MATED_IN_MAX_PLY), static_cast<int>(Search::TB_MATE_IN_MAX_PLY));
}