#pragma once

#include "board.h"

class Move {
    // See https://www.chessprogramming.org/Encoding_Moves
    // PROMO = 15
    // CAPTURE = 14
    // SPECIAL 1 = 13
    // SPECIAL 0 = 12
   private:
    uint16_t move;

   public:
    constexpr Move() { move = 0; }

    constexpr Move(u8 startSquare, u8 endSquare, int flags = STANDARD_MOVE) {
        move = startSquare;
        move |= endSquare << 6;
        move |= flags << 12;
    }

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
};