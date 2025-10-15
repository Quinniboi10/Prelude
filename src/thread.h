#pragma once

#include "types.h"
#include "search.h"
#include "ttable.h"

namespace Search {
struct ThreadInfo {
    // Quiet history is indexed [stm][from][to]
    MultiArray<i32, 2, 64, 64> quietHist;

    // Capture history is indexed [stm][pt][captured pt][to]
    MultiArray<i32, 2, 6, 7, 64> capthist;

    // Conthist is indexed [last stm][last pt][last to][stm][pt][to]
    MultiArray<ConthistSegment, 2, 6, 64> conthist;

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
    i32 getQuietHistory(Color stm, Move m) const;
    void updateQuietHistory(Color stm, Move m, i32 bonus);

    i32 getCaptureHistory(const Board& board, Move m) const;
    void updateCaptureHistory(const Board& board, Move m, i32 bonus);

    ConthistSegment* getConthistSegment(const Board& b, Move m);
    void updateConthist(SearchStack* ss, const Board& b, Move m, i32 bonus);
    i32 getConthist(const ConthistSegment* c, const Board& b, Move m) const;

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