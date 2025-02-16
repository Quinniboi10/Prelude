#include <bitset>

#include "board.h"
#include "move.h"
#include "types.h"
#include "movegen.h"

// ****** MAIN ENTRY POINT, HANDLES UCI ******
int main(int argc, char* argv[]) {
    Board board;

    string              command;
    std::vector<string> tokens;

    Movegen::initializeAllDatabases();
    Board::fillZobristTable();

    board.reset();

    cout << "Prelude ready and awaiting commands" << endl;
    while (true) {
        std::getline(std::cin, command);
        if (command == "")
            continue;
        tokens = split(command, ' ');

        if (command == "uci") {
            cout << "id name Prelude" << endl;
            cout << "id author Quinniboi10" << endl;
            cout << "uciok" << endl;
        }
        else if (command == "isready")
            cout << "readyok" << endl;
        else if (tokens[0] == "position") {
            if (tokens[1] == "startpos")
                board.reset();
            else if (tokens[1] == "kiwipete")
                board.loadFromFEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
            else if (tokens[1] == "fen")
                board.loadFromFEN(command.substr(13));
        }
        else if (command == "quit")
            return 0;


        // ************ NON-UCI ************


        else if (command == "d")
            board.display();
        else if (tokens[0] == "perft") {
            if (tokens.size() < 2) {
                cout << "Usage: perft <depth>" << endl;
                continue;
            }
            Movegen::perft(board, stoi(tokens[1]), false);
        }
        else if (tokens[0] == "bulk") {
            if (tokens.size() < 2) {
                cout << "Usage: bulk <depth>" << endl;
                continue;
            }
            Movegen::perft(board, stoi(tokens[1]), true);
        }
        else if (tokens[0] == "move") {
            board.move(Move(tokens[1], board));
        }
        else if (command == "debug.moves") {
            MoveList moves = Movegen::generateMoves(board);
            for (Move m : moves) {
                cout << m;
                if (board.isLegal(m))
                    cout << " <- legal" << endl;
                else
                    cout << " <- illegal" << endl;
            }
        }
        else if (command == "debug.gamestate") {
            Square whiteKing = Square(ctzll(board.pieces(WHITE, KING)));
            Square blackKing = Square(ctzll(board.pieces(BLACK, KING)));
            cout << "Is in check (white): " << board.isUnderAttack(WHITE, whiteKing) << endl;
            cout << "Is in check (black): " << board.isUnderAttack(BLACK, blackKing) << endl;
            cout << "En passant square: " << (board.epSquare != NO_SQUARE ? squareToAlgebraic(board.epSquare) : "-") << endl;
            cout << "Castling rights: " << std::bitset<4>(board.castlingRights) << endl;
        }
        else if (command == "debug.incheck")
            cout << "Stm is " << (board.inCheck() ? "in check" : "NOT in check") << endl;
        else if (tokens[0] == "debug.islegal")
            cout << tokens[1] << " is " << (board.isLegal(Move(tokens[1], board)) ? "" : "not ") << "legal" << endl;
        else if (tokens[0] == "perftsuite")
            Movegen::perftSuite(tokens[1]);
        else if (command == "debug.popcnt") {
            cout << "White pawns: " << popcount(board.pieces(WHITE, PAWN)) << endl;
            cout << "White knigts: " << popcount(board.pieces(WHITE, KNIGHT)) << endl;
            cout << "White bishops: " << popcount(board.pieces(WHITE, BISHOP)) << endl;
            cout << "White rooks: " << popcount(board.pieces(WHITE, ROOK)) << endl;
            cout << "White queens: " << popcount(board.pieces(WHITE, QUEEN)) << endl;
            cout << "White king: " << popcount(board.pieces(WHITE, KING)) << endl;
            cout << endl;
            cout << "Black pawns: " << popcount(board.pieces(BLACK, PAWN)) << endl;
            cout << "Black knigts: " << popcount(board.pieces(BLACK, KNIGHT)) << endl;
            cout << "Black bishops: " << popcount(board.pieces(BLACK, BISHOP)) << endl;
            cout << "Black rooks: " << popcount(board.pieces(BLACK, ROOK)) << endl;
            cout << "Black queens: " << popcount(board.pieces(BLACK, QUEEN)) << endl;
            cout << "Black king: " << popcount(board.pieces(BLACK, KING)) << endl;
        }
        else {
            cerr << "Unknown command: " << command << endl;
        }
    }
    return 0;
}