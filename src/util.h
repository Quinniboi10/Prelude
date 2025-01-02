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

#pragma once

#define m_assert(expr, msg) assert(( (void)(msg), (expr) ))

#define ctzll(x) std::countr_zero(x)
#define popcountll(x) std::popcount(x)

#define DEBUG true
#define IFDBG if constexpr (DEBUG)

using std::cerr;
using std::string;
using std::array;
using std::cout;
using std::endl;

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;

using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;

constexpr u64 INF = std::numeric_limits<uint64_t>::max();
constexpr int INF_INT = std::numeric_limits<int>::max();

static inline const union {
    uint32_t i;
    char     c[4];
} Le = { 0x01020304 };
static inline const bool IsLittleEndian = (Le.c[0] == 4);

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


std::deque<string> split(const string& s, char delim) {
    std::deque<string> result;
    std::stringstream ss(s);
    string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }
    return result;
}