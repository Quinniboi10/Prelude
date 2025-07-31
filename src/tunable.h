#pragma once

#include "types.h"

#ifdef TUNE
struct IndividualOption {
    string name;
    i32 value;
    i32 min;
    i32 max;
    i32 step;

    IndividualOption(string name, i32 value, i32 min, i32 max, i32 step);

    void setValue(i32 value) { this->value = value; }

    operator i32() const { return value; }
};

inline std::vector<IndividualOption*> tunables;

inline IndividualOption::IndividualOption(string name, i32 value, i32 min, i32 max, i32 step) {
    this->name = name;
    this->value = value;
    this->min = min;
    this->max = max;
    this->step = step;

    tunables.push_back(this);
}

static void setTunable(string name, i32 value) {
    for (auto tunable : tunables) {
        if (tunable->name == name) {
            tunable->value = value;
            break;
        }
    }
}

static void printTuneUCI() {
    for (auto tunable : tunables)
        cout << "option name " << tunable->name << " type spin default " << tunable->value << " min " << tunable->min << " max " << tunable->max << endl;
}

static void printTuneOB() {
    for (auto tunable : tunables)
        cout << tunable->name << ", int, " << tunable->value << ", " << tunable->min << ", " << tunable->max << ", " << tunable->step << ", 0.002" << endl;
}

#define Tunable(name, value, min, max) inline IndividualOption name{#name, value, min, max, (max - min) / 20}
#else
#define Tunable(name, value, min, max) constexpr i32 name = value
#endif

// Histories
Tunable(DEFAULT_HISTORY_VALUE, 0, -200, 200);
constexpr i32 MAX_HISTORY = 16384;

// Aspr windows
constexpr i32 MIN_ASP_WINDOW_DEPTH = 5;
Tunable(INITIAL_ASP_WINDOW, 30, 10, 40);
Tunable(ASP_WIDENING_FACTOR, 2560, 1536, 4096);  // Quantized by 1024

// Move ordering
Tunable(MO_VICTIM_WEIGHT, 100, 10, 200);
Tunable(MO_CAPTURE_SEE_THRESHOLD, -50, -200, 0);
Tunable(MO_CAPTHIST_WEIGHT, 1024, 128, 4096);
Tunable(MO_HIST_WEIGHT, 1024, 128, 4096);
Tunable(MO_CONTHIST_1PLY_WEIGHT, 1024, 128, 4096);
Tunable(MO_CONTHIST_2PLY_WEIGHT, 1024, 128, 4096);

// Main search heuristics
Tunable(RFP_DEPTH_SCALAR, 70, 35, 120);

constexpr i32 RAZORING_DEPTH = 4;
Tunable(RAZORING_DEPTH_SCALAR, 500, 300, 700);

constexpr int NMP_REDUCTION = 3;
constexpr int NMP_DEPTH_DIVISOR = 3;
Tunable(NMP_EVAL_DIVISOR, 160, 120, 200);

Tunable(FUTILITY_PRUNING_MARGIN, 100, 75, 130);
Tunable(FUTILITY_PRUNING_SCALAR, 80, 50, 120);

constexpr int SE_MIN_DEPTH = 8;
Tunable(SE_DOUBLE_MARGIN, 30, 20, 40);

Tunable(LMR_QUIET_CONST, 1382, 512, 2048);     // Quantized by 1024
Tunable(LMR_NOISY_CONST, 205, 0, 512);         // Quantized by 1024
Tunable(LMR_QUIET_DIVISOR, 2816, 1024, 4096);  // Quantized by 1024
Tunable(LMR_NOISY_DIVISOR, 3430, 2048, 4096);  // Quantized by 1024
Tunable(LMR_NONPV, 1024, 128, 2048);

constexpr int MIN_HIST_PRUNING_DEPTH = 5;
Tunable(HIST_PRUNING_MARGIN, -400, -600, -200);
Tunable(HIST_PRUNING_SCALAR, -2500, -3500, -1750);

Tunable(HIST_BONUS_A, 20, 0, 40);
Tunable(HIST_BONUS_B, 0, 0, 40);

Tunable(SEE_PRUNING_DEPTH_SCALAR, -90, -150, -40);

// Qsearch heuristics
Tunable(QS_FUTILITY_MARGIN, 100, 30, 150);

// Time management
Tunable(DEFAULT_MOVES_TO_GO, 20480, 15360, 40960);  // Quantized by 1024
Tunable(INC_DIVISOR, 2048, 512, 5120);              // Quantized by 1024