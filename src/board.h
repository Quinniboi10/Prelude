#pragma once

#include "movegen.h"
#include "nnue.h"
#include "move.h"
#include "transpostable.h"

class Board {
    friend int NNUE::forwardPass(const Board* board); // Needs to see accumulator
    friend Move::Move(string strIn, Board& board); // Must look at the board to set flags
    friend void playGames(); // For tracking move counts and such
    // Because of debug features in main(), all items must be public
//private:
public:
    array<u64, 6> white; // Goes pawns, knights, bishops, rooks, queens, king
    array<u64, 6> black; // Goes pawns, knights, bishops, rooks, queens, king

    u64 blackPieces;
    u64 whitePieces;

    uint64_t enPassant;
    u8 castlingRights;

    Color side = WHITE;

    int halfMoveClock;
    int fullMoveClock;

    bool doubleCheck = false;
    u64 checkMask = 0;
    u64 pinned = 0;
    array<u64, 2> pinnersPerC;

    std::vector<u64> positionHistory;
    u64 zobrist;

    alignas(32) Accumulator whiteAccum;
    alignas(32) Accumulator blackAccum;

    // Represents if the current board state is from a null move
    bool fromNull = false;

    void clearIndex(int index) {
        const u64 mask = ~(1ULL << index);
        white[0] &= mask;
        white[1] &= mask;
        white[2] &= mask;
        white[3] &= mask;
        white[4] &= mask;
        white[5] &= mask;

        black[0] &= mask;
        black[1] &= mask;
        black[2] &= mask;
        black[3] &= mask;
        black[4] &= mask;
        black[5] &= mask;
    }

    void clearIndex(const Color c, int index) {
        const u64 mask = ~(1ULL << index);
        if (c) {
            white[0] &= mask;
            white[1] &= mask;
            white[2] &= mask;
            white[3] &= mask;
            white[4] &= mask;
            white[5] &= mask;
        }
        else {
            black[0] &= mask;
            black[1] &= mask;
            black[2] &= mask;
            black[3] &= mask;
            black[4] &= mask;
            black[5] &= mask;
        }
    }

    void recompute() {
        whitePieces = white[0] | white[1] | white[2] | white[3] | white[4] | white[5];
        blackPieces = black[0] | black[1] | black[2] | black[3] | black[4] | black[5];

        positionHistory.push_back(zobrist);
    }

    PieceType getPiece(int index) {
        u64 indexBB = 1ULL << index;
        if ((white[0] | black[0]) & indexBB) return PAWN;
        else if ((white[1] | black[1]) & indexBB) return KNIGHT;
        else if ((white[2] | black[2]) & indexBB) return BISHOP;
        else if ((white[3] | black[3]) & indexBB) return ROOK;
        else if ((white[4] | black[4]) & indexBB) return QUEEN;
        else return KING;
    }

    void generatePawnMoves(MoveList& moves);

    void generateKnightMoves(MoveList& moves);

    void generateBishopMoves(MoveList& moves);

    void generateRookMoves(MoveList& moves);

    void generateKingMoves(MoveList& moves);

    int evaluateMVVLVA(Move m);

    int getHistoryBonus(Move& m, array<array<array<int, 64>, 64>, 2>& history) { return history[side][m.startSquare()][m.endSquare()]; }

    u64 attackersTo(Square sq, u64 occ) {
        return (getRookAttacks(sq, occ) & pieces(ROOK, QUEEN))
            | (getBishopAttacks(sq, occ) & pieces(BISHOP, QUEEN))
            | (pawnAttacksBB<WHITE>(sq) & pieces(BLACK, PAWN))
            | (pawnAttacksBB<BLACK>(sq) & pieces(WHITE, PAWN))
            | (KNIGHT_ATTACKS[sq] & pieces(KNIGHT))
            | (KING_ATTACKS[sq] & pieces(KING));
    }

public:
    void reset();

    bool see(Move m, int threshold);

    MoveList generateMoves(bool capturesOnly, array<array<array<int, 64>, 64>, 2>& history);

    MoveList generateLegalMoves(array<array<array<int, 64>, 64>, 2>& history);

    template<PieceType pt>
    Square square(Color c) { return Square(c * (white[pt]) + ~c * (black[pt])); }

    u64 pieces() const { return whitePieces | blackPieces; }

    u64 pieces(PieceType pt) const { return white[pt] | black[pt]; }

    u64 pieces(Color c, PieceType pt) const { return c ? white[pt] : black[pt]; }

    u64 pieces(PieceType p1, PieceType p2) const { return white[p1] | white[p2] | black[p1] | black[p2]; }

    u64 pieces(Color c, PieceType p1, PieceType p2) const { return c ? (white[p1] | white[p2]) : (black[p1] | black[p2]); }

    u64 pieces(Color c) const { return c ? whitePieces : blackPieces; }

    bool isEndgame() const { return popcountll(pieces()) < 9; }

    bool canNullMove() const { return !fromNull && !isEndgame() && !isInCheck(); }

    // This function is fairly slow and should not be used where possible
    bool isGameOver() {
        array<array<array<int, 64>, 64>, 2> history;
        return isDraw() || generateLegalMoves(history).count == 0;
    }

    // Uses 2fold check
    bool isDraw() const {
        if (halfMoveClock >= 100) return true;
        if (std::count(positionHistory.begin(), positionHistory.end(), zobrist) >= 2) {
            return true;
        }
        return false;
    }

    bool isInCheck() const { return ~checkMask != 0; } // Returns if STM is in check

    bool isUnderAttack(int square) const { return isUnderAttack(side, square); }

    bool isUnderAttack(Color side, int square) const;

    bool aligned(int from, int to, int test) const { return (LINE[from][to] & (1ULL << test)); }

    void updateCheckPin();

    u64  pinners() const { return pinnersPerC[side]; }

    u64 pinners(Color c) const { return pinnersPerC[c]; }

    bool isLegalMove(const Move m);

    void display() const;

    void addSubAccumulator(Square add, PieceType addPT, Square subtract, PieceType subPT);

    void addSubSubAccumulator(Square add, PieceType addPT, Square sub1, PieceType sub1PT, Square sub2, PieceType sub2PT);

    void addAddSubSubAccumulator(Square add1, PieceType add1PT, Square add2, PieceType add2PT, Square sub1, PieceType sub1PT, Square sub2, PieceType sub2PT);

    void placePiece(Color c, int pt, int square);

    void removePiece(Color c, int pt, int square);

    void removePiece(Color c, int square);

    void makeNullMove();

    void move(string moveIn) { move(Move(moveIn, *this)); }

    void move(Move moveIn, bool updateMoveHistory = true);

    void loadFromFEN(const std::deque<string>& inputFEN);

    string exportToFEN() const;

    int flipIndex(const int index) const;

    i16 evaluate() const;

    void updateAccum();

    void updateZobrist();
};