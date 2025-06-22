#pragma once

#include "nnue.h"

extern bool chess960;
extern bool tbEnabled;
extern i32  syzygyDepth;
extern i32  syzygyProbeLimit;
extern NNUE nnue;

extern MultiArray<u64, 64, 64> LINE;
extern MultiArray<u64, 64, 64> LINESEG;