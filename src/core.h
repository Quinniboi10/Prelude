#pragma once

#include <bit>

#include "types.h"

inline bool readBit(u64 bb, u8 sq) { return (1ULL << sq) & bb; }