#pragma once

#include "types.h"
#include "board.h"
#include "config.h"
#include "stopwatch.h"

#include <atomic>
#include <cstring>
#include <algorithm>

extern std::atomic<u64> nodes;

namespace Search {
enum ThreadType {
    MAIN      = 1,
    SECONDARY = 0
};
enum NodeType {
    NONPV,
    PV
};

struct ThreadInfo {
    // History is indexed [stm][from][to]
    array<array<array<int, 64>, 64>, 2> history;

    ThreadType type;


    ThreadInfo(ThreadType type) {
        std::memset(&history, DEFAULT_HISTORY_VALUE, sizeof(history));
        this->type = type;
    }

    void updateHist(Color stm, Move m, int bonus) {
        int clampedBonus = std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY);
        history[stm][m.from()][m.to()] += clampedBonus - history[stm][m.from()][m.to()] * abs(clampedBonus) / MAX_HISTORY;
    }

    int getHist(Color stm, Move m) { return history[stm][m.from()][m.to()]; }
};

struct SearchParams {
    u64                nodes;
    u64                mtime;
    u64                wtime;
    u64                btime;
    u64                winc;
    u64                binc;
    std::atomic<bool>* breakFlag;

    SearchParams(u64 nodes, u64 mtime, u64 wtime, u64 btime, u64 winc, u64 binc, std::atomic<bool>* breakFlag) {
        this->nodes     = nodes;
        this->mtime     = mtime;
        this->wtime     = wtime;
        this->btime     = btime;
        this->winc      = winc;
        this->binc      = binc;
        this->breakFlag = breakFlag;
    }
};

struct SearchLimit {
    Stopwatch<std::chrono::milliseconds> time;
    u64                                  maxNodes;
    i64                                  searchTime;
    std::atomic<bool>*                   breakFlag;

    SearchLimit(auto breakFlag, auto searchTime, auto maxNodes) {
        time.start();
        this->breakFlag  = breakFlag;
        this->searchTime = searchTime;
        this->maxNodes   = maxNodes;
    }

    bool outOfNodes() { return nodes >= maxNodes && maxNodes > 0; }

    bool outOfTime() {
        if (searchTime <= 0)
            return false;
        return static_cast<i64>(time.elapsed()) >= searchTime;
    }

    bool stopFlag() { return breakFlag->load(std::memory_order_relaxed); }
    void storeToFlag(bool v) { breakFlag->store(v, std::memory_order_relaxed); }

    bool stopSearch() { return outOfNodes() || outOfTime() || stopFlag(); }
};

constexpr i32 MATE_SCORE       = 32767;
constexpr i32 MATE_IN_MAX_PLY  = MATE_SCORE - MAX_PLY;
constexpr i32 MATED_IN_MAX_PLY = -MATE_SCORE + MAX_PLY;

inline bool isWin(i32 score) { return score >= MATE_IN_MAX_PLY; }
inline bool isLoss(i32 score) { return score <= MATED_IN_MAX_PLY; }
inline bool isDecisive(i32 score) { return isWin(score) || isLoss(score); }

MoveEvaluation iterativeDeepening(Board board, usize depth, ThreadInfo thisThread, SearchParams sp);

void bench();
}