#pragma once

#include <cstdlib>
#include <iostream>
#include <string>
#include <array>

#define ctzll(x) std::countr_zero(x)
#define popcountll(x) std::popcount(x)

#ifdef DEBUG
constexpr bool ISDBG = true;
#else
constexpr bool ISDBG = false;
#endif
#define IFDBG if constexpr (ISDBG)

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8  = uint8_t;

using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;

using usize = size_t;

using std::cerr;
using std::string;
using std::array;
using std::cout;
using std::endl;

#define m_assert(expr, msg) assert(((void) (msg), (expr)))

constexpr u64 INF_U64 = std::numeric_limits<u64>::max();
constexpr int INF_INT = std::numeric_limits<int>::max();
constexpr int INF_I16 = std::numeric_limits<i16>::max();

#include "config.h"

using Accumulator = array<i16, HL_SIZE>;

enum Color : int {
    WHITE = 1,
    BLACK = 0
};

//Inverts the color (WHITE -> BLACK) and (BLACK -> WHITE)
constexpr Color operator~(Color c) { return Color(c ^ 1); }

enum PieceType : int {
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    NO_PIECE_TYPE
};
array<int, 7> PIECE_VALUES = {100, 316, 328, 493, 982, 0, 0};

// clang-format off
enum Square : int {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8,
    NO_SQUARE
};

enum Direction : int {
    NORTH = 8,
    NORTH_EAST = 9,
    EAST = 1,
    SOUTH_EAST = -7,
    SOUTH = -8,
    SOUTH_WEST = -9,
    WEST = -1,
    NORTH_WEST = 7,
    NORTH_NORTH = 16,
    SOUTH_SOUTH = -16
};

enum File : int {
    AFILE, BFILE, CFILE, DFILE, EFILE, FFILE, GFILE, HFILE
};

enum Rank : int {
    RANK1, RANK2, RANK3, RANK4, RANK5, RANK6, RANK7, RANK8
};
Square& operator++(Square& s) { return s = Square(int(s) + 1); }
Square& operator--(Square& s) { return s = Square(int(s) - 1); }
constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
Square& operator+=(Square& s, Direction d) { return s = s + d; }
Square& operator-=(Square& s, Direction d) { return s = s - d; }
//clang-format on

static inline const u16  Le               = 1;
static inline const bool IS_LITTLE_ENDIAN = *reinterpret_cast<const char*>(&Le) == 1;

// Names binary encoding flags from Move class
enum MoveType {
    STANDARD_MOVE = 0, DOUBLE_PUSH = 0b1, CASTLE_K = 0b10, CASTLE_Q = 0b11, CAPTURE = 0b100, EN_PASSANT = 0b101, PROMOTION = 0b1000, KNIGHT_PROMO = 0b1000, BISHOP_PROMO = 0b1001, ROOK_PROMO = 0b1010, QUEEN_PROMO = 0b1011, KNIGHT_PROMO_CAPTURE = 0b1100, BISHOP_PROMO_CAPTURE = 0b1101, ROOK_PROMO_CAPTURE = 0b1110, QUEEN_PROMO_CAPTURE = 0b1111
};

enum flags {
    UNDEFINED, FAIL_LOW, BETA_CUTOFF, EXACT
};

struct Colors {
    // ANSI codes for colors https://raw.githubusercontent.com/fidian/ansi/master/images/color-codes.png
    static constexpr string RESET = "\033[0m";

    // Basic colors
    static constexpr string BLACK = "\033[30m";
    static constexpr string RED = "\033[31m";
    static constexpr string GREEN = "\033[32m";
    static constexpr string YELLOW = "\033[33m";
    static constexpr string BLUE = "\033[34m";
    static constexpr string MAGENTA = "\033[35m";
    static constexpr string CYAN = "\033[36m";
    static constexpr string WHITE = "\033[37m";

    // Bright colors
    static constexpr string BRIGHT_BLACK = "\033[90m";
    static constexpr string BRIGHT_RED = "\033[91m";
    static constexpr string BRIGHT_GREEN = "\033[92m";
    static constexpr string BRIGHT_YELLOW = "\033[93m";
    static constexpr string BRIGHT_BLUE = "\033[94m";
    static constexpr string BRIGHT_MAGENTA = "\033[95m";
    static constexpr string BRIGHT_GYAN = "\033[96m";
    static constexpr string BRIGHT_WHITE = "\033[97m";

    static constexpr string GREY = BRIGHT_BLACK;
};

template<usize size>
class U4array {
    static_assert(size % 2 == 0);

    array<u8, size / 2> data;

public:
    U4array() {
        data.fill(0);
    }

    u8 operator[](usize index) const {
        assert(index < size);
        if (index % 2 == 0) return data[index / 2] & 0b1111;
        return data[index / 2] >> 4;
    }

    void set(usize index, u8 value) {
        assert(value == (value & 0b1111));
        if (index % 2 == 0) {
            data[index / 2] &= 0b11110000;
            data[index / 2] |= value;
        }
        else {
            data[index / 2] &= 0b1111;
            data[index / 2] |= value << 4;
        }
    }
};

class Board;

class Move {
    // Bit indexes of various things
    // See https://www.chessprogramming.org/Encoding_Moves
    // Does not use double pawn push flag
    // PROMO = 15
    // CAPTURE = 14
    // SPECIAL 1 = 13
    // SPECIAL 0 = 12
   private:
    uint16_t move;

   public:
    constexpr Move() { move = 0; }

    Move(string in, Board& board);

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

struct Stack {
    PvList pv;
    i16    staticEval;
    bool   inCheck;
};

struct ThreadInfo {
    array<array<array<int, 64>, 64>, 2> history;

    ThreadInfo() {
        reset();
    }

    void reset() {
        for (auto& side : history) {
            for (auto& from : side) {
                from.fill(DEFAULT_HISTORY_VALUE);
            }
        }
    }

    int getHistoryBonus(Color c, Move m) {
        return history[c][m.from()][m.to()];
    }
};