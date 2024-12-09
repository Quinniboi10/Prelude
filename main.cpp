// SEARCH DESC https://discord.com/channels/435943710472011776/882956631514689597/1256706716515565638


#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <deque>
#include <map>
#include <climits>
#include <limits>
#include <chrono>
#include <array>
#include <thread>
#include <atomic>
#include <optional>
#include <random>
#include <unordered_map>
#include <bitset>
/*// For GCC or Clang
#ifdef __GNUC__
// GCC and Clang
#define ctzll(x) ((x) ? __builtin_ctzll(x) : 64)
#define clzll(x) ((x) ? __builtin_clzll(x) : 64)
#define popcountll(x) __builtin_popcountll(x)
#elif defined(_MSC_VER)
// MSVC
#define ctzll(x) ((x) ? _tzcnt_u64(x) : 64)
#define clzll(x) ((x) ? __lzcnt64(x) : 64)
#define popcountll(x) __popcnt64(x)
#else
#error "Compiler not supported. Please use GCC, Clang, or MSVC."
#endif*/


#define ctzll(x) ((x) ? __builtin_ctzll(x) : 64)
#define clzll(x) ((x) ? __builtin_clzll(x) : 64)
#define popcountll(x) __builtin_popcountll(x)

typedef uint64_t u64;
typedef uint16_t u16;

constexpr int NEG_INF = -std::numeric_limits<int>::max();
constexpr int POS_INF = std::numeric_limits<int>::max();

constexpr uint8_t a1 = 0;
constexpr uint8_t b1 = 1;
constexpr uint8_t c1 = 2;
constexpr uint8_t d1 = 3;
constexpr uint8_t e1 = 4;
constexpr uint8_t f1 = 5;
constexpr uint8_t g1 = 6;
constexpr uint8_t h1 = 7;

constexpr uint8_t a2 = 8;
constexpr uint8_t b2 = 9;
constexpr uint8_t c2 = 10;
constexpr uint8_t d2 = 11;
constexpr uint8_t e2 = 12;
constexpr uint8_t f2 = 13;
constexpr uint8_t g2 = 14;
constexpr uint8_t h2 = 15;

constexpr uint8_t a3 = 16;
constexpr uint8_t b3 = 17;
constexpr uint8_t c3 = 18;
constexpr uint8_t d3 = 19;
constexpr uint8_t e3 = 20;
constexpr uint8_t f3 = 21;
constexpr uint8_t g3 = 22;
constexpr uint8_t h3 = 23;

constexpr uint8_t a4 = 24;
constexpr uint8_t b4 = 25;
constexpr uint8_t c4 = 26;
constexpr uint8_t d4 = 27;
constexpr uint8_t e4 = 28;
constexpr uint8_t f4 = 29;
constexpr uint8_t g4 = 30;
constexpr uint8_t h4 = 31;

constexpr uint8_t a5 = 32;
constexpr uint8_t b5 = 33;
constexpr uint8_t c5 = 34;
constexpr uint8_t d5 = 35;
constexpr uint8_t e5 = 36;
constexpr uint8_t f5 = 37;
constexpr uint8_t g5 = 38;
constexpr uint8_t h5 = 39;

constexpr uint8_t a6 = 40;
constexpr uint8_t b6 = 41;
constexpr uint8_t c6 = 42;
constexpr uint8_t d6 = 43;
constexpr uint8_t e6 = 44;
constexpr uint8_t f6 = 45;
constexpr uint8_t g6 = 46;
constexpr uint8_t h6 = 47;

constexpr uint8_t a7 = 48;
constexpr uint8_t b7 = 49;
constexpr uint8_t c7 = 50;
constexpr uint8_t d7 = 51;
constexpr uint8_t e7 = 52;
constexpr uint8_t f7 = 53;
constexpr uint8_t g7 = 54;
constexpr uint8_t h7 = 55;

constexpr uint8_t a8 = 56;
constexpr uint8_t b8 = 57;
constexpr uint8_t c8 = 58;
constexpr uint8_t d8 = 59;
constexpr uint8_t e8 = 60;
constexpr uint8_t f8 = 61;
constexpr uint8_t g8 = 62;
constexpr uint8_t h8 = 63;

constexpr int EN_PASSANT = 0;
constexpr int CASTLING = 1;

struct shifts {
    static inline int NORTH = 8;
    static inline int NORTH_EAST = 9;
    static inline int EAST = 1;
    static inline int SOUTH_EAST = -7;
    static inline int SOUTH = -8;
    static inline int SOUTH_WEST = -9;
    static inline int WEST = -1;
    static inline int NORTH_WEST = 7;

    static inline std::array<int, 8> dirs = { NORTH, NORTH_EAST, EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, WEST, NORTH_WEST };
    static inline std::array<int, 4> straightDirs = { NORTH, EAST, SOUTH, WEST };
    static inline std::array<int, 4> diagonalDirs = { NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST };
};

struct indexes {
    static inline int NORTH = 0;
    static inline int NORTH_EAST = 1;
    static inline int EAST = 2;
    static inline int SOUTH_EAST = 3;
    static inline int SOUTH = 4;
    static inline int SOUTH_WEST = 5;
    static inline int WEST = 6;
    static inline int NORTH_WEST = 7;
    static inline int all = 8;

    static inline std::array<int, 8> dirs = { NORTH, NORTH_EAST, EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, WEST, NORTH_WEST };
    static inline std::array<int, 4> straightDirs = { NORTH, EAST, SOUTH, WEST };
    static inline std::array<int, 4> diagonalDirs = { NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST };
};

static inline int parseSquare(const std::string& square) {
    return (square.at(1) - '1') * 8 + (square.at(0) - 'a'); // Calculate the index of any square
}

static inline bool readBit(u64 bitboard, int index) {
    return (bitboard & (1ULL << index));
}

static inline void setBit(u64& bitboard, int index, bool value) {
    if (value) bitboard |= 1ULL << index;
    else bitboard &= ~(1ULL << index);
}

static inline bool readBit(uint16_t bitboard, int index) {
    return (bitboard & (1ULL << index));
}

static inline void setBit(uint16_t& bitboard, int index, bool value) {
    if (value) bitboard |= 1ULL << index;
    else bitboard &= ~(1ULL << index);
}

static inline bool readBit(uint8_t bitboard, int index) {
    return (bitboard & (1ULL << index));
}

static inline void setBit(uint8_t& bitboard, int index, bool value) {
    if (value) bitboard |= 1ULL << index;
    else bitboard &= ~(1ULL << index);
}

class Board;

class Move {
public:
    uint16_t move;
    // Queen *14*, rook *13*, bishop/knight *12*, ending square *6-11*, starting square *0-5* 
    Move() {
        move = 0;
    }

    Move(std::string in) {
        if (in.size() == 4) *this = Move(parseSquare(in.substr(0, 2)), parseSquare(in.substr(2, 2)));
        else {
            int promo = 0;
            if (in.at(4) == 'q') promo = 4;
            else if (in.at(4) == 'r') promo = 3;
            else if (in.at(4) == 'b') promo = 2;
            *this = Move(parseSquare(in.substr(0, 2)), parseSquare(in.substr(2, 2)), promo);
        }
    }

    Move(uint8_t startSquare, uint8_t endSquare, uint8_t promotion = 0) {
        startSquare &= 0b111111; // Make sure input is only 6 bits
        endSquare &= 0b111111;
        move = startSquare;
        move |= endSquare << 6;

        if (promotion) {
            if (promotion == 4) setBit(move, 14, 1);
            else if (promotion == 3) setBit(move, 13, 1);
            else if (promotion == 2) setBit(move, 12, 1);
        }
    }

    std::string toString(Board& board);

    inline int startSquare() { return move & 0b111111; }
    inline int endSquare() { return (move >> 6) & 0b111111; }
    inline int promo() {
        if (readBit(move, 14)) return 4;
        else if (readBit(move, 13)) return 3;
        else if (readBit(move, 12)) return 2;
        return 1;
    }

    inline int typeOf(Board& board);
};

std::deque<std::string> split(const std::string& s, char delim) {
    std::deque<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }
    return result;
}

int findIndexOf(const std::deque<std::string>& deque, std::string entry) {
    auto it = std::find(deque.begin(), deque.end(), entry);
    if (it != deque.end()) {
        return std::distance(deque.begin(), it); // Calculate the index
    }
    return -1; // Not found
}


void printBitboard(u64 bitboard) {
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << "+---+---+---+---+---+---+---+---+" << std::endl;
        for (int file = 0; file < 8; file++) {
            int i = rank * 8 + file;  // Map rank and file to bitboard index
            char currentPiece = readBit(bitboard, i) ? '1' : ' ';

            std::cout << "| " << currentPiece << " ";
        }
        std::cout << "|" << std::endl;
    }
    std::cout << "+---+---+---+---+---+---+---+---+" << std::endl;
}

class Precomputed {
public:
    static std::array<u64, 64> knightMoves;
    static std::array<u64, 64> kingMoves;
    static std::array<std::array<u64, 64>, 12> zobrist;
    static std::array<std::array<u64, 9>, 64> rays;
    static u64 isOnA;
    static u64 isOnB;
    static u64 isOnC;
    static u64 isOnD;
    static u64 isOnE;
    static u64 isOnF;
    static u64 isOnG;
    static u64 isOnH;
    static u64 isOn1;
    static u64 isOn2;
    static u64 isOn3;
    static u64 isOn4;
    static u64 isOn5;
    static u64 isOn6;
    static u64 isOn7;
    static u64 isOn8;

    static constexpr inline const std::array<int, 64> white_pawn_table = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 5, 10, 10, 5, 0, 0,
        0, 0, 10, 20, 20, 10, 0, 0,
        0, 0, 10, 20, 20, 10, 0, 0,
        0, 0, 5, 10, 10, 5, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    static constexpr inline const std::array<int, 64> white_knight_table = {
        -50, -40, -40, -40, -40, -40, -40, -50,
        -40, 20, 0, 0, 0, 0, -20, -40,
        -40, 0, 10, 20, 20, 10, 0, -40,
        -40, 0, 0, 25, 25, 0, 0, -40,
        -40, 0, 0, 25, 25, 0, 0, -40,
        -40, 0, 10, 20, 20, 10, 0, -40,
        -40, -20, 0, 0, 0, 0, -20, -40,
        -50, -40, -40, -40, -40, -40, -40, -50
    };

    static constexpr inline const std::array<int, 64> white_bishop_table = {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -20,-10,-10,-10,-10,-10,-10,-20
    };

    static constexpr inline const std::array<int, 64> white_rook_table = {
         0,  0,  0,  0,  0,  0,  0,  0,
         5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
         0,  0,  0,  5,  5,  0,  0,  0
    };

    static constexpr inline const std::array<int, 64> white_queen_table = {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -10,  5,  5,  5,  5,  5,  0,-10,
          0,  0,  5,  5,  5,  5,  0, -5,
         -5,  0,  5,  5,  5,  5,  0, -5,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    };

    static constexpr inline const std::array<int, 64> white_king_table = {
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
         20, 20,  0,  0,  0,  0, 20, 20,
         20, 30, 10,  0,  0, 10, 30, 20
    };

    static void compute() {
        // *** FILE AND COL ARRAYS ***
        isOnA = 0;
        isOnB = 0;
        isOnC = 0;
        isOnD = 0;
        isOnE = 0;
        isOnF = 0;
        isOnG = 0;
        isOnH = 0;
        isOn1 = 0;
        isOn2 = 0;
        isOn3 = 0;
        isOn4 = 0;
        isOn5 = 0;
        isOn6 = 0;
        isOn7 = 0;
        isOn8 = 0;

        for (int i = 0; i < 64; i++) {
            int file = i % 8; // File index (0 = A, 1 = B, ..., 7 = H)
            if (file == 0) isOnA |= 1ULL << i;
            if (file == 1) isOnB |= 1ULL << i;
            if (file == 2) isOnC |= 1ULL << i;
            if (file == 3) isOnD |= 1ULL << i;
            if (file == 4) isOnE |= 1ULL << i;
            if (file == 5) isOnF |= 1ULL << i;
            if (file == 6) isOnG |= 1ULL << i;
            if (file == 7) isOnH |= 1ULL << i;

            // Fill ranks (1-8)
            int rank = i / 8; // Rank index (0 = 1, 1 = 2, ..., 7 = 8)
            if (rank == 0) isOn1 |= 1ULL << i;
            if (rank == 1) isOn2 |= 1ULL << i;
            if (rank == 2) isOn3 |= 1ULL << i;
            if (rank == 3) isOn4 |= 1ULL << i;
            if (rank == 4) isOn5 |= 1ULL << i;
            if (rank == 5) isOn6 |= 1ULL << i;
            if (rank == 6) isOn7 |= 1ULL << i;
            if (rank == 7) isOn8 |= 1ULL << i;
        }

        // *** MOVE GENERATION ***

        for (int i = 0; i < 64; i++) {
            u64 all = 0;
            for (int dir : indexes::dirs) {
                u64 ray = 0;
                int currentIndex = i;

                while (true) {// Break if wrapping across ranks (for EAST/WEST)
                    if ((dir == indexes::EAST || dir == indexes::NORTH_EAST || dir == indexes::SOUTH_EAST) && (isOnH & (1ULL << currentIndex))) break;
                    if ((dir == indexes::WEST || dir == indexes::NORTH_WEST || dir == indexes::SOUTH_WEST) && (isOnA & (1ULL << currentIndex))) break;

                    currentIndex += shifts::dirs[dir];

                    // Break if out of bounds
                    if (currentIndex < 0 || currentIndex >= 64) break;

                    setBit(ray, currentIndex, 1);
                }
                rays[i][dir] = ray;
                all |= ray;
            }
            rays[i][8] = all;
        }

        // *** KNIGHT MOVES ***
        bool onNEdge;
        bool onNEdge2;
        bool onEEdge;
        bool onEEdge2;
        bool onSEdge;
        bool onSEdge2;
        bool onWEdge;
        bool onWEdge2;

        for (int i = 0; i < 64; i++) {
            onNEdge = i / 8 == 7;
            onNEdge2 = i / 8 == 7 || i / 8 == 6;
            onEEdge = i % 8 == 7;
            onEEdge2 = i % 8 == 7 || i % 8 == 6;
            onSEdge = i / 8 == 0;
            onSEdge2 = i / 8 == 0 || i / 8 == 1;
            onWEdge = i % 8 == 0;
            onWEdge2 = i % 8 == 0 || i % 8 == 1;

            u64 positions = 0;

            if (!onNEdge2 && !onEEdge) setBit(positions, i + 17, 1); // Up 2, right 1
            if (!onNEdge && !onEEdge2) setBit(positions, i + 10, 1); // Up 1, right 2
            if (!onSEdge && !onEEdge2) setBit(positions, i - 6, 1);  // Down 1, right 2
            if (!onSEdge2 && !onEEdge) setBit(positions, i - 15, 1); // Down 2, right 1

            if (!onSEdge2 && !onWEdge) setBit(positions, i - 17, 1); // Down 2, left 1
            if (!onSEdge && !onWEdge2) setBit(positions, i - 10, 1); // Down 1, left 2
            if (!onNEdge && !onWEdge2) setBit(positions, i + 6, 1);  // Up 1, left 2
            if (!onNEdge2 && !onWEdge) setBit(positions, i + 15, 1); // Up 2, left 1

            knightMoves[i] = positions;
        }
        // *** KING MOVES ***
        for (int i = 0; i < 64; i++) {
            onNEdge = (1ULL << i) & isOn8;
            onEEdge = (1ULL << i) & isOnH;
            onSEdge = (1ULL << i) & isOn1;
            onWEdge = (1ULL << i) & isOnA;

            u64 positions = 0;

            // Horizontal
            if (!onNEdge) setBit(positions, i + shifts::NORTH, 1);
            if (!onEEdge) setBit(positions, i + shifts::EAST, 1);
            if (!onSEdge) setBit(positions, i + shifts::SOUTH, 1);
            if (!onWEdge) setBit(positions, i + shifts::WEST, 1);

            // Diagonal
            if (!onNEdge && !onEEdge) setBit(positions, i + shifts::NORTH_EAST, 1);
            if (!onNEdge && !onWEdge) setBit(positions, i + shifts::NORTH_WEST, 1);
            if (!onSEdge && !onEEdge) setBit(positions, i + shifts::SOUTH_EAST, 1);
            if (!onSEdge && !onWEdge) setBit(positions, i + shifts::SOUTH_WEST, 1);

            kingMoves[i] = positions;
        }

        // *** MAKE RANDOM ZOBRIST TABLE ****
        std::random_device rd;

        std::mt19937_64 engine(rd());

        std::uniform_int_distribution<u64> dist(0, std::numeric_limits<u64>::max());

        for (auto& pieceTable : zobrist) {
            for (int i = 0; i < 64; i++) {
                pieceTable[i] = dist(engine);
            }
        }
    }
};

std::array<u64, 64> Precomputed::knightMoves;
std::array<u64, 64> Precomputed::kingMoves;
std::array<std::array<u64, 64>, 12> Precomputed::zobrist;
std::array<std::array<u64, 9>, 64> Precomputed::rays;
u64 Precomputed::isOnA;
u64 Precomputed::isOnB;
u64 Precomputed::isOnC;
u64 Precomputed::isOnD;
u64 Precomputed::isOnE;
u64 Precomputed::isOnF;
u64 Precomputed::isOnG;
u64 Precomputed::isOnH;
u64 Precomputed::isOn1;
u64 Precomputed::isOn2;
u64 Precomputed::isOn3;
u64 Precomputed::isOn4;
u64 Precomputed::isOn5;
u64 Precomputed::isOn6;
u64 Precomputed::isOn7;
u64 Precomputed::isOn8;


struct MoveEvaluation {
    Move move;
    int eval;
};

struct MoveList {
    std::array<Move, 256> moves;
    int count;

    MoveList() {
        count = 0;
    }

    inline void add(Move m) {
        moves[count++] = m;
    }
};

class Board {
public:
    std::array<u64, 6> white; // Goes pawns, knights, bishops, rooks, queens, king
    std::array<u64, 6> black; // Goes pawns, knights, bishops, rooks, queens, king

    u64 blackPieces;
    u64 whitePieces;
    u64 emptySquares;

    uint64_t enPassant; // Square for en passant
    uint8_t castlingRights;

    bool isWhite = true;

    int halfmoveClock = 0;

    std::vector<u64> moveHistory;

    MoveList legalMoves;

    u64 zobrist;

    void reset() {
        // Reset position
        white[0] = 0xFF00ULL;
        white[1] = 0x42ULL;
        white[2] = 0x24ULL;
        white[3] = 0x81ULL;
        white[4] = 0x8ULL;
        white[5] = 0x10ULL;

        black[0] = 0xFF000000000000ULL;
        black[1] = 0x4200000000000000ULL;
        black[2] = 0x2400000000000000ULL;
        black[3] = 0x8100000000000000ULL;
        black[4] = 0x800000000000000ULL;
        black[5] = 0x1000000000000000ULL;

        castlingRights = 0xF;

        halfmoveClock = 0;
        enPassant = 0; // No en passant target square
        isWhite = true;

        moveHistory.clear();
        moveHistory.reserve(128);

        calculateZobrist();

        recompute();
    }

    inline void clearIndex(int index) {
        const u64 mask = ~(1ULL << index);
        white[0] &= mask;
        white[1] &= mask;
        white[2] &= mask;
        white[3] &= mask;
        white[4] &= mask;
        white[5] &= mask;
        black[0] &= mask;
        black[1] &= mask;
        black[2] &= mask;
        black[3] &= mask;
        black[4] &= mask;
        black[5] &= mask;
    }



    inline void recompute() {
        whitePieces = white[0] | white[1] | white[2] | white[3] | white[4] | white[5];
        blackPieces = black[0] | black[1] | black[2] | black[3] | black[4] | black[5];

        emptySquares = ~(whitePieces | blackPieces);
    }

    inline void generatePawnMoves(MoveList& moves) {
        // Note: captures like "pawnCaptureLeft" means take TOWARD THE A FILE, regardless of color, AKA capture to the left if you are playing as white
        u64 pawns = (isWhite) * (white[0]) + (!isWhite) * (black[0]);

        u64 pawnPushes = (isWhite) * (white[0] << 8) + (!isWhite) * (black[0] >> 8); // Branchless if to get pawn bitboard
        pawnPushes &= emptySquares;
        int currentIndex;

        u64 pawnCaptureRight = pawns & ~(Precomputed::isOnH);
        pawnCaptureRight = (isWhite) * (pawnCaptureRight << 9) + (!isWhite) * (pawnCaptureRight >> 7); // Branchless if to get pawn bitboard
        pawnCaptureRight &= (isWhite) * (blackPieces | enPassant) + (!isWhite) * (whitePieces | enPassant);

        u64 pawnCaptureLeft = pawns & ~(Precomputed::isOnA);
        pawnCaptureLeft = (isWhite) * (pawnCaptureLeft << 7) + (!isWhite) * (pawnCaptureLeft >> 9); // Branchless if to get pawn bitboard
        pawnCaptureLeft &= (isWhite) * (blackPieces | enPassant) + (!isWhite) * (whitePieces | enPassant);

        u64 pawnDoublePush = (isWhite) * (white[0] << 16) + (!isWhite) * (black[0] >> 16); // Branchless if to get pawn bitboard
        pawnDoublePush &= emptySquares & ((isWhite) * (emptySquares << 8) + (!isWhite) * (emptySquares >> 8));
        pawnDoublePush &= (isWhite) * (Precomputed::isOn2 << 16) + (!isWhite) * (Precomputed::isOn7 >> 16);
        if (isWhite) {
            pawnDoublePush &= (Precomputed::isOn2 << 16);
        }
        else {
            pawnDoublePush &= (Precomputed::isOn7 >> 16);
        }
        while (pawnDoublePush) {
            currentIndex = ctzll(pawnDoublePush);
            moves.add(Move((isWhite) * (currentIndex - 16) + (!isWhite) * (currentIndex + 16), currentIndex)); // Another branchless loop

            pawnDoublePush &= pawnDoublePush - 1;
        }

        while (pawnPushes) {
            currentIndex = ctzll(pawnPushes);
            if ((1ULL << currentIndex) & (Precomputed::isOn1 | Precomputed::isOn8)) {
                moves.add(Move((isWhite) * (currentIndex - 8) + (!isWhite) * (currentIndex + 8), currentIndex, 2)); // Another branchless loop
                moves.add(Move((isWhite) * (currentIndex - 8) + (!isWhite) * (currentIndex + 8), currentIndex, 3)); // Another branchless loop
                moves.add(Move((isWhite) * (currentIndex - 8) + (!isWhite) * (currentIndex + 8), currentIndex, 4)); // Another branchless loop
            }
            moves.add(Move((isWhite) * (currentIndex - 8) + (!isWhite) * (currentIndex + 8), currentIndex)); // Another branchless loop

            pawnPushes &= pawnPushes - 1;
        }

        while (pawnCaptureRight) {
            currentIndex = ctzll(pawnCaptureRight);
            if ((1ULL << currentIndex) & (Precomputed::isOn1 | Precomputed::isOn8)) {
                moves.add(Move((isWhite) * (currentIndex - 9) + (!isWhite) * (currentIndex + 7), currentIndex, 2)); // Another branchless loop
                moves.add(Move((isWhite) * (currentIndex - 9) + (!isWhite) * (currentIndex + 7), currentIndex, 3)); // Another branchless loop
                moves.add(Move((isWhite) * (currentIndex - 9) + (!isWhite) * (currentIndex + 7), currentIndex, 4)); // Another branchless loop
            }
            moves.add(Move((isWhite) * (currentIndex - 9) + (!isWhite) * (currentIndex + 7), currentIndex)); // Another branchless loop

            pawnCaptureRight &= pawnCaptureRight - 1;
        }

        while (pawnCaptureLeft) {
            currentIndex = ctzll(pawnCaptureLeft);
            if ((1ULL << currentIndex) & (Precomputed::isOn1 | Precomputed::isOn8)) {
                moves.add(Move((isWhite) * (currentIndex - 7) + (!isWhite) * (currentIndex + 9), currentIndex, 2)); // Another branchless loop
                moves.add(Move((isWhite) * (currentIndex - 7) + (!isWhite) * (currentIndex + 9), currentIndex, 3)); // Another branchless loop
                moves.add(Move((isWhite) * (currentIndex - 7) + (!isWhite) * (currentIndex + 9), currentIndex, 4)); // Another branchless loop
            }
            moves.add(Move((isWhite) * (currentIndex - 7) + (!isWhite) * (currentIndex + 9), currentIndex)); // Another branchless loop

            pawnCaptureLeft &= pawnCaptureLeft - 1;
        }
    }

    inline void generateKnightMoves(MoveList& moves) {
        int currentIndex;

        u64 knightBitboard = isWhite ? white[1] : black[1];
        u64 ownBitboard = isWhite ? whitePieces : blackPieces;

        while (knightBitboard > 0) {
            currentIndex = ctzll(knightBitboard);

            u64 knightMoves = Precomputed::knightMoves[currentIndex];
            knightMoves &= ~ownBitboard;

            while (knightMoves > 0) {
                moves.add(Move(currentIndex, ctzll(knightMoves)));
                knightMoves &= knightMoves - 1;
            }
            knightBitboard &= knightBitboard - 1; // Clear least significant bit
        }
    }

    void generateBishopMoves(MoveList& moves) {
        int currentIndex;
        int& currentMoveIndex = moves.count;

        u64 bishopBitboard = isWhite ? white[2] : black[2];
        u64 ourBitboard = isWhite ? whitePieces : blackPieces;

        u64 attackMask;
        u64 blockerMask;
        uint8_t blocker;

        while (bishopBitboard > 0) {
            currentIndex = ctzll(bishopBitboard);

            for (int dir : indexes::diagonalDirs) {
                attackMask = Precomputed::rays[currentIndex][dir];
                blockerMask = attackMask & (blackPieces | whitePieces);

                if (shifts::dirs[dir] > 0) {
                    blocker = ctzll(blockerMask);
                }
                else {
                    blocker = (blockerMask > 0) ? (63 - clzll(blockerMask)) : 64;
                }
                if (blocker != 64) attackMask &= ~Precomputed::rays[blocker][dir]; // Remove the moves that would get hit

                attackMask &= ~ourBitboard;


                // Make move with each legal move in mask
                while (attackMask > 0) {
                    int maskIndex = ctzll(attackMask);
                    moves.moves[currentMoveIndex++] = Move(currentIndex, maskIndex);
                    attackMask &= attackMask - 1;
                }
            }

            bishopBitboard &= bishopBitboard - 1; // Clear least significant bit
        }
    }

    void generateRookMoves(MoveList& moves) {
        int currentIndex;
        int& currentMoveIndex = moves.count;

        u64 rookBitboard = isWhite ? white[3] : black[3];
        u64 ourBitboard = isWhite ? whitePieces : blackPieces;

        u64 attackMask;
        u64 blockerMask;
        uint8_t blocker;

        while (rookBitboard > 0) {
            currentIndex = ctzll(rookBitboard);

            for (int dir : indexes::straightDirs) {
                attackMask = Precomputed::rays[currentIndex][dir];
                blockerMask = attackMask & (blackPieces | whitePieces);

                if (shifts::dirs[dir] > 0) {
                    blocker = ctzll(blockerMask);
                }
                else {
                    blocker = (blockerMask > 0) ? (63 - clzll(blockerMask)) : 64;
                }
                if (blocker != 64) attackMask &= ~Precomputed::rays[blocker][dir]; // Remove the moves that would get hit

                attackMask &= ~ourBitboard;


                // Make move with each legal move in mask
                while (attackMask > 0) {
                    int maskIndex = ctzll(attackMask);
                    moves.moves[currentMoveIndex++] = Move(currentIndex, maskIndex);
                    attackMask &= attackMask - 1;
                }
            }

            rookBitboard &= rookBitboard - 1; // Clear least significant bit
        }
    }

    void generateQueenMoves(MoveList& moves) {
        int currentIndex;
        int& currentMoveIndex = moves.count;

        u64 queenBitboard = isWhite ? white[4] : black[4];
        u64 ourBitboard = isWhite ? whitePieces : blackPieces;

        u64 attackMask;
        u64 blockerMask;
        uint8_t blocker;

        while (queenBitboard > 0) {
            currentIndex = ctzll(queenBitboard);

            for (int dir : indexes::dirs) {
                attackMask = Precomputed::rays[currentIndex][dir];
                blockerMask = attackMask & (blackPieces | whitePieces);

                if (shifts::dirs[dir] > 0) {
                    blocker = ctzll(blockerMask);
                }
                else {
                    blocker = (blockerMask > 0) ? (63 - clzll(blockerMask)) : 64;
                }
                if (blocker != 64) attackMask &= ~Precomputed::rays[blocker][dir]; // Remove the moves that would get hit

                attackMask &= ~ourBitboard;


                // Make move with each legal move in mask
                while (attackMask > 0) {
                    int maskIndex = ctzll(attackMask);
                    moves.moves[currentMoveIndex++] = Move(currentIndex, maskIndex);
                    attackMask &= attackMask - 1;
                }
            }

            queenBitboard &= queenBitboard - 1; // Clear least significant bit
        }
    }

    void generateKingMoves(MoveList& moves) {
        int& currentMoveIndex = moves.count;

        u64 kingBitboard = isWhite ? white[5] : black[5];
        if (!kingBitboard) return;
        u64 ownBitboard = isWhite ? whitePieces : blackPieces;
        int currentIndex = ctzll(kingBitboard);

        u64 kingMoves = Precomputed::kingMoves[currentIndex];
        kingMoves &= ~ownBitboard;

        // Regular king moves
        while (kingMoves > 0) {
            moves.add(Move(currentIndex, ctzll(kingMoves)));
            kingMoves &= kingMoves - 1;
        }

        // Castling moves
        if (isWhite && currentIndex == e1) {
            // Kingside
            if (readBit(castlingRights, 3)) {
                moves.moves[currentMoveIndex++] = Move(e1, g1);
            }
            // Queenside
            if (readBit(castlingRights, 2)) {
                moves.moves[currentMoveIndex++] = Move(e1, c1);
            }
        }
        else if (!isWhite && currentIndex == e8) {
            // Kingside
            if (readBit(castlingRights, 1)) {
                moves.moves[currentMoveIndex++] = Move(e8, g8);
            }
            // Queenside
            if (readBit(castlingRights, 0)) {
                moves.moves[currentMoveIndex++] = Move(e8, c8);
            }
        }
    }

    MoveList generateMoves() {
        recompute();
        MoveList moves;
        generatePawnMoves(moves);
        generateKnightMoves(moves);
        generateBishopMoves(moves);
        generateRookMoves(moves);
        generateQueenMoves(moves);
        generateKingMoves(moves);

        return moves;
    }

    inline bool isInCheck(bool checkWhite) {
        u64 kingBit = checkWhite ? white[5] : black[5];
        if (!kingBit) return false;
        int kingSquare = ctzll(kingBit);
        return isUnderAttack(checkWhite, kingSquare);
    }

    bool isUnderAttack(bool checkWhite, int square) {
        auto& opponentPieces = checkWhite ? black : white;

        // *** SLIDING PIECE ATTACKS ***
        u64 allPieces = whitePieces | blackPieces;

        // Diagonal Directions (Bishops and Queens)
        for (int dir : indexes::diagonalDirs) {
            u64 ray = Precomputed::rays[square][dir] & allPieces;
            int blocker = (shifts::dirs[dir] > 0) ? ctzll(ray) : ((ray) ? (63 - clzll(ray)) : 64);

            if (blocker == 64) continue;

            u64 blockerMask = 1ULL << blocker;
            if ((opponentPieces[2] & blockerMask) || (opponentPieces[4] & blockerMask)) return true;
        }

        // Straight Directions (Rooks and Queens)
        for (int dir : indexes::straightDirs) {
            u64 ray = Precomputed::rays[square][dir] & allPieces;
            int blocker = (shifts::dirs[dir] > 0) ? ctzll(ray) : ((ray) ? (63 - clzll(ray)) : 64);

            if (blocker == 64) continue;

            int blockerSquare = blocker;
            u64 blockerMask = 1ULL << blockerSquare;
            if ((opponentPieces[3] & blockerMask) || (opponentPieces[4] & blockerMask)) return true;
        }

        // *** KNIGHT ATTACKS ***
        if (opponentPieces[1] & Precomputed::knightMoves[square]) return true;

        // *** KING ATTACKS ***
        if (opponentPieces[5] & Precomputed::kingMoves[square]) return true;


        // *** PAWN ATTACKS ***
        if (checkWhite) {
            if ((opponentPieces[0] & (1ULL << (square + 7))) && (square % 8 != 0)) return true;
            if ((opponentPieces[0] & (1ULL << (square + 9))) && (square % 8 != 7)) return true;
        }
        else {
            if ((opponentPieces[0] & (1ULL << (square - 7))) && (square % 8 != 7)) return true;
            if ((opponentPieces[0] & (1ULL << (square - 9))) && (square % 8 != 0)) return true;
        }

        return false;
    }

    bool isCheckmate() {
        return isInCheck(isWhite) && legalMoves.count == 0;
    }

    bool isLegalMove(Move m) {
        int start = m.startSquare();
        int end = m.endSquare();

        if (start == end) return false;

        u64 startMask = 1ULL << start;

        // Castling checks:
        // White king from e1(4) to g1(6)=K-side or c1(2)=Q-side
        // Black king from e8(60) to g8(62)=k-side or c8(58)=q-side
        bool movedKing = (isWhite && (startMask & white[5])) || (!isWhite && (startMask & black[5]));
        if (movedKing) {
            if (isWhite && start == e1 && (end == g1 || end == c1)) {
                bool kingside = (end == g1);
                if (kingside && !readBit(castlingRights, 3)) return false; // 'K'
                if (!kingside && !readBit(castlingRights, 2)) return false; // 'Q'
                if (isInCheck(isWhite)) return false;
                u64 occupied = whitePieces | blackPieces;
                if (kingside) {
                    if (occupied & ((1ULL << f1) & (1ULL << g1))) return false;
                    if ((occupied & ((1ULL << f1) | (1ULL << g1))) != 0) return false;
                    if (isUnderAttack(true, f1) || isUnderAttack(true, g1)) return false;
                }
                else {
                    if ((occupied & ((1ULL << b1) | (1ULL << c1) | (1ULL << d1))) != 0) return false;
                    if (isUnderAttack(true, d1) || isUnderAttack(true, c1)) return false;
                }
            }
            else if (!isWhite && start == e8 && (end == g8 || end == c8)) {
                bool kingside = (end == g8);
                if (kingside && !readBit(castlingRights, 1)) return false; // 'k'
                if (!kingside && !readBit(castlingRights, 0)) return false; // 'q'
                // Use false here because we are checking if black is under attack (false means black)
                if (isInCheck(false)) return false;
                u64 occupied = whitePieces | blackPieces;
                if (kingside) {
                    if ((occupied & ((1ULL << f8) | (1ULL << g8))) != 0) return false;
                    if (isUnderAttack(false, f8) || isUnderAttack(false, g8)) return false;
                }
                else {
                    if ((occupied & ((1ULL << b8) | (1ULL << c8) | (1ULL << d8))) != 0) return false;
                    if (isUnderAttack(false, d8) || isUnderAttack(false, c8)) return false;
                }
            }
        }

        // Test final position
        Board testBoard = *this;
        testBoard.move(m, false);

        return !testBoard.isInCheck(isWhite);
    }

    MoveList generateLegalMoves() {
        auto moves = generateMoves();
        MoveList legalMoves;
        for (int i = 0; i < moves.count; i++) {
            if (!isLegalMove(moves.moves[i])) continue;
            legalMoves.add(moves.moves[i]);
        }
        this->legalMoves = legalMoves;
        return legalMoves;
    }

    void display() {
        if (isWhite)
            std::cout << "White's turn" << std::endl;
        else
            std::cout << "Black's turn" << std::endl;

        for (int rank = 7; rank >= 0; rank--) {
            std::cout << "+---+---+---+---+---+---+---+---+" << std::endl;
            for (int file = 0; file < 8; file++) {
                int i = rank * 8 + file;  // Map rank and file to bitboard index
                char currentPiece = ' ';

                if (readBit(white[0], i)) currentPiece = 'P';
                else if (readBit(white[1], i)) currentPiece = 'N';
                else if (readBit(white[2], i)) currentPiece = 'B';
                else if (readBit(white[3], i)) currentPiece = 'R';
                else if (readBit(white[4], i)) currentPiece = 'Q';
                else if (readBit(white[5], i)) currentPiece = 'K';

                else if (readBit(black[0], i)) currentPiece = 'p';
                else if (readBit(black[1], i)) currentPiece = 'n';
                else if (readBit(black[2], i)) currentPiece = 'b';
                else if (readBit(black[3], i)) currentPiece = 'r';
                else if (readBit(black[4], i)) currentPiece = 'q';
                else if (readBit(black[5], i)) currentPiece = 'k';

                std::cout << "| " << currentPiece << " ";
            }
            std::cout << "|" << std::endl;
        }
        std::cout << "+---+---+---+---+---+---+---+---+" << std::endl;
    }

    inline void move(std::string& moveIn) {
        move(Move(moveIn));
    }

    void move(Move moveIn, bool updateMoveHistory = true) {
        auto& ourSide = isWhite ? white : black;

        for (int i = 0; i < 6; i++) {
            if (readBit(ourSide[i], moveIn.startSquare())) {
                auto from = moveIn.startSquare();
                auto to = moveIn.endSquare();

                clearIndex(from);
                clearIndex(to);

                if (i == 0 && (((1ULL << to) & (Precomputed::isOn1 | Precomputed::isOn8)) > 0)) { // Move is promo
                    int promo = moveIn.promo();
                    if (promo == 4) setBit(ourSide[4], moveIn.endSquare(), 1);
                    else if (promo == 3) setBit(ourSide[3], moveIn.endSquare(), 1);
                    else if (promo == 2) setBit(ourSide[2], moveIn.endSquare(), 1);
                    else if (promo == 1) setBit(ourSide[1], moveIn.endSquare(), 1);
                }
                else setBit(ourSide[i], to, 1);

                // EN PASSANT
                if (i == 0 && to == ctzll(enPassant)) {
                    if (isWhite) setBit(black[0], to + shifts::SOUTH, 0);
                    else setBit(white[0], to + shifts::NORTH, 0);
                }

                // Halfmove clock, promo and set en passant
                if (i == 0 || readBit((isWhite) ? blackPieces : whitePieces, to)) halfmoveClock = -1; // Reset halfmove clock on capture or pawn move
                if (i == 0 && std::abs(from - to) == 16) enPassant = 1ULL << ((isWhite) * (from + 8) + (!isWhite) * (from - 8)); // Set en passant bitboard
                else enPassant = 0;


                // Castling
                if (i == 5) {
                    if (from == e1 && to == g1 && readBit(castlingRights, 3)) {
                        setBit(ourSide[3], h1, 0); // Remove rook

                        setBit(ourSide[3], f1, 1); // Set rook
                    }
                    else if (from == e1 && to == c1 && readBit(castlingRights, 2)) {
                        setBit(ourSide[3], a1, 0); // Remove rook

                        setBit(ourSide[3], d1, 1); // Set rook
                    }
                    else if (from == e8 && to == g8 && readBit(castlingRights, 1)) {
                        setBit(ourSide[3], h8, 0); // Remove rook

                        setBit(ourSide[3], f8, 1); // Set rook
                    }
                    else if (from == e8 && to == c8 && readBit(castlingRights, 0)) {
                        setBit(ourSide[3], a8, 0); // Remove rook

                        setBit(ourSide[3], d8, 1); // Set rook
                    }
                }

                halfmoveClock++;
                break;
            }
        }

        // Remove castling rights (very unoptimized)
        if (castlingRights) {
            int from = moveIn.startSquare();
            int to = moveIn.endSquare();
            if (from == 0 || to == 0) setBit(castlingRights, 2, 0);
            if (from == 7 || to == 7) setBit(castlingRights, 3, 0);
            if (from == 4) { // King moved
                setBit(castlingRights, 2, 0);
                setBit(castlingRights, 3, 0);
            }
            if (from == 56 || to == 56) setBit(castlingRights, 0, 0);
            if (from == 63 || to == 63) setBit(castlingRights, 1, 0);
            if (from == 60) { // King moved
                setBit(castlingRights, 0, 0);
                setBit(castlingRights, 1, 0);
            }
        }

        isWhite = !isWhite;
        recompute();

        //if (updateMoveHistory) moveHistory.push_back(calculateZobrist()); CALCULATE ZOBRIST IS BAD. NO MORE TRIFOLD DETECTION FOR THIS BOT.
    }

    inline int black_to_white(int index) {
        // Ensure the index is within [0, 63]
        if (index < 0 || index > 63) std::cerr << "Invalid index: " << index << ". Must be between 0 and 63.\n";
        int rank = index / 8;
        int file = index % 8;
        int mirrored_rank = 7 - rank;
        return mirrored_rank * 8 + file;
    }

    bool isDraw() {
        // Fifty move rule
        if (halfmoveClock == 100) {
            return true;
        }
        // Threefold repetition check
        else if (moveHistory.size() > 8) {
            // Check if any hash appears at least three times
            // Instead of using an unordered_map, just do a simple O(n^2) check:
            // This is a small overhead, but simpler and avoids extra containers.
            for (size_t i = 0; i < moveHistory.size(); i++) {
                u64 current = moveHistory[i];
                int count = 1;
                for (size_t j = i + 1; j < moveHistory.size(); j++) {
                    if (moveHistory[j] == current) {
                        count++;
                        if (count == 3) {
                            return true;
                        }
                    }
                }
            }
        }

        // DRAW BY INSUFFICIENT MATERIAL

        // Inline color determination for bishop squares
        // Since minimal lambdas or vectors: a light square is (rank+file)%2==1
        auto isLightSquareFunc = [](int square) {
            int file = square % 8;
            int rank = square / 8;
            return ((file + rank) & 1) == 1;
            };

        // Count pieces
        int whitePawns = popcountll(white[0]);
        int whiteKnights = popcountll(white[1]);
        int whiteBishops = popcountll(white[2]);
        int whiteRooks = popcountll(white[3]);
        int whiteQueens = popcountll(white[4]);

        int blackPawns = popcountll(black[0]);
        int blackKnights = popcountll(black[1]);
        int blackBishops = popcountll(black[2]);
        int blackRooks = popcountll(black[3]);
        int blackQueens = popcountll(black[4]);

        // Condition 1:
        // A king + any pawn, rook, queen is sufficient.
        if (whitePawns > 0 || whiteRooks > 0 || whiteQueens > 0 ||
            blackPawns > 0 || blackRooks > 0 || blackQueens > 0) {
            return false;
        }

        // Condition 2:
        // A king and more than one other type of piece (e.g., knight + bishop) is sufficient.
        int whitePieceTypes = (whiteKnights > 0) + (whiteBishops > 0);
        int blackPieceTypes = (blackKnights > 0) + (blackBishops > 0);
        if (whitePieceTypes > 1 || blackPieceTypes > 1) {
            return false;
        }

        // Condition 3:
        // A king and two (or more) knights is sufficient.
        if (whiteKnights >= 2 || blackKnights >= 2) {
            return false;
        }

        // Condition 4:
        // King + knight vs king + any (rook, bishop, knight, pawn) is sufficient.
        bool whiteHasKnight = (whiteKnights >= 1);
        bool blackHasKnight = (blackKnights >= 1);
        bool blackHasOtherPieces = (blackRooks > 0 || blackBishops > 0 || blackKnights > 0 || blackPawns > 0);
        bool whiteHasOtherPieces = (whiteRooks > 0 || whiteBishops > 0 || whiteKnights > 0 || whitePawns > 0);

        if ((whiteHasKnight && blackHasOtherPieces) ||
            (blackHasKnight && whiteHasOtherPieces)) {
            return false;
        }

        // Condition 5:
        // King + bishop vs king + any (knight, pawn) is sufficient.
        bool whiteHasBishop = (whiteBishops >= 1);
        bool blackHasBishop = (blackBishops >= 1);
        bool blackHasKnightOrPawn = (blackKnights > 0 || blackPawns > 0);
        bool whiteHasKnightOrPawn = (whiteKnights > 0 || whitePawns > 0);

        if ((whiteHasBishop && blackHasKnightOrPawn) ||
            (blackHasBishop && whiteHasKnightOrPawn)) {
            return false;
        }

        // Condition 6:
        // King + bishop(s) each side is still sufficient if opposite-color bishops exist.
        bool whiteOnlyKingAndBishops = (whitePawns == 0 && whiteRooks == 0 && whiteQueens == 0 &&
            whiteKnights == 0 && whiteBishops > 0);
        bool blackOnlyKingAndBishops = (blackPawns == 0 && blackRooks == 0 && blackQueens == 0 &&
            blackKnights == 0 && blackBishops > 0);
        if (whiteOnlyKingAndBishops && blackOnlyKingAndBishops) {
            // Collect bishop squares in arrays
            // Max 10 bishops should be more than enough
            // But to be safe, just store up to 10 and break if more appear.
            int whiteCount = 0;
            int blackCount = 0;
            int whiteBishopSquares[10];
            int blackBishopSquares[10];

            for (int i = 0; i < 64 && whiteCount < 10; i++) {
                if (white[2] & (1ULL << i)) {
                    whiteBishopSquares[whiteCount++] = i;
                }
            }

            for (int i = 0; i < 64 && blackCount < 10; i++) {
                if (black[2] & (1ULL << i)) {
                    blackBishopSquares[blackCount++] = i;
                }
            }

            bool oppositeColors = false;
            for (int w = 0; w < whiteCount && !oppositeColors; w++) {
                for (int b = 0; b < blackCount && !oppositeColors; b++) {
                    if (isLightSquareFunc(whiteBishopSquares[w]) != isLightSquareFunc(blackBishopSquares[b])) {
                        oppositeColors = true;
                    }
                }
            }

            if (oppositeColors) {
                return false;
            }
        }

        // If none of the conditions are met, insufficient material -> draw.
        return true;
    }


    inline int evaluate() { // Returns evaluation in centipawns as side to move
        int eval = 0;
        int whitePieces = 0;
        int blackPieces = 0;

        // Uses some magic python buffoonery https://github.com/ianfab/chess-variant-stats/blob/main/piece_values.py
        // Based on this https://discord.com/channels/435943710472011776/1300744461281787974/1312722964915027980

        // Material evaluation
        whitePieces += popcountll(white[0]) * 100;
        whitePieces += popcountll(white[1]) * 316;
        whitePieces += popcountll(white[2]) * 328;
        whitePieces += popcountll(white[3]) * 493;
        whitePieces += popcountll(white[4]) * 982;

        blackPieces += popcountll(black[0]) * 100;
        blackPieces += popcountll(black[1]) * 316;
        blackPieces += popcountll(black[2]) * 328;
        blackPieces += popcountll(black[3]) * 493;
        blackPieces += popcountll(black[4]) * 982;

        eval = whitePieces - blackPieces;

        // Piece value adjustment
        if (std::abs(eval) < 950) { // Ignore if the eval is already very very high
            for (int i = 0; i < 6; i++) {
                u64 currentBitboard = white[i];
                while (currentBitboard > 0) {
                    int currentIndex = ctzll(currentBitboard);

                    if (i == 0) eval += Precomputed::white_pawn_table[currentIndex];
                    if (i == 1) eval += Precomputed::white_knight_table[currentIndex];
                    if (i == 2) eval += Precomputed::white_bishop_table[currentIndex];
                    if (i == 3) eval += Precomputed::white_rook_table[currentIndex];
                    if (i == 4) eval += Precomputed::white_queen_table[currentIndex];
                    if (i == 5) eval += Precomputed::white_king_table[currentIndex];

                    currentBitboard &= currentBitboard - 1;
                }
            }

            for (int i = 0; i < 6; i++) {
                u64 currentBitboard = black[i];
                while (currentBitboard > 0) {
                    int currentIndex = ctzll(currentBitboard);

                    if (i == 0) eval -= Precomputed::white_pawn_table[black_to_white(currentIndex)];
                    if (i == 1) eval -= Precomputed::white_knight_table[black_to_white(currentIndex)];
                    if (i == 2) eval -= Precomputed::white_bishop_table[black_to_white(currentIndex)];
                    if (i == 3) eval -= Precomputed::white_rook_table[black_to_white(currentIndex)];
                    if (i == 4) eval -= Precomputed::white_queen_table[black_to_white(currentIndex)];
                    if (i == 5) eval -= Precomputed::white_king_table[black_to_white(currentIndex)];

                    currentBitboard &= currentBitboard - 1;
                }
            }
        }

        //transPos[zobrist] = eval;

        // Adjust evaluation for the side to move
        return (isWhite) ? eval : -eval;;
    }

    void loadFromFEN(const std::deque<std::string>& inputFEN) {
        // Reset
        reset();

        for (int i = 0; i < 64; i++) clearIndex(i);

        // Sanitize input
        if (inputFEN.size() < 6) {
            std::cerr << "Invalid FEN string" << std::endl;
            return;
        }

        std::deque<std::string> parsedPosition = split(inputFEN.at(0), '/');
        char currentCharacter;
        int currentIndex = 56; // Start at rank 8, file 'a' (index 56)

        for (const std::string& rankString : parsedPosition) {
            for (char c : rankString) {
                currentCharacter = c;

                if (isdigit(currentCharacter)) { // Empty squares
                    int emptySquares = currentCharacter - '0';
                    currentIndex += emptySquares; // Skip the given number of empty squares
                }
                else { // Piece placement
                    switch (currentCharacter) {
                    case 'P': setBit(white[0], currentIndex, 1); break;
                    case 'N': setBit(white[1], currentIndex, 1); break;
                    case 'B': setBit(white[2], currentIndex, 1); break;
                    case 'R': setBit(white[3], currentIndex, 1); break;
                    case 'Q': setBit(white[4], currentIndex, 1); break;
                    case 'K': setBit(white[5], currentIndex, 1); break;
                    case 'p': setBit(black[0], currentIndex, 1); break;
                    case 'n': setBit(black[1], currentIndex, 1); break;
                    case 'b': setBit(black[2], currentIndex, 1); break;
                    case 'r': setBit(black[3], currentIndex, 1); break;
                    case 'q': setBit(black[4], currentIndex, 1); break;
                    case 'k': setBit(black[5], currentIndex, 1); break;
                    default: break;
                    }
                    currentIndex++;
                }
            }
            currentIndex -= 16; // Move to next rank in FEN
        }

        if (inputFEN.at(1) == "w") { // Check the next player's move
            isWhite = true;
        }
        else {
            isWhite = false;
        }

        castlingRights = 0;

        if (inputFEN.at(2).find('K') != std::string::npos) castlingRights |= 1 << 3;
        if (inputFEN.at(2).find('Q') != std::string::npos) castlingRights |= 1 << 2;
        if (inputFEN.at(2).find('k') != std::string::npos) castlingRights |= 1 << 1;
        if (inputFEN.at(2).find('q') != std::string::npos) castlingRights |= 1;

        if (inputFEN.at(3) != "-") enPassant = 1ULL << parseSquare(inputFEN.at(3));
        else enPassant = 0;

        halfmoveClock = std::stoi(inputFEN.at(4));

        recompute();
    }

    u64 calculateZobrist() {
        u64 hash = 0;
        // Pieces
        for (int table = 0; table < 6; table++) {
            for (int i = 0; i < 64; i++) {
                if (readBit(white[table], i)) hash ^= Precomputed::zobrist[table][i];
            }
        }
        for (int table = 0; table < 6; table++) {
            for (int i = 0; i < 64; i++) {
                if (readBit(black[table], i)) hash ^= Precomputed::zobrist[table][i];
            }
        }
        // Castling
        if (castlingRights) hash ^= Precomputed::zobrist[0][0] / castlingRights;

        // En Passant
        if (enPassant) hash ^= Precomputed::zobrist[0][0] / enPassant;

        // Turn
        if (castlingRights) hash ^= Precomputed::zobrist[0][0] / (isWhite + 1);
        zobrist = hash;
        return hash;
    }
};

std::string Move::toString(Board& board) {
    int startSquare = this->startSquare();
    int endSquare = this->endSquare();
    std::string out = std::string(1, (char)(startSquare % 8 + 'a')) + std::to_string(startSquare / 8 + 1) + std::string(1, (char)(endSquare % 8 + 'a')) + std::to_string(endSquare / 8 + 1);

    if (readBit(move, 14)) out += "q";
    else if (readBit(move, 13)) out += "r";
    else if (readBit(move, 12)) out += "b";
    else if (board.isWhite && ((1ULL << endSquare) & Precomputed::isOn8) && (board.white[0] & 1ULL << startSquare)) out += "n"; // Is there a pawn on the start square
    else if (!board.isWhite && ((1ULL << endSquare) & Precomputed::isOn1) && (board.black[0] & 1ULL << startSquare)) out += "n";
    return out;
}

int Move::typeOf(Board& board) {
    return EN_PASSANT;
}

u64 _perft(Board& board, int depth) {
    if (depth == 1) {
        return board.generateMoves().count;
    }

    MoveList moves = board.generateMoves();
    u64 localNodes = 0;
    for (int i = 0; i < moves.count; i++) {
        Board testBoard = board;
        testBoard.move(moves.moves[i]);
        localNodes += _perft(testBoard, depth - 1);
    }
    return localNodes;
}

u64 perft(Board& board, int depth) {
    if (depth == 1) {
        return board.generateMoves().count;
    }

    MoveList moves = board.generateMoves();
    u64 localNodes = 0;
    u64 movesThisIter = 0;
    for (int i = 0; i < moves.count; i++) {
        Board testBoard = board;
        testBoard.move(moves.moves[i]);
        movesThisIter = _perft(testBoard, depth - 1);
        localNodes += movesThisIter;
        std::cout << moves.moves[i].toString(board) << ": " << movesThisIter << std::endl;
    }
    return localNodes;
}

void perftSuite(const std::string& filePath) {
    Board board;

    std::fstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }

    std::string line;
    int totalTests = 0;
    int passedTests = 0;

    while (std::getline(file, line)) {
        auto start = std::chrono::high_resolution_clock::now();
        
        u64 nodes = 0;

        if (line.empty()) continue; // Skip empty lines

        // Split the line by ';'
        std::deque<std::string> parts = split(line, ';');

        if (parts.empty()) continue;

        // The first part is the FEN string
        std::string fen = parts.front();
        parts.pop_front();

        // Split FEN into space-separated parts
        std::deque<std::string> fenParts = split(fen, ' ');

        // Load FEN into the board
        board.loadFromFEN(fenParts);

        // Iterate over perft entries
        bool allPassed = true;
        std::cout << "Testing position: " << fen << std::endl;

        for (const auto& perftEntry : parts) {
            // Trim leading spaces
            std::string entry = perftEntry;
            size_t firstNonSpace = entry.find_first_not_of(" ");
            if (firstNonSpace != std::string::npos) {
                entry = entry.substr(firstNonSpace);
            }

            // Expecting format Dx N (e.g., D1 4)
            if (entry.empty()) continue;

            std::deque<std::string> entryParts = split(entry, ' ');
            if (entryParts.size() != 2) {
                std::cerr << "Invalid perft entry format: \"" << entry << "\"" << std::endl;
                continue;
            }

            std::string depthStr = entryParts[0];
            std::string expectedStr = entryParts[1];

            if (depthStr.size() < 2 || depthStr[0] != 'D') {
                std::cerr << "Invalid depth format: \"" << depthStr << "\"" << std::endl;
                continue;
            }

            // Extract depth and expected node count
            int depth = std::stoi(depthStr.substr(1));
            u64 expectedNodes = std::stoull(expectedStr);

            // Execute perft for the given depth
            u64 actualNodes = _perft(board, depth);

            nodes += actualNodes;

            // Compare and report
            bool pass = (actualNodes == expectedNodes);
            std::cout << "Depth " << depth << ": Expected " << expectedNodes
                << ", Got " << actualNodes << " -> "
                << (pass ? "PASS" : "FAIL") << std::endl;

            totalTests++;
            if (pass) passedTests++;
            else allPassed = false;
        }

        int nps = (int)(((double)nodes) / (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) * 1000000000);
        double timeTaken = (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count());
        if (timeTaken / 1000000 < 1000) std::cout << "Time taken: " << timeTaken / 1000000 << " milliseconds" << std::endl;
        else std::cout << "Time taken: " << timeTaken / 1000000000 << " seconds" << std::endl;
        std::cout << "Generated moves with " << nodes << " nodes and NPS of " << nps << std::endl;

        if (allPassed) {
            std::cout << "All perft tests passed for this position." << std::endl;
        }
        else {
            std::cout << "Some perft tests failed for this position." << std::endl;
        }

        std::cout << "----------------------------------------" << std::endl << std::endl;
    }

    std::cout << "Perft Suite Completed: " << passedTests << " / "
        << totalTests << " tests passed." << std::endl;
}


int nodes = 0;

static MoveEvaluation go(Board& board,
    int depth,
    std::atomic<bool>& breakFlag,
    std::chrono::steady_clock::time_point& timerStart,
    int timeToSpend = 0,
    int alpha = NEG_INF,
    int beta = POS_INF,
    int maxNodes = -1) {

    if (depth == 0) {
        nodes++;
        int eval = board.evaluate();
        return { Move(), eval };
    }

    if (board.isDraw()) {
        nodes++;
        return { Move(), 0 };
    }

    MoveList moves = board.generateLegalMoves();
    if (moves.count == 0) {
        nodes++;
        if (board.isInCheck(board.isWhite)) {
            // Checkmate
            return { Move(), NEG_INF };
        }
        else {
            // Stalemate
            return { Move(), 0 };
        }
    }

    int maxEval = NEG_INF;
    Move bestMove;

    for (int i = 0; i < moves.count; ++i) {
        // Time/node/break checks
        if (breakFlag.load()) {
            return { bestMove, maxEval };
        }
        if (timeToSpend != 0) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - timerStart).count() >= timeToSpend) {
                return { bestMove, maxEval };
            }
        }
        if (maxNodes > 0 && nodes >= maxNodes) {
            return { bestMove, maxEval };
        }

        const Move& m = moves.moves[i];

        Board testBoard = board;
        testBoard.move(m);

        MoveEvaluation evalMove = go(testBoard, depth - 1, breakFlag, timerStart, timeToSpend, -beta, -alpha, maxNodes);
        int eval = -evalMove.eval;

        if (eval > maxEval) {
            maxEval = eval;
            bestMove = m;
        }
        if (eval > alpha) {
            alpha = eval;
        }
        if (alpha >= beta) {
            break; // Alpha-beta pruning
        }
    }

    return { bestMove, maxEval };
}

void iterativeDeepening(
    Board& board,
    int maxDepth,
    std::atomic<bool>& breakFlag,
    int wtime = 0,
    int btime = 0,
    int maxNodes = -1
) {
    nodes = 0;
    int timeToSpend = 0;
    auto start = std::chrono::steady_clock::now();
    std::string bestMoveAlgebra = "";
    if (wtime != 0 || btime != 0) {
        // Simple heuristic: spend 1/20 of the available time
        timeToSpend = board.isWhite ? wtime / 20 : btime / 20;
    }

    MoveEvaluation bestMove = { Move(), NEG_INF };

    for (int depth = 1; depth <= maxDepth; depth++) {
        if (breakFlag.load()) {
            break;
        }

        MoveEvaluation currentMove = go(board, depth, breakFlag, start, timeToSpend, NEG_INF, POS_INF, maxNodes);

        if (breakFlag.load()) {
            break;
        }

        if (timeToSpend != 0) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() >= timeToSpend) {
                break;
            }
        }

        if (maxNodes > 0 && maxNodes <= nodes) {
            break;
        }

        if (currentMove.move.startSquare() != currentMove.move.endSquare()) {
            bestMove = currentMove;
        }

        auto now = std::chrono::steady_clock::now();
        double elapsedNs = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(now - start).count();
        int nps = (int)((double)nodes / (elapsedNs / 1e9));

        bestMoveAlgebra = bestMove.move.toString(board);
        if (bestMove.eval > 90000) {
            // Assume large positive value = mate
            std::cout << "info depth " << depth << " nodes " << nodes << " nps " << nps
                << " score mate " << depth / 2 << " pv " << bestMoveAlgebra << std::endl;
            break;
        }
        else {
            std::cout << "info depth " << depth << " nodes " << nodes << " nps " << nps
                << " score cp " << bestMove.eval << " pv " << bestMoveAlgebra << std::endl;
        }
    }

    std::cout << "bestmove " << bestMoveAlgebra << std::endl;
    breakFlag.store(false);
}



int main() {
    Precomputed::compute();
    std::string command;
    std::deque<std::string> parsedcommand;
    Board currentPos;
    currentPos.reset();
    currentPos.generateLegalMoves();
    std::atomic<bool> breakFlag(false);
    std::optional<std::thread> searchThreadOpt;
    std::cout << "Bot ready and awaiting commands" << std::endl;

    while (true) {
        std::getline(std::cin, command);
        parsedcommand = split(command, ' ');
        if (command == "uci") {
            std::cout << "id name Bondfish" << std::endl;
            std::cout << "id author Quinn I" << std::endl;
            std::cout << "uciok" << std::endl;
        }
        else if (command == "ucinewgame") {
            breakFlag.store(false);
            if (searchThreadOpt.has_value()) {
                if (searchThreadOpt->joinable()) {
                    searchThreadOpt->join();
                }
            }
            currentPos.reset();
        }
        else if (command == "isready") {
            std::cout << "readyok" << std::endl;
        }
        else if (command == "ucinewgame") {
            breakFlag.store(false);
        }
        else if (!parsedcommand.empty() && parsedcommand.at(0) == "position") { // Handle "position" command
            currentPos.reset();
            if (parsedcommand.size() > 3 && parsedcommand.at(2) == "moves") { // "position startpos moves ..."
                for (size_t i = 3; i < parsedcommand.size(); i++) {
                    currentPos.move(parsedcommand.at(i));
                }
            }
            else if (parsedcommand.at(1) == "fen") { // "position fen ..."
                parsedcommand.pop_front(); // Pop 'position'
                parsedcommand.pop_front(); // Pop 'fen'
                currentPos.loadFromFEN(parsedcommand);
            }
            if (parsedcommand.size() > 6 && parsedcommand.at(6) == "moves") {
                for (size_t i = 7; i < parsedcommand.size(); i++) {
                    currentPos.move(parsedcommand.at(i));
                }
            }
        }
        else if (command == "d") {
            currentPos.display();
        }
        else if (!parsedcommand.empty() && parsedcommand.at(0) == "move") {
            if (parsedcommand.size() >= 2) {
                currentPos.move(parsedcommand.at(1));
            }
        }
        else if (!parsedcommand.empty() && parsedcommand.at(0) == "go") { // Handle "go" command
            // If a search thread is already running, wait for it to finish
            if (searchThreadOpt.has_value()) {
                if (searchThreadOpt->joinable()) {
                    breakFlag.store(true);
                    searchThreadOpt->join();
                    breakFlag.store(false);
                }
                searchThreadOpt.reset();
            }

            int maxNodes = -1;

            if (findIndexOf(parsedcommand, "nodes") > 0) {
                maxNodes = stoi(parsedcommand.at(findIndexOf(parsedcommand, "nodes") + 1));
            }

            if (parsedcommand.size() > 2 && parsedcommand.at(1) == "depth") {
                int depth = stoi(parsedcommand.at(2));
                // Start a new search thread with depth
                searchThreadOpt.emplace(iterativeDeepening,
                    std::ref(currentPos),
                    depth,
                    std::ref(breakFlag),
                    0,
                    0,
                    maxNodes); // Ensure all six arguments are passed
            }
            else if (parsedcommand.size() > 4 && parsedcommand.at(1) == "wtime") {
                int wtime = stoi(parsedcommand.at(2));
                int btime = stoi(parsedcommand.at(4));
                // Start a new search thread with wtime and btime
                searchThreadOpt.emplace(iterativeDeepening,
                    std::ref(currentPos),
                    POS_INF,
                    std::ref(breakFlag),
                    wtime,
                    btime,
                    maxNodes); // Explicitly provide maxNodes
            }
            else if (parsedcommand.size() > 2 && parsedcommand.at(1) == "movetime") {
                int wtime = stoi(parsedcommand.at(2)) * 20;
                if (wtime > 200) wtime -= 100; // Margin
                else wtime -= 50;
                int btime = wtime;
                // Start a new search thread with movetime and provide maxNodes
                searchThreadOpt.emplace(iterativeDeepening,
                    std::ref(currentPos),
                    POS_INF,
                    std::ref(breakFlag),
                    wtime,
                    btime,
                    maxNodes); // Explicitly provide maxNodes
            }
            else {
                // Start a new search thread with default time parameters
                searchThreadOpt.emplace(iterativeDeepening,
                    std::ref(currentPos),
                    POS_INF,
                    std::ref(breakFlag),
                    0,
                    0,
                    maxNodes); // Ensure all six arguments are passed
            }
        }
        else if (command == "stop") {
            breakFlag.store(true);
            // Optionally, join the search thread to ensure it has stopped
            if (searchThreadOpt.has_value()) {
                if (searchThreadOpt->joinable()) {
                    searchThreadOpt->join();
                }
                searchThreadOpt.reset();
            }
        }
        else if (command == "quit") {
            breakFlag.store(true);
            // Ensure the search thread is joined before exiting
            if (searchThreadOpt.has_value()) {
                if (searchThreadOpt->joinable()) {
                    searchThreadOpt->join();
                }
            }
            return 0;
        }
        else if (command == "debug.timeit") {
            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < 1000; i++) {
                //ComputeBoard::evaluate(currentPos, ComputeBoard::calculateZobrist(currentPos));
            }
            std::cout << "Time to compute (nanoseconds): " << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count() / 1000 << std::endl;
        }
        else if (command == "debug.gamestate") {
            std::string bestMoveAlgebra;
            std::cout << "Is in check (current side to move): " << currentPos.isInCheck(currentPos.isWhite) << std::endl;
            //std::cout << "Current zobrist hash: " << currentPos.calculateZobrist() << std::endl;
            std::cout << "En passant index (64 if none): " << ctzll(currentPos.enPassant) << std::endl;
            std::cout << "Castling rights: " << std::bitset<4>(currentPos.castlingRights) << std::endl;
            std::cout << "Legal moves (current side to move):" << std::endl;
            MoveList moves = currentPos.generateLegalMoves();
            for (int i = 0; i < moves.count; i++) {
                std::cout << moves.moves[i].toString(currentPos) << std::endl;
            }
        }
        else if (parsedcommand.at(0) == "debug.perft") {
            auto start = std::chrono::high_resolution_clock::now();
            u64 nodes = perft(currentPos, stoi(parsedcommand.at(1)));
            int nps = (int)(((double)nodes) / (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) * 1000000000);
            double timeTaken = (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count());
            if (timeTaken / 1000000 < 1000) std::cout << "Time taken: " << timeTaken / 1000000 << " milliseconds" << std::endl;
            else std::cout << "Time taken: " << timeTaken / 1000000000 << " seconds" << std::endl;
            std::cout << "Generated moves with " << nodes << " nodes and NPS of " << nps << std::endl;
        }
        else if (parsedcommand.at(0) == "debug.perftsuite") {
            perftSuite(parsedcommand.at(1));
        }
        else if (command == "debug.moves") {
            std::cout << "All moves (current side to move):" << std::endl;
            auto moves = currentPos.generateMoves();
            for (int i = 0; i < moves.count; i++) {
                std::cout << moves.moves[i].toString(currentPos) << std::endl;
            }
        }
        else if (command == "debug.eval") {
            std::cout << "Current eval (current side to move): " << currentPos.evaluate() << std::endl;
        }
    }   
    return 0;
}
