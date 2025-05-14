#pragma once

#include "ttable.h"
#include "search.h"
#include "thread.h"

struct Searcher {
    TranspositionTable TT;
    std::atomic<bool>  stopFlag;
    Stopwatch<std::chrono::milliseconds> time;
    std::unique_ptr<Search::ThreadInfo> mainData = std::make_unique<Search::ThreadInfo>(Search::ThreadType::MAIN, TT, stopFlag);
    std::thread        mainThread;

    std::vector<Search::ThreadInfo> workerData;
    std::vector<std::thread>        workers;

    void start(Board& board, Search::SearchParams sp);
    void stop();

    void makeThreads(int threads);

    void resizeTT(usize size) {
        TT.reserve(size);
        TT.clear(workerData.size() + 1);
    }

    void reset() {
        TT.clear();
        mainData->reset();
        for (Search::ThreadInfo& w : workerData)
            w.reset();
    }

    string searchReport(Board& board, usize depth, i32 score, PvList& pv);
};