#pragma once

#include "types.h"
#include "move.h"
#include "board.h"
#include "movegen.h"

template<MovegenMode mode>
struct Movepicker {
    MoveList        moves;
    array<int, 256> moveScores;
    u16             seen;

    Movepicker(Board& board) {
        moves = Movegen::generateMoves<mode>(board);
        seen  = 0;
        
        for (usize i = 0; i < moves.length; i++) {
            moveScores[i] = board.evaluate(moves.moves[i]);
        }
    }

    [[nodiscard]] u16 findNext() {
        u16 best      = seen;
        int   bestScore = moveScores[seen];

        for (u16 i = seen; i < moves.length; i++) {
            if (moveScores[i] > bestScore) {
                best      = i;
                bestScore = moveScores[i];
            }
        }

        if (best != seen) {
            std::swap(moves.moves[seen], moves.moves[best]);
            std::swap(moveScores[seen], moveScores[best]);
        }

        return seen++;
    }


    bool hasNext() { return seen < moves.length; }
    Move getNext() {
        assert(hasNext());
        return moves.moves[findNext()];
    }
};