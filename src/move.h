#pragma once

#include "precomputed.h"

class Board;

class Move {
    // Bit indexes of various things
    // See https://www.chessprogramming.org/Encoding_Moves
    // Does not use double pawn push flag
    // PROMO = 15
    // CAPTURE = 14
    // SPECIAL 1 = 13
    // SPECIAL 0 = 12
private:
    uint16_t move;
public:
    constexpr Move() {
        move = 0;
    }

    Move(string in, Board& board);

    constexpr Move(u8 startSquare, u8 endSquare, int flags = STANDARD_MOVE) {
        move = startSquare;
        move |= endSquare << 6;
        move |= flags << 12;
    }

    string toString() const;

    Square startSquare() const { return Square(move & 0b111111); }
    Square endSquare() const { return Square((move >> 6) & 0b111111); }

    MoveType typeOf() const { return MoveType(move >> 12); } // Return the flag bits

    bool isNull() const { return move == 0; }

    // This should return false if
    // Move is a capture of any kind
    // Move is a queen promotion
    // Move is a knight promotion
    bool isQuiet() const {
        return (typeOf() & CAPTURE) == 0 &&
            typeOf() != QUEEN_PROMO &&
            typeOf() != KNIGHT_PROMO
            ;
    }

    bool operator==(const Move other) const {
        return move == other.move;
    }
};

// Stores a move with an evaluation
struct MoveEvaluation {
    Move move;
    int eval;

    constexpr MoveEvaluation() {
        move = Move();
        eval = -INF_INT;
    }
    constexpr MoveEvaluation(Move m, int eval) {
        move = m;
        this->eval = eval;
    }

    constexpr auto operator<=>(const MoveEvaluation& other) const { return eval <=> other.eval; }
};

struct MoveList {
    array<Move, 218> moves;
    int count;

    constexpr MoveList() {
        count = 0;
    }

    auto begin() {
        return moves.begin();
    }

    auto end() {
        return moves.begin() + count;
    }

    void add(Move m) {
        moves[count++] = m;
    }

    Move get(int index) {
        return moves[index];
    }

    size_t find(const Move entry) {
        auto it = std::find(moves.begin(), moves.begin() + count, entry);
        if (it != moves.begin() + count) {
            return std::distance(moves.begin(), it);
        }
        return -1;
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