#pragma once

#include "types.h"
#include "move.h"
#include "board.h"
#include "movegen.h"
#include "search.h"

template<MovegenMode mode>
struct Movepicker {
    MoveList        moves;
    array<int, 256> moveScores;
    u16             seen;
    Move            TTMove;

    Movepicker(const Board& board) {
        moves = Movegen::generateMoves<mode>(board);
        seen  = 0;
    }

    u16 findNext() {
        return seen++;
    }


    bool hasNext() { return seen < moves.length; }
    Move getNext() {
        assert(hasNext());
        return moves.moves[findNext()];
    }
};