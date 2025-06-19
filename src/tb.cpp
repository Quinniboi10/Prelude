#include "tb.h"
#include "globals.h"

#include "../external/Pyrrhic/tbprobe.h"


void tb::setPath(const string& path) {
	tbEnabled = path != "<empty>";
    tb_init(path.data());
    cout << "info string found " << TB_NUM_WDL << " wdl entries" << endl;
}

void tb::free() { tb_free(); }

TableProbe probePos(const Board& board) {
	// EP square should be 0 if there is no possible EP
    const Square epSquare = static_cast<Square>(board.epSquare * (board.epSquare != NO_SQUARE));

	const int result = tb_probe_wdl(board.pieces(WHITE),
									board.pieces(BLACK),
									board.pieces(KING),
									board.pieces(QUEEN),
									board.pieces(ROOK),
									board.pieces(BISHOP),
									board.pieces(KNIGHT),
									board.pieces(PAWN),
									epSquare,
									board.stm == WHITE);
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