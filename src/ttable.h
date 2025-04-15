#pragma once

#include "types.h"
#include "move.h"

#include <vector>
#include <thread>
#include <cstring>

struct Transposition {
    u64  zobrist;
    Move move;
    i16  score;
    u8   flag;
    u8   depth;

    Transposition() {
        zobrist = 0;
        move    = Move();
        flag    = 0;
        score   = 0;
        depth   = 0;
    }
    Transposition(u64 zobristKey, Move bestMove, u8 flag, i16 score, u8 depth) {
        this->zobrist = zobristKey;
        this->move    = bestMove;
        this->flag    = flag;
        this->score   = score;
        this->depth   = depth;
    }
};

class TranspositionTable {
    Transposition* table;

   public:
    u64 size;

    TranspositionTable(usize sizeInMB = 16) {
        table = nullptr;
        reserve(sizeInMB);
    }

    ~TranspositionTable() {
        if (table != nullptr)
            std::free(table);
    }


    void clear(usize threadCount = 1) {
        assert(threadCount > 0);

        std::vector<std::thread> threads;

        auto clearTT = [&](usize threadId) {
            // The segment length is the number of entries each thread must clear
            // To find where your thread should start (in entries), you can do threadId * segmentLength
            // Converting segment length into the number of entries to clear can be done via length * bytes per entry

            usize start = (size * threadId) / threadCount;
            usize end   = std::min((size * (threadId + 1)) / threadCount, size);

            std::memset(table + start, 0, (end - start) * sizeof(Transposition));
        };

        for (usize thread = 1; thread < threadCount; thread++)
            threads.emplace_back(clearTT, thread);

        clearTT(0);

        for (std::thread& t : threads)
            if (t.joinable())
                t.join();
    }

    void reserve(usize newSizeMiB) {
        assert(newSizeMiB > 0);
        // Find number of bytes allowed
        size = newSizeMiB * 1024 * 1024 / sizeof(Transposition);
        if (table != nullptr)
            std::free(table);
        table = static_cast<Transposition*>(std::malloc(size * sizeof(Transposition)));
    }

    u64 index(u64 key) {
    #ifdef _MSC_VER
        return key % size;
    #else
        return static_cast<u64>((static_cast<__int128>(key) * static_cast<__int128>(size)) >> 64);
    #endif
    }

    void setEntry(u64 key, Transposition& entry) { table[index(key)] = entry; }

    Transposition* getEntry(u64 key) { return &table[index(key)]; }
};