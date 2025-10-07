#pragma once

#include <cmath>

#include "board.h"
#include "search.h"

// From SF and SP
inline std::pair<double, double> winRateParams(const Board& board) {
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
inline std::pair<i32, i32> wdlModel(double score, const Board& board) {
    if (Search::isWin(score))
        return { 1000, 0 };
    else if (Search::isLoss(score))
        return { 0, 1000 };

    const auto [a, b] = winRateParams(board);

    return {
        static_cast<i32>(std::round(1000.0 / (1.0 + std::exp((a - score) / b)))),
        static_cast<i32>(std::round(1000.0 / (1.0 + std::exp((a + score) / b))))
    };
}

inline int scaleEval(int eval, const Board& board) {
    return eval;
    // This line disables eval scaling, it will be turned back on when the engine has better performance/converts better
    auto [a, b] = winRateParams(board);

    return std::round(100 * eval / a);
}