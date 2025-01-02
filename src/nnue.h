/*
    Prelude
    Copyright (C) 2024 Quinniboi10

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#ifndef EVALFILE
#define EVALFILE "./nnue.bin"
#endif

#include "../external/incbin.h"

INCBIN(EVAL, EVALFILE);

constexpr i16 inputQuantizationValue = 255;
constexpr i16 hiddenQuantizationValue = 64;
constexpr i16 evalScale = 400;
constexpr size_t HL_SIZE = 1024;

constexpr int ReLU = 0;
constexpr int CReLU = 1;
constexpr int SCReLU = 2;

constexpr int activation = SCReLU;

using Accumulator = array<i16, HL_SIZE>;