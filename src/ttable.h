#pragma once

#include "types.h"
#include "move.h"

#include <vector>

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
        score    = 0;
        depth   = 0;
    }
    Transposition(u64 zobristKey, Move bestMove, u8 flag, i16 score, u8 depth) {
        this->zobrist = zobristKey;
        this->move    = bestMove;
        this->flag    = flag;
        this->score    = score;
        this->depth   = depth;
    }
};

struct TranspositionTable {
   private:
    std::vector<Transposition> table;

   public:
    u64 size;

    TranspositionTable(usize sizeInMB = 16) { resize(sizeInMB); }

    void clear() { std::fill(table.begin(), table.end(), Transposition{}); }

    void resize(usize newSizeMiB) {
        // Find number of bytes allowed
        size = newSizeMiB * 1024 * 1024 / sizeof(Transposition);
        if (size == 0)
            size += 1;
        table.resize(size);
    }

    u64 index(u64 key) { return key % size; }

    void setEntry(u64 key, Transposition& entry) { table[index(key)] = entry; }

    Transposition* getEntry(u64 key) { return &table[index(key)]; }
};