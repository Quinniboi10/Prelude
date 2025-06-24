#include "searcher.h"
#include "types.h"
#include "search.h"
#include "globals.h"
#include "wdl.h"

#include <algorithm>

void Searcher::start(Board& board, Search::SearchParams sp) {
    time           = sp.time;
    mainData->nodes = 0;
    mainData->tbHits = 0;
    mainThread     = std::thread(Search::iterativeDeepening, board, std::ref(*mainData), sp, this);

    for (usize i = 0; i < workerData.size(); i++) {
        workerData[i].nodes = 0;
        workerData[i].tbHits = 0;
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

void Searcher::waitUntilFinished() { stopFlag.wait(false); }

void Searcher::makeThreads(int threads) {
    threads -= 1;

    stop();
    workerData.clear();

    for (int i = 0; i < threads; i++)
        workerData.emplace_back(Search::ThreadType::SECONDARY, TT, stopFlag);
}

string Searcher::searchReport(Board& board, usize depth, i32 score, PvList& pv) {
    std::ostringstream ans;

    u64 nodes = mainData->nodes;
    for (Search::ThreadInfo& t : workerData)
        nodes += t.nodes;

    u64 tbHits = mainData->tbHits;
    for (Search::ThreadInfo& t : workerData)
        nodes += t.tbHits;

    score = std::clamp(score, mainData->minRootScore, mainData->maxRootScore);

    #ifndef NDEBUG
    if (Search::isTBScore(score)) {
        if (Search::isWin(score))
            cout << "info string TB win found in " << Search::TB_MATE_SCORE - score << " ply" << endl;
        else if (Search::isLoss(score))
            cout << "info string TB loss found in " << Search::TB_MATE_SCORE + score << " ply" << endl;
    }
    #endif

    ans << "info depth " << depth << " seldepth " << mainData->seldepth << " time " << time.elapsed() << " hashfull " << TT.hashfull() << " nodes " << nodes;
    if (time.elapsed() > 0)
        ans << " nps " << nodes * 1000 / time.elapsed();
    if (tbEnabled)
        ans << " tbhits " << tbHits;

    ans << " score ";
    // Don't report mate scores if it's from the TB
    if (Search::isDecisive(score) && !Search::isTBScore(score)) {
        ans << "mate ";
        ans << ((score < 0) ? "-" : "") << (Search::MATE_SCORE - std::abs(score)) / 2 + 1;
    }
    else
        ans << "cp " << (Search::isTBScore(score) ? score : scaleEval(score, board));

    ans << " pv";
    for (Move m : pv)
        ans << " " << m;

    return ans.str();
}