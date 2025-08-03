#pragma once

#include "types.h"

namespace Datagen {
constexpr usize OUTPUT_BUFFER_GAMES = 50;
constexpr usize RAND_MOVES          = 8;
constexpr u64   SOFT_NODES          = 5000;
constexpr u64   HARD_NODES          = 100'000;

void run(usize threads);
void genFens(u64 numFens, u64 seed);
}