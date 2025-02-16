#pragma once

#include <cassert>

#include "types.h"

struct Board;

class Move {
    // See https://www.chessprogramming.org/Encoding_Moves
    // PROMO = 15
    // CAPTURE = 14
    // SPECIAL 1 = 13
    // SPECIAL 0 = 12
   private:
    u16 move;

   public:
    constexpr Move()  = default;
    constexpr ~Move() = default;

    constexpr Move(u8 startSquare, u8 endSquare, MoveType flags = STANDARD_MOVE) {
        move = startSquare | flags;
        move |= endSquare << 6;
        move |= flags << 12;
    }

    constexpr Move(u8 startSquare, u8 endSquare, PieceType promo) {
        move = startSquare | PROMOTION;
        move |= endSquare << 6;
        move |= (promo - 1) << 12;
    }

    Move(string strIn, Board& board);


    string toString() const;

    Square from() const { return Square(move & 0b111111); }
    Square to() const { return Square((move >> 6) & 0b111111); }

    MoveType typeOf() const { return MoveType(move & 0xC000); }  // Return the flag bits

    PieceType promo() const {
        assert(typeOf() == PROMOTION);
        return PieceType(((move >> 12) & 0b11) + 1);
    }

    bool isNull() const { return move == 0; }
    bool isCapture(u64 occ) const { return ((1ULL << to()) & occ) && typeOf() != CASTLE; }

    // This should return false if
    // Move is a capture of any kind
    // Move is a queen promotion
    // Move is a knight promotion
    bool isQuiet(u64 occ) const { return !isCapture(occ) && (typeOf() != PROMOTION || promo() == QUEEN); }

    bool operator==(const Move other) const { return move == other.move; }

    friend std::ostream& operator<<(std::ostream& os, const Move& m) {
        os << m.toString();
        return os;
    }
};

struct MoveList {
    array<Move, 256> moves;
    usize            length;

    constexpr MoveList() { length = 0; }

    void add(Move m) {
        assert(length < 256);
        moves[length++] = m;
    }

    void add(u8 from, u8 to, MoveType flags = STANDARD_MOVE) { add(Move(from, to, flags)); }
    void add(u8 from, u8 to, PieceType promo) { add(Move(from, to, promo)); }

    auto begin() { return moves.begin(); }
    auto end() { return moves.begin() + length; }
};