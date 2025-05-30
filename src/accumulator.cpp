#include "accumulator.h"
#include "globals.h"
#include "nnue.h"
#include "board.h"

void AccumulatorPair::resetAccumulators(const Board& board) {
    u64 whitePieces = board.pieces(WHITE);
    u64 blackPieces = board.pieces(BLACK);

    white = nnue.hiddenLayerBias;
    black = nnue.hiddenLayerBias;

    while (whitePieces) {
        Square sq = popLSB(whitePieces);

        usize whiteInputFeature = NNUE::feature(WHITE, WHITE, board.getPiece(sq), sq);
        usize blackInputFeature = NNUE::feature(BLACK, WHITE, board.getPiece(sq), sq);

        for (usize i = 0; i < HL_SIZE; i++) {
            white[i] += nnue.weightsToHL[whiteInputFeature * HL_SIZE + i];
            black[i] += nnue.weightsToHL[blackInputFeature * HL_SIZE + i];
        }
    }

    while (blackPieces) {
        Square sq = popLSB(blackPieces);

        usize whiteInputFeature = NNUE::feature(WHITE, BLACK, board.getPiece(sq), sq);
        usize blackInputFeature = NNUE::feature(BLACK, BLACK, board.getPiece(sq), sq);

        for (usize i = 0; i < HL_SIZE; i++) {
            white[i] += nnue.weightsToHL[whiteInputFeature * HL_SIZE + i];
            black[i] += nnue.weightsToHL[blackInputFeature * HL_SIZE + i];
        }
    }
}

void AccumulatorPair::update(const Board& board, const Move m, const PieceType toPT) {
    const Color     stm   = ~board.stm;
    const Square    from  = m.from();
    const Square    to    = m.to();
    const MoveType  mt    = m.typeOf();
    const PieceType pt    = mt == PROMOTION ? PAWN : board.getPiece(to);
    const PieceType endPT = mt == PROMOTION ? m.promo() : pt;

    if (mt == EN_PASSANT)
        addSubSub(stm, to, PAWN, from, PAWN, to + (stm == WHITE ? SOUTH : NORTH), PAWN);
    else if (mt == CASTLE) {
        const bool isKingside = to > from;
        addAddSubSub(stm, KING_CASTLE_END_SQ[castleIndex(stm, isKingside)], KING, ROOK_CASTLE_END_SQ[castleIndex(stm, isKingside)], ROOK, from, KING, to, ROOK);
    }
    else if (toPT != NO_PIECE_TYPE)
        addSubSub(stm, to, endPT, from, pt, to, toPT);
    else
        addSub(stm, to, endPT, from, pt);
}

// All friendly, for quiets
void AccumulatorPair::addSub(Color stm, Square add, PieceType addPT, Square sub, PieceType subPT) {
    int addW = NNUE::feature(WHITE, stm, addPT, add);
    int addB = NNUE::feature(BLACK, stm, addPT, add);

    int subW = NNUE::feature(WHITE, stm, subPT, sub);
    int subB = NNUE::feature(BLACK, stm, subPT, sub);

    for (usize i = 0; i < HL_SIZE; i++) {
        white[i] += nnue.weightsToHL[addW * HL_SIZE + i] - nnue.weightsToHL[subW * HL_SIZE + i];
        black[i] += nnue.weightsToHL[addB * HL_SIZE + i] - nnue.weightsToHL[subB * HL_SIZE + i];
    }
}

// Captures
void AccumulatorPair::addSubSub(Color stm, Square add, PieceType addPT, Square sub1, PieceType subPT1, Square sub2, PieceType subPT2) {
    int addW = NNUE::feature(WHITE, stm, addPT, add);
    int addB = NNUE::feature(BLACK, stm, addPT, add);

    int subW1 = NNUE::feature(WHITE, stm, subPT1, sub1);
    int subB1 = NNUE::feature(BLACK, stm, subPT1, sub1);

    int subW2 = NNUE::feature(WHITE, ~stm, subPT2, sub2);
    int subB2 = NNUE::feature(BLACK, ~stm, subPT2, sub2);

    for (usize i = 0; i < HL_SIZE; i++) {
        white[i] += nnue.weightsToHL[addW * HL_SIZE + i] - nnue.weightsToHL[subW1 * HL_SIZE + i] - nnue.weightsToHL[subW2 * HL_SIZE + i];
        black[i] += nnue.weightsToHL[addB * HL_SIZE + i] - nnue.weightsToHL[subB1 * HL_SIZE + i] - nnue.weightsToHL[subB2 * HL_SIZE + i];
    }
}

// Castling
void AccumulatorPair::addAddSubSub(Color stm, Square add1, PieceType addPT1, Square add2, PieceType addPT2, Square sub1, PieceType subPT1, Square sub2, PieceType subPT2) {
    int addW1 = NNUE::feature(WHITE, stm, addPT1, add1);
    int addB1 = NNUE::feature(BLACK, stm, addPT1, add1);

    int addW2 = NNUE::feature(WHITE, stm, addPT2, add2);
    int addB2 = NNUE::feature(BLACK, stm, addPT2, add2);

    int subW1 = NNUE::feature(WHITE, stm, subPT1, sub1);
    int subB1 = NNUE::feature(BLACK, stm, subPT1, sub1);

    int subW2 = NNUE::feature(WHITE, stm, subPT2, sub2);
    int subB2 = NNUE::feature(BLACK, stm, subPT2, sub2);

    for (usize i = 0; i < HL_SIZE; i++) {
        white[i] += nnue.weightsToHL[addW1 * HL_SIZE + i] + nnue.weightsToHL[addW2 * HL_SIZE + i] - nnue.weightsToHL[subW1 * HL_SIZE + i] - nnue.weightsToHL[subW2 * HL_SIZE + i];
        black[i] += nnue.weightsToHL[addB1 * HL_SIZE + i] + nnue.weightsToHL[addB2 * HL_SIZE + i] - nnue.weightsToHL[subB1 * HL_SIZE + i] - nnue.weightsToHL[subB2 * HL_SIZE + i];
    }
}