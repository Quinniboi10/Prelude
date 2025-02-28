#pragma once

#include "types.h"
#include "board.h"
#include "config.h"
#include "ttable.h"
#include "stopwatch.h"

#include <atomic>
#include <cstring>
#include <thread>
#include <algorithm>

struct Searcher;

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

    TranspositionTable& TT;
    std::atomic<bool>&  breakFlag;

    std::atomic<u64> nodes;

    ThreadInfo(ThreadType type, TranspositionTable& TT, std::atomic<bool>& breakFlag) :
        type(type),
        TT(TT),
        breakFlag(breakFlag) {
        std::memset(&history, DEFAULT_HISTORY_VALUE, sizeof(history));
        breakFlag.store(false, std::memory_order_relaxed);
    }

    // Copy constructor
    ThreadInfo(const ThreadInfo& other) :
        history(other.history),
        type(other.type),
        TT(other.TT),
        breakFlag(other.breakFlag) {
        nodes.store(other.nodes.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }

    void updateHist(Color stm, Move m, int bonus) {
        int clampedBonus = std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY);
        history[stm][m.from()][m.to()] += clampedBonus - history[stm][m.from()][m.to()] * abs(clampedBonus) / MAX_HISTORY;
    }

    int getHist(Color stm, Move m) { return history[stm][m.from()][m.to()]; }

    void reset() {
        for (auto& stm : history)
            for (auto& from : stm)
                from.fill(DEFAULT_HISTORY_VALUE);
    }
};

struct SearchParams {
    Stopwatch<std::chrono::milliseconds> time;

    usize              depth;
    u64                nodes;
    u64                mtime;
    u64                wtime;
    u64                btime;
    u64                winc;
    u64                binc;

    SearchParams(Stopwatch<std::chrono::milliseconds> time, usize depth, u64 nodes, u64 mtime, u64 wtime, u64 btime, u64 winc, u64 binc) :
        time(time),
        depth(depth),
        nodes(nodes),
        mtime(mtime),
        wtime(wtime),
        btime(btime),
        winc(winc),
        binc(binc) {}
};

struct SearchLimit {
    Stopwatch<std::chrono::milliseconds>& time;
    u64                                  maxNodes;
    i64                                  searchTime;

    SearchLimit(auto& time, auto searchTime, auto maxNodes) :
        time(time),
        maxNodes(maxNodes),
        searchTime(searchTime) {}

    bool outOfNodes(u64 nodes) { return nodes >= maxNodes && maxNodes > 0; }

    bool outOfTime() {
        if (searchTime == 0)
            return false;
        return static_cast<i64>(time.elapsed()) >= searchTime;
    }
};

constexpr i32 MATE_SCORE       = 32767;
constexpr i32 MATE_IN_MAX_PLY  = MATE_SCORE - MAX_PLY;
constexpr i32 MATED_IN_MAX_PLY = -MATE_SCORE + MAX_PLY;

inline bool isWin(i32 score) { return score >= MATE_IN_MAX_PLY; }
inline bool isLoss(i32 score) { return score <= MATED_IN_MAX_PLY; }
inline bool isDecisive(i32 score) { return isWin(score) || isLoss(score); }

MoveEvaluation iterativeDeepening(Board board, ThreadInfo& thisThread, SearchParams sp, Searcher* searcher = nullptr);

void bench();
}