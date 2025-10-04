#pragma once

#include "search.h"
#include "thread.h"
#include "ttable.h"

#include <thread>

struct Searcher {
    std::atomic<bool>  stopFlag;
    TranspositionTable TT;
    Stopwatch<std::chrono::milliseconds> time;
    std::unique_ptr<Search::ThreadInfo> mainData = std::make_unique<Search::ThreadInfo>(Search::ThreadType::MAIN, TT, stopFlag);
    std::thread        mainThread;

    std::vector<Search::ThreadInfo> workerData;
    std::vector<std::thread>        workers;

    Searcher() {
        stopFlag.store(true);
        time.reset();
        mainData->reset();
        TT.clear();
    }

    void start(Board& board, Search::SearchParams sp);
    void stop();
    void waitUntilFinished() const;

    void makeThreads(int threads);

    void resizeTT(usize size) {
        TT.reserve(size);
        TT.clear();
    }

    void reset() {
        mainData->reset();
        TT.clear();
        for (Search::ThreadInfo& w : workerData)
            w.reset();
    }

    string searchReport(const Board& board, const usize depth, i32 score, const PvList& pv);
};