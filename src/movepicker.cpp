#include "movepicker.h"
#include "movegen.h"
#include "transpostable.h"

Movepicker::Movepicker(Board& board) : board(board) {
    moves                = Movegen::generateMoves(board);
    seen                 = 0;
    Transposition& entry = TT.getEntry(board.zobrist);
    passQuiets           = false;

    if (entry.zobrist == board.zobrist) {
        TTMove = entry.move;
        state  = TT_MOVE;
    }
    else {
        TTMove = Move::null();
        state  = OTHER;
    }
}

bool Movepicker::hasNext() { return seen + 1 < moves.length && !(passQuiets && board.isQuiet(moves.moves[seen + 1])); }

Move Movepicker::getNext() {
    assert(hasNext());
    switch (state) {
        case TT_MOVE:
            state = OTHER;
            return TTMove;
        case OTHER:
            if (moves.moves[seen + 1] != TTMove)
                return moves.moves[seen++];
            seen++;
            return moves.moves[seen++];
    }
}

void Movepicker::skipQuiets() { passQuiets = true; }