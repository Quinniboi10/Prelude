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
array<int, 5> PIECE_VALUES = {100, 316, 328, 493, 982};

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

static inline const std::uint16_t Le     = 1;
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