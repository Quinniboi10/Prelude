#pragma once

#include "util.h"
#include "types.h"
#include "move.h"

struct Board {
    // Indexed pawns, knights, bishops, rooks, queens, king
    array<u64, 6> white;
    array<u64, 6> black;

    Square epSquare;
    // Only last 4 bits are meaningful, index KQkq
    u8 castlingRights;

    Color stm;

    int halfMoveClock;
    int fullMoveClock;

   private:
    char getPieceAt(int i) const;

    void placePiece(Color c, PieceType pt, int sq);
    void removePiece(Color c, PieceType pt, int sq);
    void removePiece(Color c, int sq);

   public:
    u64 pieces() const;
    u64 pieces(Color c) const;
    u64 pieces(PieceType pt) const;
    u64 pieces(Color c, PieceType pt) const;

    void reset();

    void loadFromFEN(string fen);

    void display() const;

    PieceType getPiece(int sq);

    void move(Move m);

    bool canCastle(Color c, bool kingside);
};