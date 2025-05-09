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
using ConthistSegment = array<array<array<int, 64>, 6>, 2>;

struct Stack {
    PvList           pv;
    ConthistSegment* conthist;
    Move             excluded;
    i16              staticEval;
};
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

    // Conthist is indexed [last stm][last pt][last to][stm][pt][to]
    array<array<array<ConthistSegment, 64>, 6>, 2> conthist;

    ThreadType type;

    TranspositionTable& TT;
    std::atomic<bool>&  breakFlag;

    std::atomic<u64> nodes;
    usize            seldepth;

    usize minNmpPly;

    ThreadInfo(ThreadType type, TranspositionTable& TT, std::atomic<bool>& breakFlag) :
        type(type),
        TT(TT),
        breakFlag(breakFlag) {
        deepFill(history, DEFAULT_HISTORY_VALUE);
        deepFill(conthist, DEFAULT_HISTORY_VALUE);
        breakFlag.store(false, std::memory_order_relaxed);
        seldepth  = 0;
        minNmpPly = 0;
    }

    // Copy constructor
    ThreadInfo(const ThreadInfo& other) :
        history(other.history),
        conthist(other.conthist),
        type(other.type),
        TT(other.TT),
        breakFlag(other.breakFlag) {
        nodes.store(other.nodes.load(std::memory_order_relaxed), std::memory_order_relaxed);
        seldepth  = other.seldepth;
        minNmpPly = other.minNmpPly;
    }

    void updateHist(Color stm, Move m, int bonus) {
        int clampedBonus = std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY);
        history[stm][m.from()][m.to()] += clampedBonus - history[stm][m.from()][m.to()] * abs(clampedBonus) / MAX_HISTORY;
    }

    int getHist(Color stm, Move m) { return history[stm][m.from()][m.to()]; }

    ConthistSegment* getConthistSegment(Board& b, Move m) { return &conthist[b.stm][b.getPiece(m.from())][m.to()]; }

    void updateConthist(ConthistSegment* c, Board& b, Move m, int bonus) {
        assert(c != nullptr);
        int  clampedBonus = std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY);
        int& entry        = (*c)[b.stm][b.getPiece(m.from())][m.to()];
        entry += clampedBonus - entry * abs(clampedBonus) / MAX_HISTORY;
    }

    int getConthist(ConthistSegment* c, Board& b, Move m) {
        assert(c != nullptr);
        return (*c)[b.stm][b.getPiece(m.from())][m.to()];
    }

    void reset() {
        deepFill(history, DEFAULT_HISTORY_VALUE);
        deepFill(conthist, DEFAULT_HISTORY_VALUE);

        nodes.store(0, std::memory_order_relaxed);
        seldepth = 0;
    }
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

    SearchParams(Stopwatch<std::chrono::milliseconds> time, usize depth, u64 nodes, u64 softNodes, u64 mtime, u64 wtime, u64 btime, u64 winc, u64 binc, usize mate) :
        time(time),
        depth(depth),
        nodes(nodes),
        softNodes(softNodes),
        mtime(mtime),
        wtime(wtime),
        btime(btime),
        winc(winc),
        binc(binc),
        mate(mate) {}
};

struct SearchLimit {
    Stopwatch<std::chrono::milliseconds>& time;
    u64                                   maxNodes;
    i64                                   searchTime;

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

void fillLmrTable();
}