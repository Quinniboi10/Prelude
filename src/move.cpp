#include "move.h"
#include "board.h"

Move::Move(string strIn, Board& board) {
    Square from = parseSquare(strIn.substr(0, 2));
    Square to   = parseSquare(strIn.substr(2, 2));

    int flags = 0;

    if (strIn.size() > 4) {  // Move must be promotion
        switch (strIn.at(4)) {
        case 'q':
            flags |= QUEEN_PROMO;
            break;
        case 'r':
            flags |= ROOK_PROMO;
            break;
        case 'b':
            flags |= BISHOP_PROMO;
            break;
        default:
            flags |= KNIGHT_PROMO;
            break;
        }
    }

    if (from == e1 && to == g1 && board.canCastle(WHITE, true))
        flags = CASTLE_K;
    else if (from == e1 && to == c1 && board.canCastle(WHITE, false))
        flags = CASTLE_Q;
    else if (from == e8 && to == g8 && board.canCastle(BLACK, true))
        flags = CASTLE_K;
    else if (from == e8 && to == c8 && board.canCastle(BLACK, false))
        flags = CASTLE_Q;

    if (to == board.epSquare && ((1ULL << from) & board.pieces(~board.stm, PAWN)))
        flags = EN_PASSANT;

    *this = Move(from, to, flags);
}

string Move::toString() const {
    MoveType mt = typeOf();

    string moveStr = squareToAlgebraic(from());
    if (mt == CASTLE_K || mt == CASTLE_Q)
        return moveStr + (squareToAlgebraic(to() + (mt == CASTLE_K ? WEST : EAST * 2)));

    moveStr += squareToAlgebraic(to());

    if ((mt & PROMOTION) == 0)
        return moveStr;

    switch (mt) {
    case KNIGHT_PROMO:
        moveStr += 'n';
        break;
    case BISHOP_PROMO:
        moveStr += 'b';
        break;
    case ROOK_PROMO:
        moveStr += 'r';
        break;
    case QUEEN_PROMO:
        moveStr += 'q';
        break;
    default:
        break;
    }

    return moveStr;
};