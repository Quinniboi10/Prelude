#pragma once

#include "types.h"
#include "config.h"
#include "move.h"

using Accumulator = array<i16, HL_SIZE>;

struct AccumulatorPair {
    Accumulator white;
    Accumulator black;

    void resetAccumulators(const Board& board);

    void update(const Move m);

    void addSub(Color stm, Square add, PieceType addPT, Square sub, PieceType subPT);
    void addSubSub(Color stm, Square add, PieceType addPT, Square sub1, PieceType subPT1, Square sub2, PieceType subPT2);
    void addAddSubSub(Color stm, Square add1, PieceType addPT1, Square add2, PieceType addPT2, Square sub1, PieceType subPT1, Square sub2, PieceType subPT2);
};