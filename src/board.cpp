#include "board.h"
#include "types.h"
#include "movegen.h"

#include <cassert>

constexpr array<u8, 64> CASTLING_RIGHTS = {0b1011, 0b1111, 0b1111, 0b1111, 0b0011, 0b1111, 0b1111, 0b0111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
                                           0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
                                           0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
                                           0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1110, 0b1111, 0b1111, 0b1111, 0b1100, 0b1111, 0b1111, 0b1101};

// Returns the piece on a square as a character
char Board::getPieceAt(int sq) const {
    if (((1ULL << sq) & pieces()) == 0)
        return ' ';
    constexpr char whiteSymbols[] = {'P', 'N', 'B', 'R', 'Q', 'K'};
    constexpr char blackSymbols[] = {'p', 'n', 'b', 'r', 'q', 'k'};
    if (((1ULL << sq) & byColor[WHITE]) != 0)
        return whiteSymbols[getPiece(sq)];
    return blackSymbols[getPiece(sq)];
}

void Board::placePiece(Color c, PieceType pt, int sq) {
    assert(sq >= 0);
    assert(sq < 64);

    auto& BB = byPieces[pt];

    assert(!readBit(BB, sq));

    BB ^= 1ULL << sq;
    byColor[c] ^= 1ULL << sq;

    mailbox[sq] = pt;
}

void Board::removePiece(Color c, PieceType pt, int sq) {
    assert(sq >= 0);
    assert(sq < 64);

    auto& BB = byPieces[pt];

    assert(readBit(BB, sq));

    BB ^= 1ULL << sq;
    byColor[c] ^= 1ULL << sq;

    mailbox[sq] = NO_PIECE_TYPE;
}

void Board::removePiece(Color c, int sq) {
    assert(sq >= 0);
    assert(sq < 64);

    auto& BB = byPieces[getPiece(sq)];

    assert(readBit(BB, sq));

    BB ^= 1ULL << sq;
    byColor[c] ^= 1ULL << sq;

    mailbox[sq] = NO_PIECE_TYPE;
}

void Board::resetMailbox() {
    mailbox.fill(NO_PIECE_TYPE);
    for (int i = 0; i < 64; i++) {
        PieceType& sq   = mailbox[i];
        u64        mask = 1ULL << i;
        if (mask & pieces(PAWN))
            sq = PAWN;
        else if (mask & pieces(KNIGHT))
            sq = KNIGHT;
        else if (mask & pieces(BISHOP))
            sq = BISHOP;
        else if (mask & pieces(ROOK))
            sq = ROOK;
        else if (mask & pieces(QUEEN))
            sq = QUEEN;
        else if (mask & pieces(KING))
            sq = KING;
    }
}

// Updates checkers and pinners
void Board::updateCheckPin() {
    u64 kingBB = pieces(stm, KING);
    int kingSq = ctzll(kingBB);

    u64 ourPieces         = pieces(stm);
    u64 enemyRookQueens   = pieces(~stm, ROOK, QUEEN);
    u64 enemyBishopQueens = pieces(~stm, BISHOP, QUEEN);

    // Direct attacks for potential checks
    u64 rookChecks   = Movegen::getRookAttacks(Square(kingSq), pieces()) & enemyRookQueens;
    u64 bishopChecks = Movegen::getBishopAttacks(Square(kingSq), pieces()) & enemyBishopQueens;
    u64 checks       = rookChecks | bishopChecks;
    checkMask        = 0;  // If no checks, will be set to all 1s later.

    // *** KNIGHT ATTACKS ***
    u64 knightAttacks = Movegen::KNIGHT_ATTACKS[kingSq] & pieces(~stm, KNIGHT);
    while (knightAttacks) {
        checkMask |= (1ULL << ctzll(knightAttacks));
        knightAttacks &= knightAttacks - 1;
    }

    // *** PAWN ATTACKS ***
    if (stm == WHITE) {
        checkMask |= shift<NORTH_WEST>(kingBB & ~MASK_FILE[AFILE]) & pieces(BLACK, PAWN);
        checkMask |= shift<NORTH_EAST>(kingBB & ~MASK_FILE[HFILE]) & pieces(BLACK, PAWN);
    }
    else {
        checkMask |= shift<SOUTH_WEST>(kingBB & ~MASK_FILE[AFILE]) & pieces(WHITE, PAWN);
        checkMask |= shift<SOUTH_EAST>(kingBB & ~MASK_FILE[HFILE]) & pieces(WHITE, PAWN);
    }

    doubleCheck = popcount(checks | checkMask) > 1;

    while (checks) {
        checkMask |= LINESEG[kingSq][ctzll(checks)];
        checks &= checks - 1;
    }

    if (checkMask == 0)
        checkMask = ~checkMask;  // If no checks, set to all ones

    // ****** PIN STUFF HERE ******
    u64 rookXrays    = Movegen::getXrayRookAttacks(Square(kingSq), pieces(), ourPieces) & enemyRookQueens;
    u64 bishopXrays  = Movegen::getXrayBishopAttacks(Square(kingSq), pieces(), ourPieces) & enemyBishopQueens;
    u64 pinners      = rookXrays | bishopXrays;
    pinnersPerC[stm] = pinners;

    pinned = 0;
    while (pinners) {
        pinned |= LINESEG[ctzll(pinners)][kingSq] & ourPieces;
        pinners &= pinners - 1;
    }
}

u64 Board::pieces() const { return byColor[WHITE] | byColor[BLACK]; }
u64 Board::pieces(Color c) const { return byColor[c]; }
u64 Board::pieces(PieceType pt) const { return byPieces[pt]; }
u64 Board::pieces(Color c, PieceType pt) const { return byPieces[pt] & byColor[c]; }
u64 Board::pieces(Color c, PieceType pt1, PieceType pt2) const { return (byPieces[pt1] | byPieces[pt2]) & byColor[c]; }

// Reset the board to startpos
void Board::reset() {
    byPieces[PAWN]   = 0xFF00ULL;
    byPieces[KNIGHT] = 0x42ULL;
    byPieces[BISHOP] = 0x24ULL;
    byPieces[ROOK]   = 0x81ULL;
    byPieces[QUEEN]  = 0x8ULL;
    byPieces[KING]   = 0x10ULL;
    byColor[WHITE]   = byPieces[PAWN] | byPieces[KNIGHT] | byPieces[BISHOP] | byPieces[ROOK] | byPieces[QUEEN] | byPieces[KING];

    byPieces[PAWN] |= 0xFF000000000000ULL;
    byPieces[KNIGHT] |= 0x4200000000000000ULL;
    byPieces[BISHOP] |= 0x2400000000000000ULL;
    byPieces[ROOK] |= 0x8100000000000000ULL;
    byPieces[QUEEN] |= 0x800000000000000ULL;
    byPieces[KING] |= 0x1000000000000000ULL;
    byColor[BLACK] = 0xFF000000000000ULL | 0x4200000000000000ULL | 0x2400000000000000ULL | 0x8100000000000000ULL | 0x800000000000000ULL | 0x1000000000000000ULL;


    stm            = WHITE;
    castlingRights = 0b1111;

    epSquare = NO_SQUARE;

    halfMoveClock = 0;
    fullMoveClock = 1;

    resetMailbox();
    updateCheckPin();
}


// Load a board from the FEN
void Board::loadFromFEN(string fen) {
    reset();

    // Clear all squares
    byPieces.fill(0);
    byColor.fill(0);

    std::vector<string> tokens = split(fen, ' ');

    std::vector<string> rankTokens = split(tokens[0], '/');

    int currIdx = 56;

    const char whitePieces[6] = {'P', 'N', 'B', 'R', 'Q', 'K'};
    const char blackPieces[6] = {'p', 'n', 'b', 'r', 'q', 'k'};

    for (const string& rank : rankTokens) {
        for (const char c : rank) {
            if (isdigit(c)) {  // Empty squares
                currIdx += c - '0';
                continue;
            }
            for (int i = 0; i < 6; i++) {
                if (c == whitePieces[i]) {
                    setBit<1>(byPieces[i], currIdx);
                    setBit<1>(byColor[WHITE], currIdx);
                    break;
                }
                if (c == blackPieces[i]) {
                    setBit<1>(byPieces[i], currIdx);
                    setBit<1>(byColor[BLACK], currIdx);
                    break;
                }
            }
            currIdx++;
        }
        currIdx -= 16;
    }

    if (tokens[1] == "w")
        stm = WHITE;
    else
        stm = BLACK;

    castlingRights = 0;

    if (tokens[2].find('K') != string::npos)
        castlingRights |= 1 << 3;
    if (tokens[2].find('Q') != string::npos)
        castlingRights |= 1 << 2;
    if (tokens[2].find('k') != string::npos)
        castlingRights |= 1 << 1;
    if (tokens[2].find('q') != string::npos)
        castlingRights |= 1;

    if (tokens[3] != "-")
        epSquare = parseSquare(tokens[3]);
    else
        epSquare = NO_SQUARE;

    halfMoveClock = tokens.size() > 4 ? (stoi(tokens[5])) : 0;
    fullMoveClock = tokens.size() > 5 ? (stoi(tokens[5])) : 1;

    resetMailbox();
    updateCheckPin();
}

// Print the board
void Board::display() const {
    cout << (stm ? "White's turn" : "Black's turn") << endl;

    for (int rank = 7; rank >= 0; rank--) {
        cout << "+---+---+---+---+---+---+---+---+" << endl;
        for (int file = 0; file < 8; file++) {
            int  i     = rank * 8 + file;
            auto color = ((1ULL << i) & pieces(WHITE)) ? Colors::YELLOW : Colors::BLUE;
            cout << "| " << color << getPieceAt(i) << Colors::RESET << " ";
        }
        cout << "| " << rank + 1 << endl;
    }
    cout << "+---+---+---+---+---+---+---+---+" << endl;
    cout << "  a   b   c   d   e   f   g   h" << endl << endl;
}

// Return the type of the piece on the square
PieceType Board::getPiece(int sq) const { return mailbox[sq]; }

// Make a move on the board
void Board::move(Move m) {
    epSquare       = NO_SQUARE;
    Square    from = m.from();
    Square    to   = m.to();
    MoveType  mt   = m.typeOf();
    PieceType pt   = getPiece(from);

    removePiece(stm, pt, from);
    if (m.isCapture(pieces()))
        removePiece(~stm, to);

    switch (mt) {
    case STANDARD_MOVE:
        placePiece(stm, pt, to);
        if (pt == PAWN && (to + 16 == from || to - 16 == from)
            && (pieces(~stm, PAWN) & (shift<EAST>((1ULL << to) & ~MASK_FILE[HFILE]) | shift<WEST>((1ULL << to) & ~MASK_FILE[AFILE]))))  // Only set EP square if it could be taken
            epSquare = Square(stm == WHITE ? from + NORTH : from + SOUTH);
        break;
    case EN_PASSANT:
        removePiece(~stm, PAWN, to + (stm == WHITE ? SOUTH : NORTH));
        placePiece(stm, pt, to);
        break;
    case CASTLE:
        if (from < to) {  // Kingside
            if (stm == WHITE) {
                placePiece(stm, pt, g1);
                removePiece(stm, ROOK, h1);
                placePiece(stm, ROOK, f1);
            }
            else {
                placePiece(stm, pt, g8);
                removePiece(stm, ROOK, h8);
                placePiece(stm, ROOK, f8);
            }
        }
        else {
            if (stm == WHITE) {
                placePiece(stm, pt, c1);
                removePiece(stm, ROOK, a1);
                placePiece(stm, ROOK, d1);
            }
            else {
                placePiece(stm, pt, c8);
                removePiece(stm, ROOK, a8);
                placePiece(stm, ROOK, d8);
            }
        }
        break;
    case PROMOTION:
        placePiece(stm, m.promo(), to);
        break;
    }

    assert(popcount(pieces(WHITE, KING)) == 1);
    assert(popcount(pieces(BLACK, KING)) == 1);

    castlingRights &= CASTLING_RIGHTS[from];
    castlingRights &= CASTLING_RIGHTS[to];

    stm = ~stm;

    updateCheckPin();
}

bool Board::canCastle(Color c, bool kingside) const { return readBit(castlingRights, castleIndex(c, kingside)); }

bool Board::isLegal(Move m) {
    assert(!m.isNull());

    // Castling checks
    if (m.typeOf() == CASTLE) {
        bool kingside = (m.from() < m.to());
        if (!canCastle(stm, kingside))
            return false;
        if (inCheck())
            return false;
        if (stm == WHITE) {
            if (kingside) {
                if ((pieces() & ((1ULL << f1) | (1ULL << g1))) != 0)
                    return false;
                if (isUnderAttack(WHITE, f1) || isUnderAttack(WHITE, g1))
                    return false;
            }
            else {
                if ((pieces() & ((1ULL << b1) | (1ULL << c1) | (1ULL << d1))) != 0)
                    return false;
                if (isUnderAttack(WHITE, d1) || isUnderAttack(WHITE, c1))
                    return false;
            }
        }
        else {
            if (kingside) {
                if ((pieces() & ((1ULL << f8) | (1ULL << g8))) != 0)
                    return false;
                if (isUnderAttack(BLACK, f8) || isUnderAttack(BLACK, g8))
                    return false;
            }
            else {
                if ((pieces() & ((1ULL << b8) | (1ULL << c8) | (1ULL << d8))) != 0)
                    return false;
                if (isUnderAttack(BLACK, d8) || isUnderAttack(BLACK, c8))
                    return false;
            }
        }
        return true;
    }

    u64& king   = byPieces[KING];
    int  kingSq = ctzll(king & byColor[stm]);

    // King moves
    if (king & (1ULL << m.from())) {
        u64& pieces = byColor[stm];

        pieces ^= king;
        king ^= 1ULL << kingSq;

        bool ans = !isUnderAttack(stm, m.to());
        king ^= 1ULL << kingSq;
        pieces ^= king;
        return ans;
    }

    if (m.typeOf() == EN_PASSANT) {
        Board testBoard = *this;
        testBoard.move(m);
        return !testBoard.isUnderAttack(stm, Square(ctzll(testBoard.pieces(stm, KING))));
    }

    // Direct checks
    if ((1ULL << m.to()) & ~checkMask)
        return false;

    // Pins
    return !(pinned & (1ULL << m.from())) || (LINE[m.from()][m.to()] & (king & byColor[stm]));
}

bool Board::inCheck() const { return ~checkMask; }

bool Board::isUnderAttack(Color c, Square square) const {
    assert(square >= a1);
    assert(square < NO_SQUARE);
    // *** SLIDING PIECE ATTACKS ***
    // Straight Directions (Rooks and Queens)
    if (pieces(~c, ROOK, QUEEN) & Movegen::getRookAttacks(square, pieces()))
        return true;

    // Diagonal Directions (Bishops and Queens)
    if (pieces(~c, BISHOP, QUEEN) & Movegen::getBishopAttacks(square, pieces()))
        return true;

    // *** KNIGHT ATTACKS ***
    if (pieces(~c, KNIGHT) & Movegen::KNIGHT_ATTACKS[square])
        return true;

    // *** KING ATTACKS ***
    if (pieces(~c, KING) & Movegen::KING_ATTACKS[square])
        return true;


    // *** PAWN ATTACKS ***
    if (c == WHITE)
        return (Movegen::pawnAttackBB(WHITE, square) & pieces(BLACK, PAWN)) != 0;
    else
        return (Movegen::pawnAttackBB(BLACK, square) & pieces(WHITE, PAWN)) != 0;
}