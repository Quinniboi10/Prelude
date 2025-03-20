#pragma once

#include "types.h"
#include "util.h"

namespace Colors {
// ANSI codes for colors https://raw.githubusercontent.com/fidian/ansi/master/images/color-codes.png
static constexpr std::string_view RESET = "\033[0m";

// Basic colors
static constexpr std::string_view BLACK   = "\033[30m";
static constexpr std::string_view RED     = "\033[31m";
static constexpr std::string_view GREEN   = "\033[32m";
static constexpr std::string_view YELLOW  = "\033[33m";
static constexpr std::string_view BLUE    = "\033[34m";
static constexpr std::string_view MAGENTA = "\033[35m";
static constexpr std::string_view CYAN    = "\033[36m";
static constexpr std::string_view WHITE   = "\033[37m";

// Bright colors
static constexpr std::string_view BRIGHT_BLACK   = "\033[90m";
static constexpr std::string_view BRIGHT_RED     = "\033[91m";
static constexpr std::string_view BRIGHT_GREEN   = "\033[92m";
static constexpr std::string_view BRIGHT_YELLOW  = "\033[93m";
static constexpr std::string_view BRIGHT_BLUE    = "\033[94m";
static constexpr std::string_view BRIGHT_MAGENTA = "\033[95m";
static constexpr std::string_view BRIGHT_CYAN    = "\033[96m";
static constexpr std::string_view BRIGHT_WHITE   = "\033[97m";

static constexpr std::string_view GREY = BRIGHT_BLACK;

struct Color {

    usize r;
    usize g;
    usize b;

    constexpr Color(usize r, usize g, usize b) :
        r(r),
        g(g),
        b(b) {}

    constexpr Color operator*(const double& other) const { return Color(r * other, g * other, b * other); }
    constexpr Color operator+(const Color& other) const { return Color(r + other.r, g + other.g, b + other.b); }
};

namespace RGB {
static constexpr Color WHITE = Color(255, 255, 255);
static constexpr Color RED   = Color(255, 0, 0);
static constexpr Color GREEN = Color(0, 255, 0);
static constexpr Color BLUE  = Color(0, 0, 255);
}

static void setColor(usize r, usize g, usize b) { cout << "\033[38;2;" << r << ";" << g << ";" << b << "m"; }
static void setColor(Color c) { setColor(c.r, c.g, c.b); }
};