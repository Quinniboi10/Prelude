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

void Board::placePiece(Color c, PieceType pt, int sq) {
    assert(sq > 0);
    assert(sq < 64);

    auto BBs = stm == WHITE ? white : black;
    BBs[pt] |= 1ULL << sq;
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

    enPassant = 0;

    halfMoveClock = 0;
    fullMoveClock = 1;
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

    switch (mt) {
    case STANDARD_MOVE :
        removePiece(stm, pt, from);
        placePiece(stm, pt, to);
        break;
    }
}