#pragma once

#include "search.h"
#include "thread.h"

#include <thread>

struct Searcher {
    std::atomic<bool>  stopFlag;
    Stopwatch<std::chrono::milliseconds> time;
    std::unique_ptr<Search::ThreadInfo> mainData = std::make_unique<Search::ThreadInfo>(Search::ThreadType::MAIN, stopFlag);
    std::thread        mainThread;

    std::vector<Search::ThreadInfo> workerData;
    std::vector<std::thread>        workers;

    Searcher() {
        stopFlag.store(true);
        time.reset();
        mainData->reset();
    }

    void start(Board& board, Search::SearchParams sp);
    void stop();
    void waitUntilFinished();

    void makeThreads(int threads);

    void resizeTT(usize size) {}

    void reset() {
        mainData->reset();
        for (Search::ThreadInfo& w : workerData)
            w.reset();
    }

    string searchReport(const Board& board, const usize depth, i32 score, const PvList& pv);
};