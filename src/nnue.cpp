#include <fstream>
#include <immintrin.h>

#include "nnue.h"
#include "board.h"


void NNUE::loadNetwork(const std::string& filepath) {
    std::ifstream stream(filepath, std::ios::binary);
    if (!stream.is_open())
    {
        cerr << "Failed to open file: " + filepath << endl;
        cerr << "Expect engine to not work as intended with bad evaluation" << endl;
    }

    // Load weightsToHL
    for (size_t i = 0; i < weightsToHL.size(); ++i)
    {
        weightsToHL[i] = read_little_endian<int16_t>(stream);
    }

    // Load hiddenLayerBias
    for (size_t i = 0; i < hiddenLayerBias.size(); ++i)
    {
        hiddenLayerBias[i] = read_little_endian<int16_t>(stream);
    }

    // Load weightsToOut
    for (size_t i = 0; i < weightsToOut.size(); ++i)
    {
        for (size_t j = 0; j < weightsToOut[i].size(); j++)
        {
            weightsToOut[i][j] = read_little_endian<int16_t>(stream);
        }
    }

    // Load outputBias
    for (size_t i = 0; i < outputBias.size(); ++i)
    {
        outputBias[i] = read_little_endian<int16_t>(stream);
    }

    cout << "Network loaded successfully from " << filepath << endl;
    cerr
      << "WARNING: You are using MSVC, this means that your nnue was NOT embedded into the exe."
      << endl;
}


// Returns the output of the NN
int NNUE::forwardPass(const Board* board) {
    const int divisor      = 32 / OUTPUT_BUCKETS;
    const int outputBucket = (popcountll(board->pieces()) - 2) / divisor;

    // Determine the side to move and the opposite side
    Color stm = board->side;

    const Accumulator& accumulatorSTM = stm ? board->whiteAccum : board->blackAccum;
    const Accumulator& accumulatorOPP = ~stm ? board->whiteAccum : board->blackAccum;

    // Accumulate output for STM and OPP using separate weight segments
    i64 eval = 0;

    if constexpr (activation != ::SCReLU)
    {
        for (int i = 0; i < HL_SIZE; i++)
        {
            // First HL_SIZE weights are for STM
            if constexpr (activation == ::ReLU)
                eval += ReLU(accumulatorSTM[i]) * weightsToOut[outputBucket][i];
            if constexpr (activation == ::CReLU)
                eval += CReLU(accumulatorSTM[i]) * weightsToOut[outputBucket][i];

            // Last HL_SIZE weights are for OPP
            if constexpr (activation == ::ReLU)
                eval += ReLU(accumulatorOPP[i]) * weightsToOut[outputBucket][HL_SIZE + i];
            if constexpr (activation == ::CReLU)
                eval += CReLU(accumulatorOPP[i]) * weightsToOut[outputBucket][HL_SIZE + i];
        }
    }
    else
    {
        const __m256i vec_zero = _mm256_setzero_si256();
        const __m256i vec_qa   = _mm256_set1_epi16(inputQuantizationValue);
        __m256i       sum      = vec_zero;

        for (int i = 0; i < HL_SIZE / 16; i++) {
            const __m256i us = _mm256_load_si256(
              reinterpret_cast<const __m256i*>(&accumulatorSTM[16 * i]));  // Load from accumulator
            const __m256i them =
              _mm256_load_si256(reinterpret_cast<const __m256i*>(&accumulatorOPP[16 * i]));

            const __m256i us_weights   = _mm256_load_si256(reinterpret_cast<const __m256i*>(&weightsToOut[outputBucket][16 * i]));  // Load from net
            const __m256i them_weights = _mm256_load_si256(
              reinterpret_cast<const __m256i*>(&weightsToOut[outputBucket][HL_SIZE + 16 * i]));

            const __m256i us_clamped   = _mm256_min_epi16(_mm256_max_epi16(us, vec_zero), vec_qa);
            const __m256i them_clamped = _mm256_min_epi16(_mm256_max_epi16(them, vec_zero), vec_qa);

            const __m256i us_results =
              _mm256_madd_epi16(_mm256_mullo_epi16(us_weights, us_clamped), us_clamped);
            const __m256i them_results =
              _mm256_madd_epi16(_mm256_mullo_epi16(them_weights, them_clamped), them_clamped);

            sum = _mm256_add_epi32(sum, us_results);
            sum = _mm256_add_epi32(sum, them_results);
        }

        __m256i v1 = _mm256_hadd_epi32(sum, sum);
        __m256i v2 = _mm256_hadd_epi32(v1, v1);

        eval = _mm256_extract_epi32(v2, 0) + _mm256_extract_epi32(v2, 4);
    }


    // Dequantization
    if constexpr (activation == ::SCReLU)
        eval /= inputQuantizationValue;

    eval += outputBias[outputBucket];

    // Apply output bias and scale the result
    return (eval * evalScale) / (inputQuantizationValue * hiddenQuantizationValue);
}

NNUE nn;