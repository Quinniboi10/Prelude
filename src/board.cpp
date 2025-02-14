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

    switch (mt) {
    case STANDARD_MOVE:
        placePiece(stm, pt, to);
        break;
    case EN_PASSANT:
        removePiece(~stm, PAWN, to + (stm == WHITE ? SOUTH : NORTH));
        placePiece(stm, pt, to);
        break;
    case DOUBLE_PUSH:
        placePiece(stm, pt, to);
        if (pieces(~stm, PAWN) & (shift<EAST>((1ULL << to) & ~MASK_FILE[HFILE]) | shift<WEST>((1ULL << to) & ~MASK_FILE[AFILE])))  // Only set EP square if it could be taken
            epSquare = Square(stm == WHITE ? from + NORTH : from + SOUTH);
        break;
    case CASTLE_K:
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
        break;
    case CASTLE_Q:
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
        break;
    case CAPTURE:
        removePiece(~stm, to);
        placePiece(stm, pt, to);
        break;

    case QUEEN_PROMO:
        placePiece(stm, QUEEN, to);
        break;
    case QUEEN_PROMO_CAPTURE:
        removePiece(~stm, to);
        placePiece(stm, QUEEN, to);
        break;
    case ROOK_PROMO:
        placePiece(stm, ROOK, to);
        break;
    case ROOK_PROMO_CAPTURE:
        removePiece(~stm, to);
        placePiece(stm, ROOK, to);
        break;
    case BISHOP_PROMO:
        placePiece(stm, BISHOP, to);
        break;
    case BISHOP_PROMO_CAPTURE:
        removePiece(~stm, to);
        placePiece(stm, BISHOP, to);
        break;
    case KNIGHT_PROMO:
        placePiece(stm, KNIGHT, to);
        break;
    case KNIGHT_PROMO_CAPTURE:
        removePiece(~stm, to);
        placePiece(stm, KNIGHT, to);
        break;
    }

    assert(popcount(pieces(KING)) == 2);

    castlingRights &= CASTLING_RIGHTS[from];
    castlingRights &= CASTLING_RIGHTS[to];

    stm = ~stm;
}

bool Board::canCastle(Color c, bool kingside) const { return readBit(castlingRights, castleIndex(c, kingside)); }

bool Board::isLegal(Move m) const {
    assert(!m.isNull());

    // Castling checks
    if (m.typeOf() == CASTLE_K || m.typeOf() == CASTLE_Q) {
        bool kingside = (m.typeOf() == CASTLE_K);
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

    Board testBoard = *this;
    testBoard.move(m);
    return !testBoard.isUnderAttack(stm, Square(ctzll(testBoard.pieces(stm, KING))));
}

bool Board::inCheck() const {
    Square kingSq = Square(ctzll(pieces(stm, KING)));

    return isUnderAttack(kingSq);
}

bool Board::isUnderAttack(Square square) const { return isUnderAttack(stm, square); }

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