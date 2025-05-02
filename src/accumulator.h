#pragma once

#include "types.h"
#include "config.h"
#include "move.h"

#if defined(__AVX512F__) && defined(__AVX512BW__)
constexpr int NNUE_ALIGNMENT = 64;
#else
constexpr int NNUE_ALIGNMENT = 32;
#endif

using Accumulator = array<i16, HL_SIZE>;

struct AccumulatorPair {
    alignas(NNUE_ALIGNMENT) Accumulator white;
    alignas(NNUE_ALIGNMENT) Accumulator black;

    void resetAccumulators(const Board& board);

    void update(const Board& board, const Move m);

    void addSub(Color stm, Square add, PieceType addPT, Square sub, PieceType subPT);
    void addSubSub(Color stm, Square add, PieceType addPT, Square sub1, PieceType subPT1, Square sub2, PieceType subPT2);
    void addAddSubSub(Color stm, Square add1, PieceType addPT1, Square add2, PieceType addPT2, Square sub1, PieceType subPT1, Square sub2, PieceType subPT2);
};