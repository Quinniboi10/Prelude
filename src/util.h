#pragma once

#include <bit>
#include <vector>
#include <sstream>
#include <cassert>
#include <cmath>
#include <cstring>

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

// Function from stockfish
template<typename IntType>
inline IntType readLittleEndian(std::istream& stream) {
    IntType result;

    if (IS_LITTLE_ENDIAN)
        stream.read(reinterpret_cast<char*>(&result), sizeof(IntType));
    else {
        std::uint8_t                  u[sizeof(IntType)];
        std::make_unsigned_t<IntType> v = 0;

        stream.read(reinterpret_cast<char*>(u), sizeof(IntType));
        for (usize i = 0; i < sizeof(IntType); ++i)
            v = (v << 8) | u[sizeof(IntType) - i - 1];

        std::memcpy(&result, &v, sizeof(IntType));
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
constexpr u8     castleIndex(Color c, bool kingside) { return c == WHITE ? (kingside ? 3 : 2) : (kingside ? 1 : 0); }

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

// Fancy formats a time
inline string formatTime(u64 timeInMS) {
    long long seconds = timeInMS / 1000;
    long long hours   = seconds / 3600;
    seconds %= 3600;
    long long minutes = seconds / 60;
    seconds %= 60;

    string result;

    if (hours > 0)
        result += std::to_string(hours) + "h ";
    if (minutes > 0 || hours > 0)
        result += std::to_string(minutes) + "m ";
    if (seconds > 0 || minutes > 0 || hours > 0)
        result += std::to_string(seconds) + "s";
    if (result == "")
        return std::to_string(timeInMS) + "ms";
    return result;
}

inline string padStr(string str, usize target, u64 minPadding = 2) {
    u64 padding = std::max(target - str.length(), minPadding);
    for (u64 i = 0; i < padding; i++)
        str += " ";
    return str;
}

inline string padStrRIght(string str, usize target, u64 minPadding = 2) {
    u64 padding = std::max(target - str.length(), minPadding);
    for (u64 i = 0; i < padding; i++)
        str = " " + str;
    return str;
}

inline int findIndexOf(const auto arr, string entry) {
    auto it = std::find(arr.begin(), arr.end(), entry);
    if (it != arr.end()) {
        return std::distance(arr.begin(), it);  // Calculate the index
    }
    return -1;  // Not found
}

template<typename _Ty>
inline _Ty lerp(_Ty a, _Ty b, double coeff) {
    assert(coeff >= 0);
    assert(coeff <= 1);
    return a * coeff + b * (1 - coeff);
}

// Throws a segfault, useful for tracing the call stack
inline void segFault() {
    int* foo = (int*) 0;
    printf("%d\n", *foo);
}