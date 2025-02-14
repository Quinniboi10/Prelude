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

    constexpr Move(u8 startSquare, u8 endSquare, int flags = STANDARD_MOVE) {
        assert(flags <= 0b1111);
        move = startSquare;
        move |= endSquare << 6;
        move |= flags << 12;
    }

    Move(string strIn, Board& board);


    string toString() const;

    Square from() const { return Square(move & 0b111111); }
    Square to() const { return Square((move >> 6) & 0b111111); }

    MoveType typeOf() const { return MoveType(move >> 12); }  // Return the flag bits

    bool isNull() const { return move == 0; }

    // This should return false if
    // Move is a capture of any kind
    // Move is a queen promotion
    // Move is a knight promotion
    bool isQuiet() const { return (typeOf() & CAPTURE) == 0 && typeOf() != QUEEN_PROMO && typeOf() != KNIGHT_PROMO; }

    // Convert a move into viri-style for datagen
    u16 toViri() const {
        static constexpr array MOVE_TYPES = {
          static_cast<u16>(0x0000),  // None
          static_cast<u16>(0xC000),  // Promo
          static_cast<u16>(0x8000),  // Castling
          static_cast<u16>(0x4000)   // EP
        };

        u16 viriMove = move & 0xFFF;  // Clear flags

        if (typeOf() == EN_PASSANT)
            viriMove |= MOVE_TYPES[3];
        else if (typeOf() == CASTLE_K || typeOf() == CASTLE_Q) {
            viriMove |= MOVE_TYPES[2];

            // Issue here is Viri encoding is FRC style, so king takes rook
            viriMove &= ~(0b111111 << 6);  // Clear the to square bits
            if (typeOf() == CASTLE_K) {
                if (from() == e1)
                    viriMove |= h1 << 6;
                else
                    viriMove |= h8 << 6;
            }
            else {
                if (from() == e1)
                    viriMove |= a1 << 6;
                else
                    viriMove |= a8 << 6;
            }
        }
        else if ((typeOf() & 0b1000) != 0) {
            viriMove |= MOVE_TYPES[1];
            viriMove |= move & (0b11 << 12);
        }

        return viriMove;
    }

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

    void add(u8 from, u8 to, int flags = STANDARD_MOVE) { add(Move(from, to, flags)); }

    auto begin() { return moves.begin(); }
    auto end() { return moves.begin() + length; }
};