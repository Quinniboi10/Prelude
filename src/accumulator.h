#pragma once

#include "types.h"
#include "config.h"
#include "move.h"

using Accumulator = array<i16, HL_SIZE>;

#ifdef __AVX512F__
constexpr usize ALIGNMENT = 64;
#else
constexpr usize ALIGNMENT = 32;
#endif

struct AccumulatorPair {
    alignas(ALIGNMENT) Accumulator white;
    alignas(ALIGNMENT) Accumulator black;

    void resetAccumulators(const Board& board);

    void update(const Board& board, const Move m, const PieceType toPT);

    void addSub(Color stm, Square add, PieceType addPT, Square sub, PieceType subPT);
    void addSubSub(Color stm, Square add, PieceType addPT, Square sub1, PieceType subPT1, Square sub2, PieceType subPT2);
    void addAddSubSub(Color stm, Square add1, PieceType addPT1, Square add2, PieceType addPT2, Square sub1, PieceType subPT1, Square sub2, PieceType subPT2);

    bool operator==(const AccumulatorPair& other) const { return white == other.white && black == other.black; }
};