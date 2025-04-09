#include "move.h"
#include "board.h"
#include "globals.h"

Move::Move(string strIn, Board& board) {
    Square from = parseSquare(strIn.substr(0, 2));
    Square to   = parseSquare(strIn.substr(2, 2));

    MoveType flags = STANDARD_MOVE;

    // Move must be promotion
    if (strIn.size() > 4) {
        switch (strIn.at(4)) {
        case 'q':
            *this = Move(from, to, QUEEN);
            return;
        case 'r':
            *this = Move(from, to, ROOK);
            return;
        case 'b':
            *this = Move(from, to, BISHOP);
            return;
        default:
            *this = Move(from, to, KNIGHT);
            return;
        }
    }

    if (!chess960
        && ((from == e1 && to == g1 && board.canCastle(WHITE, true)) || (from == e1 && to == c1 && board.canCastle(WHITE, false)) || (from == e8 && to == g8 && board.canCastle(BLACK, true))
            || (from == e8 && to == c8 && board.canCastle(BLACK, false)))) {
        const bool kingside = to > from;

        to = board.castleSq(board.stm, kingside);

        flags = CASTLE;
    }
    else if (chess960 && board.getPiece(from) == KING && ((1ULL << to) & board.pieces(board.stm, ROOK))) {
        const bool kingside = to > from;
        if (board.canCastle(board.stm, kingside))
            flags = CASTLE;
    }
    else if (to == board.epSquare && ((1ULL << from) & board.pieces(board.stm, PAWN)))
        flags = EN_PASSANT;

    *this = Move(from, to, flags);
}

string Move::toString() const {
    MoveType mt = typeOf();

    string moveStr = squareToAlgebraic(from());
    if (mt == CASTLE && !chess960)
        return moveStr + (squareToAlgebraic(to() + (from() < to() ? WEST : EAST * 2)));

    moveStr += squareToAlgebraic(to());

    if (mt != PROMOTION)
        return moveStr;

    switch (promo()) {
    case KNIGHT:
        moveStr += 'n';
        break;
    case BISHOP:
        moveStr += 'b';
        break;
    case ROOK:
        moveStr += 'r';
        break;
    case QUEEN:
        moveStr += 'q';
        break;
    default:
        break;
    }

    return moveStr;
};