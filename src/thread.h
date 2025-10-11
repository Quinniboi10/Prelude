#pragma once

#include "types.h"
#include "search.h"
#include "ttable.h"

namespace Search {
struct ThreadInfo {
    // Quiet history is indexed [from][to][from attacked][to attacked]
    MultiArray<i32, 64, 64, 2, 2> quietHist;

    // Capture history is indexed [stm][pt][captured pt][to]
    array<array<array<array<i32, 64>, 6>, 6>, 2> capthist;

    Stack<AccumulatorPair, MAX_PLY + 1> accumulatorStack;

    TranspositionTable& TT;

    ThreadType type;

    std::atomic<bool>&  breakFlag;

    SearchLimit sl;

    std::atomic<u64> nodes;
    usize            seldepth;

    i32 minRootScore;
    i32 maxRootScore;

    MoveList rootMoves;

    ThreadInfo(ThreadType type, TranspositionTable& TT, std::atomic<bool>& breakFlag);

    // Copy constructor
    ThreadInfo(const ThreadInfo& other);

    // Histories
    i32 getQuietHistory(const Board& board, Move m) const;
    void updateQuietHistory(const Board& board, Move m, i32 bonus);

    i32 getCaptureHistory(const Board& board, Move m) const;
    void updateCaptureHistory(const Board& board, Move m, i32 bonus);

    [[nodiscard]] std::pair<Board, ThreadStackManager> makeMove(const Board& board, Move m);
    [[nodiscard]] std::pair<Board, ThreadStackManager> makeNullMove(Board board);

    void refresh(const Board& b);

    void reset();
};

struct ThreadStackManager {
    ThreadInfo& thisThread;

    explicit ThreadStackManager(ThreadInfo& thisThread) :
        thisThread(thisThread) {}

    ThreadStackManager(const ThreadStackManager& other) = default;
    ThreadStackManager(ThreadStackManager&&) = default;

    ~ThreadStackManager() { thisThread.accumulatorStack.pop(); }
};
}