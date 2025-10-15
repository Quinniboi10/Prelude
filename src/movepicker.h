#pragma once

#include "types.h"
#include "move.h"
#include "board.h"
#include "movegen.h"
#include "thread.h"
#include "tunable.h"

int evaluateMove(const Board& board, const Search::ThreadInfo& thisThread, const Search::SearchStack* ss, Move m) {
    const auto evaluateMVVLVA = [&]() {
        const int victim   = PIECE_VALUES[board.getPiece(m.to())];
        const int attacker = PIECE_VALUES[board.getPiece(m.from())];

        return (victim * MO_VICTIM_WEIGHT) - attacker;
    };

    if (!board.isQuiet(m))
        return evaluateMVVLVA() + 600'000 - 800'000 * !board.see(m, MO_CAPTURE_SEE_THRESHOLD) + thisThread.getCaptureHistory(board, m) * MO_CAPTHIST_WEIGHT / 1024;

    int ret = thisThread.getQuietHistory(board.stm, m);

    if ((ss - 1)->conthist)
        ret += thisThread.getConthist((ss - 1)->conthist, board, m);

    return ret;
}

template<MovegenMode mode>
struct Movepicker {
    MoveList        moves;
    array<int, 256> moveScores;
    u16             seen;

    Movepicker(const Board& board, const Search::ThreadInfo& thisThread, const Search::SearchStack* ss, const Move ttMove) {
        moves = Movegen::generateMoves<mode>(board);
        seen  = 0;

        for (usize i = 0; i < moves.length; i++) {
            const Move m = moves.moves[i];

            moveScores[i] = evaluateMove(board, thisThread, ss, m);
            if (m == ttMove)
                moveScores[i] += 1'000'000;
        }
    }

    u16 findNext() {
        u16 best      = seen;
        int bestScore = moveScores[seen];

        for (usize i = seen; i < moves.length; i++) {
            if (moveScores[i] > bestScore) {
                best      = i;
                bestScore = moveScores[i];
            }
        }

        if (best != seen) {
            std::swap(moves.moves[seen], moves.moves[best]);
            std::swap(moveScores[seen], moveScores[best]);
        }

        seen++;

        return seen - 1;
    }


    bool hasNext() const { return seen < moves.length; }
    Move getNext() {
        assert(hasNext());
        return moves.moves[findNext()];
    }
};