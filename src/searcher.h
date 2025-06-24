#pragma once

#include "ttable.h"
#include "search.h"
#include "thread.h"

#include <thread>

struct Searcher {
    TranspositionTable TT;
    std::atomic<bool>  stopFlag;
    Stopwatch<std::chrono::milliseconds> time;
    std::unique_ptr<Search::ThreadInfo> mainData = std::make_unique<Search::ThreadInfo>(Search::ThreadType::MAIN, TT, stopFlag);
    std::thread        mainThread;

    std::vector<Search::ThreadInfo> workerData;
    std::vector<std::thread>        workers;

    Searcher() {
        TT.clear();
        stopFlag.store(true);
        time.reset();
        mainData->reset();
    }

    void start(Board& board, Search::SearchParams sp);
    void stop();
    void waitUntilFinished();

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