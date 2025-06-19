#pragma once

#include "types.h"

enum class TableProbe {
    FAILED,
    WIN,
    DRAW,
    LOSS
};

namespace tb {
void setPath(const string& path);

void free();
}

TableProbe probePos(const Board& board);