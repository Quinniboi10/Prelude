template<MovegenMode mode>
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

    if constexpr (mode == NOISY_ONLY) {
        singlePushes &= board.pieces(~board.stm);
        doublePushes &= board.pieces(~board.stm);
        captureEast &= board.pieces(~board.stm);
        captureWest &= board.pieces(~board.stm);
    }

    auto addPromos = [&](Square from, Square to) {
        assert(from >= 0);
        assert(from < 64);

        assert(to >= 0);
        assert(to < 64);

        moves.add(Move(from, to, QUEEN));
        if constexpr (mode != NOISY_ONLY) {
            moves.add(Move(from, to, ROOK));
            moves.add(Move(from, to, BISHOP));
            moves.add(Move(from, to, KNIGHT));
        }
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

        moves.add(from, to);
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

template<MovegenMode mode>
void Movegen::knightMoves(const Board& board, MoveList& moves) {
    u64 knightBB = board.pieces(board.stm, KNIGHT);

    u64 friendly = board.pieces(board.stm);

    while (knightBB > 0) {
        Square currentSquare = popLSB(knightBB);

        u64 knightMoves = KNIGHT_ATTACKS[currentSquare];
        knightMoves &= ~friendly;
        if constexpr (mode == NOISY_ONLY)
            knightMoves &= board.pieces(~board.stm);

        while (knightMoves > 0) {
            Square to = popLSB(knightMoves);
            moves.add(currentSquare, to);
        }
    }
}

template<MovegenMode mode>
void Movegen::bishopMoves(const Board& board, MoveList& moves) {
    u64 bishopBB = board.pieces(board.stm, BISHOP, QUEEN);

    u64 occ = board.pieces();
    u64 friendly = board.pieces(board.stm);

    while (bishopBB > 0) {
        Square currentSquare = popLSB(bishopBB);

        u64 bishopMoves = getBishopAttacks(currentSquare, occ);
        bishopMoves &= ~friendly;
        if constexpr (mode == NOISY_ONLY)
            bishopMoves &= board.pieces(~board.stm);

        while (bishopMoves > 0) {
            Square to = popLSB(bishopMoves);
            moves.add(currentSquare, to);
        }
    }
}

template<MovegenMode mode>
void Movegen::rookMoves(const Board& board, MoveList& moves) {
    u64 rookBB = board.pieces(board.stm, ROOK, QUEEN);

    u64 occ = board.pieces();
    u64 friendly = board.pieces(board.stm);

    while (rookBB > 0) {
        Square currentSquare = popLSB(rookBB);

        u64 rookMoves = getRookAttacks(currentSquare, occ);
        rookMoves &= ~friendly;
        if constexpr (mode == NOISY_ONLY)
            rookMoves &= board.pieces(~board.stm);

        while (rookMoves > 0) {
            Square to = popLSB(rookMoves);
            moves.add(currentSquare, to);
        }
    }
}

template<MovegenMode mode>
void Movegen::kingMoves(const Board& board, MoveList& moves) {
    Square kingSq = Square(ctzll(board.pieces(board.stm, KING)));

    assert(kingSq >= a1);
    assert(kingSq < NO_SQUARE);

    u64 kingMoves = KING_ATTACKS[kingSq];
    kingMoves &= ~board.pieces(board.stm);
    if constexpr (mode == NOISY_ONLY)
        kingMoves &= board.pieces(~board.stm);

    while (kingMoves > 0) {
        Square to = popLSB(kingMoves);
        moves.add(kingSq, to);
    }
    
    if (board.canCastle(board.stm, true)) moves.add(kingSq, castleSq(board.stm, true), CASTLE);
    if (board.canCastle(board.stm, false)) moves.add(kingSq, castleSq(board.stm, false), CASTLE);
}

template<MovegenMode mode>
MoveList Movegen::generateMoves(const Board& board) {
    MoveList moves;
    kingMoves<mode>(board, moves);
    if (board.doubleCheck)
        return moves;

    pawnMoves<mode>(board, moves);
    knightMoves<mode>(board, moves);
    bishopMoves<mode>(board, moves);
    rookMoves<mode>(board, moves);
    // Note: Queen moves are done at the same time as bishop/rook moves

    return moves;
}