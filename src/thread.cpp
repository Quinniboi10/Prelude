#include "thread.h"

#include "tunable.h"

namespace Search {
ThreadInfo::ThreadInfo(ThreadType type, TranspositionTable& TT, std::atomic<bool>& breakFlag) :
    TT(TT),
    type(type),
    breakFlag(breakFlag) {
    accumulatorStack.clear();

    nodes            = 0;
    seldepth         = 0;
    rootMoves.length = 0;

    minRootScore = 0;
    maxRootScore = 0;
}

ThreadInfo::ThreadInfo(const ThreadInfo& other) :
    accumulatorStack(other.accumulatorStack),
    TT(other.TT),
    type(other.type),
    breakFlag(other.breakFlag),
    seldepth(other.seldepth),
    minRootScore(other.minRootScore),
    maxRootScore(other.maxRootScore),
    rootMoves(other.rootMoves) {
    nodes.store(other.nodes.load(std::memory_order_relaxed), std::memory_order_relaxed);
}

ThreadStackManager ThreadInfo::makeMove(const Board& board, Board& newBoard, Move m) {
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

void ThreadInfo::refresh(const Board& b) {
    accumulatorStack.clear();

    AccumulatorPair accumulators;
    accumulators.resetAccumulators(b);
    accumulatorStack.push(accumulators);
}

void ThreadInfo::reset() {
    nodes.store(0, std::memory_order_relaxed);
    seldepth = 0;
}

}