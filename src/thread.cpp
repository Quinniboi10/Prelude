#include "thread.h"

#include "tunable.h"

namespace Search {
ThreadInfo::ThreadInfo(ThreadType type, TranspositionTable& TT, std::atomic<bool>& breakFlag) :
    TT(TT),
    type(type),
    breakFlag(breakFlag) {
    std::memset(quietHist.data(), 0, sizeof(quietHist));
    std::memset(capthist.data(), 0, sizeof(capthist));

    accumulatorStack.clear();

    nodes            = 0;
    seldepth         = 0;
    rootMoves.length = 0;

    minRootScore = 0;
    maxRootScore = 0;
}

ThreadInfo::ThreadInfo(const ThreadInfo& other) :
    quietHist(other.quietHist),
    capthist(other.capthist),
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

// Histories
i32 ThreadInfo::getQuietHistory(const Board& board, Move m) const { return quietHist[m.from()][m.to()][board.isUnderAttack(board.stm, m.from())][board.isUnderAttack(board.stm, m.to())]; }
void ThreadInfo::updateQuietHistory(const Board& board, Move m, i32 bonus) {
    i32& entry = quietHist[m.from()][m.to()][board.isUnderAttack(board.stm, m.from())][board.isUnderAttack(board.stm, m.to())];
    const i32 clampedBonus = std::clamp<i32>(bonus, -MAX_HISTORY, MAX_HISTORY);

    entry += clampedBonus - entry * abs(clampedBonus) / MAX_HISTORY;
}

i32 ThreadInfo::getCaptureHistory(const Board& board, Move m) const { return capthist[board.stm][board.getPiece(m.from())][board.getPiece(m.to())][m.to()]; }
void ThreadInfo::updateCaptureHistory(const Board& board, Move m, i32 bonus) {
    i32& entry = capthist[board.stm][board.getPiece(m.from())][board.getPiece(m.to())][m.to()];
    const i32 clampedBonus = std::clamp<i32>(bonus, -MAX_HISTORY, MAX_HISTORY);

    entry += clampedBonus - entry * abs(clampedBonus) / MAX_HISTORY;
}

std::pair<Board, ThreadStackManager>
ThreadInfo::makeMove(const Board& board, Move m) {
    Board newBoard = board;
    newBoard.move(m);

    accumulatorStack.push(accumulatorStack.top());
    accumulatorStack.topAsReference().update(newBoard, m, board.getPiece(m.to()));

    return std::pair<Board, ThreadStackManager>(
        std::piecewise_construct,
        std::forward_as_tuple(std::move(newBoard)),
        std::forward_as_tuple(*this)
    );
}

std::pair<Board, ThreadStackManager>
ThreadInfo::makeNullMove(Board board) {
    board.nullMove();

    accumulatorStack.push(accumulatorStack.top());

    return std::pair<Board, ThreadStackManager>(
        std::piecewise_construct,
        std::forward_as_tuple(std::move(board)),
        std::forward_as_tuple(*this)
    );
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