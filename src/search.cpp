#include "search.h"
#include "globals.h"
#include "movegen.h"
#include "searcher.h"
#include "config.h"
#include "movepicker.h"
#include "wdl.h"

#include <cmath>

namespace Search {
struct Stack {
    PvList pv;
    Move   excluded;
};

array<array<array<int, 219>, MAX_PLY + 1>, 2> lmrTable;
void fillLmrTable() {
    for (int isQuiet = 0; isQuiet <= 1; isQuiet++)
        for (usize depth = 0; depth <= MAX_PLY; depth++)
            for (int movesSeen = 0; movesSeen <= 218; movesSeen++) {
                // Calculate reduction factor for late move reduction
                // Based on Weiss's formulas
                int& depthReduction = lmrTable[isQuiet][depth][movesSeen];
                if (depth == 0 || movesSeen == 0) {
                    depthReduction = 0;
                    continue;
                }
                if (isQuiet)
                    depthReduction = 1.35 + std::log(depth) * std::log(movesSeen) / 2.75;
                else
                    depthReduction = 0.20 + std::log(depth) * std::log(movesSeen) / 3.35;
            }
}

// Quiesence search
i16 qsearch(Board& board, int alpha, int beta, ThreadInfo& thisThread, SearchLimit& sl) {
    int staticEval = nnue.evaluate(board);

    int bestScore = staticEval;

    if (bestScore >= beta)
        return bestScore;
    if (alpha < bestScore)
        alpha = bestScore;

    Movepicker<NOISY_ONLY> picker(board, thisThread);
    while (picker.hasNext()) {
        if (thisThread.breakFlag.load(std::memory_order_relaxed))
            return bestScore;
        if (sl.outOfNodes(thisThread.nodes)) {
            thisThread.breakFlag.store(true, std::memory_order_relaxed);
            return bestScore;
        }
        if (thisThread.nodes % 2048 == 0 && sl.outOfTime()) {
            thisThread.breakFlag.store(true, std::memory_order_relaxed);
            return bestScore;
        }

        const Move m = picker.getNext();

        if (!board.isLegal(m))
            continue;

        if (!board.see(m, 0))
            continue;

        Board testBoard = board;
        testBoard.move(m);
        thisThread.nodes++;

        i16 score = -qsearch(testBoard, -beta, -alpha, thisThread, sl);

        if (score > bestScore) {
            bestScore = score;
            if (score > alpha)
                alpha = score;
        }
        if (score >= beta)
            break;
    }

    return bestScore;
}

// Main search
template<NodeType isPV>
i32 search(Board& board, i32 depth, i32 ply, int alpha, int beta, Stack* ss, ThreadInfo& thisThread, SearchLimit& sl) {
    if (depth + ply > 255)
        depth = 255 - ply;
    ss->pv.length = 0;
    if (board.isDraw() && ply > 0)
        return 0;
    if (depth <= 0)
        return qsearch(board, alpha, beta, thisThread, sl);

    if (ply + 1 > thisThread.seldepth)
        thisThread.seldepth = ply + 1;

    Transposition* ttEntry = ss->excluded.isNull() ? thisThread.TT.getEntry(board.zobrist) : nullptr;

    if (!isPV && ttEntry != nullptr && ttEntry->zobrist == board.zobrist && ttEntry->depth >= depth
        && (ttEntry->flag == EXACT                                       // Exact score
            || (ttEntry->flag == BETA_CUTOFF && ttEntry->score >= beta)  // Lower bound, fail high
            || (ttEntry->flag == FAIL_LOW && ttEntry->score <= alpha)    // Upper bound, fail low
            )) {
        return ttEntry->score;
    }

    // Internal iterative reductions
    if (ttEntry != nullptr && (ttEntry->zobrist != board.zobrist || ttEntry->move.isNull()) && depth > 5)
        depth--;

    // Mate distance pruning
    if (ply > 0) {
        alpha = std::max(alpha, static_cast<int>(-MATE_SCORE + ply));
        beta  = std::min(beta, static_cast<int>(MATE_SCORE - ply - 1));

        if (alpha >= beta)
            return alpha;
    }

    i16 staticEval = nnue.evaluate(board);

    if (!isPV && ply > 0 && !board.inCheck() && ss->excluded.isNull()) {
        // Reverse futility pruning
        int rfpMargin = 100 * depth;
        if (staticEval - rfpMargin >= beta && depth < 7)
            return staticEval;

        // Null move pruning
        if (board.canNullMove() && staticEval >= beta && ply >= thisThread.minNmpPly) {
            Board testBoard = board;
            testBoard.nullMove();
            i32 score = -search<NodeType::NONPV>(testBoard, depth - NMP_REDUCTION, ply + 1, -beta, -beta + 1, ss + 1, thisThread, sl);

            if (score >= beta) {
                // Verification search to guard zugzwang
                if (depth <= 14 || thisThread.minNmpPly > 0)
                    return isWin(score) ? beta : score;

                thisThread.minNmpPly = ply + (depth - NMP_REDUCTION) * 3 / 4;
                score                = search<NodeType::NONPV>(board, depth - NMP_REDUCTION, ply, beta - 1, beta, ss, thisThread, sl);
                thisThread.minNmpPly = 0;

                if (score >= beta)
                    return score;
            }
        }
    }

    int  bestScore = -INF_INT;
    Move bestMove;

    TTFlag ttFlag = FAIL_LOW;

    int movesSeen = 0;

    MoveList seenQuiets;

    bool skipQuiets = false;

    Movepicker<ALL_MOVES> picker(board, thisThread);
    while (picker.hasNext()) {
        if (thisThread.breakFlag.load(std::memory_order_relaxed))
            return bestScore;
        if (sl.outOfNodes(thisThread.nodes)) {
            thisThread.breakFlag.store(true, std::memory_order_relaxed);
            return bestScore;
        }
        if (thisThread.nodes % 2048 == 0 && sl.outOfTime()) {
            thisThread.breakFlag.store(true, std::memory_order_relaxed);
            return bestScore;
        }

        const Move m = picker.getNext();

        if (m == ss->excluded)
            continue;

        if (skipQuiets && board.isQuiet(m))
            continue;

        if (board.isQuiet(m))
            seenQuiets.add(m);

        if (!board.isLegal(m))
            continue;

        if (!isLoss(bestScore)) {
            // Late move pruning
            if (!skipQuiets && board.isQuiet(m) && movesSeen >= 6 + depth * depth) {
                skipQuiets = true;
                continue;
            }

            // Futility pruning
            if (!board.inCheck() && ply > 0 && depth < 6 && !isLoss(bestScore) && board.isQuiet(m) && staticEval + FUTILITY_PRUNING_MARGIN + FUTILITY_PRUNING_SCALAR * depth < alpha) {
                skipQuiets = true;
                continue;
            }
        }

        Board testBoard = board;
        testBoard.move(m);
        thisThread.nodes++;
        movesSeen++;

        usize extension = 0;
        if (ply > 0
            && depth >= SE_MIN_DEPTH
            && ttEntry != nullptr
            && ttEntry->zobrist == board.zobrist
            && m == ttEntry->move
            && ttEntry->depth >= depth - 3
            && ttEntry->flag != FAIL_LOW) {
            const int sBeta = std::max(-INF_INT + 1, ttEntry->score - depth * 2);
            const int sDepth = (depth - 1) / 2;

            ss->excluded = m;
            const int score = search<NodeType::NONPV>(board, sDepth, ply, sBeta - 1, sBeta, ss, thisThread, sl);
            ss->excluded    = Move();

            if (score < sBeta) {
                if (!isPV && score < sBeta - SE_DOUBLE_MARGIN)
                    extension = 2;
                else
                    extension = 1;
            }
            else if (ttEntry->score >= beta)
                extension = -2;
        }

        i32 newDepth = depth + extension - 1;

        i16 score;
        if (depth >= 2 && movesSeen >= 5 + 2 * (ply == 0) && !testBoard.inCheck()) {
            int depthReduction = lmrTable[board.isQuiet(m)][depth][movesSeen] + !isPV;

            score = -search<NodeType::NONPV>(testBoard, newDepth - depthReduction, ply + 1, -alpha - 1, -alpha, ss + 1, thisThread, sl);
            if (score > alpha)
                score = -search<NodeType::NONPV>(testBoard, newDepth, ply + 1, -alpha - 1, -alpha, ss + 1, thisThread, sl);
        }
        else if (!isPV || movesSeen > 1)
            score = -search<NodeType::NONPV>(testBoard, newDepth, ply + 1, -alpha - 1, -alpha, ss + 1, thisThread, sl);
        if (isPV && (movesSeen == 1 || score > alpha))
            score = -search<isPV>(testBoard, newDepth, ply + 1, -beta, -alpha, ss + 1, thisThread, sl);

        if (score > bestScore) {
            bestScore = score;
            bestMove  = m;
            if (score > alpha) {
                ttFlag = EXACT;
                alpha  = score;
                if constexpr (isPV)
                    ss->pv.update(m, (ss + 1)->pv);
            }
        }
        if (score >= beta) {
            ttFlag = BETA_CUTOFF;
            if (board.isQuiet(m)) {
                thisThread.updateHist(board.stm, m, 20 * depth * depth);
                // History malus
                for (const Move quietMove : seenQuiets) {
                    if (quietMove == m)
                        continue;
                    thisThread.updateHist(board.stm, quietMove, -20 * depth * depth);
                }
            }
            break;
        }
    }

    if (!movesSeen) {
        if (board.inCheck()) {
            return -MATE_SCORE + ply;
        }
        return 0;
    }

    if (ttEntry != nullptr)
        *ttEntry = Transposition(board.zobrist, bestMove, ttFlag, bestScore, depth);

    return bestScore;
}

// This can't take a board as a reference because isLegal can change the current board state for a few dozen clock cycles
MoveEvaluation iterativeDeepening(Board board, ThreadInfo& thisThread, SearchParams sp, Searcher* searcher) {
    thisThread.breakFlag.store(false);
    thisThread.nodes    = 0;
    thisThread.seldepth = 0;
    const bool isMain = thisThread.type == MAIN;

    i64 searchTime;
    if (sp.mtime)
        searchTime = sp.mtime;
    else
        searchTime = (board.stm == WHITE ? sp.wtime : sp.btime) / DEFAULT_MOVES_TO_GO + (board.stm == WHITE ? sp.winc : sp.binc) / INC_DIVISOR;

    if (searchTime != 0)
        searchTime -= MOVE_OVERHEAD;

    SearchLimit depthOneSl(sp.time, 0, sp.nodes);
    SearchLimit mainSl(sp.time, searchTime, sp.nodes);

    array<Stack, MAX_PLY> stack;
    Stack*                ss = &stack[0];

    for (Stack ss : stack) {
        ss.pv.length = 0;
        ss.excluded  = Move();
    }

    usize depth = std::min(sp.depth, MAX_PLY);

    i32    lastScore{};
    PvList lastPV{};

    auto countNodes = [&]() {
        u64 nodes = thisThread.nodes;
        if (searcher == nullptr)
            return nodes;
        for (ThreadInfo& w : searcher->workerData)
            nodes += w.nodes;
        return nodes;
    };

    for (usize currDepth = 1; currDepth <= depth; currDepth++) {
        SearchLimit& sl              = currDepth == 1 ? depthOneSl : mainSl;

        i32 score = 0;

        auto         searchCancelled = [&]() {
            if (thisThread.type == MAIN)
                return sl.outOfNodes(countNodes()) || sl.outOfTime() || thisThread.breakFlag.load(std::memory_order_relaxed) || (sp.softNodes > 0 && countNodes() > sp.softNodes);
            else
                return thisThread.breakFlag.load(std::memory_order_relaxed) || (sp.softNodes > 0 && countNodes() > sp.softNodes);
        };

        if (currDepth < MIN_ASP_WINDOW_DEPTH)
            score = search<PV>(board, currDepth, 0, -INF_I16, INF_I16, ss, thisThread, sl);
        else {
            int delta = INITIAL_ASP_WINDOW;
            int alpha = std::max(lastScore - delta, -INF_I16);
            int beta  = std::min(lastScore + delta, INF_I16);

            while (!searchCancelled()) {
                score = search<PV>(board, currDepth, 0, alpha, beta, ss, thisThread, sl);
                if (score <= alpha || score >= beta) {
                    alpha = -INF_I16;
                    beta  = INF_I16;
                }
                else
                    break;
            }
        }

        if (currDepth == 1) {
            lastScore = score;
            lastPV    = ss->pv;
        }

        if (searchCancelled())
            break;

        if (isMain) {
            // Get info for hashfull
            usize samples = std::min((u64) 1000, thisThread.TT.size);
            usize hits    = 0;
            for (usize sample = 0; sample < samples; sample++)
                hits += thisThread.TT.getEntry(sample)->zobrist != 0;
            // Depth, time, hash, score
            cout << "info depth " << currDepth << " seldepth " << thisThread.seldepth << " time " << sl.time.elapsed() << " hashfull " << (int) (hits / (double) samples * 1000) << " nodes "
                 << countNodes();
            if (sl.time.elapsed() > 0)
                cout << " nps " << countNodes() * 1000 / sl.time.elapsed();
            cout << " score";
            if (isDecisive(score)) {
                cout << " mate ";
                cout << ((score < 0) ? "-" : "") << (MATE_SCORE - std::abs(score)) / 2 + 1;
            }
            else
                cout << " cp " << scaleEval(score, board);

            // PV line
            cout << " pv";
            for (Move m : ss->pv)
                cout << " " << m;
            cout << endl;
        }

        lastScore = score;
        lastPV    = ss->pv;

        // Go mate
        if (isMain && sp.mate > 0 && (MATE_SCORE - std::abs(score)) / 2 + 1 <= sp.mate)
            break;
    }

    if (isMain)
        cout << "bestmove " << lastPV.moves[0] << endl;

    thisThread.breakFlag.store(true, std::memory_order_relaxed);

    assert(lastPV.length > 0);
    assert(!lastPV.moves[0].isNull());
#ifndef NDEBUG
    MoveList moves = Movegen::generateLegalMoves(board);
    if (std::find(moves.begin(), moves.end(), lastPV.moves[0]) == moves.end()) {
        cerr << "Illegal first move in PV line" << endl;
        board.display();
        cerr << lastPV.moves[0] << endl;
        exit(-1);
    }
#endif
    return MoveEvaluation(lastPV.moves[0], lastScore);
}

void bench() {
    u64    totalNodes  = 0;
    double totalTimeMs = 0.0;

    cout << "Starting benchmark with depth " << BENCH_DEPTH << endl;

    array<string, 50> fens = {"r3k2r/2pb1ppp/2pp1q2/p7/1nP1B3/1P2P3/P2N1PPP/R2QK2R w KQkq a6 0 14",
                              "4rrk1/2p1b1p1/p1p3q1/4p3/2P2n1p/1P1NR2P/PB3PP1/3R1QK1 b - - 2 24",
                              "r3qbrk/6p1/2b2pPp/p3pP1Q/PpPpP2P/3P1B2/2PB3K/R5R1 w - - 16 42",
                              "6k1/1R3p2/6p1/2Bp3p/3P2q1/P7/1P2rQ1K/5R2 b - - 4 44",
                              "8/8/1p2k1p1/3p3p/1p1P1P1P/1P2PK2/8/8 w - - 3 54",
                              "7r/2p3k1/1p1p1qp1/1P1Bp3/p1P2r1P/P7/4R3/Q4RK1 w - - 0 36",
                              "r1bq1rk1/pp2b1pp/n1pp1n2/3P1p2/2P1p3/2N1P2N/PP2BPPP/R1BQ1RK1 b - - 2 10",
                              "3r3k/2r4p/1p1b3q/p4P2/P2Pp3/1B2P3/3BQ1RP/6K1 w - - 3 87",
                              "2r4r/1p4k1/1Pnp4/3Qb1pq/8/4BpPp/5P2/2RR1BK1 w - - 0 42",
                              "4q1bk/6b1/7p/p1p4p/PNPpP2P/KN4P1/3Q4/4R3 b - - 0 37",
                              "2q3r1/1r2pk2/pp3pp1/2pP3p/P1Pb1BbP/1P4Q1/R3NPP1/4R1K1 w - - 2 34",
                              "1r2r2k/1b4q1/pp5p/2pPp1p1/P3Pn2/1P1B1Q1P/2R3P1/4BR1K b - - 1 37",
                              "r3kbbr/pp1n1p1P/3ppnp1/q5N1/1P1pP3/P1N1B3/2P1QP2/R3KB1R b KQkq b3 0 17",
                              "8/6pk/2b1Rp2/3r4/1R1B2PP/P5K1/8/2r5 b - - 16 42",
                              "1r4k1/4ppb1/2n1b1qp/pB4p1/1n1BP1P1/7P/2PNQPK1/3RN3 w - - 8 29",
                              "8/p2B4/PkP5/4p1pK/4Pb1p/5P2/8/8 w - - 29 68",
                              "3r4/ppq1ppkp/4bnp1/2pN4/2P1P3/1P4P1/PQ3PBP/R4K2 b - - 2 20",
                              "5rr1/4n2k/4q2P/P1P2n2/3B1p2/4pP2/2N1P3/1RR1K2Q w - - 1 49",
                              "1r5k/2pq2p1/3p3p/p1pP4/4QP2/PP1R3P/6PK/8 w - - 1 51",
                              "q5k1/5ppp/1r3bn1/1B6/P1N2P2/BQ2P1P1/5K1P/8 b - - 2 34",
                              "r1b2k1r/5n2/p4q2/1ppn1Pp1/3pp1p1/NP2P3/P1PPBK2/1RQN2R1 w - - 0 22",
                              "r1bqk2r/pppp1ppp/5n2/4b3/4P3/P1N5/1PP2PPP/R1BQKB1R w KQkq - 0 5",
                              "r1bqr1k1/pp1p1ppp/2p5/8/3N1Q2/P2BB3/1PP2PPP/R3K2n b Q - 1 12",
                              "r1bq2k1/p4r1p/1pp2pp1/3p4/1P1B3Q/P2B1N2/2P3PP/4R1K1 b - - 2 19",
                              "r4qk1/6r1/1p4p1/2ppBbN1/1p5Q/P7/2P3PP/5RK1 w - - 2 25",
                              "r7/6k1/1p6/2pp1p2/7Q/8/p1P2K1P/8 w - - 0 32",
                              "r3k2r/ppp1pp1p/2nqb1pn/3p4/4P3/2PP4/PP1NBPPP/R2QK1NR w KQkq - 1 5",
                              "3r1rk1/1pp1pn1p/p1n1q1p1/3p4/Q3P3/2P5/PP1NBPPP/4RRK1 w - - 0 12",
                              "5rk1/1pp1pn1p/p3Brp1/8/1n6/5N2/PP3PPP/2R2RK1 w - - 2 20",
                              "8/1p2pk1p/p1p1r1p1/3n4/8/5R2/PP3PPP/4R1K1 b - - 3 27",
                              "8/4pk2/1p1r2p1/p1p4p/Pn5P/3R4/1P3PP1/4RK2 w - - 1 33",
                              "8/5k2/1pnrp1p1/p1p4p/P6P/4R1PK/1P3P2/4R3 b - - 1 38",
                              "8/8/1p1kp1p1/p1pr1n1p/P6P/1R4P1/1P3PK1/1R6 b - - 15 45",
                              "8/8/1p1k2p1/p1prp2p/P2n3P/6P1/1P1R1PK1/4R3 b - - 5 49",
                              "8/8/1p4p1/p1p2k1p/P2n1P1P/4K1P1/1P6/3R4 w - - 6 54",
                              "8/8/1p4p1/p1p2k1p/P2n1P1P/4K1P1/1P6/6R1 b - - 6 59",
                              "8/5k2/1p4p1/p1pK3p/P2n1P1P/6P1/1P6/4R3 b - - 14 63",
                              "8/1R6/1p1K1kp1/p6p/P1p2P1P/6P1/1Pn5/8 w - - 0 67",
                              "1rb1rn1k/p3q1bp/2p3p1/2p1p3/2P1P2N/PP1RQNP1/1B3P2/4R1K1 b - - 4 23",
                              "4rrk1/pp1n1pp1/q5p1/P1pP4/2n3P1/7P/1P3PB1/R1BQ1RK1 w - - 3 22",
                              "r2qr1k1/pb1nbppp/1pn1p3/2ppP3/3P4/2PB1NN1/PP3PPP/R1BQR1K1 w - - 4 12",
                              "2r2k2/8/4P1R1/1p6/8/P4K1N/7b/2B5 b - - 0 55",
                              "6k1/5pp1/8/2bKP2P/2P5/p4PNb/B7/8 b - - 1 44",
                              "2rqr1k1/1p3p1p/p2p2p1/P1nPb3/2B1P3/5P2/1PQ2NPP/R1R4K w - - 3 25",
                              "r1b2rk1/p1q1ppbp/6p1/2Q5/8/4BP2/PPP3PP/2KR1B1R b - - 2 14",
                              "6r1/5k2/p1b1r2p/1pB1p1p1/1Pp3PP/2P1R1K1/2P2P2/3R4 w - - 1 36",
                              "rnbqkb1r/pppppppp/5n2/8/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2",
                              "2rr2k1/1p4bp/p1q1p1p1/4Pp1n/2PB4/1PN3P1/P3Q2P/2RR2K1 w - f6 0 20",
                              "3br1k1/p1pn3p/1p3n2/5pNq/2P1p3/1PN3PP/P2Q1PB1/4R1K1 w - - 0 23",
                              "2r2b2/5p2/5k2/p1r1pP2/P2pB3/1P3P2/K1P3R1/7R w - - 23 93"};

    for (auto fen : fens) {
        if (fen.empty())
            continue;  // Skip empty lines

        Board board;
        board.reset();

        board.loadFromFEN(fen);

        // Set up for iterative deepening
        std::atomic<bool>                    benchBreakFlag(false);
        TranspositionTable                   TT;
        Search::ThreadInfo                   thisThread(Search::ThreadType::SECONDARY, TT, benchBreakFlag);
        Stopwatch<std::chrono::milliseconds> time;

        TT.clear();

        // Start the iterative deepening search
        Search::iterativeDeepening(board, thisThread, Search::SearchParams(time, BENCH_DEPTH, 0, 0, 0, 0, 0, 0, 0, 0));

        u64 durationMs = time.elapsed();

        totalNodes += thisThread.nodes;
        totalTimeMs += durationMs;

        cout << "FEN: " << fen << endl;
        cout << "Nodes: " << formatNum(thisThread.nodes) << ", Time: " << formatTime(durationMs) << endl;
        cout << "----------------------------------------" << endl;
    }

    cout << "Benchmark Completed." << endl;
    cout << "Total Nodes: " << formatNum(totalNodes) << endl;
    cout << "Total Time: " << formatTime(totalTimeMs) << endl;
    int nps = INF_INT;
    if (totalTimeMs > 0) {
        nps = static_cast<u64>((totalNodes / totalTimeMs) * 1000);
        cout << "Average NPS: " << formatNum(nps) << endl;
    }
    cout << totalNodes << " nodes " << nps << " nps" << endl;
}
}