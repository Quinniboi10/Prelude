#include "board.h"
#include "util.h"
#include "types.h"
#include "movegen.h"
#include "globals.h"
#include "search.h"
#include "constants.h"

#include <cassert>
#include <random>

// Piece zobrist table
array<array<array<u64, 64>, 6>, 2> PIECE_ZTABLE;
// En passant zobrist table
array<u64, 65> EP_ZTABLE;
// Zobrist for stm
u64 STM_ZHASH;
// Zobrist for castling rights
array<u64, 16> CASTLING_ZTABLE;

void Board::fillZobristTable() {
    std::random_device rd;
    std::mt19937_64    engine(rd());
    engine.seed(69420);  // Nice
    std::uniform_int_distribution<u64> dist(0, ~0ULL);

    for (auto& stm : PIECE_ZTABLE)
        for (auto& pt : stm)
            for (auto& piece : pt)
                piece = dist(engine);

    for (auto& ep : EP_ZTABLE)
        ep = dist(engine);

    STM_ZHASH = dist(engine);

    for (auto& right : CASTLING_ZTABLE)
        right = dist(engine);
}

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

    zobrist ^= PIECE_ZTABLE[c][pt][sq];

    BB ^= 1ULL << sq;
    byColor[c] ^= 1ULL << sq;

    mailbox[sq] = pt;
}

void Board::removePiece(Color c, PieceType pt, int sq) {
    assert(sq >= 0);
    assert(sq < 64);

    auto& BB = byPieces[pt];

    assert(readBit(BB, sq));

    zobrist ^= PIECE_ZTABLE[c][pt][sq];

    BB ^= 1ULL << sq;
    byColor[c] ^= 1ULL << sq;

    mailbox[sq] = NO_PIECE_TYPE;
}

void Board::removePiece(Color c, int sq) {
    assert(sq >= 0);
    assert(sq < 64);

    auto& BB = byPieces[getPiece(sq)];

    assert(readBit(BB, sq));

    zobrist ^= PIECE_ZTABLE[c][getPiece(sq)][sq];

    BB ^= 1ULL << sq;
    byColor[c] ^= 1ULL << sq;

    mailbox[sq] = NO_PIECE_TYPE;
}

void Board::resetMailbox() {
    mailbox.fill(NO_PIECE_TYPE);
    for (u8 i = 0; i < 64; i++) {
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

void Board::resetZobrist() {
    zobrist = 0;

    for (PieceType pt = PAWN; pt <= KING; pt = PieceType(static_cast<int>(pt) + 1)) {
        u64 pcs = pieces(WHITE, pt);
        while (pcs) {
            Square sq = popLSB(pcs);
            zobrist ^= PIECE_ZTABLE[WHITE][pt][sq];
        }

        pcs = pieces(BLACK, pt);
        while (pcs) {
            Square sq = popLSB(pcs);
            zobrist ^= PIECE_ZTABLE[BLACK][pt][sq];
        }
    }

    zobrist ^= hashCastling();
    zobrist ^= EP_ZTABLE[epSquare];
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

void Board::setCastlingRights(Color c, Square sq, bool value) { castling[castleIndex(c, ctzll(pieces(c, KING)) < sq)] = (value == false ? NO_SQUARE : sq); }

void Board::unsetCastlingRights(Color c) { castling[castleIndex(c, true)] = castling[castleIndex(c, false)] = NO_SQUARE; }

u64 Board::hashCastling() const {
    constexpr usize blackQ = 0b1;
    constexpr usize blackK = 0b10;
    constexpr usize whiteQ = 0b100;
    constexpr usize whiteK = 0b1000;

    usize flags = 0;

    if (castling[castleIndex(WHITE, true)])
        flags |= whiteK;
    if (castling[castleIndex(WHITE, false)])
        flags |= whiteQ;
    if (castling[castleIndex(BLACK, true)])
        flags |= blackK;
    if (castling[castleIndex(BLACK, false)])
        flags |= blackQ;

    return CASTLING_ZTABLE[flags];
}

u8 Board::count(PieceType pt) const { return popcount(pieces(pt)); }

u64 Board::pieces() const { return byColor[WHITE] | byColor[BLACK]; }
u64 Board::pieces(Color c) const { return byColor[c]; }
u64 Board::pieces(PieceType pt) const { return byPieces[pt]; }
u64 Board::pieces(Color c, PieceType pt) const { return byPieces[pt] & byColor[c]; }
u64 Board::pieces(PieceType pt1, PieceType pt2) const { return byPieces[pt1] | byPieces[pt2]; }
u64 Board::pieces(Color c, PieceType pt1, PieceType pt2) const { return (byPieces[pt1] | byPieces[pt2]) & byColor[c]; }

u64 Board::attackersTo(Square sq, u64 occ) const {
    return (Movegen::getRookAttacks(sq, occ) & pieces(ROOK, QUEEN)) | (Movegen::getBishopAttacks(sq, occ) & pieces(BISHOP, QUEEN)) | (Movegen::pawnAttackBB(WHITE, sq) & pieces(BLACK, PAWN))
         | (Movegen::pawnAttackBB(BLACK, sq) & pieces(WHITE, PAWN)) | (Movegen::KNIGHT_ATTACKS[sq] & pieces(KNIGHT)) | (Movegen::KING_ATTACKS[sq] & pieces(KING));
}

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


    stm      = WHITE;
    castling = {a8, h8, a1, h1};

    epSquare = NO_SQUARE;

    halfMoveClock = 0;
    fullMoveClock = 1;

    fromNull = false;

    resetMailbox();
    resetZobrist();
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

    castling.fill(NO_SQUARE);
    if (tokens[2].find('-') == string::npos) {
        // Standard FEN
        // Standard FEN and maybe XFEN later
        if (tokens[2].find('K') != string::npos)
            castling[castleIndex(WHITE, true)] = h1;
        if (tokens[2].find('Q') != string::npos)
            castling[castleIndex(WHITE, false)] = a1;
        if (tokens[2].find('k') != string::npos)
            castling[castleIndex(BLACK, true)] = h8;
        if (tokens[2].find('q') != string::npos)
            castling[castleIndex(BLACK, false)] = a8;

        // FRC FEN
        if (std::tolower(tokens[2][0]) >= 'a' && std::tolower(tokens[2][0]) <= 'h') {
            chess960 = true;
            for (char token : tokens[2]) {
                File file = static_cast<File>(std::tolower(token) - 'a');

                if (std::isupper(token))
                    setCastlingRights(WHITE, toSquare(RANK1, file), true);
                else
                    setCastlingRights(BLACK, toSquare(RANK8, file), true);
            }
        }
    }

    if (tokens[3] != "-")
        epSquare = parseSquare(tokens[3]);
    else
        epSquare = NO_SQUARE;

    halfMoveClock = tokens.size() > 4 ? (stoi(tokens[5])) : 0;
    fullMoveClock = tokens.size() > 5 ? (stoi(tokens[5])) : 1;

    fromNull = false;

    resetMailbox();
    resetZobrist();
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
    cout << endl;
    cout << "Board hash: 0x" << std::hex << std::uppercase << zobrist << std::dec << endl;
}

// Return the type of the piece on the square
PieceType Board::getPiece(int sq) const { return mailbox[sq]; }

// This should return false if
// Move is a capture of any kind
// Move is a queen promotion
// Move is a knight promotion
bool Board::isQuiet(Move m) const { return !isCapture(m) && (m.typeOf() != PROMOTION || m.promo() != QUEEN); }

bool Board::isCapture(Move m) const { return ((1ULL << m.to() & pieces(~stm)) || m.typeOf() == EN_PASSANT); }

// Make a move from a string
void Board::move(string str) { move(Move(str, *this)); }

// Make a move
void Board::move(Move m) {
    zobrist ^= hashCastling();
    zobrist ^= EP_ZTABLE[epSquare];

    epSquare        = NO_SQUARE;
    fromNull        = false;
    Square    from  = m.from();
    Square    to    = m.to();
    MoveType  mt    = m.typeOf();
    PieceType pt    = getPiece(from);
    PieceType toPT  = NO_PIECE_TYPE;

    removePiece(stm, pt, from);
    if (isCapture(m)) {
        toPT          = getPiece(to);
        halfMoveClock = 0;
        posHistory.clear();
        if (mt != EN_PASSANT) {
            removePiece(~stm, toPT, to);
        }
    }
    else {
        if (pt == PAWN)
            halfMoveClock = 0;
        else
            halfMoveClock++;
    }

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
        assert(getPiece(to) == ROOK);
        removePiece(stm, ROOK, to);
        if (stm == WHITE) {
            if (from < to) {
                placePiece(stm, KING, g1);
                placePiece(stm, ROOK, f1);
            }
            else {
                placePiece(stm, KING, c1);
                placePiece(stm, ROOK, d1);
            }
        }
        else {
            if (from < to) {
                placePiece(stm, KING, g8);
                placePiece(stm, ROOK, f8);
            }
            else {
                placePiece(stm, KING, c8);
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

    if (pt == ROOK) {
        const Square sq = castleSq(stm, from > ctzll(pieces(stm, KING)));
        if (from == sq)
            setCastlingRights(stm, from, false);
    }
    else if (pt == KING)
        unsetCastlingRights(stm);
    if (toPT == ROOK) {
        const Square sq = castleSq(~stm, to > ctzll(pieces(~stm, KING)));
        if (to == sq)
            setCastlingRights(~stm, to, false);
    }

    stm = ~stm;

    zobrist ^= hashCastling();
    zobrist ^= EP_ZTABLE[epSquare];
    zobrist ^= STM_ZHASH;

    posHistory.push_back(zobrist);

    fullMoveClock += stm == WHITE;

    updateCheckPin();
}

bool Board::canNullMove() const {
    // Don't allow back-to-back null moves
    if (fromNull == true)
        return false;
    // Pawn + king only endgame
    if (popcount(pieces(stm)) - popcount(pieces(stm, PAWN)) == 1)
        return false;

    return true;
}

void Board::nullMove() {
    // En passant
    zobrist ^= EP_ZTABLE[epSquare];
    zobrist ^= EP_ZTABLE[NO_SQUARE];

    epSquare = NO_SQUARE;

    // Stm
    zobrist ^= STM_ZHASH;
    stm = ~stm;

    posHistory.push_back(zobrist);

    fromNull = true;
    updateCheckPin();
}

bool Board::canCastle(Color c, bool kingside) const { return castleSq(c, kingside) != NO_SQUARE; }

bool Board::isLegal(Move m) {
    assert(!m.isNull());

    // Castling checks
    if (m.typeOf() == CASTLE) {
        if (inCheck())
            return false;

        const bool kingside = (m.from() < m.to());

        if (!canCastle(stm, kingside))
            return false;

        if (pinned & (1ULL << m.to()))
            return false;

        const Square kingEndSq = toSquare(stm == WHITE ? RANK1 : RANK8, kingside ? GFILE : CFILE);
        const Square rookEndSq = toSquare(stm == WHITE ? RANK1 : RANK8, kingside ? FFILE : DFILE);

        u64 betweenBB = (LINESEG[m.from()][kingEndSq] | LINESEG[m.to()][rookEndSq]) ^ (1ULL << m.from()) ^ (1ULL << m.to());

        if (pieces() & betweenBB)
            return false;

        betweenBB = LINESEG[m.from()][kingEndSq] ^ (1ULL << m.from());

        while (betweenBB)
            if (isUnderAttack(stm, popLSB(betweenBB)))
                return false;

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

bool Board::isDraw() {
    // 50 move rule
    if (halfMoveClock >= 100)
        return !inCheck() || Movegen::generateLegalMoves(*this).length != 0;

    // Insufficient material
    if (pieces(PAWN) == 0                                 // No pawns
        && pieces(QUEEN) == 0                             // No queens
        && pieces(ROOK) == 0                              // No rooks
        && ((pieces(BISHOP) & LIGHT_SQ_BB) == 0           // No light sq bishops
            || (pieces(BISHOP) & DARK_SQ_BB) == 0)        // OR no dark sq bishops
        && ((pieces(BISHOP) == 0 || pieces(KNIGHT)) == 0) // Not bishop + knight
        && popcount(pieces(KNIGHT)) < 2)                  // Under 2 knights
        return true;

    // Threefold
    u8 seen = 0;
    for (u64 hash : posHistory) {
        seen += hash == zobrist;
        if (seen >= 2)
            return true;
    }
    return false;
}

bool Board::isGameOver() {
    if (isDraw())
        return true;
    MoveList moves = Movegen::generateLegalMoves(*this);
    return moves.length == 0;
}

// Uses the swap-off algorithm, code from Stockfish
bool Board::see(Move m, int threshold) const {
    if (m.typeOf() != STANDARD_MOVE)
        return 0 >= threshold;

    Square from = m.from();
    Square to   = m.to();

    int swap = PIECE_VALUES[getPiece(to)] - threshold;
    if (swap <= 0)
        return false;

    swap = PIECE_VALUES[getPiece(from)] - swap;
    if (swap <= 0)
        return true;

    u64   occ       = pieces() ^ (1ULL << from) ^ (1ULL << to);
    Color stm       = this->stm;
    u64   attackers = attackersTo(to, occ);
    u64   stmAttackers, bb;

    int res = 1;

    while (true) {
        stm = ~stm;
        attackers &= occ;

        stmAttackers = attackers & pieces(stm);
        if (!(stmAttackers))
            break;

        if (pinnersPerC[~stm] & occ) {
            stmAttackers &= ~pinned;
            if (!stmAttackers)
                break;
        }

        res ^= 1;

        if ((bb = stmAttackers & pieces(PAWN))) {
            swap = PIECE_VALUES[PAWN] - swap;
            if (swap < res)
                break;
            occ ^= 1ULL << ctzll(bb);  // LSB as a bitboard

            attackers |= Movegen::getBishopAttacks(to, occ) & pieces(BISHOP, QUEEN);
        }

        else if ((bb = stmAttackers & pieces(KNIGHT))) {
            swap = PIECE_VALUES[KNIGHT] - swap;
            if (swap < res)
                break;
            occ ^= 1ULL << ctzll(bb);
        }

        else if ((bb = stmAttackers & pieces(BISHOP))) {
            swap = PIECE_VALUES[BISHOP] - swap;
            if (swap < res)
                break;
            occ ^= 1ULL << ctzll(bb);

            attackers |= Movegen::getBishopAttacks(to, occ) & pieces(BISHOP, QUEEN);
        }

        else if ((bb = stmAttackers & pieces(ROOK))) {
            swap = PIECE_VALUES[ROOK] - swap;
            if (swap < res)
                break;
            occ ^= 1ULL << ctzll(bb);

            attackers |= Movegen::getRookAttacks(to, occ) & pieces(ROOK, QUEEN);
        }

        else if ((bb = stmAttackers & pieces(QUEEN))) {
            swap = PIECE_VALUES[QUEEN] - swap;
            if (swap < res)
                break;
            occ ^= 1ULL << ctzll(bb);

            attackers |= (Movegen::getBishopAttacks(to, occ) & pieces(BISHOP, QUEEN)) | (Movegen::getRookAttacks(to, occ) & pieces(ROOK, QUEEN));
        }
        else
            return (attackers & ~pieces(stm)) ? res ^ 1 : res;  // King capture so flip side if enemy has attackers
    }

    return res;
}