#pragma once

#include "types.h"
#include "board.h"
#include "config.h"
#include "stopwatch.h"

#include <cstring>
#include <thread>
#include <algorithm>

struct Searcher;

namespace Search {
struct ThreadInfo;
struct ThreadStackManager;
using ConthistSegment = MultiArray<i32, 2, 6, 64>;

struct SearchStack {
    PvList           pv;
    ConthistSegment* conthist;
    i32              reduction;
    Move             excluded;
    i16              staticEval;
    bool             isQuiet;
};
enum ThreadType {
    MAIN      = 1,
    SECONDARY = 0
};
enum NodeType {
    NONPV,
    PV
};

struct SearchParams {
    Stopwatch<std::chrono::milliseconds> time;

    usize depth;
    u64   nodes;
    u64   softNodes;
    u64   mtime;
    u64   wtime;
    u64   btime;
    u64   winc;
    u64   binc;
    usize mate;

    bool isUci;

    SearchParams(Stopwatch<std::chrono::milliseconds> time, usize depth, u64 nodes, u64 softNodes, u64 mtime, u64 wtime, u64 btime, u64 winc, u64 binc, usize mate, bool isUci) :
        time(time),
        depth(depth),
        nodes(nodes),
        softNodes(softNodes),
        mtime(mtime),
        wtime(wtime),
        btime(btime),
        winc(winc),
        binc(binc),
        mate(mate),
        isUci(isUci) {}
};

struct SearchLimit {
    Stopwatch<std::chrono::milliseconds>* time;
    i64                                   searchTime;
    u64                                   maxNodes;

    SearchLimit() {
        time = nullptr;
        searchTime = 0;
        maxNodes = 0;
    }

    SearchLimit(Stopwatch<std::chrono::milliseconds>& time, i64 searchTime, u64 maxNodes) :
        time(&time),
        searchTime(searchTime),
        maxNodes(maxNodes) {}

    SearchLimit(const SearchLimit& other) = default;

    bool outOfNodes(u64 nodes) const { return nodes >= maxNodes && maxNodes > 0; }

    bool outOfTime() const {
        if (searchTime == 0)
            return false;
        return static_cast<i64>(time->elapsed()) >= searchTime;
    }
};

constexpr i32 MATE_SCORE       = 32767;
constexpr i32 MATE_IN_MAX_PLY  = MATE_SCORE - MAX_PLY;
constexpr i32 MATED_IN_MAX_PLY = -MATE_SCORE + static_cast<i32>(MAX_PLY);

constexpr i32 TB_MATE_SCORE       = std::min(30000, MATE_IN_MAX_PLY - 1);
constexpr i32 TB_MATE_IN_MAX_PLY  = TB_MATE_SCORE - MAX_PLY;
constexpr i32 TB_MATED_IN_MAX_PLY = -TB_MATE_SCORE + static_cast<i32>(MAX_PLY);

inline bool isWin(i32 score) { return score >= TB_MATE_IN_MAX_PLY; }
inline bool isLoss(i32 score) { return score <= TB_MATED_IN_MAX_PLY; }
inline bool isDecisive(i32 score) { return isWin(score) || isLoss(score); }
inline bool isTBScore(i32 score) { return isDecisive(score) && std::abs(score) <= TB_MATE_SCORE; }

MoveEvaluation iterativeDeepening(Board board, ThreadInfo& thisThread, SearchParams sp, Searcher* searcher = nullptr);

void bench(usize depth);

void fillLmrTable();

}