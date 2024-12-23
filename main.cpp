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
// Nothing

#include <iostream>
#include <string>
#include <fstream>
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

#define ctzll(x) ((x) ? _tzcnt_u64(x) : 64)
#define clzll(x) ((x) ? __lzcnt64(x) : 64)
//#define popcountll(x) __popcnt64(x)

//#define ctzll(x) ((x) ? __builtin_ctzll(x) : 64)
//#define clzll(x) ((x) ? __builtin_clzll(x) : 64)
#define popcountll(x) __builtin_popcountll(x)

#define DEBUG true
#define IFDBG if constexpr (DEBUG)

using std::cerr;
using std::string;
using std::array;
using std::cout;
using std::endl;

#define m_assert(expr, msg) assert(( (void)(msg), (expr) ))

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;

using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;

constexpr u64 INF = std::numeric_limits<uint64_t>::max();
constexpr int INF_INT = std::numeric_limits<int>::max();


enum Color : int {
    WHITE = 1, BLACK = 0
};

//Inverts the color (WHITE -> BLACK) and (BLACK -> WHITE)
constexpr Color operator~(Color c) {
    return Color(c ^ 1);
}

enum PieceType : int {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NO_PIECE_TYPE
};

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
    NORTH = 8, NORTH_EAST = 9, EAST = 1, SOUTH_EAST = -7,
    SOUTH = -8, SOUTH_WEST = -9, WEST = -1, NORTH_WEST = 7,
    NORTH_NORTH = 16, SOUTH_SOUTH = -16
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

static inline const union {
    uint32_t i;
    char     c[4];
} Le = { 0x01020304 };
static inline const bool IsLittleEndian = (Le.c[0] == 4);

// Names binary encoding flags from Move class
enum MoveType {
    STANDARD_MOVE = 0, DOUBLE_PUSH = 0b1, CASTLE_K = 0b10, CASTLE_Q = 0b11, CAPTURE = 0b100, EN_PASSANT = 0b101, KNIGHT_PROMO = 0b1000, BISHOP_PROMO = 0b1001, ROOK_PROMO = 0b1010, QUEEN_PROMO = 0b1011, KNIGHT_PROMO_CAPTURE = 0b1100, BISHOP_PROMO_CAPTURE = 0b1101, ROOK_PROMO_CAPTURE = 0b1110, QUEEN_PROMO_CAPTURE = 0b1111
};

enum flags {
    UNDEFINED, FAILLOW, BETACUTOFF, EXACT
};

struct shifts {
    static inline int NORTH = 8;
    static inline int NORTH_EAST = 9;
    static inline int EAST = 1;
    static inline int SOUTH_EAST = -7;
    static inline int SOUTH = -8;
    static inline int SOUTH_WEST = -9;
    static inline int WEST = -1;
    static inline int NORTH_WEST = 7;

    static inline array<int, 8> dirs = { NORTH, NORTH_EAST, EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, WEST, NORTH_WEST };
    static inline array<int, 4> straightDirs = { NORTH, EAST, SOUTH, WEST };
    static inline array<int, 4> diagonalDirs = { NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST };
};

struct indexes {
    static inline int NORTH = 0;
    static inline int NORTH_EAST = 1;
    static inline int EAST = 2;
    static inline int SOUTH_EAST = 3;
    static inline int SOUTH = 4;
    static inline int SOUTH_WEST = 5;
    static inline int WEST = 6;
    static inline int NORTH_WEST = 7;
    static inline int all = 8;

    static inline array<int, 8> dirs = { NORTH, NORTH_EAST, EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, WEST, NORTH_WEST };
    static inline array<int, 4> straightDirs = { NORTH, EAST, SOUTH, WEST };
    static inline array<int, 4> diagonalDirs = { NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST };
};

static int parseSquare(const string& square) {
    return (square.at(1) - '1') * 8 + (square.at(0) - 'a'); // Calculate the index of any square
}

static string squareToAlgebraic(int sq) {
    return string(1, 'a' + (sq % 8)) + string(1, '1' + (sq / 8));
};

template <typename BitboardType>
static bool readBit(BitboardType bitboard, int index) {
    return (bitboard & (1ULL << index)) != 0;
}

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
public:
    uint16_t move;

    Move() {
        move = 0;
    }

    Move(string in, Board& board);

    Move(u8 startSquare, u8 endSquare, int flags = STANDARD_MOVE) {
        move = startSquare;
        move |= endSquare << 6;
        move |= flags << 12;
    }

    string toString();

    int startSquare() { return move & 0b111111; }
    int endSquare() { return (move >> 6) & 0b111111; }

    MoveType typeOf() { return MoveType(move >> 12); } // Return the flag bits

    bool isNull() { return !move; }
};

std::deque<string> split(const string& s, char delim) {
    std::deque<string> result;
    std::stringstream ss(s);
    string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }
    return result;
}

template<typename arrType>
int findIndexOf(const arrType arr, string entry) {
    auto it = std::find(arr.begin(), arr.end(), entry);
    if (it != arr.end()) {
        return std::distance(arr.begin(), it); // Calculate the index
    }
    return -1; // Not found
}

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

string formatNum(u64 v) {
    auto s = std::to_string(v);

    int n = s.length() - 3;
    while (n > 0) {
        s.insert(n, ",");
        n -= 3;
    }
    return s;
}

class Precomputed {
public:
    static array<u64, 64> knightMoves;
    static array<u64, 64> kingMoves;
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

        // *** KNIGHT MOVES ***
        bool onNEdge;
        bool onNEdge2;
        bool onEEdge;
        bool onEEdge2;
        bool onSEdge;
        bool onSEdge2;
        bool onWEdge;
        bool onWEdge2;

        for (int i = 0; i < 64; ++i) {
            onNEdge = i / 8 == 7;
            onNEdge2 = i / 8 == 7 || i / 8 == 6;
            onEEdge = i % 8 == 7;
            onEEdge2 = i % 8 == 7 || i % 8 == 6;
            onSEdge = i / 8 == 0;
            onSEdge2 = i / 8 == 0 || i / 8 == 1;
            onWEdge = i % 8 == 0;
            onWEdge2 = i % 8 == 0 || i % 8 == 1;

            u64 positions = 0;

            if (!onNEdge2 && !onEEdge) setBit(positions, i + 17, 1); // Up 2, right 1
            if (!onNEdge && !onEEdge2) setBit(positions, i + 10, 1); // Up 1, right 2
            if (!onSEdge && !onEEdge2) setBit(positions, i - 6, 1);  // Down 1, right 2
            if (!onSEdge2 && !onEEdge) setBit(positions, i - 15, 1); // Down 2, right 1

            if (!onSEdge2 && !onWEdge) setBit(positions, i - 17, 1); // Down 2, left 1
            if (!onSEdge && !onWEdge2) setBit(positions, i - 10, 1); // Down 1, left 2
            if (!onNEdge && !onWEdge2) setBit(positions, i + 6, 1);  // Up 1, left 2
            if (!onNEdge2 && !onWEdge) setBit(positions, i + 15, 1); // Up 2, left 1

            knightMoves[i] = positions;
        }
        // *** KING MOVES ***
        for (int i = 0; i < 64; ++i) {
            onNEdge = (1ULL << i) & isOn8;
            onEEdge = (1ULL << i) & isOnH;
            onSEdge = (1ULL << i) & isOn1;
            onWEdge = (1ULL << i) & isOnA;

            u64 positions = 0;

            // Horizontal
            if (!onNEdge) setBit(positions, i + shifts::NORTH, 1);
            if (!onEEdge) setBit(positions, i + shifts::EAST, 1);
            if (!onSEdge) setBit(positions, i + shifts::SOUTH, 1);
            if (!onWEdge) setBit(positions, i + shifts::WEST, 1);

            // Diagonal
            if (!onNEdge && !onEEdge) setBit(positions, i + shifts::NORTH_EAST, 1);
            if (!onNEdge && !onWEdge) setBit(positions, i + shifts::NORTH_WEST, 1);
            if (!onSEdge && !onEEdge) setBit(positions, i + shifts::SOUTH_EAST, 1);
            if (!onSEdge && !onWEdge) setBit(positions, i + shifts::SOUTH_WEST, 1);

            kingMoves[i] = positions;
        }

        // *** MAKE RANDOM ZOBRIST TABLE ****
        std::random_device rd;

        std::mt19937_64 engine(rd());

        engine.seed(69420); // Nice

        std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<uint64_t>::max());

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

array<u64, 64> Precomputed::knightMoves;
array<u64, 64> Precomputed::kingMoves;
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


constexpr Rank rankOf(Square s) { return Rank(s >> 3); }
constexpr File fileOf(Square s) { return File(s & 0b111); }

constexpr Rank flipRank(Square s) { return Rank(s ^ 0b111000); }

// ****** MANY A PROGRAMMER HAS "BORROWED" CODE. I AM NO EXCEPTION ******
// Original code from https://github.com/nkarve/surge/blob/master/src/tables.cpp

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

struct MoveEvaluation {
    Move move;
    int eval;

    MoveEvaluation() {
        move = Move();
        eval = -INF_INT;
    }
    MoveEvaluation(Move m, int eval) {
        move = m;
        this->eval = eval;
    }

    bool operator>(const MoveEvaluation& other) const { return eval > other.eval; }
    bool operator<(const MoveEvaluation& other) const { return eval < other.eval; }
    bool operator>=(const MoveEvaluation& other) const { return eval >= other.eval; }
    bool operator<=(const MoveEvaluation& other) const { return eval <= other.eval; }
};

struct MoveList {
    array<Move, 218> moves;
    int count;

    MoveList() {
        count = 0;
    }

    void add(Move m) {
        moves[count++] = m;
    }

    Move get(int index) {
        return moves[index];
    }

    Move back() {
        return moves[count];
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
    i16 score;
    Move bestMove;
    u64 zobristKey;
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

    TranspositionTable(size_t sizeInMB = 16) {
        resize(sizeInMB);
    }

    bool isValidEntry(u64 zobristKey) {
        return (*getEntry(zobristKey)).zobristKey == zobristKey;
    }

    void clear() {
        table.clear();
    }

    void resize(size_t newSizeMB) {
        // Find number of bytes allowed
        size = newSizeMB * 1000 * 1000;
        // Divide by size of transposition entry
        size /= sizeof(Transposition);
        // Add one just to make sure to avoid div by 0
        size += 1;
        table.resize(size);
    }

    int index(u64 key, int size) {
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
constexpr size_t HL_SIZE = 384;


using Accumulator = array<i16, HL_SIZE>;
// NNUE is not UEd yet
class NNUE {
public:

    array<i16, HL_SIZE * 768> weightsToHL;

    array<i16, HL_SIZE> hiddenLayerBias;

    array<i16, HL_SIZE * 2> weightsToOut;
    i16 outputBias;

    i32 SCReLU(i16 x) {
        if (x < 0) x = 0;
        else if (x > inputQuantizationValue) x = inputQuantizationValue;
        return x * x;
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

        if (IsLittleEndian)
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

public:
    NNUE() {
        weightsToHL.fill(1);
        weightsToOut.fill(1);

        hiddenLayerBias.fill(0);
        outputBias = 0;
    }

    void loadNet(const std::string& filepath) {
        std::ifstream stream(filepath, std::ios::binary);
        if (!stream.is_open()) {
            throw std::runtime_error("Failed to open file: " + filepath);
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
            weightsToOut[i] = read_little_endian<int16_t>(stream);
        }

        // Load outputBias
        outputBias = read_little_endian<int16_t>(stream);

        cout << "Network loaded successfully from " << filepath << endl;
    }

    int forwardPass(Board* board);
};


u64 nodes = 0;
int moveOverhead = 20;
int movesToGo = 20;
TranspositionTable TT;
NNUE nn;
int msInfoInterval = 1000;
std::chrono::high_resolution_clock::time_point lastInfo;


class Board {
public:
    array<u64, 6> white; // Goes pawns, knights, bishops, rooks, queens, king
    array<u64, 6> black; // Goes pawns, knights, bishops, rooks, queens, king

    u64 blackPieces;
    u64 whitePieces;
    u64 emptySquares;

    uint64_t enPassant;
    u8 castlingRights;

    Color side = WHITE;

    int halfMoveClock;
    int fullMoveClock;

    bool doubleCheck = false;
    u64 checkMask = 0;
    u64 pinned = 0;

    std::vector<u64> positionHistory;
    u64 zobrist;

    Accumulator whiteAccum;
    Accumulator blackAccum;

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

        emptySquares = ~(whitePieces | blackPieces);

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

    int getPieceValue(PieceType pt) {
        switch (pt) {
        case PAWN: return 100;
        case KNIGHT: return 300;
        case BISHOP: return 300;
        case ROOK: return 500;
        case QUEEN: return 800;
        default: return 0;
        }
    }

    void generatePawnMoves(MoveList& moves) {
        u64 pawns = side ? white[0] : black[0];

        u64 pawnPushes = side ? (white[0] << 8) : (black[0] >> 8);
        pawnPushes &= emptySquares;
        int currentIndex;

        u64 pawnCaptureRight = pawns & ~(Precomputed::isOnH);
        pawnCaptureRight = side ? (pawnCaptureRight << 9) : (pawnCaptureRight >> 7);
        pawnCaptureRight &= side ? blackPieces : whitePieces;

        u64 pawnCaptureLeft = pawns & ~(Precomputed::isOnA);
        pawnCaptureLeft = side ? (pawnCaptureLeft << 7) : (pawnCaptureLeft >> 9);
        pawnCaptureLeft &= side ? blackPieces : whitePieces;

        u64 pawnCaptureRightEP = pawns & ~(Precomputed::isOnH);
        pawnCaptureRightEP = side ? (pawnCaptureRightEP << 9) : (pawnCaptureRightEP >> 7);
        pawnCaptureRightEP &= enPassant;

        u64 pawnCaptureLeftEP = pawns & ~(Precomputed::isOnA);
        pawnCaptureLeftEP = side ? (pawnCaptureLeftEP << 7) : (pawnCaptureLeftEP >> 9);
        pawnCaptureLeftEP &= enPassant;

        u64 pawnDoublePush = side ? (white[0] << 16) : (black[0] >> 16);
        pawnDoublePush &= emptySquares & (side ? (emptySquares << 8) : (emptySquares >> 8));
        pawnDoublePush &= side ? (Precomputed::isOn2 << 16) : (Precomputed::isOn7 >> 16);



        while (pawnDoublePush) {
            currentIndex = ctzll(pawnDoublePush);
            moves.add(Move((side) * (currentIndex - 16) + (!side) * (currentIndex + 16), currentIndex, DOUBLE_PUSH));

            pawnDoublePush &= pawnDoublePush - 1;
        }

        while (pawnPushes) {
            currentIndex = ctzll(pawnPushes);
            if ((1ULL << currentIndex) & (Precomputed::isOn1 | Precomputed::isOn8)) {
                moves.add(Move((side) * (currentIndex - 8) + (!side) * (currentIndex + 8), currentIndex, QUEEN_PROMO));
                moves.add(Move((side) * (currentIndex - 8) + (!side) * (currentIndex + 8), currentIndex, ROOK_PROMO));
                moves.add(Move((side) * (currentIndex - 8) + (!side) * (currentIndex + 8), currentIndex, BISHOP_PROMO));
                moves.add(Move((side) * (currentIndex - 8) + (!side) * (currentIndex + 8), currentIndex, KNIGHT_PROMO));
            }
            else moves.add(Move((side) * (currentIndex - 8) + (!side) * (currentIndex + 8), currentIndex));

            pawnPushes &= pawnPushes - 1;
        }

        while (pawnCaptureRight) {
            currentIndex = ctzll(pawnCaptureRight);
            if ((1ULL << currentIndex) & (Precomputed::isOn1 | Precomputed::isOn8)) {
                moves.add(Move((side) * (currentIndex - 9) + (!side) * (currentIndex + 7), currentIndex, QUEEN_PROMO_CAPTURE));
                moves.add(Move((side) * (currentIndex - 9) + (!side) * (currentIndex + 7), currentIndex, ROOK_PROMO_CAPTURE));
                moves.add(Move((side) * (currentIndex - 9) + (!side) * (currentIndex + 7), currentIndex, BISHOP_PROMO_CAPTURE));
                moves.add(Move((side) * (currentIndex - 9) + (!side) * (currentIndex + 7), currentIndex, KNIGHT_PROMO_CAPTURE));
            }
            else moves.add(Move((side) * (currentIndex - 9) + (!side) * (currentIndex + 7), currentIndex, CAPTURE));

            pawnCaptureRight &= pawnCaptureRight - 1;
        }

        while (pawnCaptureLeft) {
            currentIndex = ctzll(pawnCaptureLeft);
            if ((1ULL << currentIndex) & (Precomputed::isOn1 | Precomputed::isOn8)) {
                moves.add(Move((side) * (currentIndex - 7) + (!side) * (currentIndex + 9), currentIndex, QUEEN_PROMO_CAPTURE));
                moves.add(Move((side) * (currentIndex - 7) + (!side) * (currentIndex + 9), currentIndex, ROOK_PROMO_CAPTURE));
                moves.add(Move((side) * (currentIndex - 7) + (!side) * (currentIndex + 9), currentIndex, BISHOP_PROMO_CAPTURE));
                moves.add(Move((side) * (currentIndex - 7) + (!side) * (currentIndex + 9), currentIndex, KNIGHT_PROMO_CAPTURE));
            }
            else moves.add(Move((side) * (currentIndex - 7) + (!side) * (currentIndex + 9), currentIndex, CAPTURE));

            pawnCaptureLeft &= pawnCaptureLeft - 1;
        }

        while (pawnCaptureRightEP) {
            currentIndex = ctzll(pawnCaptureRightEP);
            moves.add(Move((side) * (currentIndex - 9) + (!side) * (currentIndex + 7), currentIndex, EN_PASSANT));

            pawnCaptureRightEP &= pawnCaptureRightEP - 1;
        }

        while (pawnCaptureLeftEP) {
            currentIndex = ctzll(pawnCaptureLeftEP);
            moves.add(Move((side) * (currentIndex - 7) + (!side) * (currentIndex + 9), currentIndex, EN_PASSANT));

            pawnCaptureLeftEP &= pawnCaptureLeftEP - 1;
        }
    }

    void generateKnightMoves(MoveList& moves) {
        int currentIndex;

        u64 knightBitboard = side ? white[1] : black[1];
        u64 ourBitboard = side ? whitePieces : blackPieces;

        while (knightBitboard > 0) {
            u64 knightEmptyMoves = 0;
            u64 knightCaptures = 0;

            currentIndex = ctzll(knightBitboard);

            u64 knightMoves = Precomputed::knightMoves[currentIndex];
            knightMoves &= ~ourBitboard;

            knightEmptyMoves = knightMoves & emptySquares;
            knightCaptures = knightMoves & ~(emptySquares | ourBitboard);

            while (knightEmptyMoves > 0) {
                moves.add(Move(currentIndex, ctzll(knightEmptyMoves)));
                knightEmptyMoves &= knightEmptyMoves - 1;
            }

            while (knightCaptures > 0) {
                moves.add(Move(currentIndex, ctzll(knightCaptures), CAPTURE));
                knightCaptures &= knightCaptures - 1;
            }
            knightBitboard &= knightBitboard - 1; // Clear least significant bit
        }
    }

    void generateBishopMoves(MoveList& moves) {
        int currentIndex;

        u64 bishopBitboard = side ? (white[2] | white[4]) : (black[2] | black[4]);
        u64 ourBitboard = side ? whitePieces : blackPieces;

        u64 occupancy = whitePieces | blackPieces;

        u64 moveMask;

        while (bishopBitboard > 0) {
            currentIndex = ctzll(bishopBitboard);

            moveMask = getBishopAttacks(Square(currentIndex), occupancy);

            moveMask &= ~ourBitboard;

            u64 emptyMoves = moveMask & emptySquares;
            u64 captures = moveMask & ~(emptySquares | ourBitboard);

            // Make move with each legal move in mask
            while (emptyMoves > 0) {
                int maskIndex = ctzll(emptyMoves);
                moves.add(Move(currentIndex, maskIndex));
                emptyMoves &= emptyMoves - 1;
            }
            while (captures > 0) {
                int maskIndex = ctzll(captures);
                moves.add(Move(currentIndex, maskIndex, CAPTURE));
                captures &= captures - 1;
            }

            bishopBitboard &= bishopBitboard - 1; // Clear least significant bit
        }
    }

    void generateRookMoves(MoveList& moves) {
        int currentIndex;

        u64 rookBitboard = side ? (white[3] | white[4]) : (black[3] | black[4]);
        u64 ourBitboard = side ? whitePieces : blackPieces;

        u64 occupancy = whitePieces | blackPieces;

        u64 moveMask;

        while (rookBitboard > 0) {
            currentIndex = ctzll(rookBitboard);

            moveMask = getRookAttacks(Square(currentIndex), occupancy);

            moveMask &= ~ourBitboard;

            u64 emptyMoves = moveMask & emptySquares;
            u64 captures = moveMask & ~(emptySquares | ourBitboard);

            // Make move with each legal move in mask
            while (emptyMoves > 0) {
                int maskIndex = ctzll(emptyMoves);
                moves.add(Move(currentIndex, maskIndex));
                emptyMoves &= emptyMoves - 1;
            }
            while (captures > 0) {
                int maskIndex = ctzll(captures);
                moves.add(Move(currentIndex, maskIndex, CAPTURE));
                captures &= captures - 1;
            }

            rookBitboard &= rookBitboard - 1; // Clear least significant bit
        }
    }

    void generateKingMoves(MoveList& moves) {
        u64 kingBitboard = side ? white[5] : black[5];
        IFDBG m_assert(kingBitboard, "King bitboard is 0");
        u64 ourBitboard = side ? whitePieces : blackPieces;
        int currentIndex = ctzll(kingBitboard);

        u64 kingMoves = Precomputed::kingMoves[currentIndex];
        kingMoves &= ~ourBitboard;


        u64 emptyMoves = kingMoves & emptySquares;
        u64 captures = kingMoves & ~(emptySquares | ourBitboard);

        // Make move with each legal move in mask
        while (emptyMoves > 0) {
            int maskIndex = ctzll(emptyMoves);
            moves.add(Move(currentIndex, maskIndex));
            emptyMoves &= emptyMoves - 1;
        }
        while (captures > 0) {
            int maskIndex = ctzll(captures);
            moves.add(Move(currentIndex, maskIndex, CAPTURE));
            captures &= captures - 1;
        }

        // Castling moves
        if (side && currentIndex == e1) {
            moves.add(Move(e1, g1, CASTLE_K));
            moves.add(Move(e1, c1, CASTLE_Q));
        }
        else if (!side && currentIndex == e8) {
            moves.add(Move(e8, g8, CASTLE_K));
            moves.add(Move(e8, c8, CASTLE_Q));
        }
    }

    int evaluateMVVLVA(Move& a) {
        int victim = getPieceValue(getPiece(a.endSquare()));
        int attacker = getPieceValue(getPiece(a.startSquare()));

        // Higher victim value and lower attacker value are prioritized
        return (victim * 100) - attacker;
    }

    MoveList generateMoves(bool capturesOnly = false) {
        MoveList allMoves;
        generateKingMoves(allMoves);

        if (doubleCheck) { // Returns early when double checks
            return allMoves;
        }

        MoveList captures, quietMoves;
        generatePawnMoves(allMoves);
        generateKnightMoves(allMoves);
        generateBishopMoves(allMoves);
        generateRookMoves(allMoves);
        // Queen moves are part of bishop/rook moves

        // Classify moves
        for (int i = 0; i < allMoves.count; ++i) {
            Move& move = allMoves.moves[i];

            if (move.typeOf() & CAPTURE) {
                captures.add(move);
            }
            else if (!capturesOnly) {
                quietMoves.add(move);
            }
        }

        Move bestMove = Move();
        Transposition& TTEntry = *TT.getEntry(zobrist);
        if (TTEntry.zobristKey == zobrist) {
            bestMove = TTEntry.bestMove;
        }

        // Initialize the prioritizedMoves list
        MoveList prioritizedMoves;

        // Add bestMove first, it will be removed as it is null
        prioritizedMoves.add(bestMove);

        std::sort(captures.moves.begin(), captures.moves.begin() + captures.count, [this](Move& a, Move& b) { return evaluateMVVLVA(a) > evaluateMVVLVA(b); });

        // Combine moves in the prioritized order
        for (int i = 0; i < captures.count; ++i) {
            prioritizedMoves.add(captures.moves[i]);
        }
        for (int i = 0; i < quietMoves.count; ++i) {
            prioritizedMoves.add(quietMoves.moves[i]);
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

    u64 pieces(PieceType p1, PieceType p2) {
        return white[p1] | white[p2] | black[p1] | black[p2];
    }

    u64 pieces(Color c) {
        return (c * whitePieces + ~c * blackPieces);
    }

    // Uses 2fold check
    bool isDraw() {
        if (halfMoveClock >= 100) return true;
        if (std::count(positionHistory.begin(), positionHistory.end(), zobrist) >= 2) {
            return true;
        }
        return false;
    }

    bool isInCheck(bool checkWhite) {
        u64 kingBit = checkWhite ? white[5] : black[5];
        if (!kingBit) return false;
        return isUnderAttack(checkWhite, ctzll(kingBit));
    }

    bool isUnderAttack(bool checkWhite, int square) {
        auto& opponentPieces = checkWhite ? black : white;

        // *** SLIDING PIECE ATTACKS ***
        u64 occupancy = whitePieces | blackPieces;

        // Straight Directions (Rooks and Queens)
        if ((opponentPieces[3] | opponentPieces[4]) & getRookAttacks(Square(square), occupancy)) return true;

        // Diagonal Directions (Bishops and Queens)
        if ((opponentPieces[2] | opponentPieces[4]) & getBishopAttacks(Square(square), occupancy)) return true;


        // *** KNIGHT ATTACKS ***
        if (opponentPieces[1] & Precomputed::knightMoves[square]) return true;

        // *** KING ATTACKS ***
        if (opponentPieces[5] & Precomputed::kingMoves[square]) return true;


        // *** PAWN ATTACKS ***
        if (checkWhite) {
            if ((opponentPieces[0] & (1ULL << (square + 7))) && (square % 8 != 0)) return true;
            if ((opponentPieces[0] & (1ULL << (square + 9))) && (square % 8 != 7)) return true;
        }
        else {
            if ((opponentPieces[0] & (1ULL << (square - 7))) && (square % 8 != 7)) return true;
            if ((opponentPieces[0] & (1ULL << (square - 9))) && (square % 8 != 0)) return true;
        }

        return false;
    }

    bool aligned(int from, int to, int test) {
        return (LINE[from][to] & (1ULL << test));
    }

    // Update checkers and pinners
    void updateCheckPin() {
        int kingIndex = ctzll(side ? white[5] : black[5]);

        u64 occ = whitePieces | blackPieces;
        u64 ourPieces = side ? whitePieces : blackPieces;
        auto& opponentPieces = side ? black : white;
        u64 enemyRooksQueens = side ? (black[3] | black[4]) : (white[3] | white[4]);
        u64 enemyBishopsQueens = side ? (black[2] | black[4]) : (white[2] | white[4]);

        // Direct attacks for potential checks
        u64 rookChecks = getRookAttacks(Square(kingIndex), occ) & enemyRooksQueens;
        u64 bishopChecks = getBishopAttacks(Square(kingIndex), occ) & enemyBishopsQueens;
        u64 checks = rookChecks | bishopChecks;
        checkMask = 0; // If no checks, will be set to all 1s later.

        // *** KNIGHT ATTACKS ***
        u64 knightAttacks = Precomputed::knightMoves[kingIndex] & opponentPieces[1];
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

        (popcountll(checks | checkMask) > 1) ? doubleCheck = true : doubleCheck = false;

        while (checks) {
            checkMask |= LINESEG[kingIndex][ctzll(checks)];
            checks &= checks - 1;
        }

        if (!checkMask) checkMask = INF; // If no checks, set to all ones

        // ****** PIN STUFF HERE ******
        u64 rookXrays = getXrayRookAttacks(Square(kingIndex), occ, ourPieces) & enemyRooksQueens;
        u64 bishopXrays = getXrayBishopAttacks(Square(kingIndex), occ, ourPieces) & enemyBishopsQueens;
        u64 pinners = rookXrays | bishopXrays;

        pinned = 0;
        while (pinners) {
            pinned |= LINESEG[ctzll(pinners)][kingIndex] & ourPieces;
            pinners &= pinners - 1;
        }
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
            if (isInCheck(side)) return false;
            u64 occupied = whitePieces | blackPieces;
            if (side) {
                if (kingside) {
                    if ((occupied & ((1ULL << f1) | (1ULL << g1))) != 0) return false;
                    if (isUnderAttack(true, f1) || isUnderAttack(true, g1)) return false;
                }
                else {
                    if ((occupied & ((1ULL << b1) | (1ULL << c1) | (1ULL << d1))) != 0) return false;
                    if (isUnderAttack(true, d1) || isUnderAttack(true, c1)) return false;
                }
            }
            else {
                if (kingside) {
                    if ((occupied & ((1ULL << f8) | (1ULL << g8))) != 0) return false;
                    if (isUnderAttack(false, f8) || isUnderAttack(false, g8)) return false;
                }
                else {
                    if ((occupied & ((1ULL << b8) | (1ULL << c8) | (1ULL << d8))) != 0) return false;
                    if (isUnderAttack(false, d8) || isUnderAttack(false, c8)) return false;
                }
            }
        }

        u64& king = side ? white[5] : black[5];
        int kingIndex = ctzll(king);

        // King moves
        if (king & (1ULL << from)) {
            u64 lastKing = king;
            u64& pieces = side ? whitePieces : blackPieces;
            u64 lastPieces = pieces;

            pieces ^= king;
            king = 0;

            bool ans = !isUnderAttack(side, to);
            pieces = lastPieces;
            king = lastKing;
            return ans;
        }

        // En passant check
        if (m.typeOf() == EN_PASSANT) {
            Board testBoard = *this;
            testBoard.move(m, false);
            return !testBoard.isInCheck(side);
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

                cout << "| " << currentPiece << " ";
            }
            cout << "| " << rank + 1 << endl;
        }
        cout << "+---+---+---+---+---+---+---+---+" << endl;
        cout << "  a   b   c   d   e   f   g   h" << endl;
        cout << endl;
        cout << "Current FEN: " << exportToFEN() << endl;
        cout << "Current key: 0x" << std::hex << std::uppercase << zobrist << std::dec << endl;
    }

    void placePiece(Color c, int pt, int square) {
        auto& us = c ? white : black;
        
        setBit(us[pt], square, 1);
        zobrist ^= Precomputed::zobrist[c][pt][square];

        // Extract the feature
        int inputFeatureWhite = NNUE::feature(WHITE, c, PieceType(pt), Square(square));
        int inputFeatureBlack = NNUE::feature(BLACK, c, PieceType(pt), Square(square));

        // Accumulate weights in the hidden layer
        for (int i = 0; i < HL_SIZE; i++) {
            whiteAccum[i] += nn.weightsToHL[inputFeatureWhite * HL_SIZE + i];
            blackAccum[i] += nn.weightsToHL[inputFeatureBlack * HL_SIZE + i];
        }
    }

    void removePiece(Color c, int pt, int square) {
        auto& us = c ? white : black;
        zobrist ^= Precomputed::zobrist[c][pt][square];

        // Extract the feature
        int inputFeatureWhite = NNUE::feature(WHITE, c, getPiece(square), Square(square));
        int inputFeatureBlack = NNUE::feature(BLACK, c, getPiece(square), Square(square));

        // Accumulate weights in the hidden layer
        for (int i = 0; i < HL_SIZE; i++) {
            whiteAccum[i] -= nn.weightsToHL[inputFeatureWhite * HL_SIZE + i];
            blackAccum[i] -= nn.weightsToHL[inputFeatureBlack * HL_SIZE + i];
        }

        setBit(us[pt], square, 0);
    }

    void removePiece(Color c, int square) {
        zobrist ^= Precomputed::zobrist[c][getPiece(square)][square];

        // Extract the feature
        int inputFeatureWhite = NNUE::feature(WHITE, c, getPiece(square), Square(square));
        int inputFeatureBlack = NNUE::feature(BLACK, c, getPiece(square), Square(square));

        // Accumulate weights in the hidden layer
        for (int i = 0; i < HL_SIZE; i++) {
            whiteAccum[i] -= nn.weightsToHL[inputFeatureWhite * HL_SIZE + i];
            blackAccum[i] -= nn.weightsToHL[inputFeatureBlack * HL_SIZE + i];
        }

        clearIndex(c, square);
    }

    void move(string& moveIn) {
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


        auto from = moveIn.startSquare();
        auto to = moveIn.endSquare();

        for (int i = 0; i < 6; ++i) {
            if (readBit(us[i], from)) {
                MoveType mt = moveIn.typeOf();

                removePiece(side, i, from);
                IFDBG m_assert(!readBit(us[i], from), "Position piece moved from was not cleared");

                enPassant = 0;

                switch (mt) {
                case STANDARD_MOVE: placePiece(side, i, to); break;
                case DOUBLE_PUSH:
                    placePiece(side, i, to);
                    enPassant = 1ULL << ((side) * (from + 8) + (!side) * (from - 8));
                    break;
                case CASTLE_K:
                    if (from == e1 && to == g1 && readBit(castlingRights, 3)) {
                        placePiece(side, i, to);

                        removePiece(side, ROOK, h1);
                        placePiece(side, ROOK, f1);
                    }
                    else if (from == e8 && to == g8 && readBit(castlingRights, 1)) {
                        placePiece(side, i, to);

                        removePiece(side, ROOK, h8);
                        placePiece(side, ROOK, f8);
                    }
                    break;
                case CASTLE_Q:
                    if (to == c1 && readBit(castlingRights, 2)) {
                        placePiece(side, i, to);

                        removePiece(side, ROOK, a1);
                        placePiece(side, ROOK, d1);
                    }
                    else if (to == c8 && readBit(castlingRights, 0)) {
                        placePiece(side, i, to);

                        removePiece(side, ROOK, a8);
                        placePiece(side, ROOK, d8);
                    }
                    break;
                case CAPTURE: removePiece(~side, to); placePiece(side, i, to); break;
                case QUEEN_PROMO_CAPTURE: removePiece(~side, to); placePiece(side, QUEEN, to); break;
                case ROOK_PROMO_CAPTURE: removePiece(~side, to); placePiece(side, ROOK, to); break;
                case BISHOP_PROMO_CAPTURE: removePiece(~side, to); placePiece(side, BISHOP, to); break;
                case KNIGHT_PROMO_CAPTURE: removePiece(~side, to); placePiece(side, KNIGHT, to); break;
                case EN_PASSANT:
                    if (side) {
                        removePiece(BLACK, PAWN, to + shifts::SOUTH);
                    }
                    else {
                        removePiece(WHITE, PAWN, to + shifts::NORTH);
                    }
                    placePiece(side, PAWN, to);
                    break;
                case QUEEN_PROMO: placePiece(side, QUEEN, to); break;
                case ROOK_PROMO: placePiece(side, ROOK, to); break;
                case BISHOP_PROMO: placePiece(side, BISHOP, to); break;
                case KNIGHT_PROMO: placePiece(side, KNIGHT, to); break;
                }

                // Halfmove clock, promo and set en passant
                if (i == 0 || readBit(blackPieces | whitePieces, to)) halfMoveClock = 0; // Reset halfmove clock on capture or pawn move
                else halfMoveClock++;

                break;
            }
        }

        // Remove castling rights (very unoptimized)
        if (castlingRights) {
            int from = moveIn.startSquare();
            int to = moveIn.endSquare();
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


        // Castling
        zobrist ^= Precomputed::zobristCastling[castlingRights];

        // En Passant
        zobrist ^= Precomputed::zobristEP[ctzll(enPassant)];

        // Turn
        zobrist ^= Precomputed::zobristSide[~side];

        // Only add 1 to full move clock if move was made as black
        fullMoveClock += ~side;

        side = ~side;

        recompute();
        updateCheckPin();
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

        for (Square sq = h8; sq >= a1; --sq) {
            if ((1ULL << (int)sq) & Precomputed::isOnH) {
                if (blankSquares / 12) ans += std::to_string(blankSquares / 12);
                blankSquares = 0;
                ans += "/";
            }
            for (int i = 0; i < 6; ++i) {
                if (readBit(white[i], sq)) {
                    if (blankSquares / 12) ans += std::to_string(blankSquares / 12); // Divide by 6 because it in incremented in a for loop for types
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

        ans.at(0) = '\0';
        ans.at(ans.length() - 1) = '\0';

        ans += side ? " w " : " b ";

        if (readBit(castlingRights, 3)) ans += "K";
        if (readBit(castlingRights, 2)) ans += "Q";
        if (readBit(castlingRights, 1)) ans += "k";
        if (readBit(castlingRights, 0)) ans += "q";

        ans += " ";

        if (enPassant) ans += squareToAlgebraic(ctzll(enPassant));
        else ans += "-";

        ans += " ";
        ans += std::to_string(halfMoveClock);
        ans += " ";
        ans += std::to_string(fullMoveClock);

        return ans;
    }

    constexpr int flipIndex(int index) {
        // Ensure the index is within [0, 63]
        IFDBG m_assert((index >= 0 && index <= 63), "Invalid index: " + std::to_string(index) + ". Must be between 0 and 63.");
        int rank = index / 8;
        int file = index % 8;
        int mirrored_rank = 7 - rank;
        return mirrored_rank * 8 + file;
    }

    int evaluate() { // Returns evaluation in centipawns as side to move
        int eval = 0;
        int whitePieces = 0;
        int blackPieces = 0;

        // Uses some magic python buffoonery https://github.com/ianfab/chess-variant-stats/blob/main/piece_values.py
        // Based on this https://discord.com/channels/435943710472011776/1300744461281787974/1312722964915027980

        // Material evaluation
        whitePieces += popcountll(white[0]) * 100;
        whitePieces += popcountll(white[1]) * 316;
        whitePieces += popcountll(white[2]) * 328;
        whitePieces += popcountll(white[3]) * 493;
        whitePieces += popcountll(white[4]) * 982;

        blackPieces += popcountll(black[0]) * 100;
        blackPieces += popcountll(black[1]) * 316;
        blackPieces += popcountll(black[2]) * 328;
        blackPieces += popcountll(black[3]) * 493;
        blackPieces += popcountll(black[4]) * 982;

        eval = whitePieces - blackPieces;

        // Only utilize the NNUE in situations when game isn't very won or lsot
        // Concept from SF
        if (std::abs(eval) < 950) {
            return nn.forwardPass(this);
        }

        // Adjust evaluation for the side to move
        return (side) ? eval : -eval;
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
    // Determine the side to move and the opposite side
    Color stm = board->side;
    
    Accumulator& accumulatorSTM = stm ? board->whiteAccum : board->blackAccum;
    Accumulator& accumulatorOPP = ~stm ? board->whiteAccum : board->blackAccum;

    // Accumulate output for STM and OPP using separate weight segments
    i64 eval = 0;

    for (int i = 0; i < HL_SIZE; i++) {
        // First HL_SIZE weights are for STM
        eval += SCReLU(accumulatorSTM[i]) * weightsToOut[i];

        // Last HL_SIZE weights are for OPP
        eval += SCReLU(accumulatorOPP[i]) * weightsToOut[HL_SIZE + i];
    }

    // Dequantization
    eval /= inputQuantizationValue;

    eval += outputBias;

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
    for (int i = 0; i < moves.count; ++i) {
        Board testBoard = board;
        testBoard.move(moves.moves[i]);
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
    for (int i = 0; i < moves.count; ++i) {
        Board testBoard = board;
        testBoard.move(moves.moves[i]);
        localNodes += _perft(testBoard, depth - 1);
    }
    return localNodes;
}

void perft(Board& board, int depth, bool bulk) {
    auto start = std::chrono::high_resolution_clock::now();

    if (depth < 1) return;
    if (depth == 1 && bulk) bulk = false;

    MoveList moves = board.generateLegalMoves();
    moves.sortByString(board);
    u64 localNodes = 0;
    u64 movesThisIter = 0;

    for (int i = 0; i < moves.count; ++i) {
        Board testBoard = board;
        testBoard.move(moves.moves[i]);
        movesThisIter = bulk ? _bulk(testBoard, depth - 1) : _perft(testBoard, depth - 1);
        localNodes += movesThisIter;
        cout << moves.moves[i].toString() << ": " << movesThisIter << endl;
    }

    int nps = (int)(((double)localNodes) / (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) * 1000000000);

    double timeTaken = (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count());
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


// ****** SEARCH FUNCTIONS ******
static MoveEvaluation _qs(Board& board,
    std::atomic<bool>& breakFlag,
    std::chrono::steady_clock::time_point& timerStart,
    int timeToSpend,
    int maxNodes,
    int alpha,
    int beta) {
    int stand_pat = board.evaluate();
    if (stand_pat >= beta) {
        return { Move(), beta };
    }
    if (alpha < stand_pat) {
        alpha = stand_pat;
    }

    if (board.isDraw()) {
        return { Move(), 0 };
    }

    MoveList moves = board.generateMoves(true);
    Move bestMove;

    int flag = FAILLOW;

    for (int i = 0; i < moves.count; ++i) {
        const Move& m = moves.moves[i];

        if (!board.isLegalMove(m)) continue;

        Board testBoard = board;
        testBoard.move(m);

        nodes++;

        int score;

        // Principal variation search stuff
        score = -(_qs(testBoard, breakFlag, timerStart, timeToSpend, maxNodes, -alpha - 1, -alpha).eval);
        // If it fails high or low we search again with the original bounds
        if (score > alpha && score < beta) {
            score = -(_qs(testBoard, breakFlag, timerStart, timeToSpend, maxNodes, -beta, -alpha).eval);
        }

        if (score >= beta) {
            return { Move(), beta };
        }
        if (score > alpha) {
            alpha = score;
            bestMove = m;
        }

        if (breakFlag.load()) break;
        if (nodes % 2048 == 0 && timeToSpend != 0) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - timerStart).count() >= timeToSpend) {
                break;
            }
        }
        if (maxNodes > 0 && nodes >= maxNodes) break;
    }

    if (TT.getEntry(board.zobrist)->zobristKey != board.zobrist) {
        // Pushes with depth of 0 so best move is saved, just always deleted by main search
        Transposition entry = Transposition(board.zobrist, bestMove, flag, alpha, 0);
        TT.setEntry(board.zobrist, entry);
    }

    return { bestMove, alpha };
}

// Full search function
static MoveEvaluation go(Board& board,
    int depth,
    std::atomic<bool>& breakFlag,
    std::chrono::steady_clock::time_point& timerStart,
    int timeToSpend,
    int alpha,
    int beta,
    int maxNodes,
    const bool isRoot = false) {
    // Only worry about draws and such if the node is a child, otherwise game would be over
    // More expensive to check isDraw() than to check !isRoot because isDraw() needs to search an array
    if (!isRoot) {
        if (depth == 0) {
            int eval = _qs(board, breakFlag, timerStart, timeToSpend, maxNodes, alpha, beta).eval;
            return { Move(), eval };
        }
        else if (board.isDraw()) {
            return { Move(), 0 };
        }
    }

    Transposition* entry = TT.getEntry(board.zobrist);

    if (!isRoot && entry->zobristKey == board.zobrist && entry->depth >= depth && (
        entry->flag == EXACT // Exact score
        || (entry->flag == BETACUTOFF && entry->score >= beta) // Lower bound, fail high
        || (entry->flag == FAILLOW && entry->score <= alpha) // Upper bound, fail low
        )) {
        return { Move(), entry->score };
    }

    MoveList moves = board.generateMoves();
    Move bestMove;
    int bestEval = -INF_INT;

    int flag = FAILLOW;

    int movesMade = 0;

    for (int i = 0; i < moves.count; ++i) {
        // Break checks
        if (alpha >= beta) break; // Alpha-beta pruning
        if (nodes % 2048 == 0 && breakFlag.load()) break;
        if (nodes % 2048 == 0 && timeToSpend != 0) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - timerStart).count() >= timeToSpend) {
                break;
            }
        }
        if (maxNodes > 0 && nodes >= maxNodes) break;


        Move& m = moves.moves[i];

        if (!board.isLegalMove(m)) {
            continue; // Validate legal moves
        }

        // Recursive call with a copied board
        Board testBoard = board;
        testBoard.move(m);
        movesMade++;

        nodes++;

        int eval;

        // Principal variation search stuff
        eval = -(go(testBoard, depth - 1, breakFlag, timerStart, timeToSpend, -alpha - 1, -alpha, maxNodes).eval);
        // If it fails high or low we search again with the original bounds
        if (eval > alpha && eval < beta) {
            eval = -(go(testBoard, depth - 1, breakFlag, timerStart, timeToSpend, -beta, -alpha, maxNodes).eval);
        }


        // Update best move and alpha-beta values
        if (eval > bestEval) {
            bestEval = eval;
            bestMove = m;
            alpha = std::max(bestEval, alpha);
        }

        // Fail high
        if (eval >= beta) {
            flag = BETACUTOFF;
        }

        if (isRoot) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastInfo).count() >= msInfoInterval) {
                int TTused = 0;
                for (int i = 0; i < TT.size; i++) {
                    if (TT.getEntry(i)->zobristKey != 0) TTused++;
                }
                double elapsedMs = (double)std::chrono::duration_cast<std::chrono::milliseconds>(now - timerStart).count();
                int nps = (int)((double)nodes / (elapsedMs / 1000));
                cout << "info depth " << depth << " nodes " << nodes << " nps " << nps << " time " << std::to_string((int)elapsedMs) << " hashfull " << (int)(TTused / (double)TT.size * 1000) << " currmove " << m.toString() << endl;
            }
        }
    }

    if (!movesMade) {
        if (board.isInCheck(board.side)) {
            return { Move(), -99999 };
        }
        return { Move(), 0 };
    }

    // Push to TT
    if (depth > 1) {
        // Only push if search was to higher depth or the zobrist should be removed
        if ((TT.getEntry(board.zobrist)->zobristKey != board.zobrist) || (depth > (*TT.getEntry(board.zobrist)).depth)) {
            Transposition entry = Transposition(board.zobrist, bestMove, flag, bestEval, depth);
            TT.setEntry(board.zobrist, entry);
        }
    }

    return { bestMove, bestEval };
}


void iterativeDeepening(
    Board& board,
    int maxDepth,
    std::atomic<bool>& breakFlag,
    int wtime = 0,
    int btime = 0,
    int mtime = 0,
    int winc = 0,
    int binc = 0,
    int maxNodes = -1
) {
    breakFlag.store(false);
    lastInfo = std::chrono::high_resolution_clock::now();
    nodes = 0;
    int timeToSpend = 0;
    int softLimit = 0;
    auto start = std::chrono::steady_clock::now();
    std::string bestMoveAlgebra = "";
    if (wtime || btime) {
        timeToSpend = board.side ? wtime / movesToGo : btime / movesToGo;
        softLimit = timeToSpend * 0.65;
    }
    else if (mtime) {
        timeToSpend = mtime;
        softLimit = timeToSpend;
    }

    // timeToSpend is a hard limit
    if (timeToSpend) {
        // 100ms margin for GUI stuff, etc.
        if (timeToSpend > 150) timeToSpend -= moveOverhead;
        // If there is less than 100 ms left, just search 1 ply ahead with quiescence search
        else maxDepth = 1;
    }

    MoveEvaluation move;
    MoveEvaluation bestMove;

    IFDBG m_assert(maxDepth >= 1, "Depth is less than 1 in ID search");

    // Cap the depth to 255
    maxDepth = std::min(255, maxDepth);

    softLimit = std::min(softLimit, timeToSpend);

    for (int depth = 1; depth <= maxDepth; depth++) {
        move = go(board, depth, breakFlag, start, timeToSpend, -INF_INT, INF_INT, maxNodes, true);

        IFDBG m_assert(!move.move.isNull(), "Returned null move in search");

        auto now = std::chrono::steady_clock::now();
        double elapsedNs = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(now - start).count();
        int nps = (int)((double)nodes / (elapsedNs / 1e9));

        // Discard searches that return null or illegal moves, usually caused by TT corruption or exiting search early before any search cycle has fisnished
        if (!move.move.isNull() && board.isLegalMove(move.move)) bestMove = move;

        bestMoveAlgebra = bestMove.move.toString();

        bool isMate = false;

        int TTused = 0;
        for (int i = 0; i < TT.size; i++) {
            if (TT.getEntry(i)->zobristKey != 0) TTused++;
        }

        string ans = "info depth " + std::to_string(depth) + " nodes " + std::to_string(nodes) + " nps " + std::to_string(nps) + " time " + std::to_string((int)(elapsedNs/1e6)) +  " hashfull " + std::to_string((int)(TTused / (double)TT.size * 1000));

        if (std::abs(bestMove.eval) >= 90000) {
            // Assume large positive value is mate
            ans += " score mate " + std::to_string((bestMove.eval > 0) ? (depth / 2) : -(depth / 2));
            isMate = true;
        }
        else {
            ans += " score cp " + std::to_string(bestMove.eval);
        }

        ans += " pv " + bestMoveAlgebra;

        cout << ans << endl;
        lastInfo = std::chrono::high_resolution_clock::now();

        if (breakFlag.load() && depth > 1) {
            IFDBG cout << "Stopping search because of break flag" << endl;
            break;
        }

        if (softLimit != 0 && depth > 1) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() >= softLimit) {
                IFDBG cout << "Stopping search because of time limit" << endl;
                break;
            }
        }

        if (maxNodes > 0 && maxNodes <= nodes && depth > 1) {
            IFDBG cout << "Stopping search because of node limit" << endl;
            break;
        }

        if (isMate) break;
    }

    std::cout << "bestmove " << bestMoveAlgebra << std::endl;
    breakFlag.store(false);
}


int main() {
    string command;
    std::deque<string> parsedcommand;
    Board currentPos;
    Precomputed::compute();
    initializeAllDatabases();
    nn.loadNet("C:\\Users\\qitag\\Downloads\\simple(2).nnue");
    currentPos.reset();
    std::atomic<bool> breakFlag(false);
    std::optional<std::thread> searchThreadOpt;
    cout << "Prelude ready and awaiting commands" << endl;
    while (true) {
        std::getline(std::cin, command);
        parsedcommand = split(command, ' ');
        if (command == "uci") {
            cout << "id name Prelude" << std::endl;
            cout << "id author Quinniboi10" << std::endl;
            cout << "option name Hash type spin default 16 min 1 max 4096" << endl;
            cout << "option name Move Overhead type spin default 20 min 0 max 1000" << endl;
            cout << "uciok" << std::endl;
        }
        else if (command == "isready") {
            cout << "readyok" << std::endl;
        }
        else if (command == "ucinewgame") {
            breakFlag.store(false);
            movesToGo = 20;
            if (searchThreadOpt.has_value()) {
                if (searchThreadOpt->joinable()) {
                    breakFlag.store(true);
                    searchThreadOpt->join();
                    breakFlag.store(false);
                }
                searchThreadOpt.reset();
            }
        }
        else if (parsedcommand.at(0) == "setoption") {
            // Assumes setoption name ...
            if (parsedcommand.at(2) == "Hash") {
                TT = TranspositionTable(stoi(parsedcommand.at(4)));
            }
            else if (parsedcommand.at(2) == "Move Overhead") {
                moveOverhead = (stoi(parsedcommand.at(3)));
            }
        }
        if (!parsedcommand.empty() && parsedcommand.at(0) == "position") { // Handle "position" command
            currentPos.reset();
            if (parsedcommand.size() > 3 && parsedcommand.at(2) == "moves") { // "position startpos moves ..."
                for (size_t i = 3; i < parsedcommand.size(); ++i) {
                    currentPos.move(parsedcommand.at(i));
                }
            }
            else if (parsedcommand.at(1) == "fen") { // "position fen ..."
                parsedcommand.pop_front(); // Pop 'position'
                parsedcommand.pop_front(); // Pop 'fen'
                currentPos.loadFromFEN(parsedcommand);
            }
            if (parsedcommand.size() > 6 && parsedcommand.at(6) == "moves") {
                for (size_t i = 7; i < parsedcommand.size(); ++i) {
                    currentPos.move(parsedcommand.at(i));
                }
            }
            else if (parsedcommand.at(1) == "kiwipete") { // "position kiwipete"
                currentPos.loadFromFEN(split("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", ' '));
            }
        }
        else if (command == "d") {
            currentPos.display();
        }
        else if (parsedcommand.size() > 1 && parsedcommand.at(0) == "move") {
            currentPos.move(parsedcommand.at(1));
        }
        else if (!parsedcommand.empty() && parsedcommand.at(0) == "go") { // Handle "go" command
            // If a search thread is already running, wait for it to finish
            if (searchThreadOpt.has_value()) {
                if (searchThreadOpt->joinable()) {
                    breakFlag.store(true);
                    searchThreadOpt->join();
                    breakFlag.store(false);
                }
                searchThreadOpt.reset();
            }

            int maxNodes = -1;
            int depth = INF_INT;

            int wtime = 0;
            int btime = 0;

            int mtime = 0;

            int winc = 0;
            int binc = 0;

            if (findIndexOf(parsedcommand, "nodes") > 0) {
                IFDBG cout << "Starting search with max nodes limit" << endl;
                maxNodes = stoi(parsedcommand.at(findIndexOf(parsedcommand, "nodes") + 1));
            }

            if (findIndexOf(parsedcommand, "depth") > 0) {
                depth = stoi(parsedcommand.at(findIndexOf(parsedcommand, "depth") + 1));
            }

            if (findIndexOf(parsedcommand, "wtime") > 0) {
                wtime = stoi(parsedcommand.at(findIndexOf(parsedcommand, "wtime") + 1));
            }

            if (findIndexOf(parsedcommand, "btime") > 0) {
                btime = stoi(parsedcommand.at(findIndexOf(parsedcommand, "btime") + 1));
            }

            if (findIndexOf(parsedcommand, "movetime") > 0) {
                mtime = stoi(parsedcommand.at(findIndexOf(parsedcommand, "movetime") + 1));
            }

            if (findIndexOf(parsedcommand, "winc") > 0) {
                winc = stoi(parsedcommand.at(findIndexOf(parsedcommand, "winc") + 1));
            }

            if (findIndexOf(parsedcommand, "binc") > 0) {
                binc = stoi(parsedcommand.at(findIndexOf(parsedcommand, "binc") + 1));
            }

            searchThreadOpt.emplace(iterativeDeepening,
                std::ref(currentPos),
                depth,
                std::ref(breakFlag),
                wtime,
                btime,
                mtime,
                winc,
                binc,
                maxNodes);
        }
        else if (command == "stop") {
            breakFlag.store(true);
            // Optionally, join the search thread to ensure it has stopped
            if (searchThreadOpt.has_value()) {
                if (searchThreadOpt->joinable()) {
                    searchThreadOpt->join();
                }
                searchThreadOpt.reset();
            }
        }
        else if (command == "quit") {
            breakFlag.store(true);
            // Ensure the search thread is joined before exiting
            if (searchThreadOpt.has_value()) {
                if (searchThreadOpt->joinable()) {
                    searchThreadOpt->join();
                }
            }
            return 0;
        }
        else if (command == "debug.gamestate") {
            string bestMoveAlgebra;
            cout << "Is in check (white): " << currentPos.isInCheck(WHITE) << endl;
            cout << "Is in check (black): " << currentPos.isInCheck(BLACK) << endl;
            cout << "En passant square: " << (ctzll(currentPos.enPassant) < 64 ? squareToAlgebraic(ctzll(currentPos.enPassant)) : "-") << endl;
            cout << "Castling rights: " << std::bitset<4>(currentPos.castlingRights) << endl;
            MoveList moves = currentPos.generateLegalMoves();
            moves.sortByString(currentPos);
            cout << "Legal moves (" << moves.count << "):" << endl;
            for (int i = 0; i < moves.count; ++i) {
                cout << moves.moves[i].toString() << endl;
            }
        }
        else if (parsedcommand.at(0) == "perft") {
            perft(currentPos, stoi(parsedcommand.at(1)), false);
        }
        else if (parsedcommand.at(0) == "bulk") {
            perft(currentPos, stoi(parsedcommand.at(1)), true);
        }
        else if (parsedcommand.at(0) == "perftsuite") {
            perftSuite(parsedcommand.at(1));
        }
        else if (command == "debug.moves") {
            cout << "All moves (current side to move):" << endl;
            auto moves = currentPos.generateMoves();
            moves.sortByString(currentPos);
            for (int i = 0; i < moves.count; ++i) {
                cout << moves.moves[i].toString() << endl;
            }
        }
        else if (command == "debug.eval") {
            cout << "Evaluation (centipawns as current side): " << currentPos.evaluate() << endl;
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
    }
    return 0;
}
