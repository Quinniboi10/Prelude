#pragma once

struct Transposition {
    u64 zobristKey;
    Move bestMove;
    i16 score;
    u8 flag;
    u8 depth;

    Transposition() {
        zobristKey = 0;
        bestMove = Move();
        flag = 0;
        score = 0;
        depth = 0;
    }
    Transposition(u64 zobristKey, Move bestMove, u8 flag, i16 score, u8 depth) {
        this->zobristKey = zobristKey;
        this->bestMove = bestMove;
        this->flag = flag;
        this->score = score;
        this->depth = depth;
    }
};

struct TranspositionTable {
private:
    std::vector<Transposition> table;
public:
    size_t size;

    TranspositionTable(float sizeInMB = 16) {
        resize(sizeInMB);
    }

    void clear() {
        table.clear();
    }

    void resize(float newSizeMiB) {
        // Find number of bytes allowed
        size = newSizeMiB * 1024 * 1024;
        // Divide by size of transposition entry
        size /= sizeof(Transposition);
        if (size == 0) size += 1;
        IFDBG cout << "Using transposition table with " << formatNum(size) << " entries" << endl;
        table.resize(size);
    }

    int index(u64 key, u64 size) {
        return key % size;
    }

    void setEntry(u64 key, Transposition& entry) {
        auto& loc = table[index(key, size)];
        loc = entry;
    }

    Transposition* getEntry(u64 key) {
        return &table[index(key, size)];
    }
};

// This is the main transposition table to be used through the rest of the code
extern TranspositionTable TT;