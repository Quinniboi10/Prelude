#pragma once

#include "types.h"

struct TrackedPointer {
    usize position;
    bool  padLeft;

    TrackedPointer() {
        position = 0;
        padLeft  = true;
    }

    void print(string str) {
        position += str.length();
        cout << str;
    }

    void print(string str, usize targetPos) {
        i64 padding = static_cast<i64>(targetPos - position - str.length());
        padding     = std::max(padding, (i64) 0);

        auto printPadding = [&]() {
            for (usize i = 0; i < padding; i++)
                print(" ");
        };

        if (padLeft) {
            print(str);
            printPadding();
        }
        else {
            printPadding();
            print(str);
        }
    }

    void print(const char* str) { print((string) str); }
    void print(const char* str, usize targetPos) { print((string) str, targetPos); }

    void print(auto str) { print(std::to_string(str)); }
    void print(auto str, usize targetPos) { print(std::to_string(str), targetPos); }

    void nextLine() {
        position = 0;
        cout << endl;
    }
};