#include "tb.h"
#include "globals.h"

#include "../external/Pyrrhic/tbprobe.h"

i32 tb::PIECES;

void tb::setPath(const string& path) {
    tbEnabled = path != "<empty>";
    tb_init(path.data());
    cout << "info string found " << TB_NUM_WDL << " wdl entries" << endl;
    cout << "info string found " << TB_NUM_DTZ << " dtz entries" << endl;
    PIECES = std::min(TB_LARGEST, syzygyProbeLimit);
}

void tb::free() { tb_free(); }

TableProbe tb::probePos(const Board& board) {
    // EP square should be 0 if there is no possible EP
    const Square epSquare = static_cast<Square>(board.epSquare * (board.epSquare != NO_SQUARE));

    const int result = tb_probe_wdl(board.pieces(WHITE), board.pieces(BLACK), board.pieces(KING), board.pieces(QUEEN), board.pieces(ROOK), board.pieces(BISHOP), board.pieces(KNIGHT),
                                    board.pieces(PAWN), epSquare, board.stm == WHITE);
    switch (result) {
    case TB_WIN:
        return TableProbe::WIN;
    case TB_DRAW:
    case TB_CURSED_WIN:
    case TB_BLESSED_LOSS:
        return TableProbe::DRAW;
    case TB_LOSS:
        return TableProbe::LOSS;
    default:
        return TableProbe::FAILED;
    }
}

TableProbe tb::probeRoot(MoveList& rootMoves, const Board& board) {
    rootMoves.length = 0;

    const auto fromTB = [&](PyrrhicMove TBMove) {
        PieceType promo;
        if (PYRRHIC_MOVE_IS_QPROMO(TBMove))
            promo = QUEEN;
        else if (PYRRHIC_MOVE_IS_RPROMO(TBMove))
            promo = ROOK;
        else if (PYRRHIC_MOVE_IS_BPROMO(TBMove))
            promo = BISHOP;
        else if (PYRRHIC_MOVE_IS_NPROMO(TBMove))
            promo = KNIGHT;
        else
            promo = NO_PIECE_TYPE;

        const Square    from  = static_cast<Square>(PYRRHIC_MOVE_FROM(TBMove));
        const Square    to    = static_cast<Square>(PYRRHIC_MOVE_TO(TBMove));

        if (promo != NO_PIECE_TYPE)
            return Move(from, to, promo);
        else if (PYRRHIC_MOVE_IS_ENPASS(TBMove))
            return Move(from, to, EN_PASSANT);
        else
            return Move(from, to);
    };

    TbRootMoves tbRootMoves{};

    // EP square should be 0 if there is no possible EP
    const Square epSquare = static_cast<Square>(board.epSquare * (board.epSquare != NO_SQUARE));

    const bool isRepeated = std::count(board.posHistory.begin(), board.posHistory.end(), board.zobrist) == 2;

    int result = tb_probe_root_dtz(board.pieces(WHITE), board.pieces(BLACK), board.pieces(KING), board.pieces(QUEEN), board.pieces(ROOK), board.pieces(BISHOP), board.pieces(KNIGHT),
                                   board.pieces(PAWN), board.halfMoveClock, epSquare, board.stm == WHITE, isRepeated, &tbRootMoves);

    if (result == 0) {
        cout << "info string DTZ probe failed, attempting WDL" << endl;

        result = tb_probe_root_wdl(board.pieces(WHITE), board.pieces(BLACK), board.pieces(KING), board.pieces(QUEEN), board.pieces(ROOK), board.pieces(BISHOP), board.pieces(KNIGHT),
                                   board.pieces(PAWN), board.halfMoveClock, epSquare, board.stm == WHITE, false, &tbRootMoves);
    }

    // Terminal state at root or failed probe
    if (result == 0 || tbRootMoves.size == 0)
        return TableProbe::FAILED;

    // Sort moves by given rank
    std::sort(&tbRootMoves.moves[0], &tbRootMoves.moves[tbRootMoves.size], [](auto a, auto b) { return a.tbRank > b.tbRank; });

    TableProbe wdl;
    i32        minRank;

    const TbRootMove& best = tbRootMoves.moves[0];

    static constexpr i32 TB_MAX_DTZ = 262144;
    static constexpr i32 WIN_BOUND  = TB_MAX_DTZ - 100;
    static constexpr i32 DRAW_BOUND = -TB_MAX_DTZ + 101;

    if (best.tbRank >= WIN_BOUND) {
        wdl     = TableProbe::WIN;
        minRank = WIN_BOUND;
    }
    else if (best.tbRank >= DRAW_BOUND) {
        wdl     = TableProbe::DRAW;
        minRank = DRAW_BOUND;
    }
    else {
        wdl     = TableProbe::LOSS;
        minRank = -TB_MAX_DTZ;
    }

    for (usize i = 0; i < tbRootMoves.size; i++) {
        TbRootMove& m = tbRootMoves.moves[i];
        if (m.tbRank < minRank)
            break;
        const Move move = fromTB(m.move);
        if (!move.isNull())
            rootMoves.add(move);
    }

    return wdl;
}