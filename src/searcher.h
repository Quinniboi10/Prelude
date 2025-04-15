#pragma once

#include "ttable.h"
#include "search.h"

struct Searcher {
    TranspositionTable TT;
    std::atomic<bool>  stopFlag;
    std::thread        mainThread;

    std::vector<Search::ThreadInfo> workerData;
    std::vector<std::thread>        workers;

    Searcher() {
        stopFlag.store(true, std::memory_order_relaxed);
        workerData.emplace_back(Search::ThreadType::MAIN, TT, stopFlag);
        reset();
    }

    void start(Board& board, Search::SearchParams sp);
    void stop();

    void makeThreads(int threads);

    void resizeTT(usize size) {
        TT.reserve(size);
        TT.clear(workerData.size() + 1);
    }

    void reset() {
        TT.clear();
        for (Search::ThreadInfo& w : workerData)
            w.reset();
    }
};