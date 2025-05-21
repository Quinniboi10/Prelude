#pragma once

#include "types.h"
#include "config.h"
#include "thread.h"
#include "accumulator.h"

#ifdef __AVX512F__
constexpr usize ALIGNMENT = 64;
#else
constexpr usize ALIGNMENT = 32;
#endif

struct NNUE {
    alignas(ALIGNMENT) array<i16, HL_SIZE * 768> weightsToHL;
    alignas(ALIGNMENT) array<i16, HL_SIZE> hiddenLayerBias;
    alignas(ALIGNMENT) array<array<i16, HL_SIZE * 2>, OUTPUT_BUCKETS> weightsToOut;
    array<i16, OUTPUT_BUCKETS> outputBias;

    static i16 ReLU(const i16 x);
    static i16 CReLU(const i16 x);
    static i32 SCReLU(const i16 x);

    i32 vectorizedSCReLU(const Accumulator& stm, const Accumulator& nstm, usize bucket);

    static usize feature(Color perspective, Color color, PieceType piece, Square square);

    void loadNetwork(const string& filepath);

    int  forwardPass(const Board* board, const AccumulatorPair& accumulators);
    void showBuckets(const Board* board, const AccumulatorPair& accumulators);

    i16 evaluate(const Board& board, Search::ThreadInfo& thisThread);
};