#pragma once

#include "util.h"
#include "types.h"
#include "move.h"
#include "accumulator.h"

constexpr array<Square, 4> ROOK_CASTLE_END_SQ = {Square(d8), Square(f8), Square(d1), Square(f1)};
constexpr array<Square, 4> KING_CASTLE_END_SQ = {Square(c8), Square(g8), Square(c1), Square(g1)};

struct Board {
    // Index is based on square, returns the piece type
    array<PieceType, 64> mailbox;
    // Indexed pawns, knights, bishops, rooks, queens, king
    array<u64, 6> byPieces;
    // Index is based on color so black is colors[0]
    array<u64, 2> byColor;
    // Board zobrist hash
    u64 zobrist;

    // History of positions
    std::vector<u64> posHistory;

    bool          doubleCheck;
    u64           checkMask;
    u64           pinned;
    array<u64, 2> pinnersPerC;


    Square epSquare;
    // Index KQkq
    array<Square, 4> castling;

    Color stm;

    usize halfMoveClock;
    usize fullMoveClock;

   private:
    bool fromNull;

    char getPieceAt(int i) const;

    void placePiece(Color c, PieceType pt, int sq);
    void removePiece(Color c, PieceType pt, int sq);
    void removePiece(Color c, int sq);
    void resetMailbox();
    void resetZobrist();
    void updateCheckPin();

    void setCastlingRights(Color c, Square sq, bool value);
    void unsetCastlingRights(Color c);

    u64 hashCastling() const;

    template<bool minimal>
    void move(Move m);

   public:
    static void fillZobristTable();

    constexpr Square castleSq(Color c, bool kingside) const { return castling[castleIndex(c, kingside)]; }

    u8 count(PieceType pt) const;

    u64 pieces() const;
    u64 pieces(Color c) const;
    u64 pieces(PieceType pt) const;
    u64 pieces(Color c, PieceType pt) const;
    u64 pieces(PieceType pt1, PieceType pt2) const;
    u64 pieces(Color c, PieceType pt1, PieceType pt2) const;

    u64 attackersTo(Square sq, u64 occ) const;

    u64 roughKeyAfter(const Move m) const;

    void reset();

    void loadFromFEN(string fen);
    string fen() const;

    void display() const;

    PieceType getPiece(int sq) const;
    bool      isCapture(Move m) const;
    bool      isQuiet(Move m) const;

    void move(Move m);
    void move(string str);

    bool canNullMove() const;
    void nullMove();

    bool canCastle(Color c) const;
    bool canCastle(Color c, bool kingside) const;

    bool isLegal(Move m);

    bool inCheck() const;
    bool isUnderAttack(Color c, Square square) const;

    bool isDraw();
    bool isGameOver();

    bool see(Move m, int threshold) const;
};