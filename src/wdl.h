#pragma once

#include "board.h"

#include <cmath>

struct WinRateParams {
    double a;
    double b;
};

// From SF
inline WinRateParams winRateParams(const Board& board) {
    int material = board.count(PAWN) + 3 * board.count(KNIGHT) + 3 * board.count(BISHOP) + 5 * board.count(ROOK) + 9 * board.count(QUEEN);

    // The fitted model only uses data for material counts in [17, 78], and is anchored at count 58.
    double m = std::clamp(material, 17, 78) / 58.0;

    // Return a = p_a(material) and b = p_b(material), see github.com/official-stockfish/WDL_model
    constexpr double as[] = {2177.30347733, -5690.74324009, 4046.88245374, 217.96867263};
    constexpr double bs[] = {65.21635672, 25.03770894, -414.88998313, 719.74678223};

    double a = (((as[0] * m + as[1]) * m + as[2]) * m) + as[3];
    double b = (((bs[0] * m + bs[1]) * m + bs[2]) * m) + bs[3];

    return {a, b};
}

// The win rate model is 1 / (1 + exp((a - eval) / b)), where a = p_a(material) and b = p_b(material).
// It fits the LTC fishtest statistics rather accurately.
inline int winRateModel(int v, const Board& board) {
    auto [a, b] = winRateParams(board);

    // Return the win rate in per mille units, rounded to the nearest integer.
    return int(0.5 + 1000 / (1 + std::exp((a - static_cast<double>(v)) / b)));
}

inline int scaleEval(int eval, const Board& board) {
    return eval;
    auto [a, b] = winRateParams(board);

    return std::round(100 * eval / a);
}
