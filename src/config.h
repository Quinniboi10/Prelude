#pragma once

#include "types.h"

// ************ SEARCH ************
constexpr usize MAX_PLY = 255;
constexpr i16 BENCH_DEPTH = 4;

// Time management
constexpr usize DEFAULT_MOVES_TO_GO = 20;
constexpr usize INC_DIVISOR         = 2;

extern usize MOVE_OVERHEAD;

// ************ NNUE ************
constexpr i16    QA             = 255;
constexpr i16    QB             = 64;
constexpr i16    EVAL_SCALE     = 400;
constexpr size_t HL_SIZE        = 256;
constexpr size_t OUTPUT_BUCKETS = 8;

constexpr int ReLU   = 0;
constexpr int CReLU  = 1;
constexpr int SCReLU = 2;

constexpr int ACTIVATION = SCReLU;