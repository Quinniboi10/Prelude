#pragma once

#include <cassert>

#include "types.h"
#include "config.h"

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

    bool operator==(const Move other) const { return move == other.move; }

    friend std::ostream& operator<<(std::ostream& os, const Move& m) {
        os << m.toString();
        return os;
    }
};

struct MoveEvaluation {
    Move move;
    i16  eval;

    MoveEvaluation(Move move, i16 eval) {
        this->move = move;
        this->eval = eval;
    }
};

struct PvList {
    array<Move, MAX_PLY> moves;
    u32                  length;

    void update(Move move, const PvList& child) {
        moves[0] = move;
        std::copy(child.moves.begin(), child.moves.begin() + child.length, moves.begin() + 1);

        length = child.length + 1;

        assert(length == 1 || moves[0] != moves[1]);
    }

    auto begin() { return moves.begin(); }
    auto end() { return moves.begin() + length; }

    auto& operator=(const PvList& other) {
        std::copy(other.moves.begin(), other.moves.begin() + other.length, moves.begin());
        length = other.length;

        return *this;
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