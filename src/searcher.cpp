#include "searcher.h"
#include "types.h"
#include "search.h"

void Searcher::start(Board& board, Search::SearchParams sp) {
    mainData.nodes = 0;
    mainThread = std::thread(Search::iterativeDeepening, board, std::ref(mainData), sp, this);

    for (usize i = 0; i < workerData.size(); i++) {
        workerData[i].nodes = 0;
        workers.emplace_back(Search::iterativeDeepening, board, std::ref(workerData[i]), sp, nullptr);
    }
}

void Searcher::stop() {
    stopFlag.store(true, std::memory_order_relaxed);

    if (mainThread.joinable())
        mainThread.join();

    if (workers.size() > 0)
        for (std::thread& t : workers)
            if (t.joinable())
                t.join();

    workers.clear();
}

void Searcher::makeThreads(int threads) {
    threads -= 1;

    stop();
    workerData.clear();

    for (int i = 0; i < threads; i++)
        workerData.emplace_back(Search::ThreadType::SECONDARY, TT, stopFlag);
}