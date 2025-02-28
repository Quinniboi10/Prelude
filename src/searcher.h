#pragma once

#include "ttable.h"
#include "search.h"

struct Searcher {
    TranspositionTable TT;
    std::atomic<bool>  stopFlag;
    Search::ThreadInfo mainData = Search::ThreadInfo(Search::ThreadType::MAIN, TT, stopFlag);
    std::thread        mainThread;

    std::vector<Search::ThreadInfo> workerData;
    std::vector<std::thread>        workers;

    void start(Board& board, Search::SearchParams sp);
    void stop();

    void makeThreads(int threads);

    void resizeTT(usize size) { TT.resize(size); }

    void reset() {
        TT.clear();
        mainData.reset();
        for (Search::ThreadInfo& w : workerData)
            w.reset();
    }
};