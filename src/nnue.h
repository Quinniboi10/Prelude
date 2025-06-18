#pragma once

#include "types.h"
#include "config.h"
#include "thread.h"
#include "accumulator.h"

#include <fstream>

struct NNUE {
    NNUE() = default;
    NNUE(const string& filepath) {
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

    alignas(ALIGNMENT) array<i16, HL_SIZE * 768> weightsToHL;
    alignas(ALIGNMENT) array<i16, HL_SIZE> hiddenLayerBias;
    alignas(ALIGNMENT) MultiArray<i16, OUTPUT_BUCKETS, HL_SIZE * 2> weightsToOut;
    array<i16, OUTPUT_BUCKETS> outputBias;

    static i16 ReLU(const i16 x);
    static i16 CReLU(const i16 x);
    static i32 SCReLU(const i16 x);

    i32 vectorizedSCReLU(const Accumulator& stm, const Accumulator& nstm, usize bucket) const;

    static usize feature(Color perspective, Color color, PieceType piece, Square square);

    int  forwardPass(const Board* board, const AccumulatorPair& accumulators) const;
    void showBuckets(const Board* board, const AccumulatorPair& accumulators) const;

    i16 evaluate(const Board& board, Search::ThreadInfo& thisThread) const;
};

extern const NNUE nnue;