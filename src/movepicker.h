#pragma once

#include "types.h"
#include "move.h"
#include "board.h"
#include "movegen.h"
#include "search.h"
#include "ttable.h"
#include "search.h"

int evaluate(Board& board, Search::ThreadInfo& thisThread, Search::SearchStack* ss, Move m) {
    auto evaluateMVVLVA = [&]() {
        int victim   = PIECE_VALUES[board.getPiece(m.to())];
        int attacker = PIECE_VALUES[board.getPiece(m.from())];

        return (victim * 100) - attacker;
    };
    if (board.isCapture(m))
        return evaluateMVVLVA() + 600'000 - 800'000 * !board.see(m, -50) + thisThread.getCapthist(board, m);

    int res = thisThread.getHist(board.stm, m);
    if (ss != nullptr && (ss - 1)->conthist != nullptr)
        res += thisThread.getConthist((ss - 1)->conthist, board, m);
    if (ss != nullptr && (ss - 2)->conthist != nullptr)
        res += thisThread.getConthist((ss - 2)->conthist, board, m);
    return res;
}

template<MovegenMode mode>
struct Movepicker {
    MoveList        moves;
    array<int, 256> moveScores;
    u16             seen;
    Move            TTMove;

    Movepicker(Board& board, Search::ThreadInfo& thisThread, Search::SearchStack* ss = nullptr) {
        moves = Movegen::generateMoves<mode>(board);
        seen  = 0;

        TTMove = thisThread.TT.getEntry(board.zobrist)->move;

        for (usize i = 0; i < moves.length; i++) {
            const Move m = moves.moves[i];

            moveScores[i] = evaluate(board, thisThread, ss, m);
            moveScores[i] += 700'000 * (m == TTMove);
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