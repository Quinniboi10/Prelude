#pragma once

#include <bit>
#include <vector>
#include <sstream>
#include <cassert>

#include "types.h"

#define ctzll(x) std::countr_zero(x)

inline bool readBit(u64 bb, int sq) { return (1ULL << sq) & bb; }

template<bool value>
inline void setBit(auto& bitboard, usize index) {
    assert(index <= sizeof(bitboard) * 8);
    if constexpr (value)
        bitboard |= (1ULL << index);
    else
        bitboard &= ~(1ULL << index);
}

inline void printBitboard(u64 bitboard);

inline Square popLSB(auto& bb) {
    if (bb == 0) {
        int* foo = (int*) -1;
        printf("%d\n", *foo);
    }
    assert(bb > 0);
    Square sq = static_cast<Square>(ctzll(bb));
    bb &= bb - 1;
    return sq;
}

template<int dir>
inline u64 shift(u64 bb) {
    return dir > 0 ? bb << dir : bb >> -dir;
}

inline u64 shift(int dir, u64 bb) { return dir > 0 ? bb << dir : bb >> -dir; }

inline std::vector<string> split(const string& str, char delim) {
    std::vector<std::string> result;

    std::istringstream stream(str);

    for (std::string token{}; std::getline(stream, token, delim);) {
        if (token.empty())
            continue;

        result.push_back(token);
    }

    return result;
}

constexpr Rank rankOf(Square s) { return Rank(s >> 3); }
constexpr File fileOf(Square s) { return File(s & 0b111); }

constexpr Rank flipRank(Square s) { return Rank(s ^ 0b111000); }

constexpr Square toSquare(Rank rank, File file) { return static_cast<Square>((rank << 3) | file); }

// Takes square (h8) and converts it into a bitboard index (64)
constexpr Square parseSquare(const string square) { return static_cast<Square>((square.at(1) - '1') * 8 + (square.at(0) - 'a')); }

// Takes a square (64) and converts into algebraic notation (h8)
constexpr string squareToAlgebraic(int sq) { return string(1, 'a' + (sq % 8)) + string(1, '1' + (sq / 8)); };

// Returns the end position for castling (uses FRC)
constexpr Square castleSq(Color c, bool kingside) { return c == WHITE ? (kingside ? h1 : a1) : (kingside ? h8 : a8); }

// Print a bitboard (for debugging individual bitboards)
inline void printBitboard(u64 bitboard) {
    for (int rank = 7; rank >= 0; --rank) {
        cout << "+---+---+---+---+---+---+---+---+" << endl;
        for (int file = 0; file < 8; ++file) {
            int  i            = rank * 8 + file;  // Map rank and file to bitboard index
            char currentPiece = readBit(bitboard, i) ? '1' : ' ';

            cout << "| " << currentPiece << " ";
        }
        cout << "|" << endl;
    }
    cout << "+---+---+---+---+---+---+---+---+" << endl;
}

// Formats a number with commas
inline string formatNum(i64 v) {
    auto s = std::to_string(v);

    int n = s.length() - 3;
    if (v < 0)
        n--;
    while (n > 0) {
        s.insert(n, ",");
        n -= 3;
    }

    return s;
}

// Throws a segfault, useful for tracing the call stack
inline void segFault() {
        int* foo = (int*) -1;
        printf("%d\n", *foo);
}