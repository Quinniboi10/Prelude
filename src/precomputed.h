#pragma once

#include <random>

// Class with precomputed constants
class Precomputed {
public:
    inline static array<array<array<u64, 64>, 12>, 2> zobrist;
    // EP zobrist is 65 because ctzll of 0 returns 64
    inline static array<u64, 65> zobristEP;
    inline static array<u64, 16> zobristCastling;
    inline static array<u64, 2>  zobristSide;
    inline static constexpr u64  isOnA = 0x101010101010101;
    inline static constexpr u64  isOnB = 0x202020202020202;
    inline static constexpr u64  isOnC = 0x404040404040404;
    inline static constexpr u64  isOnD = 0x808080808080808;
    inline static constexpr u64  isOnE = 0x1010101010101010;
    inline static constexpr u64  isOnF = 0x2020202020202020;
    inline static constexpr u64  isOnG = 0x4040404040404040;
    inline static constexpr u64  isOnH = 0x8080808080808080;
    inline static constexpr u64  isOn1 = 0xff;
    inline static constexpr u64  isOn2 = 0xff00;
    inline static constexpr u64  isOn3 = 0xff0000;
    inline static constexpr u64  isOn4 = 0xff000000;
    inline static constexpr u64  isOn5 = 0xff00000000;
    inline static constexpr u64  isOn6 = 0xff0000000000;
    inline static constexpr u64  isOn7 = 0xff000000000000;
    inline static constexpr u64  isOn8 = 0xff00000000000000;
    inline static void           compute() {
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

        // If no EP square, set it to 0 (for trifold reasons)
        zobristEP[64] = 0;

        for (auto& castlingValue : zobristCastling) {
            castlingValue = dist(engine);
        }

        for (auto& sideValue : zobristSide) {
            sideValue = dist(engine);
        }
    }
};