#pragma once

#include "types.h"

// ************ SEARCH ************
constexpr usize MAX_PLY     = 255;
constexpr i16   BENCH_DEPTH = 12;

constexpr int DEFAULT_HISTORY_VALUE = 0;
constexpr int MAX_HISTORY           = 16384;

constexpr int MIN_ASP_WINDOW_DEPTH = 5;
constexpr int INITIAL_ASP_WINDOW   = 30;

constexpr int NMP_REDUCTION = 3;

constexpr int FUTILITY_PRUNING_MARGIN = 100;
constexpr int FUTILITY_PRUNING_SCALAR = 80;

// Time management
constexpr usize DEFAULT_MOVES_TO_GO = 20;
constexpr usize INC_DIVISOR         = 2;

extern usize MOVE_OVERHEAD;

// ************ NNUE ************
constexpr i16    QA             = 255;
constexpr i16    QB             = 64;
constexpr i16    EVAL_SCALE     = 400;
constexpr size_t HL_SIZE        = 512;
constexpr size_t OUTPUT_BUCKETS = 8;

constexpr int ReLU   = 0;
constexpr int CReLU  = 1;
constexpr int SCReLU = 2;

constexpr int ACTIVATION = SCReLU;