#pragma once

#include "types.h"

// ************ SEARCH ************
constexpr int MATE_SCORE = 32767;  // Max for i16
constexpr int MAX_PLY    = 255;

constexpr int MATE_IN_MAX_PLY  = MATE_SCORE - MAX_PLY;
constexpr int MATED_IN_MAX_PLY = -MATE_SCORE + MAX_PLY;

// ****** PRUNING ******
constexpr int RFP_MARGIN            = 75;
constexpr int NMP_REDUCTION         = 3;  // NMP depth reduction
constexpr int NMP_REDUCTION_DIVISOR = 3;  // Subtract depth/n from NMP depth

constexpr int MIN_MOVES_BEFORE_LMP = 7;  // Only does LMP if moves search >= n * depth * depth
constexpr int MAX_DEPTH_FOR_LMP    = 5;  // Only does late move pruning if depth <= n

constexpr int RAZOR_MARGIN       = 475;  // Margin to use for razoring
constexpr int RAZOR_DEPTH_SCALAR = 300;  // Depth scalar to use for razoring

constexpr int PVS_SEE_QUIET_SCALAR   = -80;
constexpr int PVS_SEE_CAPTURE_SCALAR = -30;

constexpr int HISTORY_PRUNING_SCALAR = -2500;  // Depth scalar for history pruning

constexpr int SEE_MARGIN = -108;  // Margin for qsearch SEE pruning

// ****** TIME MANAGEMENT ******
constexpr int    DEFAULT_MOVES_TO_GO = 20;    // Defualt moves remaining in the game for time management
constexpr int    INC_SCALAR          = 1;     // Amount of increment to add to hard time limit
constexpr double SOFT_TIME_SCALAR    = 0.65;  // Scales the soft limit as hardLimit * n

// ****** OTHER ******
constexpr int    ASPR_DELTA           = 25;    // Used as delta size in aspiration window
constexpr double ASP_DELTA_MULTIPLIER = 1.25;  // Scalar to widen aspr window on fail

constexpr int SEE_CAPTURE_ORDERING_THRESHOLD = -50;  // Threshold for SEE move ordering (if fails, bad noisy)

constexpr int MAX_HISTORY = 16384;  // Max history bonus

// ************ DATA GEN ************
constexpr int TARGET_POSITIONS      = 1'000'000'000;  // Number of positions to generate
constexpr int DATAGEN_OUTPUT_BUFFER = 10'000;              // Size in positions of the output buffer
constexpr int RAND_MOVES            = 8;              // Number of random halfmoves before data gen begins
constexpr int NODES_PER_MOVE        = 5000;           // Soft nodes per move
constexpr int MAX_NODES_PER_MOVE    = 100'000;        // Hard nodes per move


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