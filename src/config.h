#pragma once

#include "types.h"

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