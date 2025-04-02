#pragma once

#include "types.h"
#include "move.h"
#include "board.h"
#include "stopwatch.h"

enum MovegenMode {
    ALL_MOVES,
    NOISY_ONLY,
    QUIET_ONLY
};

namespace Movegen {
// Tables from https://github.com/Disservin/chess-library/blob/cf3bd56474168605201a01eb78b3222b8f9e65e4/include/chess.hpp#L780
constexpr u64 KNIGHT_ATTACKS[64] = {0x0000000000020400, 0x0000000000050800, 0x00000000000A1100, 0x0000000000142200, 0x0000000000284400, 0x0000000000508800, 0x0000000000A01000, 0x0000000000402000,
                                    0x0000000002040004, 0x0000000005080008, 0x000000000A110011, 0x0000000014220022, 0x0000000028440044, 0x0000000050880088, 0x00000000A0100010, 0x0000000040200020,
                                    0x0000000204000402, 0x0000000508000805, 0x0000000A1100110A, 0x0000001422002214, 0x0000002844004428, 0x0000005088008850, 0x000000A0100010A0, 0x0000004020002040,
                                    0x0000020400040200, 0x0000050800080500, 0x00000A1100110A00, 0x0000142200221400, 0x0000284400442800, 0x0000508800885000, 0x0000A0100010A000, 0x0000402000204000,
                                    0x0002040004020000, 0x0005080008050000, 0x000A1100110A0000, 0x0014220022140000, 0x0028440044280000, 0x0050880088500000, 0x00A0100010A00000, 0x0040200020400000,
                                    0x0204000402000000, 0x0508000805000000, 0x0A1100110A000000, 0x1422002214000000, 0x2844004428000000, 0x5088008850000000, 0xA0100010A0000000, 0x4020002040000000,
                                    0x0400040200000000, 0x0800080500000000, 0x1100110A00000000, 0x2200221400000000, 0x4400442800000000, 0x8800885000000000, 0x100010A000000000, 0x2000204000000000,
                                    0x0004020000000000, 0x0008050000000000, 0x00110A0000000000, 0x0022140000000000, 0x0044280000000000, 0x0088500000000000, 0x0010A00000000000, 0x0020400000000000};

constexpr u64 KING_ATTACKS[64] = {0x0000000000000302, 0x0000000000000705, 0x0000000000000E0A, 0x0000000000001C14, 0x0000000000003828, 0x0000000000007050, 0x000000000000E0A0, 0x000000000000C040,
                                  0x0000000000030203, 0x0000000000070507, 0x00000000000E0A0E, 0x00000000001C141C, 0x0000000000382838, 0x0000000000705070, 0x0000000000E0A0E0, 0x0000000000C040C0,
                                  0x0000000003020300, 0x0000000007050700, 0x000000000E0A0E00, 0x000000001C141C00, 0x0000000038283800, 0x0000000070507000, 0x00000000E0A0E000, 0x00000000C040C000,
                                  0x0000000302030000, 0x0000000705070000, 0x0000000E0A0E0000, 0x0000001C141C0000, 0x0000003828380000, 0x0000007050700000, 0x000000E0A0E00000, 0x000000C040C00000,
                                  0x0000030203000000, 0x0000070507000000, 0x00000E0A0E000000, 0x00001C141C000000, 0x0000382838000000, 0x0000705070000000, 0x0000E0A0E0000000, 0x0000C040C0000000,
                                  0x0003020300000000, 0x0007050700000000, 0x000E0A0E00000000, 0x001C141C00000000, 0x0038283800000000, 0x0070507000000000, 0x00E0A0E000000000, 0x00C040C000000000,
                                  0x0302030000000000, 0x0705070000000000, 0x0E0A0E0000000000, 0x1C141C0000000000, 0x3828380000000000, 0x7050700000000000, 0xE0A0E00000000000, 0xC040C00000000000,
                                  0x0203000000000000, 0x0507000000000000, 0x0A0E000000000000, 0x141C000000000000, 0x2838000000000000, 0x5070000000000000, 0xA0E0000000000000, 0x40C0000000000000};


u64 pawnAttackBB(Color c, int sq);

void initializeAllDatabases();

MoveList generateLegalMoves(Board& board);

void perft(Board& board, usize depth, bool bulk);
void perftSuite(const string filePath);

u64 getBishopAttacks(Square square, u64 occ);
u64 getXrayBishopAttacks(Square square, u64 occ, u64 blockers);
u64 getRookAttacks(Square square, u64 occ);
u64 getXrayRookAttacks(Square square, u64 occ, u64 blockers);

template<Color stm, MovegenMode mode>
void generatePawnMoves(const Board& board, MoveList& moves) {
    const u64 occ    = board.pieces();
    const u64 opp    = board.pieces(~stm);
    const u64 free   = ~occ;
    const u64 pawnBB = board.pieces(stm, PAWN);

    constexpr u64 backrank        = MASK_RANK[RANK1] | MASK_RANK[RANK8];
    constexpr u64 doublePushMask  = stm == WHITE ? MASK_RANK[RANK3] : MASK_RANK[RANK6];
    constexpr int shiftMultiplier = stm == WHITE ? 1 : -1;

    u64 singlePush = shiftRelative<stm, NORTH>(pawnBB) & free;
    u64 pushPromo  = singlePush & backrank;
    singlePush ^= pushPromo;

    u64 doublePush = shiftRelative<stm, NORTH>(singlePush & doublePushMask) & free;

    int    backshift;
    Square to;

    if constexpr (mode == ALL_MOVES || mode == QUIET_ONLY) {
        u64 quietPromo = pushPromo;

        backshift = relativeDir(~stm, NORTH);
        while (singlePush) {
            to = popLSB(singlePush);
            moves.add(to + backshift, to);
        }
        while (quietPromo) {
            to = popLSB(quietPromo);
            moves.add(to + backshift, to, ROOK);
            moves.add(to + backshift, to, BISHOP);
            moves.add(to + backshift, to, KNIGHT);
        }
        backshift = relativeDir(~stm, NORTH_NORTH);
        while (doublePush) {
            to = popLSB(doublePush);
            moves.add(to + backshift, to);
        }
    }

    if constexpr (mode == ALL_MOVES || mode == NOISY_ONLY) {
        u64 captureEast = shiftRelative<stm, NORTH_EAST>(pawnBB & ~MASK_FILE[HFILE]) & opp;
        u64 captureWest = shiftRelative<stm, NORTH_WEST>(pawnBB & ~MASK_FILE[AFILE]) & opp;
        u64 promoEast   = captureEast & backrank;
        u64 promoWest   = captureWest & backrank;
        captureEast ^= promoEast;
        captureWest ^= promoWest;

        backshift = relativeDir(~stm, NORTH_WEST);
        while (captureEast) {
            to = popLSB(captureEast);
            moves.add(to + backshift, to);
        }
        while (promoEast) {
            to = popLSB(promoEast);
            moves.add(to + backshift, to, QUEEN);
            moves.add(to + backshift, to, ROOK);
            moves.add(to + backshift, to, BISHOP);
            moves.add(to + backshift, to, KNIGHT);
        }

        backshift = relativeDir(~stm, NORTH_EAST);
        while (captureWest) {
            to = popLSB(captureWest);
            moves.add(to + backshift, to);
        }
        while (promoWest) {
            to = popLSB(promoWest);
            moves.add(to + backshift, to, QUEEN);
            moves.add(to + backshift, to, ROOK);
            moves.add(to + backshift, to, BISHOP);
            moves.add(to + backshift, to, KNIGHT);
        }

        backshift = relativeDir(~stm, NORTH);
        while (pushPromo) {
            to = popLSB(pushPromo);
            moves.add(to + backshift, to, QUEEN);
        }

        // EP from east
        if (board.epSquare != NO_SQUARE && (shiftRelative<stm, SOUTH_EAST>((1ULL << board.epSquare) & ~MASK_FILE[HFILE]) & board.pieces(stm, PAWN))) {
            moves.add(static_cast<int>(relativeDir(stm, SOUTH_EAST)) + board.epSquare, board.epSquare, EN_PASSANT);
        }
        // EP from west
        if (board.epSquare != NO_SQUARE && (shiftRelative<stm, SOUTH_WEST>((1ULL << board.epSquare) & ~MASK_FILE[AFILE]) & board.pieces(stm, PAWN))) {
            moves.add(static_cast<int>(relativeDir(stm, SOUTH_WEST)) + board.epSquare, board.epSquare, EN_PASSANT);
        }
    }
}

template<MovegenMode mode>
void generateKnightMoves(const Board& board, MoveList& moves) {
    u64 friendly = board.pieces(board.stm);
    u64 knightBB = board.pieces(KNIGHT) & friendly;

    while (knightBB) {
        Square from = popLSB(knightBB);

        u64 attacksBB = KNIGHT_ATTACKS[from] & ~friendly;
        if constexpr (mode == QUIET_ONLY)
            attacksBB &= ~board.pieces();
        else if (mode == NOISY_ONLY)
            attacksBB &= board.pieces(~board.stm);

        while (attacksBB) {
            Square to = popLSB(attacksBB);
            moves.add(from, to);
        }
    }
}

template<MovegenMode mode>
void generateHorizontalSliders(const Board& board, MoveList& moves) {
    const u64 occ      = board.pieces();
    u64       friendly = board.pieces(board.stm);
    u64       sliderBB = (board.pieces(ROOK) | board.pieces(QUEEN)) & friendly;

    while (sliderBB) {
        Square from = popLSB(sliderBB);

        u64 attacksBB = getRookAttacks(from, occ) & ~friendly;
        if constexpr (mode == QUIET_ONLY)
            attacksBB &= ~occ;
        else if (mode == NOISY_ONLY)
            attacksBB &= board.pieces(~board.stm);

        while (attacksBB) {
            Square to = popLSB(attacksBB);
            moves.add(from, to);
        }
    }
}

template<MovegenMode mode>
void generateDiagonalSliders(const Board& board, MoveList& moves) {
    const u64 occ      = board.pieces();
    u64       friendly = board.pieces(board.stm);
    u64       sliderBB = (board.pieces(BISHOP) | board.pieces(QUEEN)) & friendly;

    while (sliderBB) {
        Square from = popLSB(sliderBB);

        u64 attacksBB = getBishopAttacks(from, occ) & ~friendly;
        if constexpr (mode == QUIET_ONLY)
            attacksBB &= ~occ;
        else if (mode == NOISY_ONLY)
            attacksBB &= board.pieces(~board.stm);

        while (attacksBB) {
            Square to = popLSB(attacksBB);
            moves.add(from, to);
        }
    }
}

template<MovegenMode mode>
void generateKingMoves(const Board& board, MoveList& moves) {
    u64       friendly = board.pieces(board.stm);
    const u64 kingBB   = board.pieces(KING) & friendly;

    Square from = static_cast<Square>(ctzll(kingBB));

    u64 attacksBB = KING_ATTACKS[from] & ~friendly;
    if constexpr (mode == QUIET_ONLY)
        attacksBB &= ~board.pieces();
    else if (mode == NOISY_ONLY)
        attacksBB &= board.pieces(~board.stm);

    while (attacksBB) {
        Square to = popLSB(attacksBB);
        moves.add(from, to);
    }

    if (board.canCastle(board.stm, true))
        moves.add(from, castleSq(board.stm, true), CASTLE);
    if (board.canCastle(board.stm, false))
        moves.add(from, castleSq(board.stm, false), CASTLE);
}

template<MovegenMode mode>
MoveList generateMoves(const Board& board) {
    MoveList moves;
    generateKingMoves<mode>(board, moves);
    if (board.doubleCheck)
        return moves;
    if (board.stm == WHITE)
        generatePawnMoves<WHITE, mode>(board, moves);
    else
        generatePawnMoves<BLACK, mode>(board, moves);
    generateHorizontalSliders<mode>(board, moves);
    generateDiagonalSliders<mode>(board, moves);
    generateKnightMoves<mode>(board, moves);

    return moves;
}
}