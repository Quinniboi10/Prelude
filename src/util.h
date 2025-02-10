#pragma once

#include <bit>
#include <vector>
#include <sstream>
#include <cassert>

#include "types.h"

inline array<u64, 8> FILE_BB = {0x101010101010101, 0x202020202020202, 0x404040404040404, 0x808080808080808, 0x1010101010101010, 0x2020202020202020, 0x4040404040404040, 0x8080808080808080};

inline bool readBit(u64 bb, int sq) {
    return (1ULL << sq) & bb;
}

template<bool value>
inline void setBit(auto& bitboard, int index) {
    assert(index <= sizeof(bitboard) * 8);
    if constexpr (value)
        bitboard |= (1ULL << index);
    else
        bitboard &= ~(1ULL << index);
}

template<Direction dir>
inline u64 shift(u64 bb) {
    return dir > 0 ? bb << dir : bb >> -dir;
}

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

// Takes square (h8) and converts it into a bitboard index (64)
static int parseSquare(const string square) { return (square.at(1) - '1') * 8 + (square.at(0) - 'a'); }

// Takes a square (64) and converts into algebraic notation (h8)
static string squareToAlgebraic(int sq) { return string(1, 'a' + (sq % 8)) + string(1, '1' + (sq / 8)); };
