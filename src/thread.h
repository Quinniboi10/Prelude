#pragma once

#include "types.h"
#include "search.h"
#include "tunable.h"

#include <utility>

namespace Search {
struct ThreadInfo {
    // History is indexed [stm][from][to]
    MultiArray<i32, 2, 64, 64> history;

    // Conthist is indexed [last stm][last pt][last to][stm][pt][to]
    MultiArray<ConthistSegment, 2, 6, 64> conthist;

    // Capthist is indexed [stm][pt][captured pt][to]
    array<array<array<array<i32, 64>, 6>, 6>, 2> capthist;

    Stack<AccumulatorPair, MAX_PLY + 1> accumulatorStack;

    ThreadType type;

    TranspositionTable& TT;
    std::atomic<bool>&  breakFlag;

    std::atomic<u64> nodes;
    std::atomic<u64> tbHits;
    usize            seldepth;
    i32              minRootScore;
    i32              maxRootScore;

    MoveList rootMoves;

    usize minNmpPly;

    ThreadInfo(ThreadType type, TranspositionTable& TT, std::atomic<bool>& breakFlag);

    // Copy constructor
    ThreadInfo(const ThreadInfo& other);

    i64 getQuietHistory(Board& b, SearchStack* ss, Move m) const;

    void updateHist(Color stm, Move m, i32 bonus) {
        i32 clampedBonus = std::clamp<i32>(bonus, -MAX_HISTORY, MAX_HISTORY);
        history[stm][m.from()][m.to()] += clampedBonus - history[stm][m.from()][m.to()] * abs(clampedBonus) / MAX_HISTORY;
    }

    i32 getHist(Color stm, Move m) const { return history[stm][m.from()][m.to()]; }

    void updateCapthist(Board& b, Move m, i32 bonus) {
        i32 clampedBonus = std::clamp<i32>(bonus, -MAX_HISTORY, MAX_HISTORY);
        i32& entry = capthist[b.stm][b.getPiece(m.from())][b.getPiece(m.to())][m.to()];
        entry += clampedBonus - entry * abs(clampedBonus) / MAX_HISTORY;
    }

    i32 getCapthist(Board& b, Move m) const { return capthist[b.stm][b.getPiece(m.from())][b.getPiece(m.to())][m.to()]; }

    ConthistSegment* getConthistSegment(Board& b, Move m) { return &conthist[b.stm][b.getPiece(m.from())][m.to()]; }

    void updateConthist(SearchStack* ss, Board& b, Move m, i32 bonus);

    i32 getConthist(ConthistSegment* c, Board& b, Move m) const {
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