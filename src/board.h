#pragma once

#include "util.h"
#include "types.h"
#include "move.h"

struct Board {
    // Indexed pawns, knights, bishops, rooks, queens, king
    array<u64, 6> byPieces;
    // Index is based on color so black is colors[0]
    array<u64, 2> byColor;
    // Index is based on square, returns the piece type
    array<PieceType, 64> mailbox;

    bool          doubleCheck = false;
    u64           checkMask   = 0;
    u64           pinned      = 0;
    array<u64, 2> pinnersPerC;


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
    void resetMailbox();
    void updateCheckPin();

   public:
    u64 pieces() const;
    u64 pieces(Color c) const;
    u64 pieces(PieceType pt) const;
    u64 pieces(Color c, PieceType pt) const;
    u64 pieces(Color c, PieceType pt1, PieceType pt2) const;

    void reset();

    void loadFromFEN(string fen);

    void display() const;

    PieceType getPiece(int sq) const;

    void move(Move m);

    bool canCastle(Color c, bool kingside) const;

    bool isLegal(Move m);

    bool inCheck() const;
    bool isUnderAttack(Square square) const;
    bool isUnderAttack(Color c, Square square) const;
};