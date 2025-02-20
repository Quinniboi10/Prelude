#pragma once

#include "move.h"
#include "board.h"
#include "movegen.h"

template<MovegenMode mode>
class Movepicker {
    MoveList moves;
    usize    seen;

    public:
    Movepicker(Board& board) {
        moves = Movegen::generateMoves<mode>(board);
        seen  = 0;
    }

    bool hasNext() { return seen < moves.length; }
    Move getNext() {
        assert(hasNext());
        return moves.moves[seen++];
    }
};