#include "thread.h"

namespace Search {
ThreadInfo::ThreadInfo(ThreadType type, TranspositionTable& TT, std::atomic<bool>& breakFlag) :
    type(type),
    TT(TT),
    breakFlag(breakFlag) {
    deepFill(history, DEFAULT_HISTORY_VALUE);
    deepFill(conthist, DEFAULT_HISTORY_VALUE);
    breakFlag.store(false, std::memory_order_relaxed);

    nodes            = 0;
    tbHits           = 0;
    seldepth         = 0;
    minRootScore     = -MATE_SCORE;
    maxRootScore     = MATE_SCORE;
    rootMoves.length = 0;
    minNmpPly        = 0;
}
ThreadInfo::ThreadInfo(const ThreadInfo& other) :
    history(other.history),
    conthist(other.conthist),
    accumulatorStack(other.accumulatorStack),
    type(other.type),
    TT(other.TT),
    breakFlag(other.breakFlag),
    seldepth(other.seldepth),
    minRootScore(other.minRootScore),
    maxRootScore(other.maxRootScore),
    rootMoves(other.rootMoves),
    minNmpPly(other.minNmpPly) {
    nodes.store(other.nodes.load(std::memory_order_relaxed), std::memory_order_relaxed);
    tbHits.store(other.tbHits.load(std::memory_order_relaxed), std::memory_order_relaxed);
}

i64 ThreadInfo::getQuietHistory(Board& b, SearchStack* ss, Move m) const {
    i64 res = getHist(b.stm, m);
    if (ss != nullptr && (ss - 1)->conthist != nullptr)
        res += getConthist((ss - 1)->conthist, b, m);
    if (ss != nullptr && (ss - 2)->conthist != nullptr)
        res += getConthist((ss - 2)->conthist, b, m);

    return res;
}

void ThreadInfo::updateConthist(SearchStack* ss, Board& b, Move m, int bonus) {
    assert(ss != nullptr);

    auto updateEntry = [&](int& entry) {
        int clampedBonus = std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY);
        entry += clampedBonus - entry * abs(clampedBonus) / MAX_HISTORY;
    };

    if ((ss - 1)->conthist != nullptr)
        updateEntry((*(ss - 1)->conthist)[b.stm][b.getPiece(m.from())][m.to()]);

    if ((ss - 2)->conthist != nullptr)
        updateEntry((*(ss - 2)->conthist)[b.stm][b.getPiece(m.from())][m.to()]);
}

ThreadStackManager ThreadInfo::makeMove(Board& board, Board& newBoard, Move m) {
    newBoard.move(m);

    accumulatorStack.push(accumulatorStack.top());
    accumulatorStack.topAsReference().update(newBoard, m, board.getPiece(m.to()));

    return ThreadStackManager(*this);
}

ThreadStackManager ThreadInfo::makeNullMove(Board& newBoard) {
    newBoard.nullMove();

    accumulatorStack.push(accumulatorStack.top());

    return ThreadStackManager(*this);
}

void ThreadInfo::refresh(Board& b) {
    accumulatorStack.clear();

    AccumulatorPair accumulators;
    accumulators.resetAccumulators(b);
    accumulatorStack.push(accumulators);
}

void ThreadInfo::reset() {
    deepFill(history, DEFAULT_HISTORY_VALUE);
    deepFill(conthist, DEFAULT_HISTORY_VALUE);

    nodes.store(0, std::memory_order_relaxed);
    tbHits.store(0, std::memory_order_relaxed);
    seldepth = 0;
}

}