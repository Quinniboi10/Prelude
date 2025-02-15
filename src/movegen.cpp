#include "movegen.h"
#include "types.h"

#include <fstream>
#include <cassert>

array<array<u64, 64>, 64> LINE;
array<array<u64, 64>, 64> LINESEG;

// Magic code from https://github.com/nkarve/surge/blob/master/src/tables.cpp
constexpr int diagonalOf(Square s) { return 7 + rankOf(s) - fileOf(s); }
constexpr int antiDiagonalOf(Square s) { return rankOf(s) + Rank(fileOf(s)); }

//Precomputed diagonal masks
const u64 MASK_DIAGONAL[15] = {
  0x80,
  0x8040,
  0x804020,
  0x80402010,
  0x8040201008,
  0x804020100804,
  0x80402010080402,
  0x8040201008040201,
  0x4020100804020100,
  0x2010080402010000,
  0x1008040201000000,
  0x804020100000000,
  0x402010000000000,
  0x201000000000000,
  0x100000000000000,
};

//Precomputed anti-diagonal masks
const u64 MASK_ANTI_DIAGONAL[15] = {
  0x1,
  0x102,
  0x10204,
  0x1020408,
  0x102040810,
  0x10204081020,
  0x1020408102040,
  0x102040810204080,
  0x204081020408000,
  0x408102040800000,
  0x810204080000000,
  0x1020408000000000,
  0x2040800000000000,
  0x4080000000000000,
  0x8000000000000000,
};

// clang-format off
//Precomputed square masks
const u64 SQUARE_BB[65] = {
    0x1, 0x2, 0x4, 0x8,
    0x10, 0x20, 0x40, 0x80,
    0x100, 0x200, 0x400, 0x800,
    0x1000, 0x2000, 0x4000, 0x8000,
    0x10000, 0x20000, 0x40000, 0x80000,
    0x100000, 0x200000, 0x400000, 0x800000,
    0x1000000, 0x2000000, 0x4000000, 0x8000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000,
    0x100000000, 0x200000000, 0x400000000, 0x800000000,
    0x1000000000, 0x2000000000, 0x4000000000, 0x8000000000,
    0x10000000000, 0x20000000000, 0x40000000000, 0x80000000000,
    0x100000000000, 0x200000000000, 0x400000000000, 0x800000000000,
    0x1000000000000, 0x2000000000000, 0x4000000000000, 0x8000000000000,
    0x10000000000000, 0x20000000000000, 0x40000000000000, 0x80000000000000,
    0x100000000000000, 0x200000000000000, 0x400000000000000, 0x800000000000000,
    0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000,
    0x0};
//clang-format on


//Reverses a bitboard
u64 reverse(u64 b) {
    b = (b & 0x5555555555555555) << 1 | ((b >> 1) & 0x5555555555555555);
    b = (b & 0x3333333333333333) << 2 | ((b >> 2) & 0x3333333333333333);
    b = (b & 0x0f0f0f0f0f0f0f0f) << 4 | ((b >> 4) & 0x0f0f0f0f0f0f0f0f);
    b = (b & 0x00ff00ff00ff00ff) << 8 | ((b >> 8) & 0x00ff00ff00ff00ff);

    return (b << 48) | ((b & 0xffff0000) << 16) | ((b >> 16) & 0xffff0000) | (b >> 48);
}

//Calculates sliding attacks from a given square, on a given axis, taking into
//account the blocking pieces. This uses the Hyperbola Quintessence Algorithm.
u64 sliding_attacks(Square square, u64 occ, u64 mask) { return (((mask & occ) - SQUARE_BB[square] * 2) ^ reverse(reverse(mask & occ) - reverse(SQUARE_BB[square]) * 2)) & mask; }

//Returns rook attacks from a given square, using the Hyperbola Quintessence Algorithm. Only used to initialize
//the magic lookup table
u64 get_rook_attacks_for_init(Square square, u64 occ) { return sliding_attacks(square, occ, MASK_FILE[fileOf(square)]) | sliding_attacks(square, occ, MASK_RANK[rankOf(square)]); }

u64 ROOK_ATTACK_MASKS[64];
int ROOK_ATTACK_SHIFTS[64];
u64 ROOK_ATTACKS[64][4096];

const u64 ROOK_MAGICS[64] = {0x0080001020400080, 0x0040001000200040, 0x0080081000200080, 0x0080040800100080, 0x0080020400080080, 0x0080010200040080, 0x0080008001000200, 0x0080002040800100,
                             0x0000800020400080, 0x0000400020005000, 0x0000801000200080, 0x0000800800100080, 0x0000800400080080, 0x0000800200040080, 0x0000800100020080, 0x0000800040800100,
                             0x0000208000400080, 0x0000404000201000, 0x0000808010002000, 0x0000808008001000, 0x0000808004000800, 0x0000808002000400, 0x0000010100020004, 0x0000020000408104,
                             0x0000208080004000, 0x0000200040005000, 0x0000100080200080, 0x0000080080100080, 0x0000040080080080, 0x0000020080040080, 0x0000010080800200, 0x0000800080004100,
                             0x0000204000800080, 0x0000200040401000, 0x0000100080802000, 0x0000080080801000, 0x0000040080800800, 0x0000020080800400, 0x0000020001010004, 0x0000800040800100,
                             0x0000204000808000, 0x0000200040008080, 0x0000100020008080, 0x0000080010008080, 0x0000040008008080, 0x0000020004008080, 0x0000010002008080, 0x0000004081020004,
                             0x0000204000800080, 0x0000200040008080, 0x0000100020008080, 0x0000080010008080, 0x0000040008008080, 0x0000020004008080, 0x0000800100020080, 0x0000800041000080,
                             0x00FFFCDDFCED714A, 0x007FFCDDFCED714A, 0x003FFFCDFFD88096, 0x0000040810002101, 0x0001000204080011, 0x0001000204000801, 0x0001000082000401, 0x0001FFFAABFAD1A2};

//Initializes the magic lookup table for rooks
void initializeRookAttacks() {
    u64 edges, subset, index;

    for (Square sq = a1; sq <= h8; ++sq) {
        edges                  = ((MASK_RANK[AFILE] | MASK_RANK[HFILE]) & ~MASK_RANK[rankOf(sq)]) | ((MASK_FILE[AFILE] | MASK_FILE[HFILE]) & ~MASK_FILE[fileOf(sq)]);
        ROOK_ATTACK_MASKS[sq]  = (MASK_RANK[rankOf(sq)] ^ MASK_FILE[fileOf(sq)]) & ~edges;
        ROOK_ATTACK_SHIFTS[sq] = 64 - popcount(ROOK_ATTACK_MASKS[sq]);

        subset = 0;
        do {
            index                   = subset;
            index                   = index * ROOK_MAGICS[sq];
            index                   = index >> ROOK_ATTACK_SHIFTS[sq];
            ROOK_ATTACKS[sq][index] = get_rook_attacks_for_init(sq, subset);
            subset                  = (subset - ROOK_ATTACK_MASKS[sq]) & ROOK_ATTACK_MASKS[sq];
        } while (subset);
    }
}

//Returns the attacks bitboard for a rook at a given square, using the magic lookup table
u64 Movegen::getRookAttacks(Square square, u64 occ) { return ROOK_ATTACKS[square][((occ & ROOK_ATTACK_MASKS[square]) * ROOK_MAGICS[square]) >> ROOK_ATTACK_SHIFTS[square]]; }

//Returns the 'x-ray attacks' for a rook at a given square. X-ray attacks cover squares that are not immediately
//accessible by the rook, but become available when the immediate blockers are removed from the board
u64 Movegen::getXrayRookAttacks(Square square, u64 occ, u64 blockers) {
    u64 attacks = getRookAttacks(square, occ);
    blockers &= attacks;
    return attacks ^ getRookAttacks(square, occ ^ blockers);
}

//Returns bishop attacks from a given square, using the Hyperbola Quintessence Algorithm. Only used to initialize
//the magic lookup table
u64 getBishopAttacksForInit(Square square, u64 occ) {
    return sliding_attacks(square, occ, MASK_DIAGONAL[diagonalOf(square)]) | sliding_attacks(square, occ, MASK_ANTI_DIAGONAL[antiDiagonalOf(square)]);
}

u64 BISHOP_ATTACK_MASKS[64];
int BISHOP_ATTACK_SHIFTS[64];
u64 BISHOP_ATTACKS[64][512];

const u64 BISHOP_MAGICS[64] = {0x0002020202020200, 0x0002020202020000, 0x0004010202000000, 0x0004040080000000, 0x0001104000000000, 0x0000821040000000, 0x0000410410400000, 0x0000104104104000,
                               0x0000040404040400, 0x0000020202020200, 0x0000040102020000, 0x0000040400800000, 0x0000011040000000, 0x0000008210400000, 0x0000004104104000, 0x0000002082082000,
                               0x0004000808080800, 0x0002000404040400, 0x0001000202020200, 0x0000800802004000, 0x0000800400A00000, 0x0000200100884000, 0x0000400082082000, 0x0000200041041000,
                               0x0002080010101000, 0x0001040008080800, 0x0000208004010400, 0x0000404004010200, 0x0000840000802000, 0x0000404002011000, 0x0000808001041000, 0x0000404000820800,
                               0x0001041000202000, 0x0000820800101000, 0x0000104400080800, 0x0000020080080080, 0x0000404040040100, 0x0000808100020100, 0x0001010100020800, 0x0000808080010400,
                               0x0000820820004000, 0x0000410410002000, 0x0000082088001000, 0x0000002011000800, 0x0000080100400400, 0x0001010101000200, 0x0002020202000400, 0x0001010101000200,
                               0x0000410410400000, 0x0000208208200000, 0x0000002084100000, 0x0000000020880000, 0x0000001002020000, 0x0000040408020000, 0x0004040404040000, 0x0002020202020000,
                               0x0000104104104000, 0x0000002082082000, 0x0000000020841000, 0x0000000000208800, 0x0000000010020200, 0x0000000404080200, 0x0000040404040400, 0x0002020202020200};

//Initializes the magic lookup table for bishops
void initializeBishopAttacks() {
    u64 edges, subset, index;

    for (Square sq = a1; sq <= h8; ++sq) {
        edges                    = ((MASK_RANK[AFILE] | MASK_RANK[HFILE]) & ~MASK_RANK[rankOf(sq)]) | ((MASK_FILE[AFILE] | MASK_FILE[HFILE]) & ~MASK_FILE[fileOf(sq)]);
        BISHOP_ATTACK_MASKS[sq]  = (MASK_DIAGONAL[diagonalOf(sq)] ^ MASK_ANTI_DIAGONAL[antiDiagonalOf(sq)]) & ~edges;
        BISHOP_ATTACK_SHIFTS[sq] = 64 - popcount(BISHOP_ATTACK_MASKS[sq]);

        subset = 0;
        do {
            index                     = subset;
            index                     = index * BISHOP_MAGICS[sq];
            index                     = index >> BISHOP_ATTACK_SHIFTS[sq];
            BISHOP_ATTACKS[sq][index] = getBishopAttacksForInit(sq, subset);
            subset                    = (subset - BISHOP_ATTACK_MASKS[sq]) & BISHOP_ATTACK_MASKS[sq];
        } while (subset);
    }
}

//Returns the attacks bitboard for a bishop at a given square, using the magic lookup table
u64 Movegen::getBishopAttacks(Square square, u64 occ) { return BISHOP_ATTACKS[square][((occ & BISHOP_ATTACK_MASKS[square]) * BISHOP_MAGICS[square]) >> BISHOP_ATTACK_SHIFTS[square]]; }

//Returns the 'x-ray attacks' for a bishop at a given square. X-ray attacks cover squares that are not immediately
//accessible by the rook, but become available when the immediate blockers are removed from the board
u64 Movegen::getXrayBishopAttacks(Square square, u64 occ, u64 blockers) {
    u64 attacks = getBishopAttacks(square, occ);
    blockers &= attacks;
    return attacks ^ getBishopAttacks(square, occ ^ blockers);
}

u64 SQUARES_BETWEEN_BB[64][64];

//Initializes the lookup table for the bitboard of squares in between two given squares (0 if the
//two squares are not aligned)
void initializeSquaresBetween() {
    u64 sqs;
    for (Square sq1 = a1; sq1 <= h8; ++sq1)
        for (Square sq2 = a1; sq2 <= h8; ++sq2) {
            sqs = SQUARE_BB[sq1] | SQUARE_BB[sq2];
            if (fileOf(sq1) == fileOf(sq2) || rankOf(sq1) == rankOf(sq2))
                SQUARES_BETWEEN_BB[sq1][sq2] = get_rook_attacks_for_init(sq1, sqs) & get_rook_attacks_for_init(sq2, sqs);
            else if (diagonalOf(sq1) == diagonalOf(sq2) || antiDiagonalOf(sq1) == antiDiagonalOf(sq2))
                SQUARES_BETWEEN_BB[sq1][sq2] = getBishopAttacksForInit(sq1, sqs) & getBishopAttacksForInit(sq2, sqs);
        }
}

//Initializes the lookup table for the bitboard of all squares along the line of two given squares (0 if the
//two squares are not aligned)
void initializeLine() {
    for (Square sq1 = a1; sq1 <= h8; ++sq1) {
        for (Square sq2 = a1; sq2 <= h8; ++sq2) {
            if (fileOf(sq1) == fileOf(sq2) || rankOf(sq1) == rankOf(sq2))
                LINE[sq1][sq2] = (get_rook_attacks_for_init(sq1, 0) & get_rook_attacks_for_init(sq2, 0)) | SQUARE_BB[sq1] | SQUARE_BB[sq2];
            else if (diagonalOf(sq1) == diagonalOf(sq2) || antiDiagonalOf(sq1) == antiDiagonalOf(sq2))
                LINE[sq1][sq2] = (getBishopAttacksForInit(sq1, 0) & getBishopAttacksForInit(sq2, 0)) | SQUARE_BB[sq1] | SQUARE_BB[sq2];
        }
    }

    for (Square sq1 = a1; sq1 <= h8; ++sq1) {
        for (Square sq2 = a1; sq2 <= h8; ++sq2) {
            u64 blockers = (1ULL << sq1) | (1ULL << sq2);
            if (fileOf(sq1) == fileOf(sq2) || rankOf(sq1) == rankOf(sq2))
                LINESEG[sq1][sq2] = (get_rook_attacks_for_init(sq1, blockers) & get_rook_attacks_for_init(sq2, blockers)) | SQUARE_BB[sq1] | SQUARE_BB[sq2];
            else if (diagonalOf(sq1) == diagonalOf(sq2) || antiDiagonalOf(sq1) == antiDiagonalOf(sq2))
                LINESEG[sq1][sq2] = (getBishopAttacksForInit(sq1, blockers) & getBishopAttacksForInit(sq2, blockers)) | SQUARE_BB[sq1] | SQUARE_BB[sq2];
        }
    }
}

//Initializes lookup tables for rook moves, bishop moves, in-between squares, aligned squares and pseudolegal moves
void Movegen::initializeAllDatabases() {
    initializeRookAttacks();
    initializeBishopAttacks();
    initializeSquaresBetween();
    initializeLine();
}

u64 Movegen::pawnAttackBB(Color c, int sq) {
    assert(sq >= a1);
    assert(sq < NO_SQUARE);

    const u64 sqBB = 1ULL << sq;
    if (c == WHITE) {
        return shift<NORTH_EAST>(sqBB & ~MASK_FILE[HFILE]) | shift<NORTH_WEST>(sqBB & ~MASK_FILE[AFILE]);
    }
    return shift<SOUTH_EAST>(sqBB & ~MASK_FILE[HFILE]) | shift<SOUTH_WEST>(sqBB & ~MASK_FILE[AFILE]);
}

void Movegen::pawnMoves(const Board& board, MoveList& moves) {
    u64 pawns = board.pieces(board.stm, PAWN);
    Direction pushDir      = board.stm == WHITE ? NORTH : SOUTH;
    u64 singlePushes = shift(pushDir, pawns) & ~board.pieces();
    u64 pushPromo = singlePushes & (MASK_RANK[RANK1] | MASK_RANK[RANK8]);
    singlePushes ^= pushPromo;

    u64 doublePushes = shift(pushDir, singlePushes) & ~board.pieces();
    doublePushes &= board.stm == WHITE ? MASK_RANK[RANK4] : MASK_RANK[RANK5];

    u64 captureEast = shift(pushDir + EAST, pawns & ~MASK_FILE[HFILE]) & board.pieces(~board.stm);
    u64 captureWest = shift(pushDir + WEST, pawns & ~MASK_FILE[AFILE]) & board.pieces(~board.stm);

    u64 eastPromo = captureEast & (MASK_RANK[RANK1] | MASK_RANK[RANK8]);
    captureEast ^= eastPromo;
    u64 westPromo = captureWest & (MASK_RANK[RANK1] | MASK_RANK[RANK8]);
    captureWest ^= westPromo;

    auto addPromos = [&](Square from, Square to) {
        assert(from >= 0);
        assert(from < 64);

        assert(to >= 0);
        assert(to < 64);

        moves.add(Move(from, to, QUEEN_PROMO));
        moves.add(Move(from, to, ROOK_PROMO));
        moves.add(Move(from, to, BISHOP_PROMO));
        moves.add(Move(from, to, KNIGHT_PROMO));
    };

    int backshift = pushDir;

    while (singlePushes) {
        Square to   = popLSB(singlePushes);
        Square from = Square(to - backshift);

        moves.add(from, to);
    }

    while (pushPromo) {
        Square to   = popLSB(pushPromo);
        Square from = Square(to - backshift);

        addPromos(from, to);
    }

    backshift += pushDir;

    while (doublePushes) {
        Square to   = popLSB(doublePushes);
        Square from = Square(to - backshift);

        moves.add(from, to, DOUBLE_PUSH);
    }

    backshift = pushDir + EAST;

    while (captureEast) {
        Square to   = popLSB(captureEast);
        Square from = Square(to - backshift);

        moves.add(from, to);
    }

    while (eastPromo) {
        Square to   = popLSB(eastPromo);
        Square from = Square(to - backshift);

        addPromos(from, to);
    }

    backshift = pushDir + WEST;

    while (captureWest) {
        Square to   = popLSB(captureWest);
        Square from = Square(to - backshift);

        moves.add(from, to);
    }

    while (westPromo) {
        Square to   = popLSB(westPromo);
        Square from = Square(to - backshift);

        addPromos(from, to);
    }

    if (board.epSquare != NO_SQUARE) {
        u64 epMoves = pawnAttackBB(~board.stm, board.epSquare) & board.pieces(board.stm, PAWN);

        while (epMoves) {
            Square from = popLSB(epMoves);

            moves.add(from, board.epSquare, EN_PASSANT);
        }
    }
}

void Movegen::knightMoves(const Board& board, MoveList& moves) {
    u64 knightBB = board.pieces(board.stm, KNIGHT);

    u64 occ = board.pieces();
    u64 friendly = board.pieces(board.stm);

    while (knightBB > 0) {
        Square currentSquare = popLSB(knightBB);

        u64 knightMoves = KNIGHT_ATTACKS[currentSquare];
        knightMoves &= ~friendly;

        while (knightMoves > 0) {
            Square to = popLSB(knightMoves);
            moves.add(currentSquare, to);
        }
    }
}

void Movegen::bishopMoves(const Board& board, MoveList& moves) {
    u64 bishopBB = board.pieces(board.stm, BISHOP, QUEEN);

    u64 occ = board.pieces();
    u64 friendly = board.pieces(board.stm);

    while (bishopBB > 0) {
        Square currentSquare = popLSB(bishopBB);

        u64 bishopMoves = getBishopAttacks(currentSquare, occ);
        bishopMoves &= ~friendly;

        while (bishopMoves > 0) {
            Square to = popLSB(bishopMoves);
            moves.add(currentSquare, to);
        }
    }
}

void Movegen::rookMoves(const Board& board, MoveList& moves) {
    u64 rookBB = board.pieces(board.stm, ROOK, QUEEN);

    u64 occ = board.pieces();
    u64 friendly = board.pieces(board.stm);

    while (rookBB > 0) {
        Square currentSquare = popLSB(rookBB);

        u64 rookMoves = getRookAttacks(currentSquare, occ);
        rookMoves &= ~friendly;

        while (rookMoves > 0) {
            Square to = popLSB(rookMoves);
            moves.add(currentSquare, to);
        }
    }
}

void Movegen::kingMoves(const Board& board, MoveList& moves) {
    Square kingSq = Square(ctzll(board.pieces(board.stm, KING)));

    assert(kingSq >= a1);
    assert(kingSq < NO_SQUARE);

    u64 kingMoves = KING_ATTACKS[kingSq];
    kingMoves &= ~board.pieces(board.stm);

    u64 occ = board.pieces();

    while (kingMoves > 0) {
        Square to = popLSB(kingMoves);
        moves.add(kingSq, to);
    }
    
    if (board.canCastle(board.stm, true)) moves.add(kingSq, castleSq(board.stm, true), CASTLE_K);
    if (board.canCastle(board.stm, false)) moves.add(kingSq, castleSq(board.stm, false), CASTLE_Q);
}

MoveList Movegen::generateMoves(const Board& board) {
    MoveList moves;
    pawnMoves(board, moves);
    knightMoves(board, moves);
    bishopMoves(board, moves);
    rookMoves(board, moves);
    kingMoves(board, moves);
    // Note: Queen moves are done at the same time as bishop/rook moves

    return moves;
}

u64 bulk(const Board& board, usize depth) {
    u64 nodes = 0;

    MoveList moves = Movegen::generateMoves(board);

    Board testBoard;

    if (depth == 0)
        return 1;

    for (Move m : moves) {
        if (!board.isLegal(m))
            continue;

        if (depth == 1) {
            nodes++;
            continue;
        }

        testBoard = board;

        testBoard.move(m);
        nodes += bulk(testBoard, depth - 1);
    }

    return nodes;
}

u64 perft(const Board& board, usize depth) {
    u64 nodes = 0;

    MoveList moves = Movegen::generateMoves(board);

    Board testBoard;

    if (depth == 0)
        return 1;

    for (Move m : moves) {
        if (!board.isLegal(m))
            continue;

        testBoard = board;

        testBoard.move(m);
        nodes += perft(testBoard, depth - 1);
    }

    return nodes;
}

void Movegen::perft(const Board& board, usize depth, bool bulk) {
    u64 nodes = 0;

    MoveList moves = generateMoves(board);

    Board testBoard;

    u64 nodesThisMove = 0;

    Stopwatch<std::chrono::milliseconds> stopwatch;

    stopwatch.start();

    for (Move m : moves) {
        if (!board.isLegal(m))
            continue;

        testBoard = board;

        testBoard.move(m);
        if (bulk)
            nodesThisMove = ::bulk(testBoard, depth - 1);
        else
            nodesThisMove = perft(testBoard, depth - 1);
        nodes += nodesThisMove;

        cout << m << ": " << nodesThisMove << endl;
    }

    u64 elapsedTime = stopwatch.elapsed();

    cout << "Total nodes: " << formatNum(nodes) << endl;
    cout << "Time spent (ms): " << elapsedTime << endl;
    cout << "Nodes per second: " << formatNum(nodes * 1000 / elapsedTime) << endl;
}

void Movegen::perftSuite(const string filePath) {
    Board board;

    Stopwatch<std::chrono::milliseconds> sw;
    sw.start();

    std::ifstream file(filePath);

    if (!file.is_open()) {
        cerr << "Failed to open file: " << filePath << endl;
        return;
    }

    string ln;

    usize totalTests = 0;
    usize passedTests = 0;
    u64 totalNodes = 0;

    while (std::getline(file, ln)) {
        if (ln.empty())
            continue;

        Stopwatch<std::chrono::milliseconds> sw;
        sw.start();

        std::vector<string> tokens = split(ln, ';');

        u64 nodes;

        board.loadFromFEN(tokens[0]);

        bool passed = true;
        cout << "Testing position: " << tokens[0] << endl;

        for (usize i = 1; i < tokens.size(); i++) {
            std::vector<string> entry = split(tokens[i], ' ');

            usize depth = stoi(entry[0].substr(1));
            u64 target = stoi(entry[1]);

            nodes = bulk(board, depth);

            totalNodes += nodes;

            bool pass = nodes == target;
            cout << "Depth " << depth << ": Expected " << target << ", Got " << nodes << " -> " << (pass ? "PASS" : "FAIL") << endl;

            totalTests++;
            if (pass)
                passedTests++;
            else
                passed = false;
        }

        u64 elapsed = sw.elapsed();
        usize nps = nodes * 1000 / elapsed;

        cout << "Time elapsed: " << formatTime(elapsed) << endl;
        cout << "Found " << formatNum(nodes) << " nodes at " << formatNum(nps) << " nodes per second" << endl;

        if (passed)
            cout << "All tests passed for this position." << endl;
        else
            cout << "Some tests failed for this position." << endl;

        cout << "----------------------------------------" << endl << endl;
    }

    cout << "Perft Suite Completed: " << passedTests << " / " << totalTests << " tests passed." << endl;
    u64 elapsed = sw.elapsed();
    usize nps = totalNodes * 1000 / elapsed;

    cout << "Time elapsed: " << formatTime(elapsed) << endl;
    cout << "Found a total of " << formatNum(totalNodes) << " nodes at " << formatNum(nps) << " nodes per second" << endl;
}