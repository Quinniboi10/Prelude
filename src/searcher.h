#pragma once

#include "ttable.h"
#include "search.h"

struct Searcher {
    TranspositionTable TT;
    Search::ThreadInfo threadData = Search::ThreadInfo(Search::ThreadType::MAIN, TT);
    std::thread        searchThread;

    void start(Board& board, Search::SearchParams sp);
    void stop();

    void resizeTT(usize size) { TT.resize(size); }

    void reset() {
        TT.clear();
        threadData.reset();
    }
};