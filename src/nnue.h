#pragma once

#include "types.h"
#include "config.h"

struct NNUE {
    alignas(32) array<i16, HL_SIZE * 768> weightsToHL;
    alignas(32) array<i16, HL_SIZE> hiddenLayerBias;
    alignas(32) array<array<i16, HL_SIZE * 2>, OUTPUT_BUCKETS> weightsToOut;
    array<i16, OUTPUT_BUCKETS> outputBias;

    i16 ReLU(const i16 x);
    i16 CReLU(const i16 x);

    static usize feature(Color perspective, Color color, PieceType piece, Square square);

    void loadNetwork(const string& filepath);

    int  forwardPass(const Board* board);
    void showBuckets(const Board* board);
};

extern NNUE nnue;