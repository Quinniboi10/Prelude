#pragma once

#include "types.h"
#include "move.h"

#include <vector>
#include <thread>
#include <cstring>

#include <cstddef>
#include <cstdlib>
#include <stdexcept>

#if defined(_MSC_VER) || defined(__MINGW32__)
    #include <malloc.h>
#elif defined(__linux__)
    #include <sys/mman.h>
#endif

// Alignment code based on Integral
inline void* alignedAlloc(usize alignment, usize size) {
    void* ptr = nullptr;

#if defined(_MSC_VER) || defined(__MINGW32__)
    ptr = _aligned_malloc(size, alignment);
    if (!ptr)
        throw std::bad_alloc();
#elif defined(__APPLE__) || defined(__unix__)
    if (posix_memalign(&ptr, alignment, size) != 0)
        throw std::bad_alloc();
#else
    // C++17+ std::aligned_alloc, size must be multiple of alignment
    if (size % alignment != 0) {
        size += alignment - (size % alignment);
    }
    ptr = std::aligned_alloc(alignment, size);
    if (!ptr)
        throw std::bad_alloc();
#endif

#if defined(__linux__)
    // Optional: Performance hint for huge pages
    madvise(ptr, size, MADV_HUGEPAGE);
#endif

    return ptr;
}

template<typename T, usize Alignment>
struct AlignedAllocator {
    using value_type = T;

    AlignedAllocator() noexcept {}
    template<typename U>
    AlignedAllocator(const AlignedAllocator<U, Alignment>&) noexcept {}

    T* allocate(usize n) {
        void* ptr = alignedAlloc(Alignment, n * sizeof(T));
        return static_cast<T*>(ptr);
    }

    void deallocate(T* p, usize) noexcept {
#if defined(_MSC_VER) || defined(__MINGW32__)
        _aligned_free(p);
#else
        std::free(p);
#endif
    }
};

template<typename T1, usize A1, typename T2, usize A2>
bool operator==(AlignedAllocator<T1, A1>, AlignedAllocator<T2, A2>) noexcept {
    return A1 == A2;
}

template<typename T1, usize A1, typename T2, usize A2>
bool operator!=(AlignedAllocator<T1, A1>, AlignedAllocator<T2, A2>) noexcept {
    return !(AlignedAllocator<T1, A1>{} == AlignedAllocator<T2, A2>{});
}


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

constexpr usize TT_ALIGNMENT = 64;

class TranspositionTable {
    Transposition* table;

   public:
    u64 size;

    TranspositionTable(usize sizeInMB = 16) :
        table(nullptr),
        size(0) {
        reserve(sizeInMB);
    }

    ~TranspositionTable() {
        if (table != nullptr) {
#if defined(_MSC_VER) || defined(__MINGW32__)
            _aligned_free(table);
#else
            std::free(table);
#endif
        }
    }

    void clear(usize threadCount = 1) {
        assert(threadCount > 0);
        std::vector<std::thread> threads;

        auto clearTT = [&](usize threadId) {
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
        size = newSizeMiB * 1024 * 1024 / sizeof(Transposition);
        if (table != nullptr) {
#if defined(_MSC_VER) || defined(__MINGW32__)
            _aligned_free(table);
#else
            std::free(table);
#endif
        }
        table = static_cast<Transposition*>(alignedAlloc(TT_ALIGNMENT, size * sizeof(Transposition)));
    }

    u64 index(u64 key) { return static_cast<u64>((static_cast<u128>(key) * static_cast<u128>(size)) >> 64); }

    void prefetch(u64 key) { __builtin_prefetch(this->getEntry(key)); }

    void setEntry(u64 key, Transposition& entry) { table[index(key)] = entry; }

    Transposition* getEntry(u64 key) { return &table[index(key)]; }

    usize hashfull() {
        usize samples = std::min((u64) 1000, size);
        usize hits    = 0;
        for (usize sample = 0; sample < samples; sample++)
            hits += table[sample].zobrist != 0;
        usize hash = (int) ((hits / (double) samples) * 1000);
        assert(hash <= 1000);
        return hash;
    }
};