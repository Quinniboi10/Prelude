#pragma once

#include "types.h"
#include "search.h"
#include "ttable.h"

namespace Search {
struct ThreadInfo {
    // Quiet history is indexed [stm][from][to]
    MultiArray<i32, 2, 64, 64> quietHist;

    Stack<AccumulatorPair, MAX_PLY + 1> accumulatorStack;

    TranspositionTable& TT;

    ThreadType type;

    std::atomic<bool>&  breakFlag;

    std::atomic<u64> nodes;
    usize            seldepth;

    i32 minRootScore;
    i32 maxRootScore;

    MoveList rootMoves;

    ThreadInfo(ThreadType type, TranspositionTable& TT, std::atomic<bool>& breakFlag);

    // Copy constructor
    ThreadInfo(const ThreadInfo& other);

    // Histories
    i32 getQuietHistory(Color stm, Move m) const;
    void updateQuietHistory(Color stm, Move m, i32 bonus);

    [[nodiscard]] ThreadStackManager makeMove(const Board& board, Board& newBoard, Move m);
    [[nodiscard]] ThreadStackManager makeNullMove(Board& newBoard);

    void refresh(const Board& b);

    void reset();
};

struct ThreadStackManager {
    ThreadInfo& thisThread;

    explicit ThreadStackManager(ThreadInfo& thisThread) :
        thisThread(thisThread) {}

    ThreadStackManager(const ThreadStackManager& other) = delete;

    ~ThreadStackManager() { thisThread.accumulatorStack.pop(); }
};
}