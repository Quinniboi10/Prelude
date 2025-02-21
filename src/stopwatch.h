#pragma once

#include <chrono>

#include "types.h"

template<typename Precision>
class Stopwatch {
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point pauseTime;

    bool paused;

    u64 pausedTime;

   public:
    Stopwatch() {
        startTime  = std::chrono::high_resolution_clock::now();
        pausedTime = 0;
        paused     = false;
    }

    void start() {
        startTime  = std::chrono::high_resolution_clock::now();
        pausedTime = 0;
        paused     = false;
    }

    u64 elapsed() {
        u64 pausedTime = this->pausedTime;
        if (paused)
            pausedTime += std::chrono::duration_cast<Precision>(std::chrono::high_resolution_clock::now() - pauseTime).count();
        return std::chrono::duration_cast<Precision>(std::chrono::high_resolution_clock::now() - startTime).count() - pausedTime;
    }

    void pause() {
        paused    = true;
        pauseTime = std::chrono::high_resolution_clock::now();
    }
    void resume() {
        paused = false;
        pausedTime += std::chrono::duration_cast<Precision>(std::chrono::high_resolution_clock::now() - pauseTime).count();
    }
};