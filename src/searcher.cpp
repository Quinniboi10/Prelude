#include "searcher.h"
#include "types.h"
#include "search.h"
#include "wdl.h"

void Searcher::start(Board& board, Search::SearchParams sp) {
    time           = sp.time;
    mainData->nodes = 0;
    mainThread     = std::thread(Search::iterativeDeepening, board, std::ref(*mainData), sp, this);

    for (usize i = 0; i < workerData.size(); i++) {
        workerData[i].nodes = 0;
        workers.emplace_back(Search::iterativeDeepening, board, std::ref(workerData[i]), sp, nullptr);
    }
}

void Searcher::stop() {
    bool wasSearching = !stopFlag.load();
    stopFlag.store(true, std::memory_order_relaxed);

    if (mainThread.joinable())
        mainThread.join();

    if (workers.size() > 0)
        for (std::thread& t : workers)
            if (t.joinable())
                t.join();

    workers.clear();

    if (wasSearching) {
        // Output a final node count, for debugging reasons
        cout << "info nodes " << countNodes() << endl;

        cout << "bestmove " << findBestThreadMove() << endl;
    }
}

Move Searcher::findBestThreadMove() {
    Move bestMove;
    i32  bestDepth;
    i16  bestScore;

    auto updateBest = [&](Search::ThreadInfo& t) {
        bestMove  = t.pv.moves[0];
        bestDepth = t.depth;
        bestScore = t.score;
    };

    updateBest(*mainData.get());

    for (Search::ThreadInfo& t : workerData) {
        if (t.depth == bestDepth && t.score > bestScore)
            updateBest(t);
        else if (Search::isDecisive(t.score) && t.score > bestScore)
            updateBest(t);
        else if (t.depth > bestDepth && !(Search::isDecisive(t.score) && Search::isDecisive(bestScore) && std::abs(t.score) < std::abs(bestScore)))
            updateBest(t);
    }

    return bestMove;
}

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

    ans << "info depth " << depth << " seldepth " << mainData->seldepth << " time " << time.elapsed() << " hashfull " << TT.hashfull() << " nodes " << nodes;
    if (time.elapsed() > 0)
        ans << " nps " << nodes * 1000 / time.elapsed();
    ans << " score ";
    if (Search::isDecisive(score)) {
        ans << "mate ";
        ans << ((score < 0) ? "-" : "") << (Search::MATE_SCORE - std::abs(score)) / 2 + 1;
    }
    else
        ans << "cp " << scaleEval(score, board);

    ans << " pv";
    for (Move m : pv)
        ans << " " << m;

    return ans.str();
}