#pragma once

#include "types.h"

constexpr u64 INF_U64     = std::numeric_limits<u64>::max();
constexpr int INF_INT     = std::numeric_limits<int>::max();
constexpr int INF_I16     = std::numeric_limits<i16>::max();

constexpr u64 LIGHT_SQ_BB = 0x55AA55AA55AA55AA;
constexpr u64 DARK_SQ_BB  = 0xAA55AA55AA55AA55;