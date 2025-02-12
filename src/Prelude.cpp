#include "board.h"
#include "move.h"
#include "types.h"
#include "movegen.h"

// ****** MAIN ENTRY POINT ******
int main(int argc, char* argv[]) {
    Board board;

    string              command;
    std::vector<string> tokens;

    Movegen::initializeAllDatabases();

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
                if (Movegen::isLegal(board, m))
                    cout << " <- legal" << endl;
                else
                    cout << " <- illegal" << endl;
            }
        }
        else if (command == "debug.incheck")
            cout << "Stm is " << (Movegen::inCheck(board) ? "in check" : "NOT in check") << endl;
        else if (tokens[0] == "debug.islegal")
            cout << tokens[1] << " is " << (Movegen::isLegal(board, Move(tokens[1], board)) ? "" : "not ") << "legal" << endl;
    }
    return 0;
}