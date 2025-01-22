/*
    Prelude
    Copyright (C) 2024 Quinniboi10

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/


// TODO (Ordered):
// Switch to new Stockfish IS_LITTLE_ENDIAN code that doesn't involve undefined behavior
// History malus
// Increasing LMR on non pv nodes
// Decrease LMR when a move is giving check
// RFP improving
// LMP improving
// Futility pruning
// 3 fold LMR
// NMP eval reduction
// Split up board class, don't do things like accumulator updates inside move


// ************ CONFIG ************
// ****** SEARCH ******
constexpr int RFP_MARGIN            = 75;
constexpr int NMP_REDUCTION         = 3;      // NMP depth reduction
constexpr int NMP_REDUCTION_DIVISOR = 3;      // Subtract depth/n from NMP depth
constexpr int MAX_HISTORY           = 16384;  // Max history bonus

constexpr int MATE_SCORE = 99999;
constexpr int MAX_PLY    = 255;

constexpr int MATE_IN_MAX_PLY  = MATE_SCORE - MAX_PLY;
constexpr int MATED_IN_MAX_PLY = -MATE_SCORE + MAX_PLY;

constexpr int MIN_MOVES_BEFORE_LMP = 7;  // Only does LMP if moves search >= n * depth * depth
constexpr int MAX_DEPTH_FOR_LMP    = 5;  // Only does late move pruning if depth <= n

constexpr int RAZOR_MARGIN       = 475;  // Margin to use for razoring
constexpr int RAZOR_DEPTH_SCALAR = 300;  // Depth scalar to use for razoring

constexpr int PVS_SEE_QUIET_SCALAR   = -80;
constexpr int PVS_SEE_CAPTURE_SCALAR = -30;

constexpr int SEE_MARGIN = -108;  // Margin for qsearch SEE pruning

constexpr int    INC_SCALAR       = 1;     // Amount of increment to add to hard time limit
constexpr double SOFT_TIME_SCALAR = 0.65;  // Scales the soft limit as hardLimit * n

constexpr int    ASPR_DELTA           = 25;    // Used as delta size in aspiration window
constexpr double ASP_DELTA_MULTIPLIER = 1.25;  // Scalar to widen aspr window on fail


// ****** DATA GEN ******
constexpr int targetPositions     = 1'000'000'000;  // Number of positions to generate
constexpr int datagenInfoInterval = 1;  // How often (in games) to send progress report to console
constexpr int saveEveryN = 1;  // Save every n positions or as soon as position passes filtering
constexpr int clearBufferEvery = 1'000;    // Push output buffer to file every n data points
constexpr int randMoves        = 8;        // Number of random halfmoves before data gen begins
constexpr int nodesPerMove     = 5000;     // Soft nodes per move
constexpr int maxNodesPerMove  = 100'000;  // Hard nodes per move


#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <deque>
#include <chrono>
#include <array>
#include <bitset>
#include <algorithm>
#include <cassert>
#include <optional>
#include <thread>
#include <memory>
#include <random>
#include <cstring>
#include <cstdlib>
#include <immintrin.h>

#ifndef EVALFILE
    #define EVALFILE "./nnue.bin"
#endif

#ifdef _MSC_VER
    #define MSVC
    #pragma push_macro("_MSC_VER")
    #undef _MSC_VER
#endif

#include "./external/incbin.h"

#ifdef MSVC
    #pragma pop_macro("_MSC_VER")
    #undef MSVC
#endif

#if !defined(_MSC_VER) || defined(__clang__)
INCBIN(EVAL, EVALFILE);
#endif

int getLine(int line) { return line; }

#define exit(code) \
    cout << "Exit from line " << getLine(__LINE__) << endl; \
    exit(code);

#define ctzll(x) std::countr_zero(x)
#define popcountll(x) std::popcount(x)

#ifdef DEBUG
constexpr bool ISDBG = true;
#else
constexpr bool ISDBG = false;
#endif
#define IFDBG if constexpr (ISDBG)

using std::cerr;
using std::string;
using std::array;
using std::cout;
using std::endl;

#define m_assert(expr, msg) assert(((void) (msg), (expr)))

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8  = uint8_t;

using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;

constexpr u64 INF     = std::numeric_limits<uint64_t>::max();
constexpr int INF_INT = std::numeric_limits<int>::max();


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

static inline const union {
    uint32_t i;
    char     c[4];
} Le = { 0x01020304 };
static inline const bool IS_LITTLE_ENDIAN = (Le.c[0] == 4);

// Names binary encoding flags from Move class
enum MoveType {
    STANDARD_MOVE = 0, DOUBLE_PUSH = 0b1, CASTLE_K = 0b10, CASTLE_Q = 0b11, CAPTURE = 0b100, EN_PASSANT = 0b101, PROMOTION = 0b1000, KNIGHT_PROMO = 0b1000, BISHOP_PROMO = 0b1001, ROOK_PROMO = 0b1010, QUEEN_PROMO = 0b1011, KNIGHT_PROMO_CAPTURE = 0b1100, BISHOP_PROMO_CAPTURE = 0b1101, ROOK_PROMO_CAPTURE = 0b1110, QUEEN_PROMO_CAPTURE = 0b1111
};

enum flags {
    UNDEFINED, FAIL_LOW, BETA_CUTOFF, EXACT
};


struct Colors {
    // ANSI codes for colors https://raw.githubusercontent.com/fidian/ansi/master/images/color-codes.png
    constexpr static string RESET = "\033[0m";

    // Basic colors
    constexpr static string BLACK = "\033[30m";
    constexpr static string RED = "\033[31m";
    constexpr static string GREEN = "\033[32m";
    constexpr static string YELLOW = "\033[33m";
    constexpr static string BLUE = "\033[34m";
    constexpr static string MAGENTA = "\033[35m";
    constexpr static string CYAN = "\033[36m";
    constexpr static string WHITE = "\033[37m";

    // Bright colors
    constexpr static string BRIGHT_BLACK = "\033[90m";
    constexpr static string BRIGHT_RED = "\033[91m";
    constexpr static string BRIGHT_GREEN = "\033[92m";
    constexpr static string BRIGHT_YELLOW = "\033[93m";
    constexpr static string BRIGHT_BLUE = "\033[94m";
    constexpr static string BRIGHT_MAGENTA = "\033[95m";
    constexpr static string BRIGHT_GYAN = "\033[96m";
    constexpr static string BRIGHT_WHITE = "\033[97m";

    constexpr static string GREY = BRIGHT_BLACK;
};

// Takes square (h8) and converts it into a bitboard index (64)
static int parseSquare(const string square) {
    return (square.at(1) - '1') * 8 + (square.at(0) - 'a');
}

// Takes a square (64) and converts into algebraic notation (h8)
static string squareToAlgebraic(int sq) {
    return string(1, 'a' + (sq % 8)) + string(1, '1' + (sq / 8));
};

// Returns true if the given index is 1
template <typename BitboardType>
static bool readBit(BitboardType bitboard, int index) {
    return (bitboard & (1ULL << index)) != 0;
}

// Set the bit in the given bitboard to 1 or 0
template <typename BitboardType>
static void setBit(BitboardType& bitboard, int index, bool value) {
    if (value) bitboard |= (1ULL << index);
    else bitboard &= ~(1ULL << index);
}

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
    constexpr Move() {
        move = 0;
    }

    Move(string in, Board& board);

    constexpr Move(u8 startSquare, u8 endSquare, int flags = STANDARD_MOVE) {
        move = startSquare;
        move |= endSquare << 6;
        move |= flags << 12;
    }

    string toString();

    Square startSquare() { return Square(move & 0b111111); }
    Square endSquare() { return Square((move >> 6) & 0b111111); }

    MoveType typeOf() { return MoveType(move >> 12); } // Return the flag bits

    bool isNull() { return move == 0; }

    // This should return false if
    // Move is a capture of any kind
    // Move is a queen promotion
    // Move is a knight promotion
    bool isQuiet() { return (typeOf() & CAPTURE) == 0 && 
        typeOf() != QUEEN_PROMO &&
        typeOf() != KNIGHT_PROMO
        ; }

    bool operator==(const Move other) const {
        return move == other.move;
    }
};

// Split a string into a deque given a deliminer to split by
std::deque<string> split(const string& s, char delim) {
    std::deque<string> result;
    std::stringstream ss(s);
    string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }
    return result;
}

// Find how much to pad a string with a minimum padding of 2 spaces
int getPadding(string str, int targetLen) {
    targetLen -= str.length();
    return std::max(targetLen, 2);
}

// Pads a string to a given length
template<typename objType>
string padStr(objType obj, int targetLen) {
    std::string objStr;
    if constexpr (std::is_same_v<objType, std::string> || std::is_same_v<objType, std::basic_string<char>>) {
        objStr = obj; // Strings make everything explode if you call to_string
    }
    else {
        objStr = std::to_string(obj); // Convert other types
    }
    int padding = getPadding(objStr, targetLen);
    for (int i = 0; i < padding; i++) {
        objStr += " ";
    }
    return objStr;
}

// Find the index of the string in the given array (used for UCI input parsing)
template<typename arrType>
int findIndexOf(const arrType arr, string entry) {
    auto it = std::find(arr.begin(), arr.end(), entry);
    if (it != arr.end()) {
        return std::distance(arr.begin(), it); // Calculate the index
    }
    return -1; // Not found
}

// Print a bitboard (for debugging individual bitboards)
void printBitboard(u64 bitboard) {
    for (int rank = 7; rank >= 0; --rank) {
        cout << "+---+---+---+---+---+---+---+---+" << endl;
        for (int file = 0; file < 8; ++file) {
            int i = rank * 8 + file;  // Map rank and file to bitboard index
            char currentPiece = readBit(bitboard, i) ? '1' : ' ';

            cout << "| " << currentPiece << " ";
        }
        cout << "|" << endl;
    }
    cout << "+---+---+---+---+---+---+---+---+" << endl;
}

// Fancy formats a time
string formatTime(u64 timeInMS) {
    long long seconds = timeInMS / 1000;
    long long hours = seconds / 3600;
    seconds %= 3600;
    long long minutes = seconds / 60;
    seconds %= 60;

    string result;

    if (hours > 0) result += std::to_string(hours) + "h ";
    if (minutes > 0 || hours > 0) result += std::to_string(minutes) + "m ";
    if (seconds > 0 || minutes > 0 || hours > 0) result += std::to_string(seconds) + "s";
    if (result == "") return std::to_string(timeInMS) + "ms";
    return result;
}

// Formats a number with commas
string formatNum(i64 v) {
    auto s = std::to_string(v);

    int n = s.length() - 3;
    if (v < 0) n--;
    while (n > 0) {
        s.insert(n, ",");
        n -= 3;
    }

    return s;
}

// Abbreviates a number into a string (1.00 gnodes instead of 1,000,000,000 nodes)
string abbreviateNum(const i64 v) {
    if (v > 1000000000) return std::format("{:.2f} g", v / 1000000000.0);
    if (v > 1000000) return std::format("{:.2f} m", v / 1000000.0);
    if (v > 1000) return std::format("{:.2f} k", v / 1000.0);

    return std::to_string(v) + " ";
}

// Class with precomputed constants
class Precomputed {
public:
    static array<array<array<u64, 64>, 12>, 2> zobrist;
    // EP zobrist is 65 because ctzll of 0 returns 64
    static array<u64, 65> zobristEP;
    static array<u64, 16> zobristCastling;
    static array<u64, 2> zobristSide;
    static u64 isOnA;
    static u64 isOnB;
    static u64 isOnC;
    static u64 isOnD;
    static u64 isOnE;
    static u64 isOnF;
    static u64 isOnG;
    static u64 isOnH;
    static u64 isOn1;
    static u64 isOn2;
    static u64 isOn3;
    static u64 isOn4;
    static u64 isOn5;
    static u64 isOn6;
    static u64 isOn7;
    static u64 isOn8;
    static void compute() {
        // *** FILE AND COL ARRAYS ***
        isOnA = 0;
        isOnB = 0;
        isOnC = 0;
        isOnD = 0;
        isOnE = 0;
        isOnF = 0;
        isOnG = 0;
        isOnH = 0;
        isOn1 = 0;
        isOn2 = 0;
        isOn3 = 0;
        isOn4 = 0;
        isOn5 = 0;
        isOn6 = 0;
        isOn7 = 0;
        isOn8 = 0;

        for (int i = 0; i < 64; ++i) {
            int file = i % 8; // File index (0 = A, 1 = B, ..., 7 = H)
            if (file == 0) isOnA |= 1ULL << i;
            if (file == 1) isOnB |= 1ULL << i;
            if (file == 2) isOnC |= 1ULL << i;
            if (file == 3) isOnD |= 1ULL << i;
            if (file == 4) isOnE |= 1ULL << i;
            if (file == 5) isOnF |= 1ULL << i;
            if (file == 6) isOnG |= 1ULL << i;
            if (file == 7) isOnH |= 1ULL << i;

            // Fill ranks (1-8)
            int rank = i / 8; // Rank index (0 = 1, 1 = 2, ..., 7 = 8)
            if (rank == 0) isOn1 |= 1ULL << i;
            if (rank == 1) isOn2 |= 1ULL << i;
            if (rank == 2) isOn3 |= 1ULL << i;
            if (rank == 3) isOn4 |= 1ULL << i;
            if (rank == 4) isOn5 |= 1ULL << i;
            if (rank == 5) isOn6 |= 1ULL << i;
            if (rank == 6) isOn7 |= 1ULL << i;
            if (rank == 7) isOn8 |= 1ULL << i;
        }

        // *** MAKE RANDOM ZOBRIST TABLE ****
        std::random_device rd;

        std::mt19937_64 engine(rd());

        engine.seed(69420); // Nice

        std::uniform_int_distribution<u64> dist(0, INF);

        for (auto& side : zobrist) {
            for (auto& pieceTable : side) {
                for (auto& square : pieceTable) {
                    square = dist(engine);
                }
            }
        }

        for (auto& square : zobristEP) {
            square = dist(engine);
        }

        // If no EP square, set it to 0
        zobristEP[64] = 0;

        for (auto& castlingValue : zobristCastling) {
            castlingValue = dist(engine);
        }

        for (auto& sideValue : zobristSide) {
            sideValue = dist(engine);
        }
    }
};

array<array<array<u64, 64>, 12>, 2> Precomputed::zobrist;
array<u64, 65> Precomputed::zobristEP;
array<u64, 16> Precomputed::zobristCastling;
array<u64, 2> Precomputed::zobristSide;
u64 Precomputed::isOnA;
u64 Precomputed::isOnB;
u64 Precomputed::isOnC;
u64 Precomputed::isOnD;
u64 Precomputed::isOnE;
u64 Precomputed::isOnF;
u64 Precomputed::isOnG;
u64 Precomputed::isOnH;
u64 Precomputed::isOn1;
u64 Precomputed::isOn2;
u64 Precomputed::isOn3;
u64 Precomputed::isOn4;
u64 Precomputed::isOn5;
u64 Precomputed::isOn6;
u64 Precomputed::isOn7;
u64 Precomputed::isOn8;

// Returns the rank or file of the given square
constexpr Rank rankOf(Square s) { return Rank(s >> 3); }
constexpr File fileOf(Square s) { return File(s & 0b111); }

constexpr Rank flipRank(Square s) { return Rank(s ^ 0b111000); }

// Shift the bitboard in the given diection
template<Direction shiftDir>
u64 shift(u64 bb) {
    if constexpr (shiftDir < 0) {
        return bb >> -shiftDir;
    }
    return bb << shiftDir;
}

// Returns the pawn attack of a piece on the given square
template<Color c>
u64 pawnAttacksBB(const int sq) {
    const u64 pawnBB = 1ULL << sq;
    if constexpr (c == WHITE) {
        if (pawnBB & Precomputed::isOnA) return shift<NORTH_EAST>(pawnBB);
        if (pawnBB & Precomputed::isOnH) return shift<NORTH_WEST>(pawnBB);
        return shift<NORTH_EAST>(pawnBB) | shift<NORTH_WEST>(pawnBB);
    }
    else {
        if (pawnBB & Precomputed::isOnA) return shift<SOUTH_EAST>(pawnBB);
        if (pawnBB & Precomputed::isOnH) return shift<SOUTH_WEST>(pawnBB);
        return shift<SOUTH_EAST>(pawnBB) | shift<SOUTH_WEST>(pawnBB);
    }
}

// Tables from https://github.com/Disservin/chess-library/blob/cf3bd56474168605201a01eb78b3222b8f9e65e4/include/chess.hpp#L780
static constexpr u64 KNIGHT_ATTACKS[64] = {
    0x0000000000020400, 0x0000000000050800, 0x00000000000A1100, 0x0000000000142200, 0x0000000000284400,
    0x0000000000508800, 0x0000000000A01000, 0x0000000000402000, 0x0000000002040004, 0x0000000005080008,
    0x000000000A110011, 0x0000000014220022, 0x0000000028440044, 0x0000000050880088, 0x00000000A0100010,
    0x0000000040200020, 0x0000000204000402, 0x0000000508000805, 0x0000000A1100110A, 0x0000001422002214,
    0x0000002844004428, 0x0000005088008850, 0x000000A0100010A0, 0x0000004020002040, 0x0000020400040200,
    0x0000050800080500, 0x00000A1100110A00, 0x0000142200221400, 0x0000284400442800, 0x0000508800885000,
    0x0000A0100010A000, 0x0000402000204000, 0x0002040004020000, 0x0005080008050000, 0x000A1100110A0000,
    0x0014220022140000, 0x0028440044280000, 0x0050880088500000, 0x00A0100010A00000, 0x0040200020400000,
    0x0204000402000000, 0x0508000805000000, 0x0A1100110A000000, 0x1422002214000000, 0x2844004428000000,
    0x5088008850000000, 0xA0100010A0000000, 0x4020002040000000, 0x0400040200000000, 0x0800080500000000,
    0x1100110A00000000, 0x2200221400000000, 0x4400442800000000, 0x8800885000000000, 0x100010A000000000,
    0x2000204000000000, 0x0004020000000000, 0x0008050000000000, 0x00110A0000000000, 0x0022140000000000,
    0x0044280000000000, 0x0088500000000000, 0x0010A00000000000, 0x0020400000000000 };

static constexpr u64 KING_ATTACKS[64] = {
    0x0000000000000302, 0x0000000000000705, 0x0000000000000E0A, 0x0000000000001C14, 0x0000000000003828,
    0x0000000000007050, 0x000000000000E0A0, 0x000000000000C040, 0x0000000000030203, 0x0000000000070507,
    0x00000000000E0A0E, 0x00000000001C141C, 0x0000000000382838, 0x0000000000705070, 0x0000000000E0A0E0,
    0x0000000000C040C0, 0x0000000003020300, 0x0000000007050700, 0x000000000E0A0E00, 0x000000001C141C00,
    0x0000000038283800, 0x0000000070507000, 0x00000000E0A0E000, 0x00000000C040C000, 0x0000000302030000,
    0x0000000705070000, 0x0000000E0A0E0000, 0x0000001C141C0000, 0x0000003828380000, 0x0000007050700000,
    0x000000E0A0E00000, 0x000000C040C00000, 0x0000030203000000, 0x0000070507000000, 0x00000E0A0E000000,
    0x00001C141C000000, 0x0000382838000000, 0x0000705070000000, 0x0000E0A0E0000000, 0x0000C040C0000000,
    0x0003020300000000, 0x0007050700000000, 0x000E0A0E00000000, 0x001C141C00000000, 0x0038283800000000,
    0x0070507000000000, 0x00E0A0E000000000, 0x00C040C000000000, 0x0302030000000000, 0x0705070000000000,
    0x0E0A0E0000000000, 0x1C141C0000000000, 0x3828380000000000, 0x7050700000000000, 0xE0A0E00000000000,
    0xC040C00000000000, 0x0203000000000000, 0x0507000000000000, 0x0A0E000000000000, 0x141C000000000000,
    0x2838000000000000, 0x5070000000000000, 0xA0E0000000000000, 0x40C0000000000000 };


// Magic code from https://github.com/nkarve/surge/blob/master/src/tables.cpp

constexpr int diagonalOf(Square s) { return 7 + rankOf(s) - fileOf(s); }
constexpr int antiDiagonalOf(Square s) { return rankOf(s) + fileOf(s); }

//Precomputed file masks
const u64 MASK_FILE[8] = {
    0x101010101010101, 0x202020202020202, 0x404040404040404, 0x808080808080808,
    0x1010101010101010, 0x2020202020202020, 0x4040404040404040, 0x8080808080808080,
};

//Precomputed rank masks
const u64 MASK_RANK[8] = {
    0xff, 0xff00, 0xff0000, 0xff000000,
    0xff00000000, 0xff0000000000, 0xff000000000000, 0xff00000000000000
};

//Precomputed diagonal masks
const u64 MASK_DIAGONAL[15] = {
    0x80, 0x8040, 0x804020,
    0x80402010, 0x8040201008, 0x804020100804,
    0x80402010080402, 0x8040201008040201, 0x4020100804020100,
    0x2010080402010000, 0x1008040201000000, 0x804020100000000,
    0x402010000000000, 0x201000000000000, 0x100000000000000,
};

//Precomputed anti-diagonal masks
const u64 MASK_ANTI_DIAGONAL[15] = {
    0x1, 0x102, 0x10204,
    0x1020408, 0x102040810, 0x10204081020,
    0x1020408102040, 0x102040810204080, 0x204081020408000,
    0x408102040800000, 0x810204080000000, 0x1020408000000000,
    0x2040800000000000, 0x4080000000000000, 0x8000000000000000,
};

//Precomputed square masks
const u64 SQUARE_BB[65] = {
    0x1, 0x2, 0x4, 0x8,
    0x10, 0x20, 0x40, 0x80,
    0x100, 0x200, 0x400, 0x800,
    0x1000, 0x2000, 0x4000, 0x8000,
    0x10000, 0x20000, 0x40000, 0x80000,
    0x100000, 0x200000, 0x400000, 0x800000,
    0x1000000, 0x2000000, 0x4000000, 0x8000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000,
    0x100000000, 0x200000000, 0x400000000, 0x800000000,
    0x1000000000, 0x2000000000, 0x4000000000, 0x8000000000,
    0x10000000000, 0x20000000000, 0x40000000000, 0x80000000000,
    0x100000000000, 0x200000000000, 0x400000000000, 0x800000000000,
    0x1000000000000, 0x2000000000000, 0x4000000000000, 0x8000000000000,
    0x10000000000000, 0x20000000000000, 0x40000000000000, 0x80000000000000,
    0x100000000000000, 0x200000000000000, 0x400000000000000, 0x800000000000000,
    0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000,
    0x0
};

//Reverses a bitboard                        
u64 reverse(u64 b) {
    b = (b & 0x5555555555555555) << 1 | (b >> 1) & 0x5555555555555555;
    b = (b & 0x3333333333333333) << 2 | (b >> 2) & 0x3333333333333333;
    b = (b & 0x0f0f0f0f0f0f0f0f) << 4 | (b >> 4) & 0x0f0f0f0f0f0f0f0f;
    b = (b & 0x00ff00ff00ff00ff) << 8 | (b >> 8) & 0x00ff00ff00ff00ff;

    return (b << 48) | ((b & 0xffff0000) << 16) |
        ((b >> 16) & 0xffff0000) | (b >> 48);
}

//Calculates sliding attacks from a given square, on a given axis, taking into
//account the blocking pieces. This uses the Hyperbola Quintessence Algorithm.
u64 sliding_attacks(Square square, u64 occ, u64 mask) {
    return (((mask & occ) - SQUARE_BB[square] * 2) ^
        reverse(reverse(mask & occ) - reverse(SQUARE_BB[square]) * 2)) & mask;
}

//Returns rook attacks from a given square, using the Hyperbola Quintessence Algorithm. Only used to initialize
//the magic lookup table
u64 get_rook_attacks_for_init(Square square, u64 occ) {
    return sliding_attacks(square, occ, MASK_FILE[fileOf(square)]) |
        sliding_attacks(square, occ, MASK_RANK[rankOf(square)]);
}

u64 ROOK_ATTACK_MASKS[64];
int ROOK_ATTACK_SHIFTS[64];
u64 ROOK_ATTACKS[64][4096];

const u64 ROOK_MAGICS[64] = {
    0x0080001020400080, 0x0040001000200040, 0x0080081000200080, 0x0080040800100080,
    0x0080020400080080, 0x0080010200040080, 0x0080008001000200, 0x0080002040800100,
    0x0000800020400080, 0x0000400020005000, 0x0000801000200080, 0x0000800800100080,
    0x0000800400080080, 0x0000800200040080, 0x0000800100020080, 0x0000800040800100,
    0x0000208000400080, 0x0000404000201000, 0x0000808010002000, 0x0000808008001000,
    0x0000808004000800, 0x0000808002000400, 0x0000010100020004, 0x0000020000408104,
    0x0000208080004000, 0x0000200040005000, 0x0000100080200080, 0x0000080080100080,
    0x0000040080080080, 0x0000020080040080, 0x0000010080800200, 0x0000800080004100,
    0x0000204000800080, 0x0000200040401000, 0x0000100080802000, 0x0000080080801000,
    0x0000040080800800, 0x0000020080800400, 0x0000020001010004, 0x0000800040800100,
    0x0000204000808000, 0x0000200040008080, 0x0000100020008080, 0x0000080010008080,
    0x0000040008008080, 0x0000020004008080, 0x0000010002008080, 0x0000004081020004,
    0x0000204000800080, 0x0000200040008080, 0x0000100020008080, 0x0000080010008080,
    0x0000040008008080, 0x0000020004008080, 0x0000800100020080, 0x0000800041000080,
    0x00FFFCDDFCED714A, 0x007FFCDDFCED714A, 0x003FFFCDFFD88096, 0x0000040810002101,
    0x0001000204080011, 0x0001000204000801, 0x0001000082000401, 0x0001FFFAABFAD1A2
};

//Initializes the magic lookup table for rooks
void initializeRookAttacks() {
    u64 edges, subset, index;

    for (Square sq = a1; sq <= h8; ++sq) {
        edges = ((MASK_RANK[AFILE] | MASK_RANK[HFILE]) & ~MASK_RANK[rankOf(sq)]) |
            ((MASK_FILE[AFILE] | MASK_FILE[HFILE]) & ~MASK_FILE[fileOf(sq)]);
        ROOK_ATTACK_MASKS[sq] = (MASK_RANK[rankOf(sq)]
            ^ MASK_FILE[fileOf(sq)]) & ~edges;
        ROOK_ATTACK_SHIFTS[sq] = 64 - popcountll(ROOK_ATTACK_MASKS[sq]);

        subset = 0;
        do {
            index = subset;
            index = index * ROOK_MAGICS[sq];
            index = index >> ROOK_ATTACK_SHIFTS[sq];
            ROOK_ATTACKS[sq][index] = get_rook_attacks_for_init(sq, subset);
            subset = (subset - ROOK_ATTACK_MASKS[sq]) & ROOK_ATTACK_MASKS[sq];
        } while (subset);
    }
}

//Returns the attacks bitboard for a rook at a given square, using the magic lookup table
constexpr u64 getRookAttacks(Square square, u64 occ) {
    return ROOK_ATTACKS[square][((occ & ROOK_ATTACK_MASKS[square]) * ROOK_MAGICS[square])
        >> ROOK_ATTACK_SHIFTS[square]];
}

//Returns the 'x-ray attacks' for a rook at a given square. X-ray attacks cover squares that are not immediately
//accessible by the rook, but become available when the immediate blockers are removed from the board 
u64 getXrayRookAttacks(Square square, u64 occ, u64 blockers) {
    u64 attacks = getRookAttacks(square, occ);
    blockers &= attacks;
    return attacks ^ getRookAttacks(square, occ ^ blockers);
}

//Returns bishop attacks from a given square, using the Hyperbola Quintessence Algorithm. Only used to initialize
//the magic lookup table
u64 getBishopAttacksForInit(Square square, u64 occ) {
    return sliding_attacks(square, occ, MASK_DIAGONAL[diagonalOf(square)]) |
        sliding_attacks(square, occ, MASK_ANTI_DIAGONAL[antiDiagonalOf(square)]);
}

u64 BISHOP_ATTACK_MASKS[64];
int BISHOP_ATTACK_SHIFTS[64];
u64 BISHOP_ATTACKS[64][512];

const u64 BISHOP_MAGICS[64] = {
    0x0002020202020200, 0x0002020202020000, 0x0004010202000000, 0x0004040080000000,
    0x0001104000000000, 0x0000821040000000, 0x0000410410400000, 0x0000104104104000,
    0x0000040404040400, 0x0000020202020200, 0x0000040102020000, 0x0000040400800000,
    0x0000011040000000, 0x0000008210400000, 0x0000004104104000, 0x0000002082082000,
    0x0004000808080800, 0x0002000404040400, 0x0001000202020200, 0x0000800802004000,
    0x0000800400A00000, 0x0000200100884000, 0x0000400082082000, 0x0000200041041000,
    0x0002080010101000, 0x0001040008080800, 0x0000208004010400, 0x0000404004010200,
    0x0000840000802000, 0x0000404002011000, 0x0000808001041000, 0x0000404000820800,
    0x0001041000202000, 0x0000820800101000, 0x0000104400080800, 0x0000020080080080,
    0x0000404040040100, 0x0000808100020100, 0x0001010100020800, 0x0000808080010400,
    0x0000820820004000, 0x0000410410002000, 0x0000082088001000, 0x0000002011000800,
    0x0000080100400400, 0x0001010101000200, 0x0002020202000400, 0x0001010101000200,
    0x0000410410400000, 0x0000208208200000, 0x0000002084100000, 0x0000000020880000,
    0x0000001002020000, 0x0000040408020000, 0x0004040404040000, 0x0002020202020000,
    0x0000104104104000, 0x0000002082082000, 0x0000000020841000, 0x0000000000208800,
    0x0000000010020200, 0x0000000404080200, 0x0000040404040400, 0x0002020202020200
};

//Initializes the magic lookup table for bishops
void initializeBishopAttacks() {
    u64 edges, subset, index;

    for (Square sq = a1; sq <= h8; ++sq) {
        edges = ((MASK_RANK[AFILE] | MASK_RANK[HFILE]) & ~MASK_RANK[rankOf(sq)]) |
            ((MASK_FILE[AFILE] | MASK_FILE[HFILE]) & ~MASK_FILE[fileOf(sq)]);
        BISHOP_ATTACK_MASKS[sq] = (MASK_DIAGONAL[diagonalOf(sq)]
            ^ MASK_ANTI_DIAGONAL[antiDiagonalOf(sq)]) & ~edges;
        BISHOP_ATTACK_SHIFTS[sq] = 64 - popcountll(BISHOP_ATTACK_MASKS[sq]);

        subset = 0;
        do {
            index = subset;
            index = index * BISHOP_MAGICS[sq];
            index = index >> BISHOP_ATTACK_SHIFTS[sq];
            BISHOP_ATTACKS[sq][index] = getBishopAttacksForInit(sq, subset);
            subset = (subset - BISHOP_ATTACK_MASKS[sq]) & BISHOP_ATTACK_MASKS[sq];
        } while (subset);
    }
}

//Returns the attacks bitboard for a bishop at a given square, using the magic lookup table
constexpr u64 getBishopAttacks(Square square, u64 occ) {
    return BISHOP_ATTACKS[square][((occ & BISHOP_ATTACK_MASKS[square]) * BISHOP_MAGICS[square])
        >> BISHOP_ATTACK_SHIFTS[square]];
}

//Returns the 'x-ray attacks' for a bishop at a given square. X-ray attacks cover squares that are not immediately
//accessible by the rook, but become available when the immediate blockers are removed from the board 
u64 getXrayBishopAttacks(Square square, u64 occ, u64 blockers) {
    u64 attacks = getBishopAttacks(square, occ);
    blockers &= attacks;
    return attacks ^ getBishopAttacks(square, occ ^ blockers);
}

u64 SQUARES_BETWEEN_BB[64][64];

//Initializes the lookup table for the bitboard of squares in between two given squares (0 if the 
//two squares are not aligned)
void initializeSquaresBetween() {
    u64 sqs;
    for (Square sq1 = a1; sq1 <= h8; ++sq1)
        for (Square sq2 = a1; sq2 <= h8; ++sq2) {
            sqs = SQUARE_BB[sq1] | SQUARE_BB[sq2];
            if (fileOf(sq1) == fileOf(sq2) || rankOf(sq1) == rankOf(sq2))
                SQUARES_BETWEEN_BB[sq1][sq2] =
                get_rook_attacks_for_init(sq1, sqs) & get_rook_attacks_for_init(sq2, sqs);
            else if (diagonalOf(sq1) == diagonalOf(sq2) || antiDiagonalOf(sq1) == antiDiagonalOf(sq2))
                SQUARES_BETWEEN_BB[sq1][sq2] =
                getBishopAttacksForInit(sq1, sqs) & getBishopAttacksForInit(sq2, sqs);
        }
}


u64 LINE[64][64];
u64 LINESEG[64][64]; // ADDITION, SEGMENTS OF A LINE BETWEEN SQUARES GIVEN, FOR FINDING PINNED PIECES.

//Initializes the lookup table for the bitboard of all squares along the line of two given squares (0 if the 
//two squares are not aligned)
void initializeLine() {
    for (Square sq1 = a1; sq1 <= h8; ++sq1) {
        for (Square sq2 = a1; sq2 <= h8; ++sq2) {
            if (fileOf(sq1) == fileOf(sq2) || rankOf(sq1) == rankOf(sq2))
                LINE[sq1][sq2] =
                get_rook_attacks_for_init(sq1, 0) & get_rook_attacks_for_init(sq2, 0)
                | SQUARE_BB[sq1] | SQUARE_BB[sq2];
            else if (diagonalOf(sq1) == diagonalOf(sq2) || antiDiagonalOf(sq1) == antiDiagonalOf(sq2))
                LINE[sq1][sq2] =
                getBishopAttacksForInit(sq1, 0) & getBishopAttacksForInit(sq2, 0)
                | SQUARE_BB[sq1] | SQUARE_BB[sq2];
        }
    }

    for (Square sq1 = a1; sq1 <= h8; ++sq1) {
        for (Square sq2 = a1; sq2 <= h8; ++sq2) {
            u64 blockers = (1ULL << sq1) | (1ULL << sq2);
            if (fileOf(sq1) == fileOf(sq2) || rankOf(sq1) == rankOf(sq2))
                LINESEG[sq1][sq2] =
                get_rook_attacks_for_init(sq1, blockers) & get_rook_attacks_for_init(sq2, blockers)
                | SQUARE_BB[sq1] | SQUARE_BB[sq2];
            else if (diagonalOf(sq1) == diagonalOf(sq2) || antiDiagonalOf(sq1) == antiDiagonalOf(sq2))
                LINESEG[sq1][sq2] =
                getBishopAttacksForInit(sq1, blockers) & getBishopAttacksForInit(sq2, blockers)
                | SQUARE_BB[sq1] | SQUARE_BB[sq2];
        }
    }
}

//Initializes lookup tables for rook moves, bishop moves, in-between squares, aligned squares and pseudolegal moves
void initializeAllDatabases() {
    initializeRookAttacks();
    initializeBishopAttacks();
    initializeSquaresBetween();
    initializeLine();
}

// Back to my code from here and below
// Stores a move with an evaluation
struct MoveEvaluation {
    Move move;
    int eval;

    constexpr MoveEvaluation() {
        move = Move();
        eval = -INF_INT;
    }
    constexpr MoveEvaluation(Move m, int eval) {
        move = m;
        this->eval = eval;
    }

    constexpr auto operator<=>(const MoveEvaluation& other) const { return eval <=> other.eval; }
};

struct MoveList {
    array<Move, 218> moves;
    int count;

    constexpr MoveList() {
        count = 0;
    }

    auto begin() {
        return moves.begin();
    }

    auto end() {
        return moves.begin() + count;
    }

    void add(Move m) {
        moves[count++] = m;
    }

    Move get(int index) {
        return moves[index];
    }

    int find(const Move entry) {
        auto it = std::find(moves.begin(), moves.begin() + count, entry);
        if (it != moves.begin() + count) {
            return std::distance(moves.begin(), it);
        }
        return -1;
    }

    void sortByString(Board& board) {
        array<string, 218> movesStr;
        movesStr.fill("zzzz"); // Fill with values to be sorted to back.
        for (int i = 0; i < count; ++i) {
            movesStr[i] = moves[i].toString();
        }
        std::stable_sort(movesStr.begin(), movesStr.end());

        for (auto& str : movesStr) {
            if (str == "zzzz") {
                str = "a1a1";
            }
        }
        for (int i = 0; i < count; ++i) {
            moves[i] = Move(movesStr[i], board);
        }
    }
};

struct Transposition {
    u64 zobristKey;
    Move bestMove;
    i16 score;
    u8 flag;
    u8 depth;

    Transposition() {
        zobristKey = 0;
        bestMove = Move();
        flag = 0;
        score = 0;
        depth = 0;
    }
    Transposition(u64 zobristKey, Move bestMove, u8 flag, i16 score, u8 depth) {
        this->zobristKey = zobristKey;
        this->bestMove = bestMove;
        this->flag = flag;
        this->score = score;
        this->depth = depth;
    }
};

struct TranspositionTable {
private:
    std::vector<Transposition> table;
public:
    size_t size;

    TranspositionTable(float sizeInMB = 16) {
        resize(sizeInMB);
    }

    void clear() {
        table.clear();
    }

    void resize(float newSizeMiB) {
        // Find number of bytes allowed
        size = newSizeMiB * 1024 * 1024;
        // Divide by size of transposition entry
        size /= sizeof(Transposition);
        if (size == 0) size += 1;
        IFDBG cout << "Using transposition table with " << formatNum(size) << " entries" << endl;
        table.resize(size);
    }

    int index(u64 key, u64 size) {
        return key % size;
    }

    void setEntry(u64 key, Transposition& entry) {
        auto& loc = table[index(key, size)];
        loc = entry;
    }

    Transposition* getEntry(u64 key) {
        return &table[index(key, size)];
    }
};

// ****** NNUE STUFF ******
constexpr i16 inputQuantizationValue = 255;
constexpr i16 hiddenQuantizationValue = 64;
constexpr i16 evalScale = 400;
constexpr size_t HL_SIZE = 128;
constexpr size_t OUTPUT_BUCKETS = 8;

constexpr int ReLU = 0;
constexpr int CReLU = 1;
constexpr int SCReLU = 2;

constexpr int activation = SCReLU;


using Accumulator = array<i16, HL_SIZE>;

class NNUE {
public:
    alignas(32) array<i16, HL_SIZE * 768> weightsToHL;
    alignas(32) array<i16, HL_SIZE> hiddenLayerBias;
    alignas(32) array<array<i16, HL_SIZE * 2>, OUTPUT_BUCKETS> weightsToOut;
    array<i16, OUTPUT_BUCKETS> outputBias;

    constexpr i16 ReLU(const i16 x) {
        if (x < 0) return 0;
        return x;
    }

    constexpr i16 CReLU(const i16 x) {
        i16 ans = x;
        if (x < 0) ans = 0;
        else if (x > inputQuantizationValue) ans = inputQuantizationValue;
        return ans;
    }

    static int feature(Color perspective, Color color, PieceType piece, Square square) {
        // Constructs a feature from the given perspective for a piece
        // of the given type and color on the given square

        int colorIndex = (perspective == color) ? 0 : 1;
        int pieceIndex = static_cast<int>(piece);
        int squareIndex = (perspective == BLACK) ? flipRank(square) : static_cast<int>(square);

        return colorIndex * 64 * 6 + pieceIndex * 64 + squareIndex;
    }


    // Function from stockfish
    template<typename IntType>
    inline IntType read_little_endian(std::istream& stream) {
        IntType result;

        if (IS_LITTLE_ENDIAN)
            stream.read(reinterpret_cast<char*>(&result), sizeof(IntType));
        else
        {
            std::uint8_t u[sizeof(IntType)];
            std::make_unsigned_t<IntType> v = 0;

            stream.read(reinterpret_cast<char*>(u), sizeof(IntType));
            for (std::size_t i = 0; i < sizeof(IntType); ++i)
                v = (v << 8) | u[sizeof(IntType) - i - 1];

            std::memcpy(&result, &v, sizeof(IntType));
        }

        return result;
    }

    NNUE() {
        weightsToHL.fill(1);

        hiddenLayerBias.fill(0);

        for (auto& w : weightsToOut) w.fill(1);
        outputBias.fill(0);
    }

    void loadNetwork(const std::string& filepath) {
        std::ifstream stream(filepath, std::ios::binary);
        if (!stream.is_open()) {
            std::cerr << "Failed to open file: " + filepath << endl;
            std::cerr << "Expect engine to not work as intended with bad evaluation" << endl;
        }

        // Load weightsToHL
        for (size_t i = 0; i < weightsToHL.size(); ++i) {
            weightsToHL[i] = read_little_endian<int16_t>(stream);
        }

        // Load hiddenLayerBias
        for (size_t i = 0; i < hiddenLayerBias.size(); ++i) {
            hiddenLayerBias[i] = read_little_endian<int16_t>(stream);
        }

        // Load weightsToOut
        for (size_t i = 0; i < weightsToOut.size(); ++i) {
            for (size_t j = 0; j < weightsToOut[i].size(); j++) {
                weightsToOut[i][j] = read_little_endian<int16_t>(stream);
            }
        }

        // Load outputBias
        for (size_t i = 0; i < outputBias.size(); ++i) {
            outputBias[i] = read_little_endian<int16_t>(stream);
        }

        cout << "Network loaded successfully from " << filepath << endl;
        std::cerr << "WARNING: You are using MSVC, this means that your nnue was NOT embedded into the exe." << endl;
    }

    int forwardPass(Board* board);
};


u64 nodes = 0;
int moveOverhead = 20;
int movesToGo = 20;
TranspositionTable TT;
NNUE nn;

// History is history[side][from][to]
array<array<array<int, 64>, 64>, 2> history;

constexpr int msInfoInterval = 1000;
std::chrono::steady_clock::time_point lastInfo;


class Board {
public:
    array<u64, 6> white; // Goes pawns, knights, bishops, rooks, queens, king
    array<u64, 6> black; // Goes pawns, knights, bishops, rooks, queens, king

    u64 blackPieces;
    u64 whitePieces;

    uint64_t enPassant;
    u8 castlingRights;

    Color side = WHITE;

    int halfMoveClock;
    int fullMoveClock;

    bool doubleCheck = false;
    u64 checkMask = 0;
    u64 pinned = 0;
    array<u64, 2> pinnersPerC;

    std::vector<u64> positionHistory;
    u64 zobrist;

    Accumulator whiteAccum;
    Accumulator blackAccum;

    // Represents if the current board state is from a null move
    bool fromNull = false;

    void reset() {
        // Reset position
        white[0] = 0xFF00ULL;
        white[1] = 0x42ULL;
        white[2] = 0x24ULL;
        white[3] = 0x81ULL;
        white[4] = 0x8ULL;
        white[5] = 0x10ULL;

        black[0] = 0xFF000000000000ULL;
        black[1] = 0x4200000000000000ULL;
        black[2] = 0x2400000000000000ULL;
        black[3] = 0x8100000000000000ULL;
        black[4] = 0x800000000000000ULL;
        black[5] = 0x1000000000000000ULL;

        castlingRights = 0xF;

        enPassant = 0; // No en passant target square
        side = WHITE;

        halfMoveClock = 0;
        fullMoveClock = 1;

        positionHistory.clear();

        recompute();
        updateCheckPin();
        updateZobrist();
        updateAccum();
    }

    void clearIndex(int index) {
        const u64 mask = ~(1ULL << index);
        white[0] &= mask;
        white[1] &= mask;
        white[2] &= mask;
        white[3] &= mask;
        white[4] &= mask;
        white[5] &= mask;

        black[0] &= mask;
        black[1] &= mask;
        black[2] &= mask;
        black[3] &= mask;
        black[4] &= mask;
        black[5] &= mask;
    }

    void clearIndex(const Color c, int index) {
        const u64 mask = ~(1ULL << index);
        if (c) {
            white[0] &= mask;
            white[1] &= mask;
            white[2] &= mask;
            white[3] &= mask;
            white[4] &= mask;
            white[5] &= mask;
        }
        else {
            black[0] &= mask;
            black[1] &= mask;
            black[2] &= mask;
            black[3] &= mask;
            black[4] &= mask;
            black[5] &= mask;
        }
    }

    void recompute() {
        whitePieces = white[0] | white[1] | white[2] | white[3] | white[4] | white[5];
        blackPieces = black[0] | black[1] | black[2] | black[3] | black[4] | black[5];

        positionHistory.push_back(zobrist);
    }

    PieceType getPiece(int index) {
        u64 indexBB = 1ULL << index;
        if ((white[0] | black[0]) & indexBB) return PAWN;
        else if ((white[1] | black[1]) & indexBB) return KNIGHT;
        else if ((white[2] | black[2]) & indexBB) return BISHOP;
        else if ((white[3] | black[3]) & indexBB) return ROOK;
        else if ((white[4] | black[4]) & indexBB) return QUEEN;
        else return KING;
    }

    void generatePawnMoves(MoveList& moves) {
        int backShift = 0; // How much to shift a position by to get back to the original

        u64 pawns = pieces(side, PAWN);

        u64 pawnPushes = side ? (shift<NORTH>(pawns)) : (shift<SOUTH>(pawns));
        pawnPushes &= ~pieces();
        int currentSquare;

        u64 pawnCaptureRight = pawns & ~(Precomputed::isOnH);
        pawnCaptureRight = side ? (shift<NORTH_EAST>(pawnCaptureRight)) : (shift<SOUTH_EAST>(pawnCaptureRight));
        u64 pawnCaptureRightEP = pawnCaptureRight & enPassant;
        pawnCaptureRight &= pieces(~side);

        u64 pawnCaptureLeft = pawns & ~(Precomputed::isOnA);
        pawnCaptureLeft = side ? (shift<NORTH_WEST>(pawnCaptureLeft)) : (shift<SOUTH_WEST>(pawnCaptureLeft));
        u64 pawnCaptureLeftEP = pawnCaptureLeft & enPassant;
        pawnCaptureLeft &= pieces(~side);

        u64 pawnDoublePush = side ? (shift<NORTH>(pawnPushes)) : (shift<SOUTH>(pawnPushes));
        pawnDoublePush &= side ? Precomputed::isOn4 : Precomputed::isOn5;
        pawnDoublePush &= ~pieces();

        backShift = side ? SOUTH_SOUTH : NORTH_NORTH;
        while (pawnDoublePush) {
            currentSquare = ctzll(pawnDoublePush);
            moves.add(Move(currentSquare + backShift, currentSquare, DOUBLE_PUSH));

            pawnDoublePush &= pawnDoublePush - 1;
        }
        
        backShift = side ? SOUTH : NORTH;
        while (pawnPushes) {
            currentSquare = ctzll(pawnPushes);
            if ((1ULL << currentSquare) & (Precomputed::isOn1 | Precomputed::isOn8)) {
                moves.add(Move(currentSquare + backShift, currentSquare, QUEEN_PROMO));
                moves.add(Move(currentSquare + backShift, currentSquare, ROOK_PROMO));
                moves.add(Move(currentSquare + backShift, currentSquare, BISHOP_PROMO));
                moves.add(Move(currentSquare + backShift, currentSquare, KNIGHT_PROMO));
            }
            else moves.add(Move(currentSquare + backShift, currentSquare));

            pawnPushes &= pawnPushes - 1;
        }

        backShift = side ? SOUTH_WEST : NORTH_WEST;
        while (pawnCaptureRight) {
            currentSquare = ctzll(pawnCaptureRight);
            if ((1ULL << currentSquare) & (Precomputed::isOn1 | Precomputed::isOn8)) {
                moves.add(Move(currentSquare + backShift, currentSquare, QUEEN_PROMO_CAPTURE));
                moves.add(Move(currentSquare + backShift, currentSquare, ROOK_PROMO_CAPTURE));
                moves.add(Move(currentSquare + backShift, currentSquare, BISHOP_PROMO_CAPTURE));
                moves.add(Move(currentSquare + backShift, currentSquare, KNIGHT_PROMO_CAPTURE));
            }
            else moves.add(Move(currentSquare + backShift, currentSquare, CAPTURE));

            pawnCaptureRight &= pawnCaptureRight - 1;
        }

        backShift = side ? SOUTH_EAST : NORTH_EAST;
        while (pawnCaptureLeft) {
            currentSquare = ctzll(pawnCaptureLeft);
            if ((1ULL << currentSquare) & (Precomputed::isOn1 | Precomputed::isOn8)) {
                moves.add(Move(currentSquare + backShift, currentSquare, QUEEN_PROMO_CAPTURE));
                moves.add(Move(currentSquare + backShift, currentSquare, ROOK_PROMO_CAPTURE));
                moves.add(Move(currentSquare + backShift, currentSquare, BISHOP_PROMO_CAPTURE));
                moves.add(Move(currentSquare + backShift, currentSquare, KNIGHT_PROMO_CAPTURE));
            }
            else moves.add(Move(currentSquare + backShift, currentSquare, CAPTURE));

            pawnCaptureLeft &= pawnCaptureLeft - 1;
        }
        
        if (pawnCaptureLeftEP) {
            currentSquare = ctzll(pawnCaptureLeftEP);
            moves.add(Move(currentSquare + backShift, currentSquare, EN_PASSANT));

            pawnCaptureLeftEP &= pawnCaptureLeftEP - 1;
        }

        backShift = side ? SOUTH_WEST : NORTH_WEST;
        if (pawnCaptureRightEP) {
            currentSquare = ctzll(pawnCaptureRightEP);
            moves.add(Move(currentSquare + backShift, currentSquare, EN_PASSANT));

            pawnCaptureRightEP &= pawnCaptureRightEP - 1;
        }
    }

    void generateKnightMoves(MoveList& moves) {
        u64 knightBB = pieces(side, KNIGHT);
        u64 ourBitboard = pieces(side);

        while (knightBB > 0) {
            int currentSquare = ctzll(knightBB);

            u64 knightMoves = KNIGHT_ATTACKS[currentSquare];
            knightMoves &= ~ourBitboard;

            while (knightMoves > 0) {
                int to = ctzll(knightMoves);
                moves.add(Move(currentSquare, to, ((1ULL << to) & pieces()) ? CAPTURE : STANDARD_MOVE));
                knightMoves &= knightMoves - 1;
            }
            knightBB &= knightBB - 1;
        }
    }

    void generateBishopMoves(MoveList& moves) {
        u64 bishopBB = pieces(side, BISHOP, QUEEN);
        u64 ourBitboard = pieces(side);

        while (bishopBB > 0) {
            int currentSquare = ctzll(bishopBB);

            u64 bishopMoves = getBishopAttacks(Square(currentSquare), pieces());
            bishopMoves &= ~ourBitboard;

            while (bishopMoves > 0) {
                int to = ctzll(bishopMoves);
                moves.add(Move(currentSquare, to, ((1ULL << to) & pieces()) ? CAPTURE : STANDARD_MOVE));
                bishopMoves &= bishopMoves - 1;
            }
            bishopBB &= bishopBB - 1;
        }
    }

    void generateRookMoves(MoveList& moves) {
        u64 rookBB = pieces(side, ROOK, QUEEN);
        u64 ourBitboard = pieces(side);

        while (rookBB > 0) {
            int currentSquare = ctzll(rookBB);

            u64 rookMoves = getRookAttacks(Square(currentSquare), pieces());
            rookMoves &= ~ourBitboard;

            while (rookMoves > 0) {
                int to = ctzll(rookMoves);
                moves.add(Move(currentSquare, to, ((1ULL << to) & pieces()) ? CAPTURE : STANDARD_MOVE));
                rookMoves &= rookMoves - 1;
            }
            rookBB &= rookBB - 1;
        }
    }

    void generateKingMoves(MoveList& moves) {
        u64 ourBitboard = pieces(side);

        int kingSquare = ctzll(pieces(side, KING));

        u64 kingMoves = KING_ATTACKS[kingSquare];
        kingMoves &= ~ourBitboard;

        while (kingMoves > 0) {
            int to = ctzll(kingMoves);
            moves.add(Move(kingSquare, to, ((1ULL << to) & pieces()) ? CAPTURE : STANDARD_MOVE));
            kingMoves &= kingMoves - 1;
        }

        if (!isInCheck()) {
            // Castling moves
            if (side && kingSquare == e1) {
                moves.add(Move(e1, g1, CASTLE_K));
                moves.add(Move(e1, c1, CASTLE_Q));
            }
            else if (!side && kingSquare == e8) {
                moves.add(Move(e8, g8, CASTLE_K));
                moves.add(Move(e8, c8, CASTLE_Q));
            }
        }
    }

    int evaluateMVVLVA(Move& a) {
        int victim = PIECE_VALUES[getPiece(a.endSquare())];
        int attacker = PIECE_VALUES[getPiece(a.startSquare())];

        // Higher victim value and lower attacker value are prioritized
        return (victim * 100) - attacker;
    }

    int getHistoryBonus(Move& m) {
        return history[side][m.startSquare()][m.endSquare()];
    }

    u64 attackersTo(Square sq, u64 occ) {
        return (getRookAttacks(sq, occ) & pieces(ROOK, QUEEN))
            | (getBishopAttacks(sq, occ) & pieces(BISHOP, QUEEN))
            | (pawnAttacksBB<WHITE>(sq) & pieces(BLACK, PAWN))
            | (pawnAttacksBB<BLACK>(sq) & pieces(WHITE, PAWN))
            | (KNIGHT_ATTACKS[sq] & pieces(KNIGHT))
            | (KING_ATTACKS[sq] & pieces(KING));
    }

    bool see(Move m, int threshold) { // Based on SF
        // Don't do anything with promo, castle, EP
        if ((m.typeOf() & ~CAPTURE) != 0) return 0 >= threshold;

        int from = m.startSquare();
        int to = m.endSquare();

        int swap = PIECE_VALUES[getPiece(to)] - threshold;
        if (swap <= 0) return false;

        swap = PIECE_VALUES[getPiece(from)] - swap;
        if (swap <= 0) return true;

        u64 occ = pieces() ^ (1ULL << from) ^ (1ULL << to);
        Color stm = side;
        u64 attackers = attackersTo(Square(to), occ);
        u64 stmAttackers, bb;

        int res = 1;

        while (true) {
            stm = ~stm;
            attackers &= occ;

            stmAttackers = attackers & pieces(stm);
            if (!(stmAttackers)) break;

            if (pinners(~stm) & occ) {
                stmAttackers &= ~pinned;
                if (!stmAttackers) break;
            }

            res ^= 1;

            if ((bb = stmAttackers & pieces(PAWN))) {
                swap = PIECE_VALUES[PAWN] - swap;
                if (swap < res) break;
                occ ^= 1ULL << ctzll(bb); // LSB as a bitboard

                attackers |= getBishopAttacks(Square(to), occ) & pieces(BISHOP, QUEEN);
            }

            else if ((bb = stmAttackers & pieces(KNIGHT))) {
                swap = PIECE_VALUES[KNIGHT] - swap;
                if (swap < res) break;
                occ ^= 1ULL << ctzll(bb);
            }

            else if ((bb = stmAttackers & pieces(BISHOP))) {
                swap = PIECE_VALUES[BISHOP] - swap;
                if (swap < res) break;
                occ ^= 1ULL << ctzll(bb);

                attackers |= getBishopAttacks(Square(to), occ) & pieces(BISHOP, QUEEN);
            }

            else if ((bb = stmAttackers & pieces(ROOK))) {
                swap = PIECE_VALUES[ROOK] - swap;
                if (swap < res) break;
                occ ^= 1ULL << ctzll(bb);

                attackers |= getRookAttacks(Square(to), occ) & pieces(ROOK, QUEEN);
            }

            else if ((bb = stmAttackers & pieces(QUEEN))) {
                swap = PIECE_VALUES[QUEEN] - swap;
                if (swap < res) break;
                occ ^= 1ULL << ctzll(bb);

                attackers |= getBishopAttacks(Square(to), occ) & pieces(BISHOP, QUEEN)
                    | getRookAttacks(Square(to), occ) & pieces(ROOK, QUEEN);
            }
            else return (attackers & ~pieces(stm)) ? res ^ 1 : res; // King capture so flip side if enemy has attackers
        }

        return res;
    }

    MoveList generateMoves(bool capturesOnly = false) {
        MoveList allMoves;
        generateKingMoves(allMoves);

        if (doubleCheck) { // Returns early when double checks
            return allMoves;
        }

        MoveList prioritizedMoves, captures, quietMoves;
        generatePawnMoves(allMoves);
        generateKnightMoves(allMoves);
        generateBishopMoves(allMoves);
        generateRookMoves(allMoves);
        // Queen moves are part of bishop/rook moves

        // Classify moves
        for (Move move : allMoves) {
            if (move.isQuiet()) {
                quietMoves.add(move);
            }
            else {
                captures.add(move);
            }
        }

        Transposition* TTEntry = TT.getEntry(zobrist);
        if (TTEntry->zobristKey == zobrist && allMoves.find(TTEntry->bestMove) != -1) {
            prioritizedMoves.add(TTEntry->bestMove); // This move CAN be used in qsearch
        }
        
        std::stable_sort(captures.moves.begin(), captures.moves.begin() + captures.count, [&](Move a, Move b) { return evaluateMVVLVA(a) > evaluateMVVLVA(b); });

        for (Move m : captures) {
            prioritizedMoves.add(m);
        }

        if (capturesOnly) {
            return prioritizedMoves;
        }

        std::stable_sort(quietMoves.moves.begin(), quietMoves.moves.begin() + quietMoves.count, [&](Move a, Move b) { return getHistoryBonus(a) > getHistoryBonus(b); });


        for (Move m : quietMoves) {
            prioritizedMoves.add(m);
        }

        return prioritizedMoves;
    }

    template<PieceType pt>
    Square square(Color c) {
        return Square(c * (white[pt]) + ~c * (black[pt]));
    }

    u64 pieces() {
        return whitePieces | blackPieces;
    }

    u64 pieces(PieceType pt) {
        return white[pt] | black[pt];
    }

    u64 pieces(Color c, PieceType pt) {
        return c ? white[pt] : black[pt];
    }
    
    u64 pieces(PieceType p1, PieceType p2) {
        return white[p1] | white[p2] | black[p1] | black[p2];
    }

    u64 pieces(Color c, PieceType p1, PieceType p2) {
        return c ? (white[p1] | white[p2]) : (black[p1] | black[p2]);
    }

    u64 pieces(Color c) {
        return c ? whitePieces : blackPieces;
    }

    bool isEndgame() {
        return popcountll(pieces()) < 9;
    }

    bool canNullMove() {
        return !fromNull && !isEndgame() && !isInCheck();
    }

    // This function is fairly slow and should not be used where possible
    bool isGameOver() {
        return isDraw() || generateLegalMoves().count == 0;
    }

    // Uses 2fold check
    bool isDraw() {
        if (halfMoveClock >= 100) return true;
        if (std::count(positionHistory.begin(), positionHistory.end(), zobrist) >= 2) {
            return true;
        }
        return false;
    }

    // Returns if STM is in check
    bool isInCheck() {
        return ~checkMask != 0;
    }

    bool isUnderAttack(int square) {
        return isUnderAttack(side, square);
    }

    bool isUnderAttack(Color side, int square) {
        // *** SLIDING PIECE ATTACKS ***
        // Straight Directions (Rooks and Queens)
        if (pieces(~side, ROOK, QUEEN) & getRookAttacks(Square(square), pieces())) return true;

        // Diagonal Directions (Bishops and Queens)
        if (pieces(~side, BISHOP, QUEEN) & getBishopAttacks(Square(square), pieces())) return true;


        // *** KNIGHT ATTACKS ***
        if (pieces(~side, KNIGHT) & KNIGHT_ATTACKS[square]) return true;

        // *** KING ATTACKS ***
        if (pieces(~side, KING) & KING_ATTACKS[square]) return true;


        // *** PAWN ATTACKS ***
        if (side == WHITE) return (pawnAttacksBB<WHITE>(square) & pieces(BLACK, PAWN)) != 0;
        else return (pawnAttacksBB<BLACK>(square) & pieces(WHITE, PAWN)) != 0;

        return false;
    }

    bool aligned(int from, int to, int test) {
        return (LINE[from][to] & (1ULL << test));
    }

    // Update checkers and pinners
    void updateCheckPin() {
        int kingIndex = ctzll(side ? white[5] : black[5]);

        u64 ourPieces = pieces(side);
        auto& opponentPieces = side ? black : white;
        u64 enemyRookQueens = pieces(~side, ROOK, QUEEN);
        u64 enemyBishopQueens = pieces(~side, BISHOP, QUEEN);

        // Direct attacks for potential checks
        u64 rookChecks = getRookAttacks(Square(kingIndex), pieces()) & enemyRookQueens;
        u64 bishopChecks = getBishopAttacks(Square(kingIndex), pieces()) & enemyBishopQueens;
        u64 checks = rookChecks | bishopChecks;
        checkMask = 0; // If no checks, will be set to all 1s later.

        // *** KNIGHT ATTACKS ***
        u64 knightAttacks = KNIGHT_ATTACKS[kingIndex] & opponentPieces[1];
        while (knightAttacks) {
            checkMask |= (1ULL << ctzll(knightAttacks));
            knightAttacks &= knightAttacks - 1;
        }

        // *** PAWN ATTACKS ***
        if (side) {
            if ((opponentPieces[0] & (1ULL << (kingIndex + 7))) && (kingIndex % 8 != 0))
                checkMask |= (1ULL << (kingIndex + 7));
            if ((opponentPieces[0] & (1ULL << (kingIndex + 9))) && (kingIndex % 8 != 7))
                checkMask |= (1ULL << (kingIndex + 9));
        }
        else {
            if ((opponentPieces[0] & (1ULL << (kingIndex - 7))) && (kingIndex % 8 != 7))
                checkMask |= (1ULL << (kingIndex - 7));
            if ((opponentPieces[0] & (1ULL << (kingIndex - 9))) && (kingIndex % 8 != 0))
                checkMask |= (1ULL << (kingIndex - 9));
        }

        popcountll(checks | checkMask) > 1 ? doubleCheck = true : doubleCheck = false;

        while (checks) {
            checkMask |= LINESEG[kingIndex][ctzll(checks)];
            checks &= checks - 1;
        }

        if (!checkMask) checkMask = INF; // If no checks, set to all ones

        // ****** PIN STUFF HERE ******
        u64 rookXrays = getXrayRookAttacks(Square(kingIndex), pieces(), ourPieces) & enemyRookQueens;
        u64 bishopXrays = getXrayBishopAttacks(Square(kingIndex), pieces(), ourPieces) & enemyBishopQueens;
        u64 pinners = rookXrays | bishopXrays;
        pinnersPerC[side] = pinners;

        pinned = 0;
        while (pinners) {
            pinned |= LINESEG[ctzll(pinners)][kingIndex] & ourPieces;
            pinners &= pinners - 1;
        }
    }

    u64 pinners(Color c) {
        return pinnersPerC[c];
    }

    bool isLegalMove(Move m) {
        int from = m.startSquare();
        int to = m.endSquare();

        // Delete null moves
        if (from == to) return false;

        // Castling checks
        if (m.typeOf() == CASTLE_K || m.typeOf() == CASTLE_Q) {
            bool kingside = (m.typeOf() == CASTLE_K);
            int rightsIndex = side ? (kingside ? 3 : 2) : (kingside ? 1 : 0);
            if (!readBit(castlingRights, rightsIndex)) return false;
            if (isInCheck()) return false;
            u64 occupied = whitePieces | blackPieces;
            if (side) {
                if (kingside) {
                    if ((occupied & ((1ULL << f1) | (1ULL << g1))) != 0) return false;
                    if (isUnderAttack(WHITE, f1) || isUnderAttack(WHITE, g1)) return false;
                }
                else {
                    if ((occupied & ((1ULL << b1) | (1ULL << c1) | (1ULL << d1))) != 0) return false;
                    if (isUnderAttack(WHITE, d1) || isUnderAttack(WHITE, c1)) return false;
                }
            }
            else {
                if (kingside) {
                    if ((occupied & ((1ULL << f8) | (1ULL << g8))) != 0) return false;
                    if (isUnderAttack(BLACK, f8) || isUnderAttack(BLACK, g8)) return false;
                }
                else {
                    if ((occupied & ((1ULL << b8) | (1ULL << c8) | (1ULL << d8))) != 0) return false;
                    if (isUnderAttack(BLACK, d8) || isUnderAttack(BLACK, c8)) return false;
                }
            }
        }

        u64& king = side ? white[5] : black[5];
        int kingIndex = ctzll(king);

        // King moves
        if (king & (1ULL << from)) {
            u64& pieces = side ? whitePieces : blackPieces;

            pieces ^= king;
            king ^= 1ULL << kingIndex;

            bool ans = !isUnderAttack(side, to);
            king ^= 1ULL << kingIndex;
            pieces ^= king;
            return ans;
        }

        // En passant check
        if (m.typeOf() == EN_PASSANT) {
            Board testBoard = *this;
            testBoard.move(m, false);
            // Needs to use a different test because isInCheck does not work with NSTM
            return !testBoard.isUnderAttack(side, ctzll(king));
        }

        // Validate move
        if ((1ULL << to) & ~checkMask) return false;

        // Handle pin scenario
        if ((pinned & 1ULL << from) && !aligned(from, to, kingIndex)) return false;

        // If we reach here, the move is legal
        return true;
    }

    MoveList generateLegalMoves() {
        auto moves = generateMoves();
        int i = 0;

        while (i < moves.count) {
            if (!isLegalMove(moves.moves[i])) {
                // Replace the current move with the last move and decrease count
                moves.moves[i] = moves.moves[moves.count - 1];
                moves.count--;
            }
            else {
                ++i;
            }
        }
        return moves;
    }

    void display() {
        if (side)
            cout << "White's turn" << endl;
        else
            cout << "Black's turn" << endl;

        for (int rank = 7; rank >= 0; rank--) {
            cout << "+---+---+---+---+---+---+---+---+" << endl;
            for (int file = 0; file < 8; file++) {
                int i = rank * 8 + file;  // Map rank and file to bitboard index
                char currentPiece = ' ';

                if (readBit(white[0], i)) currentPiece = 'P';
                else if (readBit(white[1], i)) currentPiece = 'N';
                else if (readBit(white[2], i)) currentPiece = 'B';
                else if (readBit(white[3], i)) currentPiece = 'R';
                else if (readBit(white[4], i)) currentPiece = 'Q';
                else if (readBit(white[5], i)) currentPiece = 'K';

                else if (readBit(black[0], i)) currentPiece = 'p';
                else if (readBit(black[1], i)) currentPiece = 'n';
                else if (readBit(black[2], i)) currentPiece = 'b';
                else if (readBit(black[3], i)) currentPiece = 'r';
                else if (readBit(black[4], i)) currentPiece = 'q';
                else if (readBit(black[5], i)) currentPiece = 'k';

                cout << "| " << ((1ULL << i) & whitePieces ? Colors::YELLOW : Colors::BLUE) << currentPiece << Colors::RESET << " ";
            }
            cout << "| " << rank + 1 << endl;
        }
        cout << "+---+---+---+---+---+---+---+---+" << endl;
        cout << "  a   b   c   d   e   f   g   h" << endl;
        cout << endl;
        cout << "Current FEN: " << exportToFEN() << endl;
        cout << "Current key: 0x" << std::hex << std::uppercase << zobrist << std::dec << endl;
    }

    // Used for quiets, can assume all moving pieces are friendly
    void addSubAccumulator(Square add, PieceType addPT, Square subtract, PieceType subPT) {
        // Extract the features
        int addFeatureWhite = NNUE::feature(WHITE, side, addPT, add);
        int addFeatureBlack = NNUE::feature(BLACK, side, addPT, add);
        
        int subFeatureWhite = NNUE::feature(WHITE, side, subPT, subtract);
        int subFeatureBlack = NNUE::feature(BLACK, side, subPT, subtract);

        // Accumulate weights in the hidden layer
        for (int i = 0; i < HL_SIZE; i++) {
            whiteAccum[i] += nn.weightsToHL[addFeatureWhite * HL_SIZE + i];
            blackAccum[i] += nn.weightsToHL[addFeatureBlack * HL_SIZE + i];
            
            whiteAccum[i] -= nn.weightsToHL[subFeatureWhite * HL_SIZE + i];
            blackAccum[i] -= nn.weightsToHL[subFeatureBlack * HL_SIZE + i];
        }
    }

    // Add and sub1 should both be friendly, other pieces are captured
    void addSubSubAccumulator(Square add, PieceType addPT, Square sub1, PieceType sub1PT, Square sub2, PieceType sub2PT) {
        // Extract the features
        int addFeatureWhite = NNUE::feature(WHITE, side, addPT, add);
        int addFeatureBlack = NNUE::feature(BLACK, side, addPT, add);
        
        int sub1FeatureWhite = NNUE::feature(WHITE, side, sub1PT, sub1);
        int sub1FeatureBlack = NNUE::feature(BLACK, side, sub1PT, sub1);

        int sub2FeatureWhite = NNUE::feature(WHITE, ~side, sub2PT, sub2);
        int sub2FeatureBlack = NNUE::feature(BLACK, ~side, sub2PT, sub2);

        // Accumulate weights in the hidden layer
        for (int i = 0; i < HL_SIZE; i++) {
            whiteAccum[i] += nn.weightsToHL[addFeatureWhite * HL_SIZE + i];
            blackAccum[i] += nn.weightsToHL[addFeatureBlack * HL_SIZE + i];
            
            whiteAccum[i] -= nn.weightsToHL[sub1FeatureWhite * HL_SIZE + i];
            blackAccum[i] -= nn.weightsToHL[sub1FeatureBlack * HL_SIZE + i];
            
            whiteAccum[i] -= nn.weightsToHL[sub2FeatureWhite * HL_SIZE + i];
            blackAccum[i] -= nn.weightsToHL[sub2FeatureBlack * HL_SIZE + i];
        }
    }

    // Castling only, all friendly
    void addAddSubSubAccumulator(Square add1, PieceType add1PT, Square add2, PieceType add2PT, Square sub1, PieceType sub1PT, Square sub2, PieceType sub2PT) {
        // Extract the features
        int add1FeatureWhite = NNUE::feature(WHITE, side, add1PT, add1);
        int add1FeatureBlack = NNUE::feature(BLACK, side, add1PT, add1);
        
        int add2FeatureWhite = NNUE::feature(WHITE, side, add2PT, add2);
        int add2FeatureBlack = NNUE::feature(BLACK, side, add2PT, add2);
        
        int sub1FeatureWhite = NNUE::feature(WHITE, side, sub1PT, sub1);
        int sub1FeatureBlack = NNUE::feature(BLACK, side, sub1PT, sub1);

        int sub2FeatureWhite = NNUE::feature(WHITE, side, sub2PT, sub2);
        int sub2FeatureBlack = NNUE::feature(BLACK, side, sub2PT, sub2);

        // Accumulate weights in the hidden layer
        for (int i = 0; i < HL_SIZE; i++) {
            whiteAccum[i] += nn.weightsToHL[add1FeatureWhite * HL_SIZE + i];
            blackAccum[i] += nn.weightsToHL[add1FeatureBlack * HL_SIZE + i];
            
            whiteAccum[i] += nn.weightsToHL[add2FeatureWhite * HL_SIZE + i];
            blackAccum[i] += nn.weightsToHL[add2FeatureBlack * HL_SIZE + i];
            
            whiteAccum[i] -= nn.weightsToHL[sub1FeatureWhite * HL_SIZE + i];
            blackAccum[i] -= nn.weightsToHL[sub1FeatureBlack * HL_SIZE + i];
            
            whiteAccum[i] -= nn.weightsToHL[sub2FeatureWhite * HL_SIZE + i];
            blackAccum[i] -= nn.weightsToHL[sub2FeatureBlack * HL_SIZE + i];
        }
    }

    void placePiece(Color c, int pt, int square) {
        auto& us = c ? white : black;

        setBit(us[pt], square, 1);
        zobrist ^= Precomputed::zobrist[c][pt][square];
    }

    void removePiece(Color c, int pt, int square) {
        auto& us = c ? white : black;
        zobrist ^= Precomputed::zobrist[c][pt][square];

        setBit(us[pt], square, 0);
    }

    void removePiece(Color c, int square) {
        zobrist ^= Precomputed::zobrist[c][getPiece(square)][square];

        clearIndex(c, square);
    }

    void makeNullMove() {
        // En Passant
        zobrist ^= Precomputed::zobristEP[ctzll(enPassant)];
        zobrist ^= Precomputed::zobristEP[64];

        enPassant = 0;

        // Turn
        zobrist ^= Precomputed::zobristSide[side];
        zobrist ^= Precomputed::zobristSide[~side];

        side = ~side;

        fromNull = true;

        updateCheckPin();
    }

    void move(string moveIn) {
        move(Move(moveIn, *this));
    }

    void move(Move moveIn, bool updateMoveHistory = true) {
        auto& us = side ? white : black;

        // Take out old zobrist stuff that will be re-added at the end of the turn
        // Castling
        zobrist ^= Precomputed::zobristCastling[castlingRights];

        // En Passant
        zobrist ^= Precomputed::zobristEP[ctzll(enPassant)];

        // Turn
        zobrist ^= Precomputed::zobristSide[side];

        fromNull = false;

        Square from = moveIn.startSquare();
        Square to = moveIn.endSquare();

        IFDBG{
            if ((1ULL << to) & (white[5] | black[5])) {
                cout << "WARNING: ATTEMPTED CAPTURE OF THE KING. MOVE: " << moveIn.toString() << endl;
                display();

                Transposition* TTEntry = TT.getEntry(zobrist);
                if (TTEntry->zobristKey == zobrist && TTEntry->bestMove == moveIn) cout << "MOVE WAS FROM TT." << endl;
                cout << "MOVE WAS NOT FROM TT." << endl;

                int whiteKing = ctzll(white[5]);
                int blackKing = ctzll(black[5]);
                cout << "Is in check (white): " << isUnderAttack(WHITE, whiteKing) << endl;
                cout << "Is in check (black): " << isUnderAttack(BLACK, blackKing) << endl;
                cout << "En passant square: " << (ctzll(enPassant) < 64 ? squareToAlgebraic(ctzll(enPassant)) : "-") << endl;
                cout << "Castling rights: " << std::bitset<4>(castlingRights) << endl;

                cout << "White pawns: " << popcountll(white[0]) << endl;
                cout << "White knigts: " << popcountll(white[1]) << endl;
                cout << "White bishops: " << popcountll(white[2]) << endl;
                cout << "White rooks: " << popcountll(white[3]) << endl;
                cout << "White queens: " << popcountll(white[4]) << endl;
                cout << "White king: " << popcountll(white[5]) << endl;
                cout << endl;
                cout << "Black pawns: " << popcountll(black[0]) << endl;
                cout << "Black knigts: " << popcountll(black[1]) << endl;
                cout << "Black bishops: " << popcountll(black[2]) << endl;
                cout << "Black rooks: " << popcountll(black[3]) << endl;
                cout << "Black queens: " << popcountll(black[4]) << endl;
                cout << "Black king: " << popcountll(black[5]) << endl;
                cout << endl;

                MoveList moves = generateLegalMoves();
                moves.sortByString(*this);
                cout << "Legal moves (" << moves.count << "):" << endl;
                for (Move m : moves) {
                    cout << m.toString() << endl;
                }
                exit(-1);
            }
        }

        PieceType pt = getPiece(from);
        MoveType mt = moveIn.typeOf();

        removePiece(side, pt, from);
        IFDBG m_assert(!readBit(us[pt], from), "Position piece moved from was not cleared");

        enPassant = 0;

        switch (mt) {
        case STANDARD_MOVE: placePiece(side, pt, to); addSubAccumulator(to, pt, from, pt); break;
        case DOUBLE_PUSH:
            placePiece(side, pt, to);
            enPassant = 1ULL << ((side) * (from + 8) + (!side) * (from - 8));
            addSubAccumulator(to, pt, from, pt);
            break;
        case CASTLE_K:
            if (side == WHITE) {
                placePiece(side, pt, to);

                removePiece(side, ROOK, h1);
                placePiece(side, ROOK, f1);

                addAddSubSubAccumulator(g1, KING, f1, ROOK, e1, KING, h1, ROOK);
            }
            else {
                placePiece(side, pt, to);

                removePiece(side, ROOK, h8);
                placePiece(side, ROOK, f8);

                addAddSubSubAccumulator(g8, KING, f8, ROOK, e8, KING, h8, ROOK);
            }
            break;
        case CASTLE_Q:
            if (side == WHITE) {
                placePiece(side, pt, to);

                removePiece(side, ROOK, a1);
                placePiece(side, ROOK, d1);

                addAddSubSubAccumulator(c1, KING, d1, ROOK, e1, KING, a1, ROOK);
            }
            else {
                placePiece(side, pt, to);

                removePiece(side, ROOK, a8);
                placePiece(side, ROOK, d8);

                addAddSubSubAccumulator(c8, KING, d8, ROOK, e8, KING, a8, ROOK);
            }
            break;
        case CAPTURE: addSubSubAccumulator(to, pt, from, pt, to, getPiece(to)), removePiece(~side, to); placePiece(side, pt, to); break;
        case QUEEN_PROMO_CAPTURE: addSubSubAccumulator(to, QUEEN, from, pt, to, getPiece(to)); removePiece(~side, to); placePiece(side, QUEEN, to); break;
        case ROOK_PROMO_CAPTURE: addSubSubAccumulator(to, ROOK, from, pt, to, getPiece(to)); removePiece(~side, to); placePiece(side, ROOK, to); break;
        case BISHOP_PROMO_CAPTURE: addSubSubAccumulator(to, BISHOP, from, pt, to, getPiece(to)); removePiece(~side, to); placePiece(side, BISHOP, to); break;
        case KNIGHT_PROMO_CAPTURE: addSubSubAccumulator(to, KNIGHT, from, pt, to, getPiece(to)); removePiece(~side, to); placePiece(side, KNIGHT, to); break;
        case EN_PASSANT:
            if (side) {
                removePiece(BLACK, PAWN, to + SOUTH);
                addSubSubAccumulator(to, pt, from, pt, to + SOUTH, PAWN);
            }
            else {
                removePiece(WHITE, PAWN, to + NORTH);
                addSubSubAccumulator(to, pt, from, pt, to + NORTH, PAWN);
            }
            placePiece(side, PAWN, to);

            break;
        case QUEEN_PROMO: placePiece(side, QUEEN, to); addSubAccumulator(to, QUEEN, from, pt); break;
        case ROOK_PROMO: placePiece(side, ROOK, to); addSubAccumulator(to, ROOK, from, pt); break;
        case BISHOP_PROMO: placePiece(side, BISHOP, to); addSubAccumulator(to, BISHOP, from, pt); break;
        case KNIGHT_PROMO: placePiece(side, KNIGHT, to); addSubAccumulator(to, KNIGHT, from, pt); break;
        }

        // Halfmove clock, promo and set en passant
        if (pt == PAWN || readBit(pieces(), to)) halfMoveClock = 0; // Reset halfmove clock on capture or pawn move
        else halfMoveClock++;


        // Remove castling rights
        if (castlingRights) {
            if (from == a1 || to == a1) setBit(castlingRights, 2, 0);
            if (from == h1 || to == h1) setBit(castlingRights, 3, 0);
            if (from == e1) { // King moved
                setBit(castlingRights, 2, 0);
                setBit(castlingRights, 3, 0);
            }
            if (from == a8 || to == a8) setBit(castlingRights, 0, 0);
            if (from == h8 || to == h8) setBit(castlingRights, 1, 0);
            if (from == e8) { // King moved
                setBit(castlingRights, 0, 0);
                setBit(castlingRights, 1, 0);
            }
        }

        side = ~side;

        recompute();
        updateCheckPin();

        // Castling
        zobrist ^= Precomputed::zobristCastling[castlingRights];

        // En Passant
        zobrist ^= Precomputed::zobristEP[ctzll(enPassant)];

        // Turn
        zobrist ^= Precomputed::zobristSide[~side];

        // Only add 1 to full move clock if move was made as black
        fullMoveClock += ~side;
    }

    void loadFromFEN(const std::deque<string>& inputFEN) {
        // Reset
        reset();

        for (int i = 0; i < 64; ++i) clearIndex(i);

        // Sanitize input
        if (inputFEN.size() < 6) {
            std::cerr << "Invalid FEN string" << endl;
            return;
        }

        std::deque<string> parsedPosition = split(inputFEN.at(0), '/');
        char currentCharacter;
        int currentIndex = 56; // Start at rank 8, file 'a' (index 56)

        for (const string& rankString : parsedPosition) {
            for (char c : rankString) {
                currentCharacter = c;

                if (isdigit(currentCharacter)) { // Empty squares
                    int emptySquares = currentCharacter - '0';
                    currentIndex += emptySquares; // Skip the given number of empty squares
                }
                else { // Piece placement
                    switch (currentCharacter) {
                    case 'P': setBit(white[0], currentIndex, 1); break;
                    case 'N': setBit(white[1], currentIndex, 1); break;
                    case 'B': setBit(white[2], currentIndex, 1); break;
                    case 'R': setBit(white[3], currentIndex, 1); break;
                    case 'Q': setBit(white[4], currentIndex, 1); break;
                    case 'K': setBit(white[5], currentIndex, 1); break;
                    case 'p': setBit(black[0], currentIndex, 1); break;
                    case 'n': setBit(black[1], currentIndex, 1); break;
                    case 'b': setBit(black[2], currentIndex, 1); break;
                    case 'r': setBit(black[3], currentIndex, 1); break;
                    case 'q': setBit(black[4], currentIndex, 1); break;
                    case 'k': setBit(black[5], currentIndex, 1); break;
                    default: break;
                    }
                    currentIndex++;
                }
            }
            currentIndex -= 16; // Move to next rank in FEN
        }

        if (inputFEN.at(1) == "w") { // Check the next player's move
            side = WHITE;
        }
        else {
            side = BLACK;
        }

        castlingRights = 0;

        if (inputFEN.at(2).find('K') != string::npos) castlingRights |= 1 << 3;
        if (inputFEN.at(2).find('Q') != string::npos) castlingRights |= 1 << 2;
        if (inputFEN.at(2).find('k') != string::npos) castlingRights |= 1 << 1;
        if (inputFEN.at(2).find('q') != string::npos) castlingRights |= 1;

        if (inputFEN.at(3) != "-") enPassant = 1ULL << parseSquare(inputFEN.at(3));
        else enPassant = 0;

        halfMoveClock = stoi(inputFEN.at(4));
        fullMoveClock = stoi(inputFEN.at(5));

        recompute();
        updateCheckPin();
        updateZobrist();
        updateAccum();
    }

    string exportToFEN() {
        string ans = "";

        int blankSquares = 0;

        for (int rank = 7; rank >= 0; rank--) {
            for (int file = 0; file <= 7; file++) {
                int sq = rank * 8 + file;
                for (int i = 0; i < 6; ++i) {
                    if (readBit(white[i], sq)) {
                        if (blankSquares / 12) ans += std::to_string(blankSquares / 12); // Divide by 12 because it in incremented in a for loop for types
                        blankSquares = 0;
                        switch (i) {
                        case 0: ans += "P"; break;
                        case 1: ans += "N"; break;
                        case 2: ans += "B"; break;
                        case 3: ans += "R"; break;
                        case 4: ans += "Q"; break;
                        case 5: ans += "K"; break;
                        }
                        break;
                    }
                    else blankSquares++;
                }
                for (int i = 0; i < 6; ++i) {
                    if (readBit(black[i], sq)) {
                        if (blankSquares / 12) ans += std::to_string(blankSquares / 12);
                        blankSquares = 0;
                        switch (i) {
                        case 0: ans += "p"; break;
                        case 1: ans += "n"; break;
                        case 2: ans += "b"; break;
                        case 3: ans += "r"; break;
                        case 4: ans += "q"; break;
                        case 5: ans += "k"; break;
                        }
                        break;
                    }
                    else blankSquares++;
                }
            }
            if (blankSquares / 12) ans += std::to_string(blankSquares / 12);
            ans += "/";
            blankSquares = 0;
        }

        // Remove the trailing / on the end of the fen
        ans[ans.length() - 1] = '\0';

        ans += side ? " w " : " b ";

        if (readBit(castlingRights, 3)) ans += "K";
        if (readBit(castlingRights, 2)) ans += "Q";
        if (readBit(castlingRights, 1)) ans += "k";
        if (readBit(castlingRights, 0)) ans += "q";

        if (castlingRights == 0) ans += "- ";
        else ans += " ";

        if (enPassant) ans += squareToAlgebraic(ctzll(enPassant));
        else ans += "-";

        ans += " ";
        ans += std::to_string(halfMoveClock);
        ans += " ";
        ans += std::to_string(fullMoveClock);

        return ans;
    }

    int flipIndex(int index) {
        // Ensure the index is within [0, 63]
        IFDBG m_assert((index >= 0 && index <= 63), "Invalid index: " + std::to_string(index) + ". Must be between 0 and 63.");
        int rank = index / 8;
        int file = index % 8;
        int mirrored_rank = 7 - rank;
        return mirrored_rank * 8 + file;
    }

    int evaluate() { // Returns evaluation in centipawns as side to move
        return std::clamp(nn.forwardPass(this), -MATE_SCORE, MATE_SCORE);

        // Code below here can be used when there are 2 NNUEs
        int matEval = 0;

        // Uses some magic python buffoonery https://github.com/ianfab/chess-variant-stats/blob/main/piece_values.py
        // Based on this https://discord.com/channels/435943710472011776/1300744461281787974/1312722964915027980

        // Material evaluation
        matEval += popcountll(white[0]) * 100;
        matEval += popcountll(white[1]) * 316;
        matEval += popcountll(white[2]) * 328;
        matEval += popcountll(white[3]) * 493;
        matEval += popcountll(white[4]) * 982;

        matEval -= popcountll(black[0]) * 100;
        matEval -= popcountll(black[1]) * 316;
        matEval -= popcountll(black[2]) * 328;
        matEval -= popcountll(black[3]) * 493;
        matEval -= popcountll(black[4]) * 982;

        // Only utilize the large NNUE in situations when game isn't very won or lost
        // Concept from SF
        if (std::abs(matEval) < 950) {
            return nn.forwardPass(this);
        }
    }

    void updateAccum() {
        // This code assumes white is the STM
        u64 whitePieces = this->whitePieces;
        u64 blackPieces = this->blackPieces;

        blackAccum = nn.hiddenLayerBias;
        whiteAccum = nn.hiddenLayerBias;

        // Accumulate contributions for STM pieces
        while (whitePieces) {
            Square currentSq = Square(ctzll(whitePieces)); // Find the least significant set bit

            // Extract the feature for STM
            int inputFeatureWhite = NNUE::feature(WHITE, WHITE, getPiece(currentSq), currentSq);
            int inputFeatureBlack = NNUE::feature(BLACK, WHITE, getPiece(currentSq), currentSq);

            // Accumulate weights for STM hidden layer
            for (int i = 0; i < HL_SIZE; i++) {
                whiteAccum[i] += nn.weightsToHL[inputFeatureWhite * HL_SIZE + i];
                blackAccum[i] += nn.weightsToHL[inputFeatureBlack * HL_SIZE + i];
            }

            whitePieces &= (whitePieces - 1); // Clear the least significant set bit
        }

        // Accumulate contributions for OPP pieces
        while (blackPieces) {
            Square currentSq = Square(ctzll(blackPieces)); // Find the least significant set bit

            // Extract the feature for STM
            int inputFeatureWhite = NNUE::feature(WHITE, BLACK, getPiece(currentSq), currentSq);
            int inputFeatureBlack = NNUE::feature(BLACK, BLACK, getPiece(currentSq), currentSq);

            // Accumulate weights for STM hidden layer
            for (int i = 0; i < HL_SIZE; i++) {
                whiteAccum[i] += nn.weightsToHL[inputFeatureWhite * HL_SIZE + i];
                blackAccum[i] += nn.weightsToHL[inputFeatureBlack * HL_SIZE + i];
            }

            blackPieces &= (blackPieces - 1); // Clear the least significant set bit
        }
    }

    void updateZobrist() {
        zobrist = 0;
        // Pieces
        for (int table = 0; table < 6; table++) {
            u64 tableBB = white[table];
            while (tableBB) {
                int currentIndex = ctzll(tableBB);
                zobrist ^= Precomputed::zobrist[WHITE][table][currentIndex];
                tableBB &= tableBB - 1;
            }
        }
        for (int table = 0; table < 6; table++) {
            u64 tableBB = black[table];
            while (tableBB) {
                int currentIndex = ctzll(tableBB);
                zobrist ^= Precomputed::zobrist[BLACK][table][currentIndex];
                tableBB &= tableBB - 1;
            }
        }

        // Castling
        zobrist ^= Precomputed::zobristCastling[castlingRights];

        // En Passant
        zobrist ^= Precomputed::zobristEP[ctzll(enPassant)];

        // Turn
        zobrist ^= Precomputed::zobristSide[side];
    }
};


// Returns the output of the NN
int NNUE::forwardPass(Board* board) {
    const int divisor = 32 / OUTPUT_BUCKETS;
    const int outputBucket = (popcountll(board->pieces()) - 2) / divisor;

    // Determine the side to move and the opposite side
    Color stm = board->side;

    Accumulator& accumulatorSTM = stm ? board->whiteAccum : board->blackAccum;
    Accumulator& accumulatorOPP = ~stm ? board->whiteAccum : board->blackAccum;

    // Accumulate output for STM and OPP using separate weight segments
    i64 eval = 0;

    if constexpr (activation != ::SCReLU) {
        for (int i = 0; i < HL_SIZE; i++) {
            // First HL_SIZE weights are for STM
            if constexpr (activation == ::ReLU) eval += ReLU(accumulatorSTM[i]) * weightsToOut[outputBucket][i];
            if constexpr (activation == ::CReLU) eval += CReLU(accumulatorSTM[i]) * weightsToOut[outputBucket][i];

            // Last HL_SIZE weights are for OPP
            if constexpr (activation == ::ReLU) eval += ReLU(accumulatorOPP[i]) * weightsToOut[outputBucket][HL_SIZE + i];
            if constexpr (activation == ::CReLU) eval += CReLU(accumulatorOPP[i]) * weightsToOut[outputBucket][HL_SIZE + i];
        }
    }
    else {
        const __m256i vec_zero = _mm256_setzero_si256();
        const __m256i vec_qa = _mm256_set1_epi16(inputQuantizationValue);
        __m256i sum = vec_zero;

        for (int i = 0; i < HL_SIZE / 16; i++) {
            const __m256i us = _mm256_load_si256(reinterpret_cast<const __m256i*>(&accumulatorSTM[16 * i])); // Load from accumulator
            const __m256i them = _mm256_load_si256(reinterpret_cast<const __m256i*>(&accumulatorOPP[16 * i]));

            const __m256i us_weights = _mm256_load_si256(reinterpret_cast<const __m256i*>(&weightsToOut[outputBucket][16 * i])); // Load from net
            const __m256i them_weights = _mm256_load_si256(reinterpret_cast<const __m256i*>(&weightsToOut[outputBucket][HL_SIZE + 16 * i]));

            const __m256i us_clamped = _mm256_min_epi16(_mm256_max_epi16(us, vec_zero), vec_qa);
            const __m256i them_clamped = _mm256_min_epi16(_mm256_max_epi16(them, vec_zero), vec_qa);

            const __m256i us_results = _mm256_madd_epi16(_mm256_mullo_epi16(us_weights, us_clamped), us_clamped);
            const __m256i them_results = _mm256_madd_epi16(_mm256_mullo_epi16(them_weights, them_clamped), them_clamped);

            sum = _mm256_add_epi32(sum, us_results);
            sum = _mm256_add_epi32(sum, them_results);
        }

        __m256i v1 = _mm256_hadd_epi32(sum, sum);
        __m256i v2 = _mm256_hadd_epi32(v1, v1);

        eval = _mm256_extract_epi32(v2, 0) + _mm256_extract_epi32(v2, 4);
    }


    // Dequantization
    if constexpr (activation == ::SCReLU) eval /= inputQuantizationValue;

    eval += outputBias[outputBucket];

    // Apply output bias and scale the result
    return (eval * evalScale) / (inputQuantizationValue * hiddenQuantizationValue);
}


string Move::toString() {
    int start = startSquare();
    int end = endSquare();

    string moveStr = squareToAlgebraic(start) + squareToAlgebraic(end);

    // Handle promotions
    MoveType mt = typeOf();
    char promoChar = '\0';
    switch (mt) {
    case KNIGHT_PROMO:
    case KNIGHT_PROMO_CAPTURE:
        promoChar = 'n';
        break;
    case BISHOP_PROMO:
    case BISHOP_PROMO_CAPTURE:
        promoChar = 'b';
        break;
    case ROOK_PROMO:
    case ROOK_PROMO_CAPTURE:
        promoChar = 'r';
        break;
    case QUEEN_PROMO:
    case QUEEN_PROMO_CAPTURE:
        promoChar = 'q';
        break;
    default:
        break;
    }

    if (promoChar != '\0') {
        moveStr += promoChar;
    }

    return moveStr;
}

Move::Move(string strIn, Board& board) {
    int from = parseSquare(strIn.substr(0, 2));
    int to = parseSquare(strIn.substr(2, 2));

    int flags = 0;

    if ((board.side ? board.blackPieces : board.whitePieces) & (1ULL << to)) flags = CAPTURE;

    if (strIn.size() > 4) { // Move must be promotion
        switch (strIn.at(4)) {
        case 'q': flags |= QUEEN_PROMO; break;
        case 'r': flags |= ROOK_PROMO; break;
        case 'b': flags |= BISHOP_PROMO; break;
        default: flags |= KNIGHT_PROMO; break;
        }
    }

    if (from == e1 && to == g1 && ctzll(board.white[5]) == e1 && readBit(board.castlingRights, 3)) flags = CASTLE_K;
    else if (from == e1 && to == c1 && ctzll(board.white[5]) == e1 && readBit(board.castlingRights, 2)) flags = CASTLE_Q;
    else if (from == e8 && to == g8 && ctzll(board.black[5]) == e8 && readBit(board.castlingRights, 1)) flags = CASTLE_K;
    else if (from == e8 && to == c8 && ctzll(board.black[5]) == e8 && readBit(board.castlingRights, 0)) flags = CASTLE_Q;

    if (to == ctzll(board.enPassant) && ((1ULL << from) & (board.side ? board.white[0] : board.black[0]))) flags = EN_PASSANT;

    if (board.getPiece(from) == PAWN && std::abs(to - from) == 16) flags = DOUBLE_PUSH;

    *this = Move(from, to, flags);
}


// ****** PERFT TESTING *******
u64 _bulk(Board& board, int depth) {
    if (depth == 1) {
        return board.generateLegalMoves().count;
    }

    MoveList moves = board.generateLegalMoves();
    u64 localNodes = 0;
    for (Move m : moves) {
        Board testBoard = board;
        testBoard.move(m);
        localNodes += _bulk(testBoard, depth - 1);
    }
    return localNodes;
}

u64 _perft(Board& board, int depth) {
    if (depth == 0) {
        return 1ULL;
    }

    MoveList moves = board.generateLegalMoves();
    u64 localNodes = 0;
    for (Move m : moves) {
        Board testBoard = board;
        testBoard.move(m);
        localNodes += _perft(testBoard, depth - 1);
    }
    return localNodes;
}

void perft(Board& board, int depth, bool bulk) {
    auto start = std::chrono::steady_clock::now();

    if (depth < 1) return;
    if (depth == 1 && bulk) bulk = false;

    MoveList moves = board.generateLegalMoves();
    moves.sortByString(board);
    u64 localNodes = 0;
    u64 movesThisIter = 0;

    for (Move m : moves) {
        Board testBoard = board;
        testBoard.move(m);
        movesThisIter = bulk ? _bulk(testBoard, depth - 1) : _perft(testBoard, depth - 1);
        localNodes += movesThisIter;
        cout << m.toString() << ": " << movesThisIter << endl;
    }

    int nps = (int)(((double)localNodes) / (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count()) * 1000000000);

    double timeTaken = (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count());
    if (timeTaken / 1000000 < 1000) cout << "Time taken: " << timeTaken / 1000000 << " milliseconds" << endl;
    else cout << "Time taken: " << timeTaken / 1000000000 << " seconds" << endl;
    cout << "Generated moves with " << formatNum(localNodes) << " nodes and NPS of " << formatNum(nps) << endl;
}

void perftSuite(const string& filePath) {
    Board board;
    auto start = std::chrono::high_resolution_clock::now();

    std::fstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << endl;
        return;
    }

    string line;
    int totalTests = 0;
    int passedTests = 0;
    u64 totalNodes = 0;

    while (std::getline(file, line)) {
        auto start = std::chrono::high_resolution_clock::now();

        u64 nodes = 0;

        if (line.empty()) continue; // Skip empty lines

        // Split the line by ';'
        std::deque<string> parts = split(line, ';');

        if (parts.empty()) continue;

        // The first part is the FEN string
        string fen = parts.front();
        parts.pop_front();

        // Split FEN into space-separated parts
        std::deque<string> fenParts = split(fen, ' ');

        // Load FEN into the board
        board.loadFromFEN(fenParts);

        // Iterate over perft entries
        bool allPassed = true;
        cout << "Testing position: " << fen << endl;

        for (const auto& perftEntry : parts) {
            // Trim leading spaces
            string entry = perftEntry;
            size_t firstNonSpace = entry.find_first_not_of(" ");
            if (firstNonSpace != string::npos) {
                entry = entry.substr(firstNonSpace);
            }

            // Expecting format Dx N (e.g., D1 4)
            if (entry.empty()) continue;

            std::deque<string> entryParts = split(entry, ' ');
            if (entryParts.size() != 2) {
                std::cerr << "Invalid perft entry format: \"" << entry << "\"" << endl;
                continue;
            }

            string depthStr = entryParts[0];
            string expectedStr = entryParts[1];

            if (depthStr.size() < 2 || depthStr[0] != 'D') {
                std::cerr << "Invalid depth format: \"" << depthStr << "\"" << endl;
                continue;
            }

            // Extract depth and expected node count
            int depth = stoi(depthStr.substr(1));
            u64 expectedNodes = std::stoull(expectedStr);

            // Execute perft for the given depth
            u64 actualNodes = _bulk(board, depth);

            nodes += actualNodes;

            // Compare and report
            bool pass = (actualNodes == expectedNodes);
            cout << "Depth " << depth << ": Expected " << expectedNodes
                << ", Got " << actualNodes << " -> "
                << (pass ? "PASS" : "FAIL") << endl;

            totalTests++;
            if (pass) passedTests++;
            else allPassed = false;

            totalNodes += actualNodes;
        }

        int nps = (int)(((double)nodes) / (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) * 1000000000);
        double timeTaken = (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count());
        if (timeTaken / 1000000 < 1000) cout << "Time taken: " << timeTaken / 1000000 << " milliseconds" << endl;
        else cout << "Time taken: " << timeTaken / 1000000000 << " seconds" << endl;
        cout << "Generated moves with " << formatNum(nodes) << " nodes and NPS of " << formatNum(nps) << endl;

        if (allPassed) {
            cout << "All perft tests passed for this position." << endl;
        }
        else {
            cout << "Some perft tests failed for this position." << endl;
        }

        cout << "----------------------------------------" << endl << endl;
    }

    cout << "Perft Suite Completed: " << passedTests << " / " << totalTests << " tests passed." << endl;
    int nps = (int)(((double)totalNodes) / (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) * 1000000000);
    double timeTaken = (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count());
    if (timeTaken / 1000000 < 1000) cout << "Time taken: " << timeTaken / 1000000 << " milliseconds" << endl;
    else cout << "Time taken: " << timeTaken / 1000000000 << " seconds" << endl;
    cout << "Generated moves with " << formatNum(totalNodes) << " nodes and NPS of " << formatNum(nps) << endl;
}


struct PvList {
		array<Move, MAX_PLY> moves;
		u32 length;

		void update(Move move, const PvList &child) {
			moves[0] = move;
			std::copy(child.moves.begin(), child.moves.begin() + child.length, moves.begin() + 1);

			length = child.length + 1;

			IFDBG assert(length == 1 || moves[0] != moves[1]);
		}

		auto& operator=(const PvList &other) {
			std::copy(other.moves.begin(), other.moves.begin() + other.length, moves.begin());
			length = other.length;

			return *this;
		}
	};


// ****** SEARCH FUNCTIONS ******
bool isUci = false; // Flag to represent if output should be human or UCI

int seldepth = 0;


bool isWin(int v) {
    return v >= MATE_IN_MAX_PLY;
}

bool isLoss(int v) {
    return v <= MATED_IN_MAX_PLY;
}

bool isDecisive(int v) { return isWin(v) || isLoss(v); }

struct Stack {
    PvList pv;
};

struct SearchLimit {
    std::chrono::steady_clock::time_point searchStart;
    int maxNodes;
    int searchTime;
    std::atomic<bool>* breakFlag;

    SearchLimit() {
        searchStart = std::chrono::steady_clock::now();
        maxNodes = 0;
        searchTime = 0;
        breakFlag = nullptr;
    }
    SearchLimit(auto searchStart, auto breakFlag, auto searchTime, auto maxNodes) {
        this->searchStart = searchStart;
        this->breakFlag = breakFlag;
        this->searchTime = searchTime;
        this->maxNodes = maxNodes;
    }

    bool outOfNodes() {
        return nodes >= maxNodes && maxNodes > 0;
    }

    bool outOfTime() {
        if (searchTime <= 0) return false;
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - searchStart).count() >= searchTime;
    }

    bool stopSearch() {
        return outOfNodes() || outOfTime();
    }
};

template<bool isPV, bool mainThread>
int qsearch(Board& board,
    int alpha,
    int beta,
    SearchLimit* sl) {
    int stand_pat = board.evaluate();
    if (stand_pat >= beta) {
        return beta;
    }
    if (alpha < stand_pat) {
        alpha = stand_pat;
    }

    if (board.isDraw()) {
        return 0;
    }

    Transposition* entry = TT.getEntry(board.zobrist);

    if (!isPV && entry->zobristKey == board.zobrist && (
        entry->flag == EXACT // Exact score
        || (entry->flag == BETA_CUTOFF && entry->score >= beta) // Lower bound, fail high
        || (entry->flag == FAIL_LOW && entry->score <= alpha) // Upper bound, fail low
        )) {
        return entry->score;
    }

    MoveList moves = board.generateMoves(true);
    Move bestMove;

    int flag = FAIL_LOW;

    for (Move m : moves) {
        if (sl->breakFlag->load() && !bestMove.isNull()) break;
        if (sl->outOfNodes()) break;
        if (mainThread && nodes % 2048 == 0 && sl->outOfTime()) {
            sl->breakFlag->store(true);
            break;
        }

        if (!board.isLegalMove(m) || !board.see(m, SEE_MARGIN)) continue;

        Board testBoard = board;
        testBoard.move(m);

        nodes++;

        int score;

        // Principal variation search stuff
        score = -qsearch<isPV, mainThread>(testBoard, -alpha - 1, -alpha, sl);
        // If it fails high or low we search again with the original bounds
        if (score > alpha && score < beta) {
            score = -qsearch<isPV, mainThread>(testBoard, -beta, -alpha, sl);
        }

        if (score >= beta) {
            return beta;
        }
        if (score > alpha) {
            alpha = score;
            bestMove = m;
        }
    }

    *entry = Transposition(board.zobrist, bestMove, flag, alpha, 0);

    return alpha;
}

// Full search function
template<bool isPV, bool mainThread = false>
MoveEvaluation search(Board& board,
    Stack* ss,
    int depth,
    int alpha,
    int beta,
    int ply,
    SearchLimit* sl) {
    // Only worry about draws and such if the node is a child, otherwise game would be over
    if (ply > 0) {
        if (depth <= 0) {
            int eval = qsearch<isPV, mainThread>(board, alpha, beta, sl);
            return { Move(), eval };
        }
        else if (board.isDraw()) {
            return { Move(), 0 };
        }
    }
    if (ply + 1 > seldepth && !mainThread) seldepth = ply + 1;

    Transposition* entry = TT.getEntry(board.zobrist);

    if (!isPV && ply > 0 && entry->zobristKey == board.zobrist && entry->depth >= depth && (
        entry->flag == EXACT // Exact score
        || (entry->flag == BETA_CUTOFF && entry->score >= beta) // Lower bound, fail high
        || (entry->flag == FAIL_LOW && entry->score <= alpha) // Upper bound, fail low
        )) {
        return { Move(), entry->score };
    }


    int staticEval = board.evaluate();

    // Razoring (+9 +- 5)
    if (staticEval <= alpha - RAZOR_MARGIN - RAZOR_DEPTH_SCALAR * depth * depth) {
        int value = qsearch<false, mainThread>(board, alpha - 1, alpha, sl);
        if (value < alpha && !isDecisive(value)) return { Move(), value };
    }

    // Internal iterative reductions (+19 +- 10)
    if (entry->zobristKey != board.zobrist && depth > 3) depth -= 1;

    // Mate distance pruning
    if (ply > 0) {
        int mateValue = MATE_SCORE - ply;

        if (mateValue < beta) {
            beta = mateValue;
            if (alpha >= mateValue) return { Move(), mateValue };
        }

        mateValue = -mateValue;

        if (mateValue > alpha) {
            alpha = mateValue;
            if (beta <= mateValue) return { Move(), mateValue };
        }
    }

    if constexpr (!isPV) {
        // Late move pruning


        // Reverse futility pruning (+32 +- 34)
        if (staticEval - RFP_MARGIN * depth >= beta && depth < 4 && !board.isInCheck()) {
            return { Move(), staticEval - RFP_MARGIN };
        }

        // Null move pruning (+75 +- 33)
        // Don't prune PV nodes and don't prune king + pawn only
        // King + pawn is likely redundant because the position would already be considered endgame, but removing it seems to lose elo
        if (board.canNullMove() && staticEval >= beta && !board.isInCheck() && popcountll(board.side ? board.white[0] : board.black[0]) + 1 != popcountll(board.side ? board.whitePieces : board.blackPieces)) {
            Board testBoard = board;
            testBoard.makeNullMove();
            int eval = -search<false, mainThread>(testBoard, ss + 1, depth - NMP_REDUCTION - depth / NMP_REDUCTION_DIVISOR, -beta, -beta + 1, ply + 1, sl).eval;
            if (eval >= beta) {
                return { Move(), eval };
            }
        }
    }


    MoveList moves = board.generateMoves();
    Move bestMove;
    int bestEval = -INF_INT;

    int flag = FAIL_LOW;

    int movesMade = 0;

    for (Move m : moves) {
        // Break checks
        if (sl->breakFlag->load() && !bestMove.isNull()) break;
        if (sl->outOfNodes()) break;
        if (mainThread && nodes % 2048 == 0 && sl->outOfTime()) {
            sl->breakFlag->store(true);
            break;
        }

        if (!board.isLegalMove(m)) {
            continue; // Validate legal moves
        }
        if (!isPV && !isDecisive(bestEval) && !board.isInCheck() && movesMade >= MIN_MOVES_BEFORE_LMP + depth * depth && depth <= MAX_DEPTH_FOR_LMP && m.isQuiet()) {
            continue; // Skip quiets for late move pruning
        }

        if (ply > 0 && bestEval > MATED_IN_MAX_PLY) {
            // PVS SEE pruning
            const int seeThreshold = !(m.typeOf() & CAPTURE) ? PVS_SEE_QUIET_SCALAR * depth : PVS_SEE_CAPTURE_SCALAR * depth * depth;

            if (depth <= 8 && movesMade > 0 && !board.see(m, seeThreshold)) continue;
        }

        // Calculate reduction factor for late move reduction (+14 +- 8)
        // Based on Weiss
        int depthReduction;
        // Captures or promos are not reduced
        // Assumes std::log(0) is -inf
        if (m.typeOf() & 0b1100 || movesMade < 4) depthReduction = 0;
        else depthReduction = 1.35 + std::log(depth) * std::log(movesMade) / 2.75;

        if (depth < 3) depthReduction = 0;

        // Recursive call with a copied board
        Board testBoard = board;
        testBoard.move(m);
        movesMade++;

        nodes++;

        int eval;

        int newDepth = depth - 1;

        // Only run PVS with more than one move already searched
        if (movesMade == 1) {
            eval = -search<true, mainThread>(testBoard, ss + 1, newDepth, -beta, -alpha, ply + 1, sl).eval;
        }
        else {
            // Principal variation search stuff
            eval = -search<false, mainThread>(testBoard, ss + 1, newDepth - depthReduction, -alpha - 1, -alpha, ply + 1, sl).eval;
            // If it fails high and isPV or used reduction, go again with full bounds
            if (eval > alpha && (isPV || depthReduction > 0)) {
                eval = -search<true, mainThread>(testBoard, ss + 1, newDepth, -beta, -alpha, ply + 1, sl).eval;
            }
        }

        // Update best move and alpha-beta values
        if (eval > bestEval) {
            bestEval = eval;
            bestMove = m;
            if (bestEval > alpha) {
                // This flag seems to lose elo, should be re-tested later on.
                //flag = EXACT;
                alpha = bestEval;
            }
        }

        // Fail high
        if (eval >= beta) {
            flag = BETA_CUTOFF;
        }

        if (alpha >= beta) {
            if (!(m.typeOf() & CAPTURE)) {
                int clampedBonus = std::clamp(depth * depth, -MAX_HISTORY, MAX_HISTORY);
                history[board.side][m.startSquare()][m.endSquare()] += clampedBonus - history[board.side][m.startSquare()][m.endSquare()] * abs(clampedBonus) / MAX_HISTORY;
            }
            break;
        }

        if (ply == 0) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastInfo).count() >= msInfoInterval) {
                int TTused = 0;
                int sampleSize = TT.size > 2000 ? 2000 : TT.size;
                for (int i = 0; i < sampleSize; i++) {
                    if (TT.getEntry(i)->zobristKey != 0) TTused++;
                }
                double elapsedMs = (double)std::chrono::duration_cast<std::chrono::milliseconds>(now - sl->searchStart).count();
                int nps = (int)((double)nodes / (elapsedMs / 1000));
                if (mainThread && isUci) cout << "info depth " << depth << " nodes " << nodes << " nps " << nps << " time " << std::to_string((int)elapsedMs) << " hashfull " << (int)(TTused / (double)sampleSize * 1000) << " currmove " << m.toString() << endl;
            }
        }
    }

    if (!movesMade) {
        if (board.isInCheck()) {
            return { Move(), -MATE_SCORE + ply };
        }
        return { Move(), 0 };
    }

    // Uses entry that was already fetched above
    *entry = Transposition(board.zobrist, bestMove, flag, bestEval, depth);

    return { bestMove, bestEval };
}

template<bool mainThread>
MoveEvaluation iterativeDeepening(
    Board& board,
    int maxDepth,
    std::atomic<bool>& breakFlag,
    int wtime = 0,
    int btime = 0,
    int mtime = 0,
    int winc = 0,
    int binc = 0,
    int maxNodes = -1,
    int softNodes = -1
) {
    if (mainThread) breakFlag.store(false);
    lastInfo = std::chrono::steady_clock::now();
    nodes = 0;
    int hardLimit = 0;
    int softLimit = 0;
    auto start = std::chrono::steady_clock::now();
    std::string bestMoveAlgebra = "";
    if (wtime || btime) {
        hardLimit = board.side ? wtime / movesToGo : btime / movesToGo;
        hardLimit += (board.side ? winc : binc) * INC_SCALAR;
        softLimit = hardLimit * SOFT_TIME_SCALAR;
    }
    else if (mtime) {
        hardLimit = mtime;
        softLimit = hardLimit;
    }

    if (hardLimit) {
        // Set margin for GUI stuff, canceling search, etc.
        // Assumes max search time at depth 1 is 15 ms
        if (hardLimit - moveOverhead > 15) hardLimit -= moveOverhead;
        // If there is less than moveOverhead ms left, just search 1 ply ahead with quiescence search
        // This search is very fast, usually around 5-15ms depending on the CPU and position
        else maxDepth = 1;
    }

    MoveEvaluation move;
    MoveEvaluation bestMove;

    if (ISDBG && mainThread) m_assert(maxDepth >= 1, "Depth is less than 1 in ID search");

    // Cap the depth to 255
    maxDepth = std::min(255, maxDepth);

    softLimit = std::min(softLimit, hardLimit);

    SearchLimit sl = SearchLimit(std::chrono::steady_clock::now(), &breakFlag, hardLimit, maxNodes);

    Stack stack[MAX_PLY];
    Stack* ss = stack;

    for (int depth = 1; depth <= maxDepth; depth++) {
        seldepth = depth;

        // Full search on depth 1, otherwise try with aspiration window
        if (depth == 1) move = search<true, mainThread>(board, ss, depth, -INF_INT, INF_INT, 0, &sl);
        else { // From Clarity
            int alpha = std::max(-MATE_SCORE, move.eval - ASPR_DELTA);
            int beta = std::min(MATE_SCORE, move.eval + ASPR_DELTA);
            int delta = ASPR_DELTA;
            while (true) {
                move = search<true, mainThread>(board, ss, depth, alpha, beta, 0, &sl);
                if (sl.stopSearch()) break;
                if (move.eval >= beta) {
                    beta = std::min(beta + delta, MATE_SCORE);
                }
                else if (move.eval <= alpha) {
                    beta = (alpha + beta) / 2;
                    alpha = std::max(alpha - delta, -MATE_SCORE);
                }
                else break;

                delta *= ASP_DELTA_MULTIPLIER;
            }
        }

        if (ISDBG && mainThread) m_assert(!move.move.isNull(), "Returned null move in search");

        auto now = std::chrono::steady_clock::now();
        double elapsedNs = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(now - start).count();
        int nps = (int)((double)nodes / (elapsedNs / 1e9));

        // Discard searches that return null or illegal moves, usually caused by TT corruption or exiting search early before any search cycle has fisnished
        if (!move.move.isNull() && board.isLegalMove(move.move)) bestMove = move;

        bestMoveAlgebra = bestMove.move.toString();

        int TTused = 0;
        int sampleSize = TT.size > 2000 ? 2000 : TT.size;
        for (int i = 0; i < sampleSize; i++) {
            if (TT.getEntry(i)->zobristKey != 0) TTused++;
        }

        bool isMate = false;
        int mateDist = (MATE_SCORE - std::abs(bestMove.eval)) / 2 + 1;

        if (std::abs(bestMove.eval) >= MATE_IN_MAX_PLY) isMate = true;

        string ans;

        if (breakFlag.load() && depth > 1) {
            if (ISDBG && mainThread) cout << "Stopping search because of break flag" << endl;
            break;
        }

        if (softLimit != 0 && depth > 1) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() >= softLimit) {
                if (ISDBG && mainThread) cout << "Stopping search because of time limit" << endl;
                break;
            }
        }

        if (((maxNodes > 0 && nodes >= maxNodes) || (softNodes > 0 && nodes >= softNodes)) && depth > 1) {
            if (ISDBG && mainThread) cout << "Stopping search because of node limit" << endl;
            break;
        }

        // This is the code to handle UCI
        if (isUci && mainThread) {
            ans = "info depth " + std::to_string(depth) + " nodes " + std::to_string(nodes) + " nps " + std::to_string(nps) + " time " + std::to_string((int)(elapsedNs / 1e6)) + " hashfull " + std::to_string((int)(TTused / (double)sampleSize * 1000));
            if (isMate) {
                ans += " score mate ";
                ans += ((bestMove.eval < 0) ? "-" : "") + std::to_string(mateDist);
            }
            else {
                ans += " score cp " + std::to_string(bestMove.eval);
            }

            ans += " pv " + bestMoveAlgebra;

            cout << ans << endl;
        }
        else if (mainThread) {
            cout << std::fixed << std::setprecision(2);
            const double hashPercent = TTused / (double)sampleSize * 100;
            string fancyEval = "";
            if (isMate) {
                if (bestMove.eval < 0) fancyEval = "-";
                fancyEval += "M" + std::to_string(mateDist);
            }
            else {
                fancyEval = (bestMove.eval > 0 ? '+' : '\0') + std::format("{:.2f}", bestMove.eval / 100.0);
            }
            cout << padStr(std::to_string(depth) + "/" + std::to_string(seldepth), 9);
            cout << Colors::GREY;
            cout << padStr(formatTime((int)(elapsedNs / 1e6)), 7);
            cout << padStr(abbreviateNum(nodes) + "nodes", 15);
            cout << padStr(abbreviateNum(nps) + "nps", 14);
            cout << Colors::CYAN;
            cout << "TT: ";
            cout << Colors::GREY;
            cout << padStr(std::format("{:.1f}%", hashPercent), 9);
            cout << Colors::BRIGHT_GREEN;
            cout << padStr(fancyEval, 7);
            cout << Colors::BLUE;
            cout << bestMoveAlgebra;
            cout << Colors::RESET << std::defaultfloat << std::setprecision(6) << endl;
        }

        lastInfo = std::chrono::steady_clock::now();
    }

    if (mainThread && isUci) std::cout << "bestmove " << bestMoveAlgebra << std::endl;
    if (mainThread) breakFlag.store(true);
    return bestMove;
}

// Run a worker/helper thread to fill TT
void runWorker(Board& board, std::atomic<bool>& breakFlag, std::atomic<bool>& killFlag) {
    IFDBG cout << "New thread created" << endl;
    while (true) {
        if (killFlag.load()) return;
        while (breakFlag.load()) {
            if (killFlag.load()) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        iterativeDeepening<false>(
            std::ref(board), // Board
            INF_INT, // Depth
            std::ref(breakFlag), // Breakflag
            0, // Wtime
            0, // Btime
            0, // Movetime
            0, // Winc
            0, // Binc
            -1, // Soft nodes
            -1); // Hard nodes
    }
}

// ****** BENCH STUFF ******
void bench(int depth) {
    u64 totalNodes = 0;
    double totalTimeMs = 0.0;

    cout << "Starting benchmark with depth " << depth << std::endl;

    array<string, 50> fens = {
        "r3k2r/2pb1ppp/2pp1q2/p7/1nP1B3/1P2P3/P2N1PPP/R2QK2R w KQkq a6 0 14",
        "4rrk1/2p1b1p1/p1p3q1/4p3/2P2n1p/1P1NR2P/PB3PP1/3R1QK1 b - - 2 24",
        "r3qbrk/6p1/2b2pPp/p3pP1Q/PpPpP2P/3P1B2/2PB3K/R5R1 w - - 16 42",
        "6k1/1R3p2/6p1/2Bp3p/3P2q1/P7/1P2rQ1K/5R2 b - - 4 44",
        "8/8/1p2k1p1/3p3p/1p1P1P1P/1P2PK2/8/8 w - - 3 54",
        "7r/2p3k1/1p1p1qp1/1P1Bp3/p1P2r1P/P7/4R3/Q4RK1 w - - 0 36",
        "r1bq1rk1/pp2b1pp/n1pp1n2/3P1p2/2P1p3/2N1P2N/PP2BPPP/R1BQ1RK1 b - - 2 10",
        "3r3k/2r4p/1p1b3q/p4P2/P2Pp3/1B2P3/3BQ1RP/6K1 w - - 3 87",
        "2r4r/1p4k1/1Pnp4/3Qb1pq/8/4BpPp/5P2/2RR1BK1 w - - 0 42",
        "4q1bk/6b1/7p/p1p4p/PNPpP2P/KN4P1/3Q4/4R3 b - - 0 37",
        "2q3r1/1r2pk2/pp3pp1/2pP3p/P1Pb1BbP/1P4Q1/R3NPP1/4R1K1 w - - 2 34",
        "1r2r2k/1b4q1/pp5p/2pPp1p1/P3Pn2/1P1B1Q1P/2R3P1/4BR1K b - - 1 37",
        "r3kbbr/pp1n1p1P/3ppnp1/q5N1/1P1pP3/P1N1B3/2P1QP2/R3KB1R b KQkq b3 0 17",
        "8/6pk/2b1Rp2/3r4/1R1B2PP/P5K1/8/2r5 b - - 16 42",
        "1r4k1/4ppb1/2n1b1qp/pB4p1/1n1BP1P1/7P/2PNQPK1/3RN3 w - - 8 29",
        "8/p2B4/PkP5/4p1pK/4Pb1p/5P2/8/8 w - - 29 68",
        "3r4/ppq1ppkp/4bnp1/2pN4/2P1P3/1P4P1/PQ3PBP/R4K2 b - - 2 20",
        "5rr1/4n2k/4q2P/P1P2n2/3B1p2/4pP2/2N1P3/1RR1K2Q w - - 1 49",
        "1r5k/2pq2p1/3p3p/p1pP4/4QP2/PP1R3P/6PK/8 w - - 1 51",
        "q5k1/5ppp/1r3bn1/1B6/P1N2P2/BQ2P1P1/5K1P/8 b - - 2 34",
        "r1b2k1r/5n2/p4q2/1ppn1Pp1/3pp1p1/NP2P3/P1PPBK2/1RQN2R1 w - - 0 22",
        "r1bqk2r/pppp1ppp/5n2/4b3/4P3/P1N5/1PP2PPP/R1BQKB1R w KQkq - 0 5",
        "r1bqr1k1/pp1p1ppp/2p5/8/3N1Q2/P2BB3/1PP2PPP/R3K2n b Q - 1 12",
        "r1bq2k1/p4r1p/1pp2pp1/3p4/1P1B3Q/P2B1N2/2P3PP/4R1K1 b - - 2 19",
        "r4qk1/6r1/1p4p1/2ppBbN1/1p5Q/P7/2P3PP/5RK1 w - - 2 25",
        "r7/6k1/1p6/2pp1p2/7Q/8/p1P2K1P/8 w - - 0 32",
        "r3k2r/ppp1pp1p/2nqb1pn/3p4/4P3/2PP4/PP1NBPPP/R2QK1NR w KQkq - 1 5",
        "3r1rk1/1pp1pn1p/p1n1q1p1/3p4/Q3P3/2P5/PP1NBPPP/4RRK1 w - - 0 12",
        "5rk1/1pp1pn1p/p3Brp1/8/1n6/5N2/PP3PPP/2R2RK1 w - - 2 20",
        "8/1p2pk1p/p1p1r1p1/3n4/8/5R2/PP3PPP/4R1K1 b - - 3 27",
        "8/4pk2/1p1r2p1/p1p4p/Pn5P/3R4/1P3PP1/4RK2 w - - 1 33",
        "8/5k2/1pnrp1p1/p1p4p/P6P/4R1PK/1P3P2/4R3 b - - 1 38",
        "8/8/1p1kp1p1/p1pr1n1p/P6P/1R4P1/1P3PK1/1R6 b - - 15 45",
        "8/8/1p1k2p1/p1prp2p/P2n3P/6P1/1P1R1PK1/4R3 b - - 5 49",
        "8/8/1p4p1/p1p2k1p/P2n1P1P/4K1P1/1P6/3R4 w - - 6 54",
        "8/8/1p4p1/p1p2k1p/P2n1P1P/4K1P1/1P6/6R1 b - - 6 59",
        "8/5k2/1p4p1/p1pK3p/P2n1P1P/6P1/1P6/4R3 b - - 14 63",
        "8/1R6/1p1K1kp1/p6p/P1p2P1P/6P1/1Pn5/8 w - - 0 67",
        "1rb1rn1k/p3q1bp/2p3p1/2p1p3/2P1P2N/PP1RQNP1/1B3P2/4R1K1 b - - 4 23",
        "4rrk1/pp1n1pp1/q5p1/P1pP4/2n3P1/7P/1P3PB1/R1BQ1RK1 w - - 3 22",
        "r2qr1k1/pb1nbppp/1pn1p3/2ppP3/3P4/2PB1NN1/PP3PPP/R1BQR1K1 w - - 4 12",
        "2r2k2/8/4P1R1/1p6/8/P4K1N/7b/2B5 b - - 0 55",
        "6k1/5pp1/8/2bKP2P/2P5/p4PNb/B7/8 b - - 1 44",
        "2rqr1k1/1p3p1p/p2p2p1/P1nPb3/2B1P3/5P2/1PQ2NPP/R1R4K w - - 3 25",
        "r1b2rk1/p1q1ppbp/6p1/2Q5/8/4BP2/PPP3PP/2KR1B1R b - - 2 14",
        "6r1/5k2/p1b1r2p/1pB1p1p1/1Pp3PP/2P1R1K1/2P2P2/3R4 w - - 1 36",
        "rnbqkb1r/pppppppp/5n2/8/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2",
        "2rr2k1/1p4bp/p1q1p1p1/4Pp1n/2PB4/1PN3P1/P3Q2P/2RR2K1 w - f6 0 20",
        "3br1k1/p1pn3p/1p3n2/5pNq/2P1p3/1PN3PP/P2Q1PB1/4R1K1 w - - 0 23",
        "2r2b2/5p2/5k2/p1r1pP2/P2pB3/1P3P2/K1P3R1/7R w - - 23 93"
    };

    for (auto fen : fens) {
        if (fen.empty()) continue; // Skip empty lines

        Board benchBoard;
        benchBoard.reset();

        // Split the FEN string into components
        std::deque<std::string> fenParts = split(fen, ' ');
        benchBoard.loadFromFEN(fenParts);

        nodes = 0; // Reset node count

        // Set up for iterative deepening
        std::atomic<bool> benchBreakFlag(false);
        std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

        // Start the iterative deepening search
        iterativeDeepening<false>(benchBoard, depth, benchBreakFlag, 0, 0, 0, 0, 0, -1, -1);

        std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
        double durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

        totalNodes += nodes;
        totalTimeMs += durationMs;

        cout << "FEN: " << fen << std::endl;
        cout << "Nodes: " << nodes << ", Time: " << durationMs << " ms" << std::endl;
        cout << "----------------------------------------" << std::endl;
    }

    cout << "Benchmark Completed." << std::endl;
    cout << "Total Nodes: " << formatNum(totalNodes) << std::endl;
    cout << "Total Time: " << formatNum(totalTimeMs) << " ms" << std::endl;
    int nps = INF_INT;
    if (totalTimeMs > 0) {
        nps = static_cast<long long>((totalNodes / totalTimeMs) * 1000);
        cout << "Average NPS: " << formatNum(nps) << std::endl;
    }
    cout << totalNodes << " nodes " << nps << " nps" << endl;
}

// ****** DATA GEN ******
struct DataUnit {
    string data = ""; // This can be changed to use different data methods later

    DataUnit() {
        data = "";
    }
    DataUnit(string data) {
        this->data = data;
    }
};

struct PartialDataUnit {
    string data = ""; // This can be changed to use different data methods later

    PartialDataUnit() {
        data = "";
    }
    PartialDataUnit(string data) {
        this->data = data;
    }
};

// Returns true if the position is checkmate by random chance (is used to reset and start over)
void makeRandomMove(Board& board) {
    MoveList moves = board.generateLegalMoves();
    if (moves.count == 0) return;

    std::random_device rd;

    std::mt19937_64 engine(rd());

    std::uniform_int_distribution<int> dist(0, moves.count - 1);

    Board testBoard;
    Move m = moves.moves[dist(engine)];

    board.move(m);
}

string makeFileName() {
    std::random_device rd;

    std::mt19937_64 engine(rd());

    std::uniform_int_distribution<int> dist(0, INF_INT);

    // Get current time
    auto now = std::chrono::system_clock::now();

    // Convert to time_t
    std::time_t t = std::chrono::system_clock::to_time_t(now);

    // Convert to tm structure
    std::tm tm;

#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    string randomStr = std::to_string(dist(engine));

    return "data-" + std::format("{:04}-{:02}-{:02}", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday) + "-" + randomStr + ".preludedata";
}

void writeToFile(const string& filePath, const std::vector<DataUnit>& data) {
    // Open the file in write mode, should make a new file every time if using random u64 filename, but will append otherwise
    std::ofstream outFile(filePath, std::ios::app);

    // Check if the file was opened successfully
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open the file " << filePath << " for writing." << std::endl;
        exit(-1);
    }

    for (const auto& dataUnit : data) {
        outFile << dataUnit.data << endl;
    }

    outFile.close();
}

void playGames() {
    string filePath = makeFileName(); // Should be added as a cli param later

    std::atomic<bool> breakFlag(false);
    Board board;

    std::vector<DataUnit> outputBuffer;
    std::atomic<bool> bufferLock(false);

    auto startTime = std::chrono::steady_clock::now();

    int cachedPositions = 0;
    int savedPositions = 0;
    int totalPositions = 0;

    int saveEveryN = ::saveEveryN + (::saveEveryN % 2 == 0); // Force to be odd to get equal white/black positions

    saveEveryN = std::max(saveEveryN, 1);

    if (saveEveryN > 1) cout << "Saving every " << saveEveryN << " positions" << endl;
    else cout << "Saving every position" << endl;

    auto clear = []() {
#if defined(_WIN32) || defined(_WIN64) 
        system("cls");
#else
        system("clear");
#endif
        };

    std::random_device rd;

    std::mt19937_64 engine(rd());

    std::uniform_int_distribution<int> dist(0, 1);

    auto randBool = [&]() { return dist(engine); };

    outputBuffer.reserve(clearBufferEvery + 100);

    while (totalPositions < targetPositions) {
        int randMoves = ::randMoves + randBool(); // Half of the games start with first 

        auto now = std::chrono::steady_clock::now();
        int timeSpent = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
        clear();
        double avgPosPerSec = totalPositions / (timeSpent / 1000.0);
        cout << "Positions/second: " + std::format("{:.2f}", avgPosPerSec) << endl;
        cout << "Positions (cached): " << formatNum(cachedPositions) << endl;
        cout << "Positions (saved): " << formatNum(savedPositions) << endl;
        cout << "Time: " << formatTime(timeSpent) << endl;
        cout << endl;

        std::vector<PartialDataUnit> gameData;
        int movesSinceStore = 0; // Moves since the position was put into the buffer

        TT.clear();
        for (auto& side : history) {
            for (auto& from : side) {
                for (auto& to : from) {
                    to = 0;
                }
            }
        }

        board.reset();

        for (int i = 0; i < randMoves; i++) {
            makeRandomMove(board);
        }

        while (!board.isGameOver()) {
            MoveEvaluation bestMove = iterativeDeepening<false>(
                std::ref(board), // Board
                INF_INT, // Depth
                std::ref(breakFlag), // Breakflag
                0, // Wtime
                0, // Btime
                0, // Movetime
                0, // Winc
                0, // Binc
                nodesPerMove,
                maxNodesPerMove);
            board.move(bestMove.move);
            movesSinceStore++;
            if (movesSinceStore >= saveEveryN && !board.isInCheck() && !(bestMove.move.typeOf() & CAPTURE)) {
                gameData.push_back(PartialDataUnit(board.exportToFEN() + " | " + std::to_string(bestMove.eval)));
                totalPositions++;
                cachedPositions++;
                movesSinceStore = 0;
            }
        }

        // Take partial move data and add the game result
        for (PartialDataUnit i : gameData) {
            string finalData = i.data + " | ";
            if (board.isDraw()) finalData += "0.5";
            else if (board.side == WHITE) finalData += "0"; // White was STM when game was over, so white loses
            else finalData += "1";

            while (bufferLock.load()) std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bufferLock.store(true);

            outputBuffer.push_back(DataUnit(finalData));

            bufferLock.store(false);
        }
        if (outputBuffer.size() > clearBufferEvery) {
            while (bufferLock.load()) std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bufferLock.store(true);

            writeToFile(filePath, outputBuffer);
            cachedPositions = 0;
            savedPositions += outputBuffer.size();
            outputBuffer.clear();

            bufferLock.store(false);
        }
    }
}

// ****** MULTITHREADING STUFF ******
std::vector<std::thread> threads;
std::atomic<bool> killThreads(false);

void startThreads(int n, Board& board, std::atomic<bool>& breakFlag) {
    IFDBG cout << "Attempting to start " << n << " new threads" << endl;
    killThreads.store(true);
    breakFlag.store(true);
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    threads.clear();
    killThreads.store(false);

    for (int i = 0; i < n; i++) threads.push_back(std::thread(runWorker, std::ref(board), std::ref(breakFlag), std::ref(killThreads)));
}

// ****** MAIN ENTRY POINT ******
int main(int argc, char* argv[]) {
    string command;
    std::deque<string> parsedCommand;
    Board currentPos;
    Precomputed::compute();
    initializeAllDatabases();
#if defined(_MSC_VER) && !defined(__clang__)
    nn.loadNetwork(EVALFILE);
#else
    nn = *reinterpret_cast<const NNUE*>(gEVALData);
#endif

    currentPos.reset();
    std::atomic<bool> breakFlag(false);
    std::thread searchThread;

    auto stopSearch = [&](bool killWorkers = false) {
        breakFlag.store(true);
        // Ensure the search thread is joined before exiting
        if (searchThread.joinable()) {
            searchThread.join();
        }
        if (killWorkers) {
            killThreads.store(true);
            for (auto& t : threads) {
                if (t.joinable()) {
                    t.join();
                }
            }
        }
        };

    if (argc > 1) {
        string arg1 = argv[1];
        if (arg1 == "bench") {
            bench(11);
        }
        else if (arg1 == "datagen") {
            playGames();
        }
        stopSearch(true);
        return 0;
    }

    // MAIN UCI LOOP
    cout << "Prelude ready and awaiting commands" << endl;
    while (true) {
        std::getline(std::cin, command);
        if (command == "") continue;
        parsedCommand = split(command, ' ');
        if (command == "uci") {
            isUci = true;
            cout << "id name Prelude" << endl;
            cout << "id author Quinniboi10" << endl;
            cout << "option name Threads type spin default 1 min 1 max 1024" << endl;
            cout << "option name Hash type spin default 16 min 1 max 4096" << endl;
            cout << "option name Move Overhead type spin default 20 min 0 max 1000" << endl;
            cout << "uciok" << endl;
        }
        else if (command == "isready") {
            cout << "readyok" << endl;
        }
        else if (command == "ucinewgame") {
            isUci = true;
            movesToGo = 20;

            stopSearch();

            TT.clear();
            for (auto& side : history) {
                for (auto& from : side) {
                    for (auto& to : from) {
                        to = 0;
                    }
                }
            }
        }
        else if (parsedCommand[0] == "setoption") {
            // Assumes setoption name ...
            if (parsedCommand[2] == "Hash") {
                TT = TranspositionTable(stof(parsedCommand[findIndexOf(parsedCommand, "value") + 1]));
            }
            else if (parsedCommand[2] == "Move" && parsedCommand[3] == "Overhead") {
                moveOverhead = stoi(parsedCommand[findIndexOf(parsedCommand, "value") + 1]);
            }
            else if (parsedCommand[2] == "Threads") {
                startThreads(stoi(parsedCommand[findIndexOf(parsedCommand, "value") + 1]) - 1, currentPos, breakFlag);
            }
        }
        else if (!parsedCommand.empty() && parsedCommand[0] == "position") { // Handle "position" command
            currentPos.reset();
            if (parsedCommand.size() > 3 && parsedCommand[2] == "moves") { // "position startpos moves ..."
                for (size_t i = 3; i < parsedCommand.size(); ++i) {
                    currentPos.move(parsedCommand[i]);
                }
            }
            else if (parsedCommand[1] == "fen") { // "position fen ..."
                parsedCommand.pop_front(); // Pop 'position'
                parsedCommand.pop_front(); // Pop 'fen'
                currentPos.loadFromFEN(parsedCommand);
            }
            if (parsedCommand.size() > 6 && parsedCommand[6] == "moves") {
                for (size_t i = 7; i < parsedCommand.size(); ++i) {
                    currentPos.move(parsedCommand[i]);
                }
            }
            else if (parsedCommand[1] == "kiwipete") currentPos.loadFromFEN(split("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", ' '));
        }
        else if (command == "d") {
            currentPos.display();
        }
        else if (parsedCommand.size() > 1 && parsedCommand[0] == "move") {
            currentPos.move(parsedCommand[1]);
        }
        else if (!parsedCommand.empty() && parsedCommand[0] == "go") { // Handle "go" command
            // If the main search thread is already running (it shouldn't), wait for it to finish
            stopSearch();

            auto exists = [&](string sub) { return command.find(sub) != string::npos; };
            auto index = [&](string sub, int offset = 0) { return findIndexOf(parsedCommand, sub) + offset; };

            auto getValueFromUCI = [&](string value, int defaultValue) { return exists(value) ? stoi(parsedCommand[index(value, 1)]) : defaultValue; };


            int depth = getValueFromUCI("depth", INF_INT);

            int maxNodes = getValueFromUCI("nodes", -1);
            int maxSoftNodes = getValueFromUCI("softnodes", -1);

            int wtime = getValueFromUCI("wtime", 0);
            int btime = getValueFromUCI("btime", 0);

            int mtime = getValueFromUCI("movetime", 0);

            int winc = getValueFromUCI("winc", 0);
            int binc = getValueFromUCI("binc", 0);


            searchThread = std::thread(iterativeDeepening<true>,
                std::ref(currentPos),
                depth,
                std::ref(breakFlag),
                wtime,
                btime,
                mtime,
                winc,
                binc,
                maxNodes,
                maxSoftNodes);
        }
        else if (command == "stop") {
            stopSearch();
        }
        else if (command == "quit") {
            stopSearch();
            return 0;
        }

        // ***** DEBUG COMMANDS ******
        else if (command == "debug.gamestate") {
            string bestMoveAlgebra;
            int whiteKing = ctzll(currentPos.white[5]);
            int blackKing = ctzll(currentPos.black[5]);
            cout << "Is in check (white): " << currentPos.isUnderAttack(WHITE, whiteKing) << endl;
            cout << "Is in check (black): " << currentPos.isUnderAttack(BLACK, blackKing) << endl;
            cout << "En passant square: " << (ctzll(currentPos.enPassant) < 64 ? squareToAlgebraic(ctzll(currentPos.enPassant)) : "-") << endl;
            cout << "Castling rights: " << std::bitset<4>(currentPos.castlingRights) << endl;
            MoveList moves = currentPos.generateLegalMoves();
            moves.sortByString(currentPos);
            cout << "Legal moves (" << moves.count << "):" << endl;
            for (Move m : moves) {
                cout << m.toString() << endl;
            }
        }
        else if (parsedCommand[0] == "perft") {
            perft(currentPos, stoi(parsedCommand[1]), false);
        }
        else if (parsedCommand[0] == "bulk") {
            perft(currentPos, stoi(parsedCommand[1]), true);
        }
        else if (parsedCommand[0] == "perftsuite") {
            perftSuite(parsedCommand[1]);
        }
        else if (command == "debug.moves") {
            cout << "All moves (current side to move):" << endl;
            auto moves = currentPos.generateMoves();
            moves.sortByString(currentPos);
            for (Move m : moves) {
                cout << m.toString() << endl;
            }
        }
        else if (command == "debug.nullmove") {
            currentPos.makeNullMove();
        }
        else if (!parsedCommand.empty() && parsedCommand[0] == "bench") {
            int depth = 11; // Default depth

            if (parsedCommand.size() >= 2) {
                depth = stoi(parsedCommand[1]);
            }

            // Start benchmarking
            bench(depth);
        }
        else if (command == "debug.eval") {
            cout << "Evaluation (centipawns as current side): " << currentPos.evaluate() << endl;
        }
        else if (command == "debug.checkmask") {
            printBitboard(currentPos.checkMask);
        }
        else if (command == "debug.pinned") {
            printBitboard(currentPos.pinned);
        }
        else if (command == "debug.datagen") {
            playGames();
        }
        else if (parsedCommand[0] == "debug.see") {
            cout << "SEE evaluation: " << currentPos.see(Move(parsedCommand[1], currentPos), 0) << endl;
        }
        else if (parsedCommand[0] == "debug.attackers") {
            cout << "Attackers:" << endl;
            printBitboard(currentPos.attackersTo(Square(parseSquare(parsedCommand[1])), currentPos.pieces()));
        }
        else if (command == "debug.searchinfo") {
            cout << threads.size() << " helper threads running" << endl;
        }
        else if (command == "debug.popcnt") {
            cout << "White pawns: " << popcountll(currentPos.white[0]) << endl;
            cout << "White knigts: " << popcountll(currentPos.white[1]) << endl;
            cout << "White bishops: " << popcountll(currentPos.white[2]) << endl;
            cout << "White rooks: " << popcountll(currentPos.white[3]) << endl;
            cout << "White queens: " << popcountll(currentPos.white[4]) << endl;
            cout << "White king: " << popcountll(currentPos.white[5]) << endl;
            cout << endl;
            cout << "Black pawns: " << popcountll(currentPos.black[0]) << endl;
            cout << "Black knigts: " << popcountll(currentPos.black[1]) << endl;
            cout << "Black bishops: " << popcountll(currentPos.black[2]) << endl;
            cout << "Black rooks: " << popcountll(currentPos.black[3]) << endl;
            cout << "Black queens: " << popcountll(currentPos.black[4]) << endl;
            cout << "Black king: " << popcountll(currentPos.black[5]) << endl;
        }
        else {
            std::cerr << "Unknown command: " << command << std::endl;
        }
    }
    return 0;
}