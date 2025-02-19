#pragma once

#include "types.h"
#include "board.h"
#include "config.h"
#include "stopwatch.h"

#include <atomic>

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
    ThreadType type;

    ThreadInfo(ThreadType type) { this->type = type; }
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
        return time.elapsed() >= searchTime;
    }

    bool stopFlag() { return breakFlag->load(std::memory_order_relaxed); }
    void storeToFlag(bool v) { breakFlag->store(v, std::memory_order_relaxed); }

    bool stopSearch() { return outOfNodes() || outOfTime() || stopFlag(); }
};

constexpr i16 MATE_SCORE       = 32767;
constexpr i16 MATE_IN_MAX_PLY  = MATE_SCORE - MAX_PLY;
constexpr i16 MATED_IN_MAX_PLY = -MATE_SCORE + MAX_PLY;

inline bool isWin(i16 score) { return score >= MATE_IN_MAX_PLY; }
inline bool isLoss(i16 score) { return score <= MATED_IN_MAX_PLY; }
inline bool isDecisive(i16 score) { return isWin(score) || isLoss(score); }

MoveEvaluation iterativeDeepening(Board board, usize depth, ThreadInfo thisThread, SearchParams sp);

void bench();
}