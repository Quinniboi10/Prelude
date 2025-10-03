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

// Move ordering
Tunable(MO_VICTIM_WEIGHT, 100, 50, 150);

// Time management
Tunable(DEFAULT_MOVES_TO_GO, 19018, 15360, 40960);  // Quantized by 1024
Tunable(INC_DIVISOR, 2156, 512, 5120);              // Quantized by 1024