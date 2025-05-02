#include "accumulator.h"
#include "globals.h"
#include "nnue.h"
#include "board.h"

void AccumulatorPair::resetAccumulators(const Board& board) {
    const u64 whiteKing = board.pieces(WHITE, KING);
    const u64 blackKing = board.pieces(BLACK, KING);

    const bool mirrorWhite = fileOf(getLSB(whiteKing)) >= EFILE && HORIZONTAL_MIRRORING;
    const bool mirrorBlack = fileOf(getLSB(blackKing)) >= EFILE && HORIZONTAL_MIRRORING;

    resetAccumulator(WHITE, board, mirrorWhite);
    resetAccumulator(BLACK, board, mirrorBlack);

    mirrored = {mirrorBlack, mirrorWhite};
}

void AccumulatorPair::resetAccumulator(const Color c, const Board& board, const bool mirror) {
    Accumulator& accum       = c == WHITE ? white : black;
    u64          whitePieces = board.pieces(WHITE);
    u64          blackPieces = board.pieces(BLACK);

    accum = nnue.hiddenLayerBias;

    while (whitePieces) {
        Square sq = popLSB(whitePieces);

        usize inputFeature = NNUE::feature(c, WHITE, board.getPiece(sq), sq, mirror);

        for (usize i = 0; i < HL_SIZE; i++)
            accum[i] += nnue.weightsToHL[inputFeature * HL_SIZE + i];
    }

    while (blackPieces) {
        Square sq = popLSB(blackPieces);

        usize inputFeature = NNUE::feature(c, BLACK, board.getPiece(sq), sq, mirror);

        for (usize i = 0; i < HL_SIZE; i++)
            accum[i] += nnue.weightsToHL[inputFeature * HL_SIZE + i];
    }
}

void AccumulatorPair::update(const Board& board, const Move m) {
    const Color     stm       = board.stm;
    const Square    from      = m.from();
    const Square    to        = m.to();
    const MoveType  mt        = m.typeOf();
    const PieceType pt        = board.getPiece(from);
    const PieceType toPT      = board.getPiece(to);
    const PieceType endPT     = m.typeOf() == PROMOTION ? m.promo() : pt;

    // If the move is a castle and the king ends on the right half of the board
    // Or the king is moving to the right half of the board
    const bool shouldMirror = HORIZONTAL_MIRRORING && (mt == CASTLE ? (fileOf(KING_CASTLE_END_SQ[castleIndex(stm, to > from)]) >= EFILE) :
        (pt == KING ? (fileOf(to) >= EFILE) : mirrored[stm]));
    bool&      isMirrored   = mirrored[stm];

    if (isMirrored == shouldMirror) {
        if (mt == EN_PASSANT)
            addSubSub(stm, to, PAWN, from, PAWN, to + (stm == WHITE ? SOUTH : NORTH), PAWN, shouldMirror);
        else if (mt == CASTLE) {
            const bool   isKingside = to > from;
            const Square kingEndSq  = KING_CASTLE_END_SQ[castleIndex(stm, isKingside)];
            const Square rookEndSq  = ROOK_CASTLE_END_SQ[castleIndex(stm, isKingside)];
            addAddSubSub(stm, kingEndSq, KING, rookEndSq, ROOK, from, KING, to, ROOK, shouldMirror);
        }
        else if (board.isCapture(m))
            addSubSub(stm, to, endPT, from, pt, to, toPT, shouldMirror);
        else
            addSub(stm, to, endPT, from, pt, shouldMirror);
    }
    else
        resetAccumulator(stm, board, shouldMirror);

    isMirrored = shouldMirror;
}

// All friendly, for quiets
void AccumulatorPair::addSub(Color stm, Square add, PieceType addPT, Square sub, PieceType subPT, bool mirror) {
    int addW = NNUE::feature(WHITE, stm, addPT, add, mirror);
    int addB = NNUE::feature(BLACK, stm, addPT, add, mirror);

    int subW = NNUE::feature(WHITE, stm, subPT, sub, mirror);
    int subB = NNUE::feature(BLACK, stm, subPT, sub, mirror);

    for (usize i = 0; i < HL_SIZE; i++) {
        white[i] += nnue.weightsToHL[addW * HL_SIZE + i] - nnue.weightsToHL[subW * HL_SIZE + i];
        black[i] += nnue.weightsToHL[addB * HL_SIZE + i] - nnue.weightsToHL[subB * HL_SIZE + i];
    }
}

// Captures
void AccumulatorPair::addSubSub(Color stm, Square add, PieceType addPT, Square sub1, PieceType subPT1, Square sub2, PieceType subPT2, bool mirror) {
    int addW = NNUE::feature(WHITE, stm, addPT, add, mirror);
    int addB = NNUE::feature(BLACK, stm, addPT, add, mirror);

    int subW1 = NNUE::feature(WHITE, stm, subPT1, sub1, mirror);
    int subB1 = NNUE::feature(BLACK, stm, subPT1, sub1, mirror);

    int subW2 = NNUE::feature(WHITE, ~stm, subPT2, sub2, mirror);
    int subB2 = NNUE::feature(BLACK, ~stm, subPT2, sub2, mirror);

    for (usize i = 0; i < HL_SIZE; i++) {
        white[i] += nnue.weightsToHL[addW * HL_SIZE + i] - nnue.weightsToHL[subW1 * HL_SIZE + i] - nnue.weightsToHL[subW2 * HL_SIZE + i];
        black[i] += nnue.weightsToHL[addB * HL_SIZE + i] - nnue.weightsToHL[subB1 * HL_SIZE + i] - nnue.weightsToHL[subB2 * HL_SIZE + i];
    }
}

// Castling
void AccumulatorPair::addAddSubSub(Color stm, Square add1, PieceType addPT1, Square add2, PieceType addPT2, Square sub1, PieceType subPT1, Square sub2, PieceType subPT2, bool mirror) {
    int addW1 = NNUE::feature(WHITE, stm, addPT1, add1, mirror);
    int addB1 = NNUE::feature(BLACK, stm, addPT1, add1, mirror);

    int addW2 = NNUE::feature(WHITE, stm, addPT2, add2, mirror);
    int addB2 = NNUE::feature(BLACK, stm, addPT2, add2, mirror);

    int subW1 = NNUE::feature(WHITE, stm, subPT1, sub1, mirror);
    int subB1 = NNUE::feature(BLACK, stm, subPT1, sub1, mirror);

    int subW2 = NNUE::feature(WHITE, stm, subPT2, sub2, mirror);
    int subB2 = NNUE::feature(BLACK, stm, subPT2, sub2, mirror);

    for (usize i = 0; i < HL_SIZE; i++) {
        white[i] += nnue.weightsToHL[addW1 * HL_SIZE + i] + nnue.weightsToHL[addW2 * HL_SIZE + i] - nnue.weightsToHL[subW1 * HL_SIZE + i] - nnue.weightsToHL[subW2 * HL_SIZE + i];
        black[i] += nnue.weightsToHL[addB1 * HL_SIZE + i] + nnue.weightsToHL[addB2 * HL_SIZE + i] - nnue.weightsToHL[subB1 * HL_SIZE + i] - nnue.weightsToHL[subB2 * HL_SIZE + i];
    }
}