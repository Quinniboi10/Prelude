#include <iostream>
#include <string>
#include <vector>
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
// For GCC or Clang
#ifdef __GNUC__
// Define popcount and count trailing zeros built-ins for GCC/Clang
#define popcountll(x) __builtin_popcountll(x)     // 64-bit popcount
#define ctzll(x) __builtin_ctzll(x)  // 64-bit count trailing zeros
#define clzll(x) __builtin_clzll(x) // 64-bit count leading zeros
#endif

// For MSVC
#ifdef _MSC_VER
// Define popcount and count trailing zeros intrinsics for MSVC
#define popcountll(x) __popcnt64(x)               // 64-bit popcount
#define ctzll(x) _tzcnt_u64(x)   // 64-bit count trailing zeros
#define clzll(x) ((x) == 0 ? 64 : (63 - __msb64(x)))   // 64-bit count leading zeros
#endif

inline int __msb64(uint64_t x) {
    unsigned long index; // Stores the position of the most significant bit
    if (_BitScanReverse64(&index, x)) {
        return index; // Index of the most significant set bit
    }
    return -1; // Should never reach here for non-zero inputs
}

constexpr int NEG_INF = -std::numeric_limits<int>::max();
constexpr int POS_INF = std::numeric_limits<int>::max();

int nodes = 0;

class Move;

std::deque<std::string> split(const std::string& s, char delim) {
    std::deque<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }
    return result;
}

class Precomputed {
public:
    static std::vector<Move> knightMoves[64];
    static std::array<bool, 64> isOnA;
    static std::array<bool, 64> isOnB;
    static std::array<bool, 64> isOnC;
    static std::array<bool, 64> isOnD;
    static std::array<bool, 64> isOnE;
    static std::array<bool, 64> isOnF;
    static std::array<bool, 64> isOnG;
    static std::array<bool, 64> isOnH;
    static std::array<bool, 64> isOn1;
    static std::array<bool, 64> isOn2;
    static std::array<bool, 64> isOn3;
    static std::array<bool, 64> isOn4;
    static std::array<bool, 64> isOn5;
    static std::array<bool, 64> isOn6;
    static std::array<bool, 64> isOn7;
    static std::array<bool, 64> isOn8;
    static std::array<std::array<uint64_t, 64>, 12> zobrist;
    static void compute();
};

int parseSquare(const std::string& square) {
    return (square.at(1) - '1') * 8 + (square.at(0) - 'a'); // Calculate the index of any square
}

class Move {
public:
    int startindex;
    int endindex;
    bool isCastle = false;
    bool isKingSide;
    int promotion = 0;
    Move() {
        startindex = -1;
        endindex = -1;
    }
    Move(const std::string& move) {
        if (move == "e1h1" || move == "e8h8") {
            isCastle = true;
            isKingSide = true;
        }
        else if (move == "e1a1" || move == "e8a8") {
            isCastle = true;
            isKingSide = false;
        }
        else {
            startindex = parseSquare(move.substr(0, 2));
            endindex = parseSquare(move.substr(2, 2));
        }
        if (move.length() > 4) {
            if (move.at(4) == 'q') promotion = 4;
            if (move.at(4) == 'r') promotion = 3;
            if (move.at(4) == 'b') promotion = 2;
            if (move.at(4) == 'k') promotion = 1;
        }
    }
    Move(int startindex, int endindex) {
        this->startindex = startindex;
        this->endindex = endindex;
    }
    Move(int startindex, int endindex, int promotion) {
        this->startindex = startindex;
        this->endindex = endindex;
        this->promotion = promotion;
    }
    Move(bool isWhite, bool isKingSide) {
        isCastle = true;
        this->isKingSide = isKingSide;
    }
    std::string algebra() const {
        if (!isCastle && promotion == 0) return std::string(1, (char)(startindex % 8 + 'a')) + std::to_string(startindex / 8 + 1) + std::string(1, (char)(endindex % 8 + 'a')) + std::to_string(endindex / 8 + 1);
        if (isCastle && isKingSide) return "O-O";
        else if (isCastle && !isKingSide) return "O-O-O";
        if (promotion == 1) return std::string(1, (char)(startindex % 8 + 'a')) + std::to_string(startindex / 8 + 1) + std::string(1, (char)(endindex % 8 + 'a')) + std::to_string(endindex / 8 + 1) + 'k';
        else if (promotion == 2) return std::string(1, (char)(startindex % 8 + 'a')) + std::to_string(startindex / 8 + 1) + std::string(1, (char)(endindex % 8 + 'a')) + std::to_string(endindex / 8 + 1) + 'b';
        else if (promotion == 3) return std::string(1, (char)(startindex % 8 + 'a')) + std::to_string(startindex / 8 + 1) + std::string(1, (char)(endindex % 8 + 'a')) + std::to_string(endindex / 8 + 1) + 'r';
        else if (promotion == 4) return std::string(1, (char)(startindex % 8 + 'a')) + std::to_string(startindex / 8 + 1) + std::string(1, (char)(endindex % 8 + 'a')) + std::to_string(endindex / 8 + 1) + 'q';
        return "UNKNOWN ERROR WITH MOVE CONVERSION\nMOVE FROM " + std::to_string(startindex) + " TO " + std::to_string(endindex);
    }
};


struct MoveEvaluation {
    Move move;
    int eval;
};

struct TableEntry {
    uint64_t zobrist;
    int eval;

    TableEntry() {
        zobrist = 0;
        eval = 0;
    }
    TableEntry(uint64_t zobrist, int eval) {
        this->zobrist = zobrist;
        this->eval = eval;
    }
};

struct MoveList {
    std::array<Move, 256> moves;
    int count;

    MoveList() {
        count = 0;
    }
};


std::vector<Move> Precomputed::knightMoves[64];
std::array<bool, 64> Precomputed::isOnA;
std::array<bool, 64> Precomputed::isOnB;
std::array<bool, 64> Precomputed::isOnC;
std::array<bool, 64> Precomputed::isOnD;
std::array<bool, 64> Precomputed::isOnE;
std::array<bool, 64> Precomputed::isOnF;
std::array<bool, 64> Precomputed::isOnG;
std::array<bool, 64> Precomputed::isOnH;
std::array<bool, 64> Precomputed::isOn1;
std::array<bool, 64> Precomputed::isOn2;
std::array<bool, 64> Precomputed::isOn3;
std::array<bool, 64> Precomputed::isOn4;
std::array<bool, 64> Precomputed::isOn5;
std::array<bool, 64> Precomputed::isOn6;
std::array<bool, 64> Precomputed::isOn7;
std::array<bool, 64> Precomputed::isOn8;
std::array<std::array<uint64_t, 64>, 12> Precomputed::zobrist;


void Precomputed::compute() {
    // *** FILE AND COL ARRAYS ***
    isOnA.fill(false);
    isOnB.fill(false);
    isOnC.fill(false);
    isOnD.fill(false);
    isOnE.fill(false);
    isOnF.fill(false);
    isOnG.fill(false);
    isOnH.fill(false);
    isOn1.fill(false);
    isOn2.fill(false);
    isOn3.fill(false);
    isOn4.fill(false);
    isOn5.fill(false);
    isOn6.fill(false);
    isOn7.fill(false);
    isOn8.fill(false);



    for (int i = 0; i < 64; ++i) {
        int file = i % 8; // File index (0 = A, 1 = B, ..., 7 = H)
        if (file == 0) isOnA[i] = true;
        if (file == 1) isOnB[i] = true;
        if (file == 2) isOnC[i] = true;
        if (file == 3) isOnD[i] = true;
        if (file == 4) isOnE[i] = true;
        if (file == 5) isOnF[i] = true;
        if (file == 6) isOnG[i] = true;
        if (file == 7) isOnH[i] = true;

        // Fill ranks (1-8)
        int rank = i / 8; // Rank index (0 = 1, 1 = 2, ..., 7 = 8)
        if (rank == 0) isOn1[i] = true;
        if (rank == 1) isOn2[i] = true;
        if (rank == 2) isOn3[i] = true;
        if (rank == 3) isOn4[i] = true;
        if (rank == 4) isOn5[i] = true;
        if (rank == 5) isOn6[i] = true;
        if (rank == 6) isOn7[i] = true;
        if (rank == 7) isOn8[i] = true;
    }

    // *** MOVE GENERATION ***


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
        onNEdge = isOn8[i];
        onNEdge2 = isOn8[i] || isOn7[i];
        onEEdge = isOnH[i];
        onEEdge2 = isOnH[i] || isOnG[i];
        onSEdge = isOn1[i];
        onSEdge2 = isOn1[i] || isOn2[i];
        onWEdge = isOnA[i];
        onWEdge2 = isOnA[i] || isOnB[i];

        std::vector<Move> positions;

        if (!onNEdge2 && !onEEdge) positions.push_back(Move(i, i + 17)); // Up 2, right 1
        if (!onNEdge && !onEEdge2) positions.push_back(Move(i, i + 10)); // Up 1, right 2
        if (!onSEdge && !onEEdge2) positions.push_back(Move(i, i - 6));  // Down 1, right 2
        if (!onSEdge2 && !onEEdge) positions.push_back(Move(i, i - 15)); // Down 2, right 1

        if (!onSEdge2 && !onWEdge) positions.push_back(Move(i, i - 17)); // Down 2, left 1
        if (!onSEdge && !onWEdge2) positions.push_back(Move(i, i - 10)); // Down 1, left 2
        if (!onNEdge && !onWEdge2) positions.push_back(Move(i, i + 6));  // Up 1, left 2
        if (!onNEdge2 && !onWEdge) positions.push_back(Move(i, i + 15)); // Up 2, left 1

        knightMoves[i] = positions;
    }

    // *** MAKE RANDOM ZOBRIST TABLE ****
    std::random_device rd;

    std::mt19937_64 engine(rd());

    std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<uint64_t>::max());

    for (auto& pieceTable : zobrist) {
        for (int i = 0; i < 64; i++) {
            pieceTable[i] = dist(engine);
        }
    }
}

class TranspositionTable {
    std::array<TableEntry, 1024> table;
    int count;
    TranspositionTable() {
        count = 0;
    }
    void add(TableEntry tableIn) {
        table[count++] = tableIn;
    }
};

class Board {
public:
    std::array<uint64_t, 6> white; // Goes pawns, knights, bishops, rooks, queens, king
    std::array<uint64_t, 6> black; // Goes pawns, knights, bishops, rooks, queens, king

    uint64_t blackPieces;
    uint64_t whitePieces;

    int enPassantIndex; // Square index for en passant target square
    uint8_t castlingRights;

    bool isWhite = true;

    int halfmoveClock = 0;

    std::vector<uint64_t> moveHistory;

    bool readBit(uint64_t bitboard, int index) {
        return (bitboard & (1ULL << index)) != 0;
    }

    void setBit(uint64_t& bitboard, int index, bool value) {
        if (value) bitboard |= 1ULL << index;
        else bitboard &= ~(1ULL << index);
    }

    bool readBit(uint8_t bitboard, int index) {
        return (bitboard & (1ULL << index)) != 0;
    }

    void setBit(uint8_t& bitboard, int index, bool value) {
        if (value) bitboard |= 1ULL << index;
        else bitboard &= ~(1ULL << index);
    }

    void clearIndex(int index) {
        for (uint64_t &currentBitboard : white) {
            setBit(currentBitboard, index, 0);
        }
        for (uint64_t &currentBitboard : black) {
            setBit(currentBitboard, index, 0);
        }
    }

    void recomputePosition() {
        whitePieces = 0;
        blackPieces = 0;
        for (uint64_t currentBitboard : white) {
            whitePieces |= currentBitboard;
        }
        for (uint64_t currentBitboard : black) {
            blackPieces |= currentBitboard;
        }
    }

    void reset() {
        // Compute knight moves
        Precomputed::compute();

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
        enPassantIndex = -1; // No en passant target square
        isWhite = true;
        recomputePosition();
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

    void move(const std::string& moveIn) {
        move(Move(moveIn));
    }

    void move(const Move& moveIn) {
        // Reference to the active player's bitboards
        std::array<uint64_t, 6>& bitboardArray = (isWhite) ? white : black;

        if (!moveIn.isCastle) {
            for (uint64_t& currentBitboard : bitboardArray) {
                if (readBit(currentBitboard, moveIn.startindex)) {
                    // Remove piece from start square
                    setBit(currentBitboard, moveIn.startindex, 0);

                    // Remove castling rights (very unoptimized)
                    if (castlingRights) {
                        if (moveIn.startindex == 0) setBit(castlingRights, 2, 0);
                        if (moveIn.startindex == 7) setBit(castlingRights, 3, 0);
                        if (moveIn.startindex == 4) { // King moved
                            setBit(castlingRights, 2, 0);
                            setBit(castlingRights, 3, 0);
                        }
                        if (moveIn.startindex == 56) setBit(castlingRights, 0, 0);
                        if (moveIn.startindex == 63) setBit(castlingRights, 1, 0);
                        if (moveIn.startindex == 60) { // King moved
                            setBit(castlingRights, 0, 0);
                            setBit(castlingRights, 1, 0);
                        }
                    }

                    // Handle capture: Check if any black piece is on the end square
                    if (readBit(blackPieces, moveIn.endindex)) {
                        halfmoveClock = 0; // Reset halfmove clock on capture
                    }
                    else if (currentBitboard == bitboardArray[0]) { // If it's a pawn move
                        halfmoveClock = 0; // Reset halfmove clock on pawn move
                        if (moveIn.startindex + 16 == moveIn.endindex) enPassantIndex = moveIn.startindex + 8; // Set en passant square
                        else if (moveIn.startindex - 16 == moveIn.endindex) enPassantIndex = moveIn.startindex - 8; // Set en passant square
                    }
                    else {
                        halfmoveClock++;
                    }

                    // Clear the destination square from all pieces
                    clearIndex(moveIn.endindex);

                    // Place the piece on the destination square
                    if (moveIn.promotion == 0) setBit(currentBitboard, moveIn.endindex, 1);
                    else {
                        if (moveIn.promotion == 1) setBit(bitboardArray[1], moveIn.endindex, 1);
                        else if (moveIn.promotion == 2) setBit(bitboardArray[2], moveIn.endindex, 1);
                        else if (moveIn.promotion == 3) setBit(bitboardArray[3], moveIn.endindex, 1);
                        else if (moveIn.promotion == 4) setBit(bitboardArray[4], moveIn.endindex, 1);
                    }
                    break;
                }
            }
        }
        else {
            if (isWhite && moveIn.isKingSide) {
                white[5] = 0; // Remove king
                setBit(white[3], 7, 0); // Remove kingside rook

                setBit(white[5], 6, 1); // Set king
                setBit(white[3], 5, 1); // Set rook
            }
            else if (isWhite && !moveIn.isKingSide) {
                white[5] = 0; // Remove king
                setBit(white[3], 0, 0); // Remove queenside rook

                setBit(white[5], 2, 1); // Set king
                setBit(white[3], 3, 1); // Set rook
            }


            else if (!isWhite && moveIn.isKingSide) {
                black[5] = 0; // Remove king
                setBit(black[3], 63, 0); // Remove kingside rook

                setBit(black[5], 62, 1); // Set king
                setBit(black[3], 61, 1); // Set rook
            }
            else if (!isWhite && !moveIn.isKingSide) {
                black[5] = 0; // Remove king
                setBit(black[3], 56, 0); // Remove queenside rook

                setBit(black[5], 58, 1); // Set king
                setBit(black[3], 59, 1); // Set rook
            }
        }
        

        isWhite = !isWhite;
        recomputePosition();

        moveHistory.push_back(calculateZobrist());
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

        if (inputFEN.at(3) != "-") enPassantIndex = parseSquare(inputFEN.at(3));
        else enPassantIndex = -1;

        halfmoveClock = std::stoi(inputFEN.at(4));

        recomputePosition();
    }

    

    bool isDraw() {
        bool sufficientMaterial = false;
        if (generateLegalMoves().count == 0) { // Stalemate
            return true;
        }
        else if (halfmoveClock == 100) { // 50 move rule
            return true;
        }
        else if (moveHistory.size() > 8) {
            std::unordered_map<uint64_t, int> hashCounts;

            for (const auto& hash : moveHistory) {
                // Increment the count for the current hash
                hashCounts[hash]++;

                // Check if this hash has been seen three times
                if (hashCounts[hash] == 3) {
                    return true;
                }
            }
        }
        
        // DRAW BY INSUFFICIENT MATERIAL
        // Helper lambda to determine if a square is light-colored
        auto isLightSquare = [&](int square) -> bool {
            int file = square % 8;
            int rank = square / 8;
            return (file + rank) % 2 == 1;
            };

        // Count pieces for White
        int whitePawns = popcountll(white[0]);
        int whiteKnights = popcountll(white[1]);
        int whiteBishops = popcountll(white[2]);
        int whiteRooks = popcountll(white[3]);
        int whiteQueens = popcountll(white[4]);

        // Count pieces for Black
        int blackPawns = popcountll(black[0]);
        int blackKnights = popcountll(black[1]);
        int blackBishops = popcountll(black[2]);
        int blackRooks = popcountll(black[3]);
        int blackQueens = popcountll(black[4]);

        // ---------------------------
        // Condition 1:
        // A king + any pawn, rook, queen is sufficient.
        // ---------------------------
        if (whitePawns > 0 || whiteRooks > 0 || whiteQueens > 0 ||
            blackPawns > 0 || blackRooks > 0 || blackQueens > 0) {
            return false; // Sufficient material exists, not a draw
        }

        // ---------------------------
        // Condition 2:
        // A king and more than one other type of piece is sufficient (e.g., knight + bishop).
        // ---------------------------
        int whitePieceTypes = (whiteKnights > 0) + (whiteBishops > 0);
        int blackPieceTypes = (blackKnights > 0) + (blackBishops > 0);
        if (whitePieceTypes > 1 || blackPieceTypes > 1) {
            return false; // Sufficient material exists, not a draw
        }

        // ---------------------------
        // Condition 3:
        // A king and two (or more) knights is sufficient.
        // ---------------------------
        if (whiteKnights >= 2 || blackKnights >= 2) {
            return false; // Sufficient material exists, not a draw
        }

        // ---------------------------
        // Condition 4:
        // King + knight against king + any (rook, bishop, knight, pawn) is sufficient.
        // ---------------------------
        bool whiteHasKnight = whiteKnights >= 1;
        bool blackHasKnight = blackKnights >= 1;
        bool blackHasOtherPieces = (blackRooks > 0 || blackBishops > 0 || blackKnights > 0 || blackPawns > 0);
        bool whiteHasOtherPieces = (whiteRooks > 0 || whiteBishops > 0 || whiteKnights > 0 || whitePawns > 0);

        if ((whiteHasKnight && blackHasOtherPieces) ||
            (blackHasKnight && whiteHasOtherPieces)) {
            return false; // Sufficient material exists, not a draw
        }

        // ---------------------------
        // Condition 5:
        // King + bishop against king + any (knight, pawn) is sufficient.
        // ---------------------------
        bool whiteHasBishop = whiteBishops >= 1;
        bool blackHasBishop = blackBishops >= 1;
        bool blackHasKnightOrPawn = (blackKnights > 0 || blackPawns > 0);
        bool whiteHasKnightOrPawn = (whiteKnights > 0 || whitePawns > 0);

        if ((whiteHasBishop && blackHasKnightOrPawn) ||
            (blackHasBishop && whiteHasKnightOrPawn)) {
            return false; // Sufficient material exists, not a draw
        }

        // ---------------------------
        // Condition 6:
        // King + bishop(s) is also sufficient if there's bishops on opposite colours.
        // ---------------------------
        bool whiteOnlyKingAndBishops = (whitePawns == 0 && whiteRooks == 0 && whiteQueens == 0 &&
            whiteKnights == 0 && whiteBishops > 0);
        bool blackOnlyKingAndBishops = (blackPawns == 0 && blackRooks == 0 && blackQueens == 0 &&
            blackKnights == 0 && blackBishops > 0);
        if (whiteOnlyKingAndBishops && blackOnlyKingAndBishops) {
            // Get all bishop squares for White
            std::vector<int> whiteBishopSquares;
            for (int i = 0; i < 64; ++i) {
                if (white[2] & (1ULL << i)) {
                    whiteBishopSquares.push_back(i);
                }
            }

            // Get all bishop squares for Black
            std::vector<int> blackBishopSquares;
            for (int i = 0; i < 64; ++i) {
                if (black[2] & (1ULL << i)) {
                    blackBishopSquares.push_back(i);
                }
            }

            // Check if any pair of White and Black bishops are on opposite colors
            bool oppositeColors = false;
            for (const auto& whiteBishop : whiteBishopSquares) {
                for (const auto& blackBishop : blackBishopSquares) {
                    if (isLightSquare(whiteBishop) != isLightSquare(blackBishop)) {
                        oppositeColors = true;
                        break;
                    }
                }
                if (oppositeColors) break;
            }

            if (oppositeColors) {
                return false; // Sufficient material exists, not a draw
            }
        }

        // ---------------------------
        // If none of the conditions are met, it's a draw due to insufficient material
        // ---------------------------
        return true; // Insufficient material, it's a draw
    }

    bool isSlidingPieceAttacking(int targetSquare, uint64_t pieceBitboard, uint64_t occupancy, bool diagonal) {
        // Directions: Bishop (diagonal) or Rook (straight lines)
        int directions[4];
        if (diagonal) {
            directions[0] = 9;   // Northeast
            directions[1] = 7;   // Northwest
            directions[2] = -7;  // Southeast
            directions[3] = -9;  // Southwest
        }
        else {
            directions[0] = 8;   // North
            directions[1] = -8;  // South
            directions[2] = 1;   // East
            directions[3] = -1;  // West
        }

        for (int dir : directions) {
            int currentSquare = targetSquare;
            while (true) {
                int file = currentSquare % 8;

                currentSquare += dir;

                // Stop if we go out of bounds
                if (currentSquare < 0 || currentSquare >= 64)
                    break;

                int newFile = currentSquare % 8;

                // For East, check if we've wrapped around to a new rank (file 0)
                if (dir == 1 && newFile == 0)
                    break;

                // For West, check if we've wrapped around to a new rank (file 7)
                if (dir == -1 && newFile == 7)
                    break;

                // For Northeast and Southeast, check if we've wrapped around from H to A
                if ((dir == 9 || dir == -7) && newFile == 0)
                    break;

                // For Northwest and Southwest, check if we've wrapped around from A to H
                if ((dir == 7 || dir == -9) && newFile == 7)
                    break;

                // Check if an opponent's piece is on this square
                if (readBit(occupancy, currentSquare)) {
                    if (readBit(pieceBitboard, currentSquare)) {
                        return true;
                    }
                    break; // Blocked by another piece
                }
            }
        }

        return false;
    }



    bool isInCheck(bool isWhite) {
        uint64_t kingBitboard = (isWhite) ? white[5] : black[5];
        if (kingBitboard == 0) {
            // No king found for the current player, invalid position
            return false;
        }

        int kingSquare = ctzll(kingBitboard); // Get the index of the king

        // Determine opponent's pieces
        uint64_t opponentBitboard = isWhite ? blackPieces : whitePieces;
        std::array<uint64_t, 6>& opponentPieces = isWhite ? black : white;

        // Combine occupancy of both sides
        uint64_t allOccupancy = whitePieces | blackPieces;

        // *** Sliding Piece Attacks (Bishops, Rooks, Queens) ***
        if (isSlidingPieceAttacking(kingSquare, opponentPieces[2], allOccupancy, true) || // Bishop
            isSlidingPieceAttacking(kingSquare, opponentPieces[3], allOccupancy, false) || // Rook
            isSlidingPieceAttacking(kingSquare, opponentPieces[4], allOccupancy, true) ||  // Queen (diagonals)
            isSlidingPieceAttacking(kingSquare, opponentPieces[4], allOccupancy, false)) { // Queen (straight lines)
            return true;
        }

        // *** Knight Attacks ***
        for (const Move& m : Precomputed::knightMoves[kingSquare]) {
            if (readBit(opponentPieces[1], m.endindex)) { // Knight attacks
                return true;
            }
        }

        // *** Pawn Attacks ***
        uint64_t pawnAttackMask;
        if (isWhite) {
            // For white king, check attacks from black pawns (which move downward)
            // Shift left by 7 for down-left attacks, ensuring not on the a-file
            // Shift left by 9 for down-right attacks, ensuring not on the h-file
            pawnAttackMask = ((kingBitboard << 7) & ~0x0101010101010101ULL) |
                ((kingBitboard << 9) & ~0x8080808080808080ULL);
        }
        else {
            // For black king, check attacks from white pawns (which move upward)
            // Shift right by 7 for up-left attacks, ensuring not on the h-file
            // Shift right by 9 for up-right attacks, ensuring not on the a-file
            pawnAttackMask = ((kingBitboard >> 7) & ~0x8080808080808080ULL) |
                ((kingBitboard >> 9) & ~0x0101010101010101ULL);
        }
        if (pawnAttackMask & opponentPieces[0]) {
            return true;
        }

        // *** Adjacent King Check ***
        // Ensure that the opposing king is not on any adjacent square
        uint64_t oppKingBitboard = isWhite ? black[5] : white[5];
        if (oppKingBitboard != 0) {
            int oppKingSquare = ctzll(oppKingBitboard); // Get the index of the opposing king

            int kingFile = kingSquare % 8;
            int kingRank = kingSquare / 8;
            int oppKingFile = oppKingSquare % 8;
            int oppKingRank = oppKingSquare / 8;

            int fileDifference = abs(oppKingFile - kingFile);
            int rankDifference = abs(oppKingRank - kingRank);

            // Kings are adjacent if both file and rank differences are <= 1
            if (fileDifference <= 1 && rankDifference <= 1) {
                return true;
            }
        }

        // If no attacks detected
        return false;
    }


    bool isCheckmate(bool isWhite) {
        return isInCheck(isWhite) && generateLegalMoves().count == 0;
    }

    void generatePawnMoves(bool isWhite, MoveList& moves, int& currentMoveIndex) {
        uint64_t pawnBitboard = isWhite ? this->white[0] : this->black[0];
        uint64_t opponentBitboard = isWhite ? this->blackPieces : this->whitePieces;
        uint64_t occupancy = whitePieces | blackPieces;

        int currentIndex;

        // Iterate through each pawn
        while (pawnBitboard > 0) {
            currentIndex = ctzll(pawnBitboard); // Find the next pawn's position
            bool isOnA = currentIndex % 8 == 0;
            bool isOnH = currentIndex % 8 == 7;

            if (isWhite) {
                // Single forward move
                int forwardIndex = currentIndex + 8;
                if (forwardIndex < 64 && !readBit(occupancy, forwardIndex)) {
                    // Promotions
                    if (Precomputed::isOn8[forwardIndex]) {
                        moves.moves[currentMoveIndex++] = Move(currentIndex, forwardIndex, 1);
                        moves.moves[currentMoveIndex++] = Move(currentIndex, forwardIndex, 2);
                        moves.moves[currentMoveIndex++] = Move(currentIndex, forwardIndex, 3);
                        moves.moves[currentMoveIndex++] = Move(currentIndex, forwardIndex, 4);
                    }
                    else
                        moves.moves[currentMoveIndex++] = Move(currentIndex, forwardIndex);

                    // Double forward move from starting rank
                    int doubleForwardIndex = currentIndex + 16;
                    if (currentIndex >= 8 && currentIndex <= 15 && !readBit(occupancy, doubleForwardIndex)) {
                        moves.moves[currentMoveIndex++] = Move(currentIndex, doubleForwardIndex);
                    }
                }

                // Diagonal captures
                if (!isOnH) {
                    int captureRightIndex = currentIndex + 9;
                    if (captureRightIndex < 64 && (readBit(opponentBitboard, captureRightIndex) || captureRightIndex == enPassantIndex)) {
                        // Promotions
                        if (Precomputed::isOn8[captureRightIndex]) {
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureRightIndex, 1);
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureRightIndex, 2);
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureRightIndex, 3);
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureRightIndex, 4);
                        }
                        else
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureRightIndex);
                    }
                }
                if (!isOnA) {
                    int captureLeftIndex = currentIndex + 7;
                    if (captureLeftIndex < 64 && (readBit(opponentBitboard, captureLeftIndex) || captureLeftIndex == enPassantIndex)) {
                        // Promotions
                        if (Precomputed::isOn8[captureLeftIndex]) {
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureLeftIndex, 1);
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureLeftIndex, 2);
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureLeftIndex, 3);
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureLeftIndex, 4);
                        }
                        else
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureLeftIndex);
                    }
                }
            }
            else {
                // Single forward move
                int forwardIndex = currentIndex - 8;
                if (forwardIndex >= 0 && !readBit(occupancy, forwardIndex)) {
                    // Promotions
                    if (Precomputed::isOn1[forwardIndex]) {
                        moves.moves[currentMoveIndex++] = Move(currentIndex, forwardIndex, 1);
                        moves.moves[currentMoveIndex++] = Move(currentIndex, forwardIndex, 2);
                        moves.moves[currentMoveIndex++] = Move(currentIndex, forwardIndex, 3);
                        moves.moves[currentMoveIndex++] = Move(currentIndex, forwardIndex, 4);
                    }
                    else
                        moves.moves[currentMoveIndex++] = Move(currentIndex, forwardIndex);

                    // Double forward move from starting rank
                    int doubleForwardIndex = currentIndex - 16;
                    if (currentIndex >= 48 && currentIndex <= 55 && !readBit(occupancy, doubleForwardIndex)) {
                        moves.moves[currentMoveIndex++] = Move(currentIndex, doubleForwardIndex);
                    }
                }

                // Diagonal captures
                if (!isOnH) {
                    int captureRightIndex = currentIndex - 7;
                    if (captureRightIndex >= 0 && readBit(opponentBitboard, captureRightIndex)) {
                        // Promotions
                        if (Precomputed::isOn1[captureRightIndex]) {
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureRightIndex, 1);
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureRightIndex, 2);
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureRightIndex, 3);
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureRightIndex, 4);
                        }
                        else
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureRightIndex);
                    }
                }
                if (!isOnA) {
                    int captureLeftIndex = currentIndex - 9;
                    if (captureLeftIndex >= 0 && readBit(opponentBitboard, captureLeftIndex)) {
                        // Promotions
                        if (Precomputed::isOn1[captureLeftIndex]) {
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureLeftIndex, 1);
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureLeftIndex, 2);
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureLeftIndex, 3);
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureLeftIndex, 4);
                        }
                        else
                            moves.moves[currentMoveIndex++] = Move(currentIndex, captureLeftIndex);
                    }
                }
            }

            // Remove the processed pawn from the bitboard
            pawnBitboard &= pawnBitboard - 1; // Clear least significant bit
        }
    }


    void generateKnightMoves(bool isWhite, MoveList& moves, int& currentMoveIndex) {
        int currentIndex;

        uint64_t knightBitboard = isWhite ? this->white[1] : this->black[1];
        uint64_t ownBitboard = isWhite ? this->whitePieces : this->blackPieces;

        while (knightBitboard > 0) {
            currentIndex = ctzll(knightBitboard);

            for (const Move& m : Precomputed::knightMoves[currentIndex]) {
                if (m.endindex >= 0 && m.endindex < 64 && !readBit(ownBitboard, m.endindex)) {
                    moves.moves[currentMoveIndex] = m;
                    currentMoveIndex++;
                }
            }

            knightBitboard &= knightBitboard - 1; // Clear least significant bit
        }
    }

    void generateBishopMoves(bool isWhite, MoveList& moves, int& currentMoveIndex) {
        int currentIndex;
        int startIndex;

        uint64_t bishopBitboard = isWhite ? this->white[2] : this->black[2];
        uint64_t opponentBitboard = isWhite ? this->blackPieces : this->whitePieces;
        uint64_t ownBitboard = isWhite ? this->whitePieces : this->blackPieces;

        while (bishopBitboard > 0) {
            startIndex = ctzll(bishopBitboard);

            // Northeast
            currentIndex = startIndex + 9;
            while (currentIndex < 64 && currentIndex % 8 != 0) {
                if (readBit(ownBitboard, currentIndex)) break;
                moves.moves[currentMoveIndex] = Move(startIndex, currentIndex);
                currentMoveIndex++;
                if (readBit(opponentBitboard, currentIndex)) break;
                currentIndex += 9;
            }

            // Northwest
            currentIndex = startIndex + 7;
            while (currentIndex < 64 && currentIndex % 8 != 7) {
                if (readBit(ownBitboard, currentIndex)) break;
                moves.moves[currentMoveIndex] = Move(startIndex, currentIndex);
                currentMoveIndex++;
                if (readBit(opponentBitboard, currentIndex)) break;
                currentIndex += 7;
            }

            // Southeast
            currentIndex = startIndex - 7;
            while (currentIndex >= 0 && currentIndex % 8 != 0) {
                if (readBit(ownBitboard, currentIndex)) break;
                moves.moves[currentMoveIndex] = Move(startIndex, currentIndex);
                currentMoveIndex++;
                if (readBit(opponentBitboard, currentIndex)) break;
                currentIndex -= 7;
            }

            // Southwest
            currentIndex = startIndex - 9;
            while (currentIndex >= 0 && currentIndex % 8 != 7) {
                if (readBit(ownBitboard, currentIndex)) break;
                moves.moves[currentMoveIndex] = Move(startIndex, currentIndex);
                currentMoveIndex++;
                if (readBit(opponentBitboard, currentIndex)) break;
                currentIndex -= 9;
            }

            bishopBitboard &= bishopBitboard - 1; // Clear least significant bit
        }
    }

    void generateRookMoves(bool isWhite, MoveList& moves, int& currentMoveIndex) {
        int currentIndex;
        int startIndex;

        uint64_t rookBitboard = isWhite ? this->white[3] : this->black[3];
        uint64_t opponentBitboard = isWhite ? this->blackPieces : this->whitePieces;
        uint64_t ownBitboard = isWhite ? this->whitePieces : this->blackPieces;

        while (rookBitboard > 0) {
            startIndex = ctzll(rookBitboard);

            // North
            currentIndex = startIndex + 8;
            while (currentIndex < 64) {
                if (readBit(ownBitboard, currentIndex)) break;
                moves.moves[currentMoveIndex] = Move(startIndex, currentIndex);
                currentMoveIndex++;
                if (readBit(opponentBitboard, currentIndex)) break;
                currentIndex += 8;
            }

            // South
            currentIndex = startIndex - 8;
            while (currentIndex >= 0) {
                if (readBit(ownBitboard, currentIndex)) break;
                moves.moves[currentMoveIndex] = Move(startIndex, currentIndex);
                currentMoveIndex++;
                if (readBit(opponentBitboard, currentIndex)) break;
                currentIndex -= 8;
            }

            // East
            currentIndex = startIndex + 1;
            while (currentIndex % 8 != 0) {
                if (readBit(ownBitboard, currentIndex)) break;
                moves.moves[currentMoveIndex] = Move(startIndex, currentIndex);
                currentMoveIndex++;
                if (readBit(opponentBitboard, currentIndex)) break;
                currentIndex += 1;
            }

            // West
            currentIndex = startIndex - 1;
            while (currentIndex % 8 != 7 && currentIndex >= 0) {
                if (readBit(ownBitboard, currentIndex)) break;
                moves.moves[currentMoveIndex] = Move(startIndex, currentIndex);
                currentMoveIndex++;
                if (readBit(opponentBitboard, currentIndex)) break;
                currentIndex -= 1;
            }

            rookBitboard &= rookBitboard - 1; // Clear least significant bit
        }
    }

    void generateQueenMoves(bool isWhite, MoveList& moves, int& currentMoveIndex) {
        uint64_t queenBitboard = isWhite ? this->white[4] : this->black[4];
        uint64_t opponentBitboard = isWhite ? this->blackPieces : this->whitePieces;
        uint64_t ownBitboard = isWhite ? this->whitePieces : this->blackPieces;

        int startIndex, currentIndex;

        while (queenBitboard > 0) {
            startIndex = ctzll(queenBitboard);

            // Diagonal moves (Bishop-like)
            // Northeast
            currentIndex = startIndex + 9;
            while (currentIndex < 64 && currentIndex % 8 != 0) {
                if (readBit(ownBitboard, currentIndex)) break;
                moves.moves[currentMoveIndex] = Move(startIndex, currentIndex);
                currentMoveIndex++;
                if (readBit(opponentBitboard, currentIndex)) break;
                currentIndex += 9;
            }

            // Northwest
            currentIndex = startIndex + 7;
            while (currentIndex < 64 && currentIndex % 8 != 7) {
                if (readBit(ownBitboard, currentIndex)) break;
                moves.moves[currentMoveIndex] = Move(startIndex, currentIndex);
                currentMoveIndex++;
                if (readBit(opponentBitboard, currentIndex)) break;
                currentIndex += 7;
            }

            // Southeast
            currentIndex = startIndex - 7;
            while (currentIndex >= 0 && currentIndex % 8 != 0) {
                if (readBit(ownBitboard, currentIndex)) break;
                moves.moves[currentMoveIndex] = Move(startIndex, currentIndex);
                currentMoveIndex++;
                if (readBit(opponentBitboard, currentIndex)) break;
                currentIndex -= 7;
            }

            // Southwest
            currentIndex = startIndex - 9;
            while (currentIndex >= 0 && currentIndex % 8 != 7) {
                if (readBit(ownBitboard, currentIndex)) break;
                moves.moves[currentMoveIndex] = Move(startIndex, currentIndex);
                currentMoveIndex++;
                if (readBit(opponentBitboard, currentIndex)) break;
                currentIndex -= 9;
            }

            // Straight moves (Rook-like)
            // North
            currentIndex = startIndex + 8;
            while (currentIndex < 64) {
                if (readBit(ownBitboard, currentIndex)) break;
                moves.moves[currentMoveIndex] = Move(startIndex, currentIndex);
                currentMoveIndex++;
                if (readBit(opponentBitboard, currentIndex)) break;
                currentIndex += 8;
            }

            // South
            currentIndex = startIndex - 8;
            while (currentIndex >= 0) {
                if (readBit(ownBitboard, currentIndex)) break;
                moves.moves[currentMoveIndex] = Move(startIndex, currentIndex);
                currentMoveIndex++;
                if (readBit(opponentBitboard, currentIndex)) break;
                currentIndex -= 8;
            }

            // East
            currentIndex = startIndex + 1;
            while (currentIndex % 8 != 0) {
                if (readBit(ownBitboard, currentIndex)) break;
                moves.moves[currentMoveIndex] = Move(startIndex, currentIndex);
                currentMoveIndex++;
                if (readBit(opponentBitboard, currentIndex)) break;
                currentIndex += 1;
            }

            // West
            currentIndex = startIndex - 1;
            while (currentIndex % 8 != 7 && currentIndex >= 0) {
                if (readBit(ownBitboard, currentIndex)) break;
                moves.moves[currentMoveIndex] = Move(startIndex, currentIndex);
                currentMoveIndex++;
                if (readBit(opponentBitboard, currentIndex)) break;
                currentIndex -= 1;
            }

            queenBitboard &= queenBitboard - 1; // Clear least significant bit
        }
    }

    void generateKingMoves(bool isWhite, MoveList& moves, int& currentMoveIndex) {
        uint64_t kingBitboard = isWhite ? this->white[5] : this->black[5];
        uint64_t opponentBitboard = isWhite ? this->blackPieces : this->whitePieces;
        uint64_t ownBitboard = isWhite ? this->whitePieces : this->blackPieces;

        int startIndex = ctzll(kingBitboard);
        int dirs[8] = { -1, 1, -8, 8, -9, -7, 7, 9 }; // The possible king moves

        for (int d = 0; d < 8; d++) {
            int currentIndex = startIndex + dirs[d];
            if (currentIndex >= 0 && currentIndex < 64) {
                // Exclude moves that wrap around the board
                if (abs((startIndex % 8) - (currentIndex % 8)) > 1) continue;

                if (!readBit(ownBitboard, currentIndex)) {
                    moves.moves[currentMoveIndex] = Move(startIndex, currentIndex);
                    currentMoveIndex++;
                }
            }
        }

        if (isWhite) {
            if (readBit(castlingRights, 3)) { // Kingside
                if (!readBit(whitePieces | blackPieces, 5) && !readBit(whitePieces | blackPieces, 6)) {
                    moves.moves[currentMoveIndex] = Move(true, true);
                    currentMoveIndex++;
                }
            }
            if (readBit(castlingRights, 2)) { // Queenside
                if (!readBit(whitePieces | blackPieces, 1) && !readBit(whitePieces | blackPieces, 2) && !readBit(whitePieces | blackPieces, 3)) {
                    moves.moves[currentMoveIndex] = Move(true, false);
                    currentMoveIndex++;
                }
            }
        }
        else {
            if (readBit(castlingRights, 1)) { // Kingside
                if (!readBit(whitePieces | blackPieces, 61) && !readBit(whitePieces | blackPieces, 62)) {
                    moves.moves[currentMoveIndex] = Move(false, true);
                    currentMoveIndex++;
                }
            }
            if (readBit(castlingRights, 0)) { // Queenside
                if (!readBit(whitePieces | blackPieces, 57) && !readBit(whitePieces | blackPieces, 58) && !readBit(whitePieces | blackPieces, 59)) {
                    moves.moves[currentMoveIndex] = Move(false, false);
                    currentMoveIndex++;
                }
            }
        }
    }

    MoveList generateMoves() {
        MoveList allMoves;
        MoveList checks, captures, quietMoves;
        uint64_t opponentPieces = isWhite ? blackPieces : whitePieces;
        std::array<uint64_t, 6>& ownPieces = isWhite ? white : black;

        // Generate moves for all pieces
        generatePawnMoves(isWhite, allMoves, allMoves.count);
        generateKnightMoves(isWhite, allMoves, allMoves.count);
        generateBishopMoves(isWhite, allMoves, allMoves.count);
        generateRookMoves(isWhite, allMoves, allMoves.count);
        generateQueenMoves(isWhite, allMoves, allMoves.count);
        generateKingMoves(isWhite, allMoves, allMoves.count);

        // Classify moves
        for (int i = 0; i < allMoves.count; ++i) {
            const Move& move = allMoves.moves[i];

            // Check if move is a capture
            bool isCapture = (opponentPieces & (1ULL << move.endindex)) != 0;

            // Check if the move results in a check
            Board testBoard = *this;
            testBoard.move(move);
            bool isCheck = testBoard.isInCheck(!isWhite);

            // Categorize the move
            if (isCheck) {
                checks.moves[checks.count++] = move;
            }
            else if (isCapture) {
                captures.moves[captures.count++] = move;
            }
            else {
                quietMoves.moves[quietMoves.count++] = move;
            }
        }

        // Combine moves in the prioritized order
        MoveList prioritizedMoves;
        for (int i = 0; i < checks.count; ++i) {
            prioritizedMoves.moves[prioritizedMoves.count++] = checks.moves[i];
        }
        for (int i = 0; i < captures.count; ++i) {
            prioritizedMoves.moves[prioritizedMoves.count++] = captures.moves[i];
        }
        for (int i = 0; i < quietMoves.count; ++i) {
            prioritizedMoves.moves[prioritizedMoves.count++] = quietMoves.moves[i];
        }

        return prioritizedMoves;
    }

    MoveList generateLegalMoves() {
        MoveList allMoves = generateMoves();
        MoveList legalMoves;
        for (int i = 0; i < allMoves.count; i++) {
            Board testBoard = *this;
            testBoard.move(allMoves.moves[i]);
            if (!testBoard.isInCheck(isWhite)) {
                legalMoves.moves[legalMoves.count++] = allMoves.moves[i];
            }
        }
        return legalMoves;
    }

    bool isLegalMove(Move m) {
        Board testBoard = *this;
        testBoard.move(m);
        return !testBoard.isInCheck(isWhite);
    }


    int evaluate() { // Returns evaluation in centipawns
        if (isDraw()) return 0;
        int eval = 0;

        // Material evaluation
        eval += popcountll(white[0]) * 100;
        eval += popcountll(white[1]) * 300;
        eval += popcountll(white[2]) * 300;
        eval += popcountll(white[3]) * 500;
        eval += popcountll(white[4]) * 900;

        eval -= popcountll(black[0]) * 100;
        eval -= popcountll(black[1]) * 300;
        eval -= popcountll(black[2]) * 300;
        eval -= popcountll(black[3]) * 500;
        eval -= popcountll(black[4]) * 900;

        // Adjust evaluation for the side to move
        return isWhite ? eval : -eval;
    }


    uint64_t calculateZobrist() {
        uint64_t hash = 0;
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
        if (castlingRights) hash ^= Precomputed::zobrist[0][0]/castlingRights;

        // En Passant
        if (castlingRights) hash ^= Precomputed::zobrist[0][0] / enPassantIndex;

        // Turn
        if (castlingRights) hash ^= Precomputed::zobrist[0][0] / (isWhite + 1);
        return hash;
    }
};


MoveEvaluation go(Board& board, int depth, std::atomic<bool>& breakFlag,
    std::chrono::high_resolution_clock::time_point timerStart,
    int timeToSpend = 0, int alpha = NEG_INF, int beta = POS_INF, bool doPruning = true) {

    // Check breakFlag at the start
    if (breakFlag.load()) {
        return { Move(), 0 };
    }

    if (depth == 0) {
        // At depth 0, check if opponent is in checkmate or stalemate
        MoveList oppMoves = board.generateLegalMoves();
        if (oppMoves.count == 0) {
            if (board.isInCheck(board.isWhite)) {
                // Opponent is in checkmate
                return { Move(), 100000 - depth };
            }
            else {
                // Stalemate
                return { Move(), 0 };
            }
        }
        else {
            int eval = board.evaluate();
            return { Move(), eval };
        }
    }


    MoveList moves = board.generateLegalMoves();

    if (moves.count == 0) {
        if (board.isInCheck(board.isWhite)) {
            return { Move(), -100000 + depth };
        }
        return { Move(), 0 }; // Stalemate
    }

    int maxEval = NEG_INF;
    Move bestMove;

    for (int i = 0; i < moves.count; ++i) {
        const Move& m = moves.moves[i];
        Board newBoard = board;
        newBoard.move(m);

        MoveEvaluation evalMove = go(newBoard, depth - 1, breakFlag, timerStart, timeToSpend, -beta, -alpha, doPruning);
        int eval = -evalMove.eval;

        if (eval > maxEval) {
            maxEval = eval;
            bestMove = m;
        }
        if (eval > alpha) {
            alpha = eval;
        }
        if (maxEval > 90000) {
            doPruning = false;
        }
        if (alpha >= beta && doPruning) {
            break; // Alpha-beta pruning
        }

        // Handle time constraints
        if (timeToSpend != 0) {
            auto now = std::chrono::high_resolution_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - timerStart).count() >= timeToSpend) {
                return { bestMove, maxEval };
            }
        }
        if (breakFlag.load()) {
            return { bestMove, maxEval };
        }
    }
    return { bestMove, maxEval };
}





void iterativeDeepening(Board& board, int maxDepth, std::atomic<bool>& breakFlag, int wtime = 0, int btime = 0) {
    int timeToSpend = 0;
    auto start = std::chrono::high_resolution_clock::now();
    std::string bestMoveAlgebra = "";
    if (wtime != 0) {
        timeToSpend = board.isWhite ? wtime / 20 : btime / 20;
    }

    MoveEvaluation bestMove = { Move(0, 0), board.evaluate() };
    MoveEvaluation currentMove = { Move(0, 0), board.evaluate() };

    for (int depth = 1; depth <= maxDepth; depth++) {
        // Always check breakFlag
        if (breakFlag.load()) {
            break;
        }

        // Check time constraints if timeToSpend is non-zero
        if (timeToSpend != 0) {
            auto now = std::chrono::high_resolution_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() >= timeToSpend) {
                break;
            }
        }

        currentMove = go(board, depth, breakFlag, start, timeToSpend, NEG_INF, POS_INF);
        if (currentMove.move.startindex != currentMove.move.endindex) bestMove = currentMove;
        bestMoveAlgebra = bestMove.move.algebra();
        if (bestMoveAlgebra == "O-O") {
            if (board.isWhite) bestMoveAlgebra = "e1h1";
            else bestMoveAlgebra = "e8h8";
        }
        else if (bestMoveAlgebra == "O-O-O") {
            if (board.isWhite) bestMoveAlgebra = "e1a1";
            else bestMoveAlgebra = "e8a8";
        }
        std::cout << "info depth " << depth << " score cp " << bestMove.eval << " pv " << bestMoveAlgebra << std::endl;
        if (bestMove.eval > 90000) break;
    }

    std::cout << "bestmove " << bestMoveAlgebra << std::endl;
    breakFlag.store(false);
}




int main() {
    std::string command;
    std::deque<std::string> parsedcommand;
    std::cout << "Bot ready and awaiting commands" << std::endl;
    Board currentPos;
    currentPos.reset();
    std::atomic<bool> breakFlag(false);
    std::optional<std::thread> searchThreadOpt; // Optional thread object

    while (true) {
        std::getline(std::cin, command);
        parsedcommand = split(command, ' ');
        if (command == "uci") {
            std::cout << "id name Bondfish" << std::endl;
            std::cout << "id author Quinn I" << std::endl;
            std::cout << "uciok" << std::endl;
        }
        else if (command == "isready") {
            std::cout << "readyok" << std::endl;
        }
        else if (command == "ucinewgame") {
            breakFlag.store(false);
        }
        else if (!parsedcommand.empty() && parsedcommand.at(0) == "position") { // Handle "position" command
            if (parsedcommand.size() > 3 && parsedcommand.at(2) == "moves") { // "position startpos moves ..."
                currentPos.reset();
                for (size_t i = 3; i < parsedcommand.size(); i++) {
                    currentPos.move(parsedcommand.at(i));
                }
            }
            else if (parsedcommand.at(1) == "fen") { // "position fen ..."
                currentPos.reset();
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
        else if (!parsedcommand.empty() && parsedcommand.at(0) == "go") { // Handle "go" command
            // If a search thread is already running, wait for it to finish
            if (searchThreadOpt.has_value()) {
                if (searchThreadOpt->joinable()) {
                    searchThreadOpt->join();
                }
                searchThreadOpt.reset();
            }

            if (parsedcommand.size() > 2 && parsedcommand.at(1) == "depth") {
                int depth = stoi(parsedcommand.at(2));
                // Start a new search thread with depth
                searchThreadOpt.emplace(iterativeDeepening,
                    std::ref(currentPos),
                    depth,
                    std::ref(breakFlag),
                    0,
                    0);
            }
            else if (parsedcommand.size() > 4 && parsedcommand.at(1) == "wtime") {
                int wtime = stoi(parsedcommand.at(2));
                int btime = stoi(parsedcommand.at(4));
                // Start a new search thread with wtime and btime
                searchThreadOpt.emplace(iterativeDeepening,
                    std::ref(currentPos),
                    std::numeric_limits<int>::max(),
                    std::ref(breakFlag),
                    wtime,
                    btime);
            }
            else if (parsedcommand.size() > 4 && parsedcommand.at(1) == "movetime") {
                int wtime = stoi(parsedcommand.at(2)) * 20;
                int btime = wtime;
                // Start a new search thread with wtime and btime
                searchThreadOpt.emplace(iterativeDeepening,
                    std::ref(currentPos),
                    std::numeric_limits<int>::max(),
                    std::ref(breakFlag),
                    wtime,
                    btime);
            }
            else {
                // Start a new search thread with default time parameters
                searchThreadOpt.emplace(iterativeDeepening,
                    std::ref(currentPos),
                    POS_INF,
                    std::ref(breakFlag),
                    0,
                    0);
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
        else if (command == "d") {
            currentPos.display();
        }
        else if (!parsedcommand.empty() && parsedcommand.at(0) == "move") {
            if (parsedcommand.size() >= 2) {
                currentPos.move(parsedcommand.at(1));
            }
        }
        else if (command == "eval") {
            std::cout << "Current evaluation with no depth: " << currentPos.evaluate() << std::endl;
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
        else if (command == "cmd") {
            std::string bestMoveAlgebra;
            std::cout << "Is in check (current side to move): " << currentPos.isInCheck(currentPos.isWhite) << std::endl;
            std::cout << "Is checkmate (current side to move): " << currentPos.isCheckmate(currentPos.isWhite) << std::endl;
            std::cout << "Current zobrist hash: " << currentPos.calculateZobrist() << std::endl;
            std::cout << "Legal moves (current side to move): " << std::endl;
            MoveList moves = currentPos.generateLegalMoves();
            for (int i = 0; i < moves.count; i++) {
                bestMoveAlgebra = moves.moves[i].algebra();
                if (bestMoveAlgebra == "O-O") {
                    if (currentPos.isWhite) bestMoveAlgebra = "e1h1";
                    else bestMoveAlgebra = "e8h8";
                }
                else if (bestMoveAlgebra == "O-O-O") {
                    if (currentPos.isWhite) bestMoveAlgebra = "e1a1";
                    else bestMoveAlgebra = "e1a1";
                }
                std::cout << bestMoveAlgebra << std::endl;
            }
        }
    }

    return 0;
}
