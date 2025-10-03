#pragma once

#include <bit>
#include <vector>
#include <sstream>
#include <cassert>
#include <cstring>
#include <string_view>

#include "types.h"
#include "../external/fmt/fmt/color.h"
#include "../external/fmt/fmt/format.h"

#include <algorithm>

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
    assert(bb > 0);
    const auto sq = static_cast<Square>(ctzll(bb));
    bb &= bb - 1;
    return sq;
}

inline Square getLSB(auto bb) {
    assert(bb > 0);
    return static_cast<Square>(ctzll(bb));
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

inline string mergeFromIndex(const std::vector<string>& arr, usize n) {
    std::ostringstream oss;
    for (usize i = n; i < arr.size(); ++i) {
        if (i > n)
            oss << ' ';
        oss << arr[i];
    }
    return oss.str();
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

template<typename T, typename U>
inline void deepFill(T& dest, const U& val) {
    dest = val;
}

template<typename T, usize N, typename U>
inline void deepFill(std::array<T, N>& arr, const U& value) {
    for (auto& element : arr) {
        deepFill(element, value);
    }
}


constexpr Rank rankOf(Square s) { return Rank(s >> 3); }
constexpr File fileOf(Square s) { return File(s & 0b111); }

constexpr Rank flipRank(Square s) { return Rank(s ^ 0b111000); }

constexpr Square toSquare(Rank rank, File file) { return static_cast<Square>((static_cast<int>(rank) << 3) | file); }

// Takes square (h8) and converts it into a bitboard index (64)
constexpr Square parseSquare(const std::string_view square) { return static_cast<Square>((square.at(1) - '1') * 8 + (square.at(0) - 'a')); }

// Takes a square (64) and converts into algebraic notation (h8)
inline string squareToAlgebraic(int sq) { return fmt::format("{}{}", static_cast<char>('a' + (sq % 8)), static_cast<char>('1' + (sq / 8))); }

constexpr u8 castleIndex(Color c, bool kingside) { return c == WHITE ? (kingside ? 3 : 2) : (kingside ? 1 : 0); }

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

// Suffix a number
inline std::pair<string, char> suffixNum(double num, bool uppercase = true) {
    char suffix = ' ';
    if (num >= static_cast<double>(1'000'000'000) * 10) {
        num /= 1'000'000'000;
        if (uppercase)
            suffix = 'G';
        else
            suffix = 'g';
    }
    else if (num >= 1'000'000 * 10) {
        num /= 1'000'000;
        if (uppercase)
            suffix = 'M';
        else
            suffix = 'm';
    }
    else if (num >= 1'000 * 10) {
        num /= 1'000;
        if (uppercase)
            suffix = 'K';
        else
            suffix = 'k';
    }

    return { fmt::format("{:.2f}", num), suffix };
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
    if (result.empty())
        return std::to_string(timeInMS) + "ms";
    return result;
}

// Score color
inline void printColoredScore(i32 score) {
    double colorWdl = std::clamp(score / 500.0, -1.0, 1.0);
    int    r, g, b;

    const auto lerp = [](const double a, const double b, const double t) { return a + t * (b - a); };

    if (colorWdl < 0) {
        double t = colorWdl + 1.0;
        r        = static_cast<int>(lerp(200, 255, t));  // red stays max
        g        = static_cast<int>(lerp(0, 200, t));    // green rises
        b        = static_cast<int>(lerp(0, 200, t));    // blue rises
    }
    else {
        double t = colorWdl;                             // maps 0 -> 1
        r        = static_cast<int>(lerp(200, 0, t));    // red drops
        g        = static_cast<int>(lerp(200, 255, t));  // green stays max
        b        = static_cast<int>(lerp(200, 0, t));    // blue drops
    }

    fmt::print(fmt::fg(fmt::rgb(r, g, b)), "{:+.2f}", score / 100.0f);
}

inline string padStr(string str, i64 target, u64 minPadding = 2) {
    i64 padding = std::max<i64>(target - static_cast<i64>(str.length()), minPadding);
    for (i64 i = 0; i < padding; i++)
        str += " ";
    return str;
}

inline int findIndexOf(const auto arr, string entry) {
    auto it = std::find(arr.begin(), arr.end(), entry);
    if (it != arr.end()) {
        return std::distance(arr.begin(), it);  // Calculate the index
    }
    return -1;  // Not found
}