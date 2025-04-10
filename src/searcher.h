#pragma once

#include "ttable.h"
#include "search.h"

struct Searcher {
    TranspositionTable TT;
    std::atomic<bool>  stopFlag;
    std::atomic<bool>  killFlag;

    Search::ThreadInfo mainData = Search::ThreadInfo(Search::ThreadType::MAIN, TT, stopFlag);
    std::thread        mainThread;

    std::vector<Search::ThreadInfo> workerData;
    std::vector<std::thread>        workers;

    Board& board;

    Searcher(Board& board) :
    board(board) {
        TT.clear();
        stopFlag.store(true, std::memory_order_relaxed);
        killFlag.store(false, std::memory_order_relaxed);
    }

    void start(Search::SearchParams sp);
    void stop();

    void makeThreads(usize threads);
    void killThreads();

    void resizeTT(usize size) {
        TT.reserve(size);
        TT.clear(workerData.size() + 1);
    }

    void reset() {
        TT.clear();
        mainData.reset();
        for (Search::ThreadInfo& w : workerData)
            w.reset();
    }
};