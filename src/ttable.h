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

    explicit TranspositionTable(usize sizeInMB = 16) {
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

            const usize start = (size * threadId) / threadCount;
            const usize end   = std::min((size * (threadId + 1)) / threadCount, size);

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

    u64 index(u64 key) const { return static_cast<u64>((static_cast<u128>(key) * static_cast<u128>(size)) >> 64); }

    void prefetch(u64 key) { __builtin_prefetch(&getEntry(key)); }

    void setEntry(u64 key, const Transposition& entry) { getEntry(key) = entry; }

    Transposition& getEntry(u64 key) { return table[index(key)]; }

    bool shouldReplace(const Transposition& entry, const Transposition& newEntry) const { return true; }

    usize hashfull() const {
        const usize samples = std::min<u64>(1000, size);
        usize hits    = 0;
        for (usize sample = 0; sample < samples; sample++)
            hits += table[sample].zobrist != 0;
        usize hash = (int) (hits / (double) samples * 1000);
        assert(hash <= 1000);
        return hash;
    }
};