#include "search.h"
#include "nnue.h"
#include "movegen.h"
#include "config.h"

struct Stack {
    PvList pv;
};

// Main search
template<Search::NodeType isPV>
i16 search(Board& board, i16 depth, i16 ply, int alpha, int beta, Stack* ss, Search::ThreadInfo& thisThread, Search::SearchLimit& sl) {
    if (depth + ply > 255)
        depth = 255 - ply;
    ss->pv.length = 0;
    if (depth <= 0)
        return nnue.evaluate(board);

    int bestEval = -INF_INT;

    usize movesSeen = 0;

    MoveList moves = Movegen::generateMoves(board);
    for (const Move m : moves) {
        if (!board.isLegal(m))
            continue;
        if (sl.stopFlag())
            return bestEval;
        if (sl.outOfNodes()) {
            sl.storeToFlag(true);
            return bestEval;
        }
        if (nodes % 2048 == 0 && sl.outOfTime()) {
            sl.storeToFlag(true);
            return bestEval;
        }
        Board testBoard = board;
        testBoard.move(m);
        nodes++;
        movesSeen++;

        int eval = -search<isPV>(testBoard, depth - 1, ply + 1, -beta, -alpha, ss + 1, thisThread, sl);

        if (eval > bestEval) {
            bestEval = eval;
            if (eval > alpha) {
                alpha = eval;
                if constexpr (isPV)
                    ss->pv.update(m, (ss + 1)->pv);
            }
        }
        if (eval >= beta) {
            break;
        }
    }

    if (!movesSeen) {
        if (board.inCheck()) {
            return -Search::MATE_SCORE + ply;
        }
        return 0;
    }

    return bestEval;
}

// This can't take a board as a reference because isLegal can change the current board state for a few dozen clock cycles
MoveEvaluation Search::iterativeDeepening(Board board, usize depth, ThreadInfo thisThread, SearchParams sp) {
    const bool isMain = thisThread.type == MAIN;

    u64 searchTime;
    if (sp.mtime)
        searchTime = sp.mtime;
    else
        searchTime = (board.stm == WHITE ? sp.wtime : sp.btime) / DEFAULT_MOVES_TO_GO + (board.stm == WHITE ? sp.winc : sp.binc) / INC_DIVISOR;

    sp.breakFlag->store(false);

    SearchLimit sl(sp.breakFlag, searchTime, sp.nodes);

    array<Stack, MAX_PLY> stack;
    Stack*                ss = &stack[0];

    depth = std::min(depth, MAX_PLY);

    i16 lastEval;
    PvList lastPV;

    for (usize currDepth = 1; currDepth <= depth; currDepth++) {
        i16 eval = search<PV>(board, currDepth, 0, -INF_I16, INF_I16, ss, thisThread, sl);

        if (isMain) {
            cout << "info depth " << currDepth << " cp " << eval << " pv";
            for (Move m : ss->pv)
                cout << " " << m;
            cout << endl;
        }

        if (sl.stopSearch())
            break;

        lastEval = eval;
        lastPV   = ss->pv;
    }

    if (isMain)
        cout << "bestmove " << ss->pv.moves[0] << endl;

    return MoveEvaluation(lastPV.moves[0], lastEval);
}