#include "searcher.h"
#include "types.h"
#include "search.h"

void runWorker(Board& board, Search::ThreadInfo& thisThread, std::atomic<bool>& killFlag) {
    thisThread.breakFlag.store(true, std::memory_order_relaxed);
    while (true) {
        if (killFlag.load(std::memory_order_relaxed))
            return;
        else if (thisThread.breakFlag.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        Search::iterativeDeepening(board, thisThread, Search::SearchParams());
    }
}

void Searcher::start(Search::SearchParams sp) {
    mainThread = std::thread(Search::iterativeDeepening, board, std::ref(mainData), sp, this);
}

void Searcher::stop() {
    stopFlag.store(true, std::memory_order_relaxed);

    if (mainThread.joinable())
        mainThread.join();
}

void Searcher::makeThreads(usize threads) {
    assert(stopFlag.load(std::memory_order_relaxed));

    threads -= 1;

    killThreads();

    for (usize i = 0; i < threads; i++) {
        workerData.emplace_back(Search::ThreadType::SECONDARY, TT, stopFlag);
        workers.emplace_back(runWorker, std::ref(board), std::ref(workerData[i]), std::ref(killFlag));
    }
}

void Searcher::killThreads() {
    killFlag.store(true, std::memory_order_relaxed);

    stop();

    for (std::thread& t : workers)
        if (t.joinable())
            t.join();

    killFlag.store(false, std::memory_order_relaxed);

    workerData.clear();
    workers.clear();
}