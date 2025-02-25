#pragma once

#include "types.h"
#include "move.h"
#include "board.h"
#include "movegen.h"
#include "search.h"
#include "ttable.h"

int evaluate(Board& board, Search::ThreadInfo& thisThread, Move m) {
    auto evaluateMVVLVA = [&]() {
        int victim   = PIECE_VALUES[board.getPiece(m.to())];
        int attacker = PIECE_VALUES[board.getPiece(m.from())];

        return (victim * 100) - attacker;
    };
    if (board.isCapture(m)) {
        return evaluateMVVLVA();
    }
    else
        return thisThread.getHist(board.stm, m);
}

template<MovegenMode mode>
struct Movepicker {
    MoveList        moves;
    array<int, 256> moveScores;
    u16             seen;
    Move            TTMove;

    Movepicker(Board& board, Search::ThreadInfo& thisThread) {
        moves = Movegen::generateMoves<mode>(board);
        seen  = 0;

        TTMove = TT.getEntry(board.zobrist)->move;

        for (usize i = 0; i < moves.length; i++) {
            moveScores[i] = evaluate(board, thisThread, moves.moves[i]);
            moveScores[i] += 700'000 * (moves.moves[i] == TTMove);
        }
    }

    [[nodiscard]] u16 findNext() {
        u16 best      = seen;
        int bestScore = moveScores[seen];

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