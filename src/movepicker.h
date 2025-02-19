#pragma once

#include "types.h"
#include "move.h"
#include "board.h"

class Movepicker {
    enum PickerStage {
        TT_MOVE,
        OTHER
    };

    Board&      board;
    MoveList    moves;
    PickerStage state;
    u16         seen;
    Move        TTMove;
    bool        passQuiets;

    int evaluateMove(Move m);

   public:
    Movepicker(Board& board);

    bool hasNext();
    Move getNext();

    void skipQuiets();
};