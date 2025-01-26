#pragma once

#include <deque>
#include <format>
#include <sstream>
#include <cassert>

#include "precomputed.h"

#define exit(code) \
    cout << "Exit from line " << __LINE__ << endl; \
    exit(code);

#define ctzll(x) std::countr_zero(x)
#define popcountll(x) std::popcount(x)

#ifdef DEBUG
constexpr bool ISDBG = true;
#else
constexpr bool ISDBG = false;
#endif
#define IFDBG if constexpr (ISDBG)

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

// Split a string into a deque given a deliminer to split by
inline std::deque<string> split(const string& s, char delim) {
    std::deque<string> result;
    std::stringstream ss(s);
    string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }
    return result;
}

// Find how much to pad a string with a minimum padding of 2 spaces
inline int getPadding(string str, int targetLen) {
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
inline void printBitboard(u64 bitboard) {
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
inline string formatTime(u64 timeInMS) {
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
inline string formatNum(i64 v) {
    auto s = std::to_string(v);

    int n = s.length() - 3;
    if (v < 0) n--;
    while (n > 0) {
        s.insert(n, ",");
        n -= 3;
    }

    return s;
}

//Reverses a bitboard
inline u64 reverse(u64 b) {
    b = (b & 0x5555555555555555) << 1 | (b >> 1) & 0x5555555555555555;
    b = (b & 0x3333333333333333) << 2 | (b >> 2) & 0x3333333333333333;
    b = (b & 0x0f0f0f0f0f0f0f0f) << 4 | (b >> 4) & 0x0f0f0f0f0f0f0f0f;
    b = (b & 0x00ff00ff00ff00ff) << 8 | (b >> 8) & 0x00ff00ff00ff00ff;

    return (b << 48) | ((b & 0xffff0000) << 16) | ((b >> 16) & 0xffff0000) | (b >> 48);
}

// Abbreviates a number into a string (1.00 gnodes instead of 1,000,000,000 nodes)
inline string abbreviateNum(const i64 v) {
    if (v > 1000000000) return std::format("{:.2f} g", v / 1000000000.0);
    if (v > 1000000) return std::format("{:.2f} m", v / 1000000.0);
    if (v > 1000) return std::format("{:.2f} k", v / 1000.0);

    return std::to_string(v);
}

// Shift the bitboard in the given diection
template<Direction shiftDir>
u64 shift(u64 bb) {
    if constexpr (shiftDir < 0) {
        return bb >> -shiftDir;
    }
    return bb << shiftDir;
}

// Returns the rank or file of the given square
constexpr Rank rankOf(Square s) { return Rank(s >> 3); }
constexpr File fileOf(Square s) { return File(s & 0b111); }

constexpr Rank flipRank(Square s) { return Rank(s ^ 0b111000); }

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

// Function from stockfish
template<typename IntType>
inline IntType read_little_endian(std::istream& stream) {
    IntType result;

    if (IS_LITTLE_ENDIAN)
        stream.read(reinterpret_cast<char*>(&result), sizeof(IntType));
    else
    {
        std::uint8_t                  u[sizeof(IntType)];
        std::make_unsigned_t<IntType> v = 0;

        stream.read(reinterpret_cast<char*>(u), sizeof(IntType));
        for (std::size_t i = 0; i < sizeof(IntType); ++i)
            v = (v << 8) | u[sizeof(IntType) - i - 1];

        std::memcpy(&result, &v, sizeof(IntType));
    }

    return result;
}