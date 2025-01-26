#include <cstdlib>
#include <bitset>
#include <array>

#include "board.h"
#include "types.h"
#include "utils.h"
#include "move.h"
#include "nnue.h"

void Board::generatePawnMoves(MoveList& moves) {
    int backShift = 0; // How much to shift a position by to get back to the original

    u64 pawns = pieces(side, PAWN);

    u64 pawnPushes = side ? (shift<NORTH>(pawns)) : (shift<SOUTH>(pawns));
    pawnPushes &= ~pieces();
    int currentSquare;

    u64 pawnCaptureRight = pawns & ~(Precomputed::isOnH);
    pawnCaptureRight = side ? (shift<NORTH_EAST>(pawnCaptureRight)) : (shift<SOUTH_EAST>(pawnCaptureRight));
    u64 pawnCaptureRightEP = pawnCaptureRight & enPassant;
    pawnCaptureRight &= pieces(~side);

    u64 pawnCaptureLeft = pawns & ~(Precomputed::isOnA);
    pawnCaptureLeft = side ? (shift<NORTH_WEST>(pawnCaptureLeft)) : (shift<SOUTH_WEST>(pawnCaptureLeft));
    u64 pawnCaptureLeftEP = pawnCaptureLeft & enPassant;
    pawnCaptureLeft &= pieces(~side);

    u64 pawnDoublePush = side ? (shift<NORTH>(pawnPushes)) : (shift<SOUTH>(pawnPushes));
    pawnDoublePush &= side ? Precomputed::isOn4 : Precomputed::isOn5;
    pawnDoublePush &= ~pieces();

    backShift = side ? SOUTH_SOUTH : NORTH_NORTH;
    while (pawnDoublePush) {
        currentSquare = ctzll(pawnDoublePush);
        moves.add(Move(currentSquare + backShift, currentSquare, DOUBLE_PUSH));

        pawnDoublePush &= pawnDoublePush - 1;
    }

    backShift = side ? SOUTH : NORTH;
    while (pawnPushes) {
        currentSquare = ctzll(pawnPushes);
        if ((1ULL << currentSquare) & (Precomputed::isOn1 | Precomputed::isOn8)) {
            moves.add(Move(currentSquare + backShift, currentSquare, QUEEN_PROMO));
            moves.add(Move(currentSquare + backShift, currentSquare, ROOK_PROMO));
            moves.add(Move(currentSquare + backShift, currentSquare, BISHOP_PROMO));
            moves.add(Move(currentSquare + backShift, currentSquare, KNIGHT_PROMO));
        }
        else moves.add(Move(currentSquare + backShift, currentSquare));

        pawnPushes &= pawnPushes - 1;
    }

    backShift = side ? SOUTH_WEST : NORTH_WEST;
    while (pawnCaptureRight) {
        currentSquare = ctzll(pawnCaptureRight);
        if ((1ULL << currentSquare) & (Precomputed::isOn1 | Precomputed::isOn8)) {
            moves.add(Move(currentSquare + backShift, currentSquare, QUEEN_PROMO_CAPTURE));
            moves.add(Move(currentSquare + backShift, currentSquare, ROOK_PROMO_CAPTURE));
            moves.add(Move(currentSquare + backShift, currentSquare, BISHOP_PROMO_CAPTURE));
            moves.add(Move(currentSquare + backShift, currentSquare, KNIGHT_PROMO_CAPTURE));
        }
        else moves.add(Move(currentSquare + backShift, currentSquare, CAPTURE));

        pawnCaptureRight &= pawnCaptureRight - 1;
    }

    backShift = side ? SOUTH_EAST : NORTH_EAST;
    while (pawnCaptureLeft) {
        currentSquare = ctzll(pawnCaptureLeft);
        if ((1ULL << currentSquare) & (Precomputed::isOn1 | Precomputed::isOn8)) {
            moves.add(Move(currentSquare + backShift, currentSquare, QUEEN_PROMO_CAPTURE));
            moves.add(Move(currentSquare + backShift, currentSquare, ROOK_PROMO_CAPTURE));
            moves.add(Move(currentSquare + backShift, currentSquare, BISHOP_PROMO_CAPTURE));
            moves.add(Move(currentSquare + backShift, currentSquare, KNIGHT_PROMO_CAPTURE));
        }
        else moves.add(Move(currentSquare + backShift, currentSquare, CAPTURE));

        pawnCaptureLeft &= pawnCaptureLeft - 1;
    }

    if (pawnCaptureLeftEP) {
        currentSquare = ctzll(pawnCaptureLeftEP);
        moves.add(Move(currentSquare + backShift, currentSquare, EN_PASSANT));

        pawnCaptureLeftEP &= pawnCaptureLeftEP - 1;
    }

    backShift = side ? SOUTH_WEST : NORTH_WEST;
    if (pawnCaptureRightEP) {
        currentSquare = ctzll(pawnCaptureRightEP);
        moves.add(Move(currentSquare + backShift, currentSquare, EN_PASSANT));

        pawnCaptureRightEP &= pawnCaptureRightEP - 1;
    }
}


void Board::generateKnightMoves(MoveList& moves) {
    u64 knightBB = pieces(side, KNIGHT);
    u64 ourBitboard = pieces(side);

    while (knightBB > 0) {
        int currentSquare = ctzll(knightBB);

        u64 knightMoves = KNIGHT_ATTACKS[currentSquare];
        knightMoves &= ~ourBitboard;

        while (knightMoves > 0) {
            int to = ctzll(knightMoves);
            moves.add(Move(currentSquare, to, ((1ULL << to) & pieces()) ? CAPTURE : STANDARD_MOVE));
            knightMoves &= knightMoves - 1;
        }
        knightBB &= knightBB - 1;
    }
}


void Board::generateBishopMoves(MoveList& moves) {
    u64 bishopBB = pieces(side, BISHOP, QUEEN);
    u64 ourBitboard = pieces(side);

    while (bishopBB > 0) {
        int currentSquare = ctzll(bishopBB);

        u64 bishopMoves = getBishopAttacks(Square(currentSquare), pieces());
        bishopMoves &= ~ourBitboard;

        while (bishopMoves > 0) {
            int to = ctzll(bishopMoves);
            moves.add(Move(currentSquare, to, ((1ULL << to) & pieces()) ? CAPTURE : STANDARD_MOVE));
            bishopMoves &= bishopMoves - 1;
        }
        bishopBB &= bishopBB - 1;
    }
}


void Board::generateRookMoves(MoveList& moves) {
    u64 rookBB = pieces(side, ROOK, QUEEN);
    u64 ourBitboard = pieces(side);

    while (rookBB > 0) {
        int currentSquare = ctzll(rookBB);

        u64 rookMoves = getRookAttacks(Square(currentSquare), pieces());
        rookMoves &= ~ourBitboard;

        while (rookMoves > 0) {
            int to = ctzll(rookMoves);
            moves.add(Move(currentSquare, to, ((1ULL << to) & pieces()) ? CAPTURE : STANDARD_MOVE));
            rookMoves &= rookMoves - 1;
        }
        rookBB &= rookBB - 1;
    }
}


void Board::generateKingMoves(MoveList& moves) {
    u64 ourBitboard = pieces(side);

    int kingSquare = ctzll(pieces(side, KING));

    u64 kingMoves = KING_ATTACKS[kingSquare];
    kingMoves &= ~ourBitboard;

    while (kingMoves > 0) {
        int to = ctzll(kingMoves);
        moves.add(Move(kingSquare, to, ((1ULL << to) & pieces()) ? CAPTURE : STANDARD_MOVE));
        kingMoves &= kingMoves - 1;
    }

    if (!isInCheck()) {
        // Castling moves
        if (side && kingSquare == e1) {
            moves.add(Move(e1, g1, CASTLE_K));
            moves.add(Move(e1, c1, CASTLE_Q));
        }
        else if (!side && kingSquare == e8) {
            moves.add(Move(e8, g8, CASTLE_K));
            moves.add(Move(e8, c8, CASTLE_Q));
        }
    }
}

// Returns the MMV-LVA eval of a move
int Board::evaluateMVVLVA(Move m) {
    int victim = PIECE_VALUES[getPiece(m.endSquare())];
    int attacker = PIECE_VALUES[getPiece(m.startSquare())];

    // Higher victim value and lower attacker value are prioritized
    return (victim * 100) - attacker;
}


// Reset the board
void Board::reset() {
    // Reset position
    white[0] = 0xFF00ULL;
    white[1] = 0x42ULL;
    white[2] = 0x24ULL;
    white[3] = 0x81ULL;
    white[4] = 0x8ULL;
    white[5] = 0x10ULL;

    black[0] = 0xFF000000000000ULL;
    black[1] = 0x4200000000000000ULL;
    black[2] = 0x2400000000000000ULL;
    black[3] = 0x8100000000000000ULL;
    black[4] = 0x800000000000000ULL;
    black[5] = 0x1000000000000000ULL;

    castlingRights = 0xF;

    enPassant = 0; // No en passant target square
    side = WHITE;

    halfMoveClock = 0;
    fullMoveClock = 1;

    positionHistory.clear();

    recompute();
    updateCheckPin();
    updateZobrist();
    updateAccum();
}


// Returns the static exchange evaluation
bool Board::see(Move m, int threshold) { // Based on SF
    // Don't do anything with quiets or castling
    if ((m.typeOf() & ~CAPTURE) != 0) return 0 >= threshold;

    int from = m.startSquare();
    int to = m.endSquare();

    int swap = PIECE_VALUES[getPiece(to)] - threshold;
    if (swap <= 0) return false;

    swap = PIECE_VALUES[getPiece(from)] - swap;
    if (swap <= 0) return true;

    u64 occ = pieces() ^ (1ULL << from) ^ (1ULL << to);
    Color stm = side;
    u64 attackers = attackersTo(Square(to), occ);
    u64 stmAttackers, bb;

    int res = 1;

    while (true) {
        stm = ~stm;
        attackers &= occ;

        stmAttackers = attackers & pieces(stm);
        if (!(stmAttackers)) break;

        if (pinners(~stm) & occ) {
            stmAttackers &= ~pinned;
            if (!stmAttackers) break;
        }

        res ^= 1;

        if ((bb = stmAttackers & pieces(PAWN))) {
            swap = PIECE_VALUES[PAWN] - swap;
            if (swap < res) break;
            occ ^= 1ULL << ctzll(bb); // LSB as a bitboard

            attackers |= getBishopAttacks(Square(to), occ) & pieces(BISHOP, QUEEN);
        }

        else if ((bb = stmAttackers & pieces(KNIGHT))) {
            swap = PIECE_VALUES[KNIGHT] - swap;
            if (swap < res) break;
            occ ^= 1ULL << ctzll(bb);
        }

        else if ((bb = stmAttackers & pieces(BISHOP))) {
            swap = PIECE_VALUES[BISHOP] - swap;
            if (swap < res) break;
            occ ^= 1ULL << ctzll(bb);

            attackers |= getBishopAttacks(Square(to), occ) & pieces(BISHOP, QUEEN);
        }

        else if ((bb = stmAttackers & pieces(ROOK))) {
            swap = PIECE_VALUES[ROOK] - swap;
            if (swap < res) break;
            occ ^= 1ULL << ctzll(bb);

            attackers |= getRookAttacks(Square(to), occ) & pieces(ROOK, QUEEN);
        }

        else if ((bb = stmAttackers & pieces(QUEEN))) {
            swap = PIECE_VALUES[QUEEN] - swap;
            if (swap < res) break;
            occ ^= 1ULL << ctzll(bb);

            attackers |= getBishopAttacks(Square(to), occ) & pieces(BISHOP, QUEEN)
                | getRookAttacks(Square(to), occ) & pieces(ROOK, QUEEN);
        }
        else return (attackers & ~pieces(stm)) ? res ^ 1 : res; // King capture so flip side if enemy has attackers
    }

    return res;
}


// Generates all pseudo legal moves
MoveList Board::generateMoves(bool capturesOnly, array<array<array<int, 64>, 64>, 2>& history) {
    MoveList allMoves;
    generateKingMoves(allMoves);

    if (doubleCheck) { // Returns early when double checks
        return allMoves;
    }

    MoveList prioritizedMoves, captures, quietMoves;
    generatePawnMoves(allMoves);
    generateKnightMoves(allMoves);
    generateBishopMoves(allMoves);
    generateRookMoves(allMoves);
    // Queen moves are part of bishop/rook moves

    // Classify moves
    for (Move move : allMoves) {
        if (move.isQuiet()) {
            quietMoves.add(move);
        }
        else {
            captures.add(move);
        }
    }

    Transposition* TTEntry = TT.getEntry(zobrist);
    if (TTEntry->zobristKey == zobrist && allMoves.find(TTEntry->bestMove) != -1) {
        prioritizedMoves.add(TTEntry->bestMove); // This move CAN be used in qsearch
    }

    std::stable_sort(captures.moves.begin(), captures.moves.begin() + captures.count, [&](Move a, Move b) { return evaluateMVVLVA(a) > evaluateMVVLVA(b); });

    for (Move m : captures) {
        prioritizedMoves.add(m);
    }

    if (capturesOnly) {
        return prioritizedMoves;
    }

    std::stable_sort(quietMoves.moves.begin(), quietMoves.moves.begin() + quietMoves.count, [&](Move a, Move b) { return getHistoryBonus(a, history) > getHistoryBonus(b, history); });


    for (Move m : quietMoves) {
        prioritizedMoves.add(m);
    }

    return prioritizedMoves;
}


// Returns if the given position for the given side is under attack
bool Board::isUnderAttack(Color side, int square) const {
    // *** SLIDING PIECE ATTACKS ***
    // Straight Directions (Rooks and Queens)
    if (pieces(~side, ROOK, QUEEN) & getRookAttacks(Square(square), pieces())) return true;

    // Diagonal Directions (Bishops and Queens)
    if (pieces(~side, BISHOP, QUEEN) & getBishopAttacks(Square(square), pieces())) return true;


    // *** KNIGHT ATTACKS ***
    if (pieces(~side, KNIGHT) & KNIGHT_ATTACKS[square]) return true;

    // *** KING ATTACKS ***
    if (pieces(~side, KING) & KING_ATTACKS[square]) return true;


    // *** PAWN ATTACKS ***
    if (side == WHITE) return (pawnAttacksBB<WHITE>(square) & pieces(BLACK, PAWN)) != 0;
    else return (pawnAttacksBB<BLACK>(square) & pieces(WHITE, PAWN)) != 0;

    return false;
}


// Update checkers and pinners
void Board::updateCheckPin() {
    int kingIndex = ctzll(side ? white[5] : black[5]);

    u64 ourPieces = pieces(side);
    auto& opponentPieces = side ? black : white;
    u64 enemyRookQueens = pieces(~side, ROOK, QUEEN);
    u64 enemyBishopQueens = pieces(~side, BISHOP, QUEEN);

    // Direct attacks for potential checks
    u64 rookChecks = getRookAttacks(Square(kingIndex), pieces()) & enemyRookQueens;
    u64 bishopChecks = getBishopAttacks(Square(kingIndex), pieces()) & enemyBishopQueens;
    u64 checks = rookChecks | bishopChecks;
    checkMask = 0; // If no checks, will be set to all 1s later.

    // *** KNIGHT ATTACKS ***
    u64 knightAttacks = KNIGHT_ATTACKS[kingIndex] & opponentPieces[1];
    while (knightAttacks) {
        checkMask |= (1ULL << ctzll(knightAttacks));
        knightAttacks &= knightAttacks - 1;
    }

    // *** PAWN ATTACKS ***
    if (side) {
        if ((opponentPieces[0] & (1ULL << (kingIndex + 7))) && (kingIndex % 8 != 0))
            checkMask |= (1ULL << (kingIndex + 7));
        if ((opponentPieces[0] & (1ULL << (kingIndex + 9))) && (kingIndex % 8 != 7))
            checkMask |= (1ULL << (kingIndex + 9));
    }
    else {
        if ((opponentPieces[0] & (1ULL << (kingIndex - 7))) && (kingIndex % 8 != 7))
            checkMask |= (1ULL << (kingIndex - 7));
        if ((opponentPieces[0] & (1ULL << (kingIndex - 9))) && (kingIndex % 8 != 0))
            checkMask |= (1ULL << (kingIndex - 9));
    }

    popcountll(checks | checkMask) > 1 ? doubleCheck = true : doubleCheck = false;

    while (checks) {
        checkMask |= LINESEG[kingIndex][ctzll(checks)];
        checks &= checks - 1;
    }

    if (!checkMask) checkMask = INF; // If no checks, set to all ones

    // ****** PIN STUFF HERE ******
    u64 rookXrays = getXrayRookAttacks(Square(kingIndex), pieces(), ourPieces) & enemyRookQueens;
    u64 bishopXrays = getXrayBishopAttacks(Square(kingIndex), pieces(), ourPieces) & enemyBishopQueens;
    u64 pinners = rookXrays | bishopXrays;
    pinnersPerC[side] = pinners;

    pinned = 0;
    while (pinners) {
        pinned |= LINESEG[ctzll(pinners)][kingIndex] & ourPieces;
        pinners &= pinners - 1;
    }
}


// Returns if the given move is legal on the board (assumes move is pseudolegal)
bool Board::isLegalMove(const Move m) {
    int from = m.startSquare();
    int to = m.endSquare();

    // Delete null moves
    if (from == to) return false;

    // Castling checks
    if (m.typeOf() == CASTLE_K || m.typeOf() == CASTLE_Q) {
        bool kingside = (m.typeOf() == CASTLE_K);
        int rightsIndex = side ? (kingside ? 3 : 2) : (kingside ? 1 : 0);
        if (!readBit(castlingRights, rightsIndex)) return false;
        if (isInCheck()) return false;
        u64 occupied = whitePieces | blackPieces;
        if (side) {
            if (kingside) {
                if ((occupied & ((1ULL << f1) | (1ULL << g1))) != 0) return false;
                if (isUnderAttack(WHITE, f1) || isUnderAttack(WHITE, g1)) return false;
            }
            else {
                if ((occupied & ((1ULL << b1) | (1ULL << c1) | (1ULL << d1))) != 0) return false;
                if (isUnderAttack(WHITE, d1) || isUnderAttack(WHITE, c1)) return false;
            }
        }
        else {
            if (kingside) {
                if ((occupied & ((1ULL << f8) | (1ULL << g8))) != 0) return false;
                if (isUnderAttack(BLACK, f8) || isUnderAttack(BLACK, g8)) return false;
            }
            else {
                if ((occupied & ((1ULL << b8) | (1ULL << c8) | (1ULL << d8))) != 0) return false;
                if (isUnderAttack(BLACK, d8) || isUnderAttack(BLACK, c8)) return false;
            }
        }
    }

    u64& king = side ? white[5] : black[5];
    int kingIndex = ctzll(king);

    // King moves
    if (king & (1ULL << from)) {
        u64& pieces = side ? whitePieces : blackPieces;

        pieces ^= king;
        king ^= 1ULL << kingIndex;

        bool ans = !isUnderAttack(side, to);
        king ^= 1ULL << kingIndex;
        pieces ^= king;
        return ans;
    }

    // En passant check
    if (m.typeOf() == EN_PASSANT) {
        Board testBoard = *this;
        testBoard.move(m, false);
        // Needs to use a different test because isInCheck does not work with NSTM
        return !testBoard.isUnderAttack(side, ctzll(king));
    }

    // Validate move
    if ((1ULL << to) & ~checkMask) return false;

    // Handle pin scenario
    if ((pinned & 1ULL << from) && !aligned(from, to, kingIndex)) return false;

    // If we reach here, the move is legal
    return true;
}


// Generate legal moves
MoveList Board::generateLegalMoves(array<array<array<int, 64>, 64>, 2>& history) {
    auto moves = generateMoves(false, history);
    int i = 0;

    while (i < moves.count) {
        if (!isLegalMove(moves.moves[i])) {
            // Replace the current move with the last move and decrease count
            moves.moves[i] = moves.moves[moves.count - 1];
            moves.count--;
        }
        else {
            ++i;
        }
    }
    return moves;
}


// Pretty print the board
void Board::display() const {
    if (side)
        cout << "White's turn" << endl;
    else
        cout << "Black's turn" << endl;

    for (int rank = 7; rank >= 0; rank--) {
        cout << "+---+---+---+---+---+---+---+---+" << endl;
        for (int file = 0; file < 8; file++) {
            int i = rank * 8 + file;  // Map rank and file to bitboard index
            char currentPiece = ' ';

            if (readBit(white[0], i)) currentPiece = 'P';
            else if (readBit(white[1], i)) currentPiece = 'N';
            else if (readBit(white[2], i)) currentPiece = 'B';
            else if (readBit(white[3], i)) currentPiece = 'R';
            else if (readBit(white[4], i)) currentPiece = 'Q';
            else if (readBit(white[5], i)) currentPiece = 'K';

            else if (readBit(black[0], i)) currentPiece = 'p';
            else if (readBit(black[1], i)) currentPiece = 'n';
            else if (readBit(black[2], i)) currentPiece = 'b';
            else if (readBit(black[3], i)) currentPiece = 'r';
            else if (readBit(black[4], i)) currentPiece = 'q';
            else if (readBit(black[5], i)) currentPiece = 'k';

            cout << "| " << ((1ULL << i) & whitePieces ? Colors::YELLOW : Colors::BLUE) << currentPiece << Colors::RESET << " ";
        }
        cout << "| " << rank + 1 << endl;
    }
    cout << "+---+---+---+---+---+---+---+---+" << endl;
    cout << "  a   b   c   d   e   f   g   h" << endl;
    cout << endl;
    cout << "Current FEN: " << exportToFEN() << endl;
    cout << "Current key: 0x" << std::hex << std::uppercase << zobrist << std::dec << endl;
}


// Quiets
// Assumes all pieces are friendly
void Board::addSubAccumulator(Square add, PieceType addPT, Square subtract, PieceType subPT) {
    // Extract the features
    int addFeatureWhite = NNUE::feature(WHITE, side, addPT, add);
    int addFeatureBlack = NNUE::feature(BLACK, side, addPT, add);

    int subFeatureWhite = NNUE::feature(WHITE, side, subPT, subtract);
    int subFeatureBlack = NNUE::feature(BLACK, side, subPT, subtract);

    // Accumulate weights in the hidden layer
    for (int i = 0; i < HL_SIZE; i++) {
        whiteAccum[i] += nn.weightsToHL[addFeatureWhite * HL_SIZE + i];
        blackAccum[i] += nn.weightsToHL[addFeatureBlack * HL_SIZE + i];

        whiteAccum[i] -= nn.weightsToHL[subFeatureWhite * HL_SIZE + i];
        blackAccum[i] -= nn.weightsToHL[subFeatureBlack * HL_SIZE + i];
    }
}


// Captures
// Add and sub1 should both be friendly, other pieces are captured
void Board::addSubSubAccumulator(Square add, PieceType addPT, Square sub1, PieceType sub1PT, Square sub2, PieceType sub2PT) {
    // Extract the features
    int addFeatureWhite = NNUE::feature(WHITE, side, addPT, add);
    int addFeatureBlack = NNUE::feature(BLACK, side, addPT, add);

    int sub1FeatureWhite = NNUE::feature(WHITE, side, sub1PT, sub1);
    int sub1FeatureBlack = NNUE::feature(BLACK, side, sub1PT, sub1);

    int sub2FeatureWhite = NNUE::feature(WHITE, ~side, sub2PT, sub2);
    int sub2FeatureBlack = NNUE::feature(BLACK, ~side, sub2PT, sub2);

    // Accumulate weights in the hidden layer
    for (int i = 0; i < HL_SIZE; i++) {
        whiteAccum[i] += nn.weightsToHL[addFeatureWhite * HL_SIZE + i];
        blackAccum[i] += nn.weightsToHL[addFeatureBlack * HL_SIZE + i];

        whiteAccum[i] -= nn.weightsToHL[sub1FeatureWhite * HL_SIZE + i];
        blackAccum[i] -= nn.weightsToHL[sub1FeatureBlack * HL_SIZE + i];

        whiteAccum[i] -= nn.weightsToHL[sub2FeatureWhite * HL_SIZE + i];
        blackAccum[i] -= nn.weightsToHL[sub2FeatureBlack * HL_SIZE + i];
    }
}


// Update accumulators for castling
// All pieces should be friendly
void Board::addAddSubSubAccumulator(Square add1, PieceType add1PT, Square add2, PieceType add2PT, Square sub1, PieceType sub1PT, Square sub2, PieceType sub2PT) {
    // Extract the features
    int add1FeatureWhite = NNUE::feature(WHITE, side, add1PT, add1);
    int add1FeatureBlack = NNUE::feature(BLACK, side, add1PT, add1);

    int add2FeatureWhite = NNUE::feature(WHITE, side, add2PT, add2);
    int add2FeatureBlack = NNUE::feature(BLACK, side, add2PT, add2);

    int sub1FeatureWhite = NNUE::feature(WHITE, side, sub1PT, sub1);
    int sub1FeatureBlack = NNUE::feature(BLACK, side, sub1PT, sub1);

    int sub2FeatureWhite = NNUE::feature(WHITE, side, sub2PT, sub2);
    int sub2FeatureBlack = NNUE::feature(BLACK, side, sub2PT, sub2);

    // Accumulate weights in the hidden layer
    for (int i = 0; i < HL_SIZE; i++) {
        whiteAccum[i] += nn.weightsToHL[add1FeatureWhite * HL_SIZE + i];
        blackAccum[i] += nn.weightsToHL[add1FeatureBlack * HL_SIZE + i];

        whiteAccum[i] += nn.weightsToHL[add2FeatureWhite * HL_SIZE + i];
        blackAccum[i] += nn.weightsToHL[add2FeatureBlack * HL_SIZE + i];

        whiteAccum[i] -= nn.weightsToHL[sub1FeatureWhite * HL_SIZE + i];
        blackAccum[i] -= nn.weightsToHL[sub1FeatureBlack * HL_SIZE + i];

        whiteAccum[i] -= nn.weightsToHL[sub2FeatureWhite * HL_SIZE + i];
        blackAccum[i] -= nn.weightsToHL[sub2FeatureBlack * HL_SIZE + i];
    }
}


// Puts a piece on the board
void Board::placePiece(Color c, int pt, int square) {
    auto& us = c ? white : black;

    setBit(us[pt], square, 1);
    zobrist ^= Precomputed::zobrist[c][pt][square];
}


// Remove a piece from the given square and piece type
void Board::removePiece(Color c, int pt, int square) {
    auto& us = c ? white : black;
    zobrist ^= Precomputed::zobrist[c][pt][square];

    setBit(us[pt], square, 0);
}


// Remove a piece from the given square
void Board::removePiece(Color c, int square) {
    zobrist ^= Precomputed::zobrist[c][getPiece(square)][square];

    clearIndex(c, square);
}


// Make a null move
void Board::makeNullMove() {
    // En Passant
    zobrist ^= Precomputed::zobristEP[ctzll(enPassant)];
    zobrist ^= Precomputed::zobristEP[64];

    enPassant = 0;

    // Turn
    zobrist ^= Precomputed::zobristSide[side];
    zobrist ^= Precomputed::zobristSide[~side];

    side = ~side;

    fromNull = true;

    updateCheckPin();
}


// Make a move on the board
void Board::move(Move moveIn, bool updateMoveHistory) {
    auto& us = side ? white : black;

    // Take out old zobrist stuff that will be re-added at the end of the turn
    // Castling
    zobrist ^= Precomputed::zobristCastling[castlingRights];

    // En Passant
    zobrist ^= Precomputed::zobristEP[ctzll(enPassant)];

    // Turn
    zobrist ^= Precomputed::zobristSide[side];

    fromNull = false;

    Square from = moveIn.startSquare();
    Square to = moveIn.endSquare();

    IFDBG{
        if ((1ULL << to) & (white[5] | black[5])) {
            cout << "WARNING: ATTEMPTED CAPTURE OF THE KING. MOVE: " << moveIn.toString() << endl;
            display();

            int whiteKing = ctzll(white[5]);
            int blackKing = ctzll(black[5]);
            cout << "Is in check (white): " << isUnderAttack(WHITE, whiteKing) << endl;
            cout << "Is in check (black): " << isUnderAttack(BLACK, blackKing) << endl;
            cout << "En passant square: " << (ctzll(enPassant) < 64 ? squareToAlgebraic(ctzll(enPassant)) : "-") << endl;
            cout << "Castling rights: " << std::bitset<4>(castlingRights) << endl;

            cout << "White pawns: " << popcountll(white[0]) << endl;
            cout << "White knigts: " << popcountll(white[1]) << endl;
            cout << "White bishops: " << popcountll(white[2]) << endl;
            cout << "White rooks: " << popcountll(white[3]) << endl;
            cout << "White queens: " << popcountll(white[4]) << endl;
            cout << "White king: " << popcountll(white[5]) << endl;
            cout << endl;
            cout << "Black pawns: " << popcountll(black[0]) << endl;
            cout << "Black knigts: " << popcountll(black[1]) << endl;
            cout << "Black bishops: " << popcountll(black[2]) << endl;
            cout << "Black rooks: " << popcountll(black[3]) << endl;
            cout << "Black queens: " << popcountll(black[4]) << endl;
            cout << "Black king: " << popcountll(black[5]) << endl;
            cout << endl;

            exit(-1);
        }
    }

    PieceType pt = getPiece(from);
    MoveType mt = moveIn.typeOf();

    removePiece(side, pt, from);
    IFDBG m_assert(!readBit(us[pt], from), "Position piece moved from was not cleared");

    enPassant = 0;

    switch (mt) {
    case STANDARD_MOVE: placePiece(side, pt, to); addSubAccumulator(to, pt, from, pt); break;
    case DOUBLE_PUSH:
        placePiece(side, pt, to);
        enPassant = 1ULL << ((side) * (from + 8) + (!side) * (from - 8));
        addSubAccumulator(to, pt, from, pt);
        break;
    case CASTLE_K:
        if (side == WHITE) {
            placePiece(side, pt, to);

            removePiece(side, ROOK, h1);
            placePiece(side, ROOK, f1);

            addAddSubSubAccumulator(g1, KING, f1, ROOK, e1, KING, h1, ROOK);
        }
        else {
            placePiece(side, pt, to);

            removePiece(side, ROOK, h8);
            placePiece(side, ROOK, f8);

            addAddSubSubAccumulator(g8, KING, f8, ROOK, e8, KING, h8, ROOK);
        }
        break;
    case CASTLE_Q:
        if (side == WHITE) {
            placePiece(side, pt, to);

            removePiece(side, ROOK, a1);
            placePiece(side, ROOK, d1);

            addAddSubSubAccumulator(c1, KING, d1, ROOK, e1, KING, a1, ROOK);
        }
        else {
            placePiece(side, pt, to);

            removePiece(side, ROOK, a8);
            placePiece(side, ROOK, d8);

            addAddSubSubAccumulator(c8, KING, d8, ROOK, e8, KING, a8, ROOK);
        }
        break;
    case CAPTURE: addSubSubAccumulator(to, pt, from, pt, to, getPiece(to)), removePiece(~side, to); placePiece(side, pt, to); break;
    case QUEEN_PROMO_CAPTURE: addSubSubAccumulator(to, QUEEN, from, pt, to, getPiece(to)); removePiece(~side, to); placePiece(side, QUEEN, to); break;
    case ROOK_PROMO_CAPTURE: addSubSubAccumulator(to, ROOK, from, pt, to, getPiece(to)); removePiece(~side, to); placePiece(side, ROOK, to); break;
    case BISHOP_PROMO_CAPTURE: addSubSubAccumulator(to, BISHOP, from, pt, to, getPiece(to)); removePiece(~side, to); placePiece(side, BISHOP, to); break;
    case KNIGHT_PROMO_CAPTURE: addSubSubAccumulator(to, KNIGHT, from, pt, to, getPiece(to)); removePiece(~side, to); placePiece(side, KNIGHT, to); break;
    case EN_PASSANT:
        if (side) {
            removePiece(BLACK, PAWN, to + SOUTH);
            addSubSubAccumulator(to, pt, from, pt, to + SOUTH, PAWN);
        }
        else {
            removePiece(WHITE, PAWN, to + NORTH);
            addSubSubAccumulator(to, pt, from, pt, to + NORTH, PAWN);
        }
        placePiece(side, PAWN, to);

        break;
    case QUEEN_PROMO: placePiece(side, QUEEN, to); addSubAccumulator(to, QUEEN, from, pt); break;
    case ROOK_PROMO: placePiece(side, ROOK, to); addSubAccumulator(to, ROOK, from, pt); break;
    case BISHOP_PROMO: placePiece(side, BISHOP, to); addSubAccumulator(to, BISHOP, from, pt); break;
    case KNIGHT_PROMO: placePiece(side, KNIGHT, to); addSubAccumulator(to, KNIGHT, from, pt); break;
    }

    // Halfmove clock, promo and set en passant
    if (pt == PAWN || readBit(pieces(), to)) halfMoveClock = 0; // Reset halfmove clock on capture or pawn move
    else halfMoveClock++;


    // Remove castling rights
    if (castlingRights) {
        if (from == a1 || to == a1) setBit(castlingRights, 2, 0);
        if (from == h1 || to == h1) setBit(castlingRights, 3, 0);
        if (from == e1) { // King moved
            setBit(castlingRights, 2, 0);
            setBit(castlingRights, 3, 0);
        }
        if (from == a8 || to == a8) setBit(castlingRights, 0, 0);
        if (from == h8 || to == h8) setBit(castlingRights, 1, 0);
        if (from == e8) { // King moved
            setBit(castlingRights, 0, 0);
            setBit(castlingRights, 1, 0);
        }
    }

    side = ~side;

    recompute();
    updateCheckPin();

    // Castling
    zobrist ^= Precomputed::zobristCastling[castlingRights];

    // En Passant
    zobrist ^= Precomputed::zobristEP[ctzll(enPassant)];

    // Turn
    zobrist ^= Precomputed::zobristSide[~side];

    // Only add 1 to full move clock if move was made as black
    fullMoveClock += ~side;
}


void Board::loadFromFEN(const std::deque<string>& inputFEN) {
    // Reset
    reset();

    for (int i = 0; i < 64; ++i) clearIndex(i);

    // Sanitize input
    if (inputFEN.size() < 6) {
        std::cerr << "Invalid FEN string" << endl;
        return;
    }

    std::deque<string> parsedPosition = split(inputFEN.at(0), '/');
    char currentCharacter;
    int currentIndex = 56; // Start at rank 8, file 'a' (index 56)

    for (const string& rankString : parsedPosition) {
        for (char c : rankString) {
            currentCharacter = c;

            if (isdigit(currentCharacter)) { // Empty squares
                int emptySquares = currentCharacter - '0';
                currentIndex += emptySquares; // Skip the given number of empty squares
            }
            else { // Piece placement
                switch (currentCharacter) {
                case 'P': setBit(white[0], currentIndex, 1); break;
                case 'N': setBit(white[1], currentIndex, 1); break;
                case 'B': setBit(white[2], currentIndex, 1); break;
                case 'R': setBit(white[3], currentIndex, 1); break;
                case 'Q': setBit(white[4], currentIndex, 1); break;
                case 'K': setBit(white[5], currentIndex, 1); break;
                case 'p': setBit(black[0], currentIndex, 1); break;
                case 'n': setBit(black[1], currentIndex, 1); break;
                case 'b': setBit(black[2], currentIndex, 1); break;
                case 'r': setBit(black[3], currentIndex, 1); break;
                case 'q': setBit(black[4], currentIndex, 1); break;
                case 'k': setBit(black[5], currentIndex, 1); break;
                default: break;
                }
                currentIndex++;
            }
        }
        currentIndex -= 16; // Move to next rank in FEN
    }

    if (inputFEN.at(1) == "w") { // Check the next player's move
        side = WHITE;
    }
    else {
        side = BLACK;
    }

    castlingRights = 0;

    if (inputFEN.at(2).find('K') != string::npos) castlingRights |= 1 << 3;
    if (inputFEN.at(2).find('Q') != string::npos) castlingRights |= 1 << 2;
    if (inputFEN.at(2).find('k') != string::npos) castlingRights |= 1 << 1;
    if (inputFEN.at(2).find('q') != string::npos) castlingRights |= 1;

    if (inputFEN.at(3) != "-") enPassant = 1ULL << parseSquare(inputFEN.at(3));
    else enPassant = 0;

    halfMoveClock = stoi(inputFEN.at(4));
    fullMoveClock = stoi(inputFEN.at(5));

    recompute();
    updateCheckPin();
    updateZobrist();
    updateAccum();
}


string Board::exportToFEN() const {
    string ans = "";

    int blankSquares = 0;

    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file <= 7; file++) {
            int sq = rank * 8 + file;
            for (int i = 0; i < 6; ++i) {
                if (readBit(white[i], sq)) {
                    if (blankSquares / 12) ans += std::to_string(blankSquares / 12); // Divide by 12 because it in incremented in a for loop for types
                    blankSquares = 0;
                    switch (i) {
                    case 0: ans += "P"; break;
                    case 1: ans += "N"; break;
                    case 2: ans += "B"; break;
                    case 3: ans += "R"; break;
                    case 4: ans += "Q"; break;
                    case 5: ans += "K"; break;
                    }
                    break;
                }
                else blankSquares++;
            }
            for (int i = 0; i < 6; ++i) {
                if (readBit(black[i], sq)) {
                    if (blankSquares / 12) ans += std::to_string(blankSquares / 12);
                    blankSquares = 0;
                    switch (i) {
                    case 0: ans += "p"; break;
                    case 1: ans += "n"; break;
                    case 2: ans += "b"; break;
                    case 3: ans += "r"; break;
                    case 4: ans += "q"; break;
                    case 5: ans += "k"; break;
                    }
                    break;
                }
                else blankSquares++;
            }
        }
        if (blankSquares / 12) ans += std::to_string(blankSquares / 12);
        ans += "/";
        blankSquares = 0;
    }

    // Remove the trailing / on the end of the fen
    ans[ans.length() - 1] = '\0';

    ans += side ? " w " : " b ";

    if (readBit(castlingRights, 3)) ans += "K";
    if (readBit(castlingRights, 2)) ans += "Q";
    if (readBit(castlingRights, 1)) ans += "k";
    if (readBit(castlingRights, 0)) ans += "q";

    if (castlingRights == 0) ans += "- ";
    else ans += " ";

    if (enPassant) ans += squareToAlgebraic(ctzll(enPassant));
    else ans += "-";

    ans += " ";
    ans += std::to_string(halfMoveClock);
    ans += " ";
    ans += std::to_string(fullMoveClock);

    return ans;
}


int Board::flipIndex(const int index) const {
    // Ensure the index is within [0, 63]
    IFDBG m_assert((index >= 0 && index <= 63), "Invalid index: " + std::to_string(index) + ". Must be between 0 and 63.");
    int rank = index / 8;
    int file = index % 8;
    int mirrored_rank = 7 - rank;
    return mirrored_rank * 8 + file;
}


// Returns evaluation in centipawns as side to move
i16 Board::evaluate() const {
    return std::clamp(nn.forwardPass(this), MATED_IN_MAX_PLY + 1, MATE_IN_MAX_PLY - 1);

    // Code below here can be used when there are 2 NNUEs
    int matEval = 0;

    // Uses some magic python buffoonery https://github.com/ianfab/chess-variant-stats/blob/main/piece_values.py
    // Based on this https://discord.com/channels/435943710472011776/1300744461281787974/1312722964915027980

    // Material evaluation
    matEval += popcountll(white[0]) * 100;
    matEval += popcountll(white[1]) * 316;
    matEval += popcountll(white[2]) * 328;
    matEval += popcountll(white[3]) * 493;
    matEval += popcountll(white[4]) * 982;

    matEval -= popcountll(black[0]) * 100;
    matEval -= popcountll(black[1]) * 316;
    matEval -= popcountll(black[2]) * 328;
    matEval -= popcountll(black[3]) * 493;
    matEval -= popcountll(black[4]) * 982;

    // Only utilize the large NNUE in situations when game isn't very won or lost
    // Concept from SF
    if (std::abs(matEval) < 950) {
        return nn.forwardPass(this);
    }
}


// Recompute the accumulators for white and black based on the current position
void Board::updateAccum() {
    // This code assumes white is the STM
    u64 whitePieces = this->whitePieces;
    u64 blackPieces = this->blackPieces;

    blackAccum = nn.hiddenLayerBias;
    whiteAccum = nn.hiddenLayerBias;

    // Accumulate contributions for STM pieces
    while (whitePieces) {
        Square currentSq = Square(ctzll(whitePieces)); // Find the least significant set bit

        // Extract the feature for STM
        int inputFeatureWhite = NNUE::feature(WHITE, WHITE, getPiece(currentSq), currentSq);
        int inputFeatureBlack = NNUE::feature(BLACK, WHITE, getPiece(currentSq), currentSq);

        // Accumulate weights for STM hidden layer
        for (int i = 0; i < HL_SIZE; i++) {
            whiteAccum[i] += nn.weightsToHL[inputFeatureWhite * HL_SIZE + i];
            blackAccum[i] += nn.weightsToHL[inputFeatureBlack * HL_SIZE + i];
        }

        whitePieces &= (whitePieces - 1); // Clear the least significant set bit
    }

    // Accumulate contributions for OPP pieces
    while (blackPieces) {
        Square currentSq = Square(ctzll(blackPieces)); // Find the least significant set bit

        // Extract the feature for STM
        int inputFeatureWhite = NNUE::feature(WHITE, BLACK, getPiece(currentSq), currentSq);
        int inputFeatureBlack = NNUE::feature(BLACK, BLACK, getPiece(currentSq), currentSq);

        // Accumulate weights for STM hidden layer
        for (int i = 0; i < HL_SIZE; i++) {
            whiteAccum[i] += nn.weightsToHL[inputFeatureWhite * HL_SIZE + i];
            blackAccum[i] += nn.weightsToHL[inputFeatureBlack * HL_SIZE + i];
        }

        blackPieces &= (blackPieces - 1); // Clear the least significant set bit
    }
}


// Recompute the zobrist table
void Board::updateZobrist() {
    zobrist = 0;
    // Pieces
    for (int table = 0; table < 6; table++) {
        u64 tableBB = white[table];
        while (tableBB) {
            int currentIndex = ctzll(tableBB);
            zobrist ^= Precomputed::zobrist[WHITE][table][currentIndex];
            tableBB &= tableBB - 1;
        }
    }
    for (int table = 0; table < 6; table++) {
        u64 tableBB = black[table];
        while (tableBB) {
            int currentIndex = ctzll(tableBB);
            zobrist ^= Precomputed::zobrist[BLACK][table][currentIndex];
            tableBB &= tableBB - 1;
        }
    }

    // Castling
    zobrist ^= Precomputed::zobristCastling[castlingRights];

    // En Passant
    zobrist ^= Precomputed::zobristEP[ctzll(enPassant)];

    // Turn
    zobrist ^= Precomputed::zobristSide[side];
}