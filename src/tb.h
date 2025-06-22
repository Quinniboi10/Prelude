#pragma once

#include "types.h"
#include "move.h"
#include "globals.h"

enum class TableProbe {
    FAILED,
    WIN,
    DRAW,
    LOSS
};

namespace tb {
extern i32 PIECES;

void setPath(const string& path);

void free();


TableProbe probePos(const Board& board);

TableProbe probeRoot(MoveList& rootMoves, const Board& board);
}

struct TBManager {
    ~TBManager() {
        if (tbEnabled)
            tb::free();
    }
};