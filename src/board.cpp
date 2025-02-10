#include "board.h"

#include <cassert>

// Returns the piece on a square as a character
char Board::getPieceAt(int i) const {
    constexpr char whiteSymbols[] = {'P', 'N', 'B', 'R', 'Q', 'K'};
    constexpr char blackSymbols[] = {'p', 'n', 'b', 'r', 'q', 'k'};
    for (int j = 0; j < 6; ++j) {
        if (readBit(white[j], i))
            return whiteSymbols[j];
        if (readBit(black[j], i))
            return blackSymbols[j];
    }
    return ' ';
}

u64 Board::pieces() const { return pieces(WHITE) | pieces(BLACK); }
u64 Board::pieces(Color c) const {
    if (c == WHITE) {
        return white[0] | white[1] | white[2] | white[3] | white[4] | white[5];
    }
    return black[0] | black[1] | black[2] | black[3] | black[4] | black[5];
}
u64 Board::pieces(PieceType pt) const { return white[pt] | black[pt]; }

template<Color c>
u64 Board::pawnAttackBB(int sq) {
    assert(sq > 0);
    assert(sq < 64);

    const u64 sqBB = 1ULL << sq;
    if constexpr (c == WHITE) {
        return shift<NORTH_EAST>(sqBB & ~FILE_BB[HFILE]) | shift<NORTH_WEST>(sqBB & ~FILE_BB[AFILE]);
    }
    return shift<SOUTH_EAST>(sqBB & ~FILE_BB[HFILE]) | shift<SOUTH_WEST>(sqBB & ~FILE_BB[AFILE]);
}

void Board::placePiece(Color c, PieceType pt, int sq) {
    assert(sq > 0);
    assert(sq < 64);

    auto& BB = c == WHITE ? white[pt] : black[pt];

    assert(!readBit(BB, sq));

    BB ^= 1ULL << sq;
}

void Board::removePiece(Color c, PieceType pt, int sq) {
    assert(sq > 0);
    assert(sq < 64);

    auto& BB = c == WHITE ? white[pt] : black[pt];

    assert(readBit(BB, sq));

    BB ^= 1ULL << sq;
}

void Board::removePiece(Color c, int sq) {
    assert(sq > 0);
    assert(sq < 64);

    auto& BB = c == WHITE ? white[getPiece(sq)] : black[getPiece(sq)];

    assert(readBit(BB, sq));

    BB ^= 1ULL << sq;
}

// Reset the board to startpos
void Board::reset() {
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

    stm            = WHITE;
    castlingRights = 0b1111;

    epSquare = 0;

    halfMoveClock = 0;
    fullMoveClock = 1;
}


// Load a board from the FEN
void Board::loadFromFEN(string fen) {
    reset();

    // Clear all squares
    for (auto& bb : white) bb = 0;
    for (auto& bb : black) bb = 0;

    std::vector<string> tokens = split(fen, ' ');

    std::vector<string> rankTokens = split(tokens[0], '/');

    int currIdx = 56;

    const char whitePieces[6] = { 'P', 'N', 'B', 'R', 'Q', 'K' };
    const char blackPieces[6] = { 'p', 'n', 'b', 'r', 'q', 'k' };

    for (const string& rank : rankTokens) {
        for (const char c : rank) {
            if (isdigit(c)) { // Empty squares
                currIdx += c - '0';
                continue;
            }
            for (int i = 0; i < 6; i++) {
                if (c == whitePieces[i]) {
                    setBit<1>(white[i], currIdx);
                    break;
                }
                if (c == blackPieces[i]) {
                    setBit<1>(black[i], currIdx);
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
        epSquare = 0;

    halfMoveClock = tokens.size() > 4 ? (stoi(tokens[5])) : 0;
    fullMoveClock = tokens.size() > 5 ? (stoi(tokens[5])) : 1;
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
PieceType Board::getPiece(int sq) {
    u64 mask = 1ULL << sq;
    if (mask & pieces(PAWN))
        return PAWN;
    else if (mask & pieces(KNIGHT))
        return KNIGHT;
    else if (mask & pieces(BISHOP))
        return BISHOP;
    else if (mask & pieces(ROOK))
        return ROOK;
    else if (mask & pieces(QUEEN))
        return QUEEN;
    return KING;
}

// Make a move on the board
void Board::move(Move m) {
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
        removePiece(~stm, PAWN, epSquare);
        placePiece(stm, pt, to);
        break;
    case DOUBLE_PUSH:
        placePiece(stm, pt, to);
        if (pieces(~stm) & (shift<EAST>(1ULL << to) | shift<WEST>(1ULL << to))) // Only set EP square if it could be taken
            epSquare = stm == WHITE ? from + 8 : from - 8;
        break;
    case CASTLE_K:
        placePiece(stm, pt, to);

        if (stm == WHITE) {
            removePiece(stm, ROOK, h1);
            placePiece(stm, ROOK, f1);
        }
        else {
            removePiece(stm, ROOK, h8);
            placePiece(stm, ROOK, f8);
        }
        break;
    case CASTLE_Q:
        placePiece(stm, pt, to);

        if (stm == WHITE) {
            removePiece(stm, ROOK, a1);
            placePiece(stm, ROOK, d1);
        }
        else {
            removePiece(stm, ROOK, a8);
            placePiece(stm, ROOK, d8);
        }
        break;
    case CAPTURE: // This falls through
        removePiece(~stm, to);
    case QUEEN_PROMO:
    case QUEEN_PROMO_CAPTURE:
        placePiece(stm, QUEEN, to);
        break;
    case ROOK_PROMO:
    case ROOK_PROMO_CAPTURE:
        placePiece(stm, ROOK, to);
        break;
    case BISHOP_PROMO:
    case BISHOP_PROMO_CAPTURE:
        placePiece(stm, BISHOP, to);
        break;
    case KNIGHT_PROMO:
    case KNIGHT_PROMO_CAPTURE:
        placePiece(stm, KNIGHT, to);
        break;
    }


    stm = ~stm;
}