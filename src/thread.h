#pragma once

#include "types.h"
#include "search.h"

#include <utility>

namespace Search {
struct ThreadInfo {
    // History is indexed [stm][from][to]
    MultiArray<int, 2, 64, 64> history;

    // Conthist is indexed [last stm][last pt][last to][stm][pt][to]
    MultiArray<ConthistSegment, 2, 6, 64> conthist;

    Stack<AccumulatorPair, MAX_PLY + 1> accumulatorStack;

    ThreadType type;

    TranspositionTable& TT;
    std::atomic<bool>&  breakFlag;

    std::atomic<u64> nodes;
    usize            seldepth;

    usize minNmpPly;

    ThreadInfo(ThreadType type, TranspositionTable& TT, std::atomic<bool>& breakFlag);

    // Copy constructor
    ThreadInfo(const ThreadInfo& other);

    i64 getQuietHistory(Board& b, SearchStack* ss, Move m) const;

    void updateHist(Color stm, Move m, int bonus) {
        int clampedBonus = std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY);
        history[stm][m.from()][m.to()] += clampedBonus - history[stm][m.from()][m.to()] * abs(clampedBonus) / MAX_HISTORY;
    }

    int getHist(Color stm, Move m) const { return history[stm][m.from()][m.to()]; }

    ConthistSegment* getConthistSegment(Board& b, Move m) { return &conthist[b.stm][b.getPiece(m.from())][m.to()]; }

    void updateConthist(SearchStack* ss, Board& b, Move m, int bonus);

    int getConthist(ConthistSegment* c, Board& b, Move m) const {
        assert(c != nullptr);
        return (*c)[b.stm][b.getPiece(m.from())][m.to()];
    }

    ThreadStackManager makeMove(Board& board, Board& newBoard, Move m);
    ThreadStackManager makeNullMove(Board& newBoard);

    void refresh(Board& b);

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