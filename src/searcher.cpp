#include "searcher.h"
#include "types.h"
#include "search.h"

void Searcher::start(Board& board, Search::SearchParams sp) {
    searchThread = std::thread(Search::iterativeDeepening, board, std::ref(threadData), sp);
}

void Searcher::stop() {
    threadData.breakFlag.store(true, std::memory_order_relaxed);

    if (searchThread.joinable())
        searchThread.join();
}