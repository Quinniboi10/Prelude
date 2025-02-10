#pragma once

#include "util.h"
#include "types.h"
#include "move.h"

class Board {
    // Indexed pawns, knights, bishops, rooks, queens, king
    array<u64, 6> white;
    array<u64, 6> black;

    int epSquare;
    // Only last 4 bits are meaningful, index KQkq
    u8  castlingRights;

    Color stm = WHITE;

    int halfMoveClock;
    int fullMoveClock;

    char getPieceAt(int i) const;

    u64 pieces() const;
    u64 pieces(Color c) const;
    u64 pieces(PieceType pt) const;

    template<Color c>
    u64 pawnAttackBB(int sq);

    void placePiece(Color c, PieceType pt, int sq);
    void removePiece(Color c, PieceType pt, int sq);
    void removePiece(Color c, int sq);

   public:
    void reset();

    void loadFromFEN(string fen);

    void display() const;

    PieceType getPiece(int sq);

    void move(Move m);
};