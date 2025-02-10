#include "board.h"
#include "move.h"
#include "types.h"

// ****** MAIN ENTRY POINT ******
int main(int argc, char* argv[]) {
    Board board;

    string command;
    std::vector<string> parsedCommand;

    board.reset();
    cout << "Prelude ready and awaiting commands" << endl;
    while (true) {
        std::getline(std::cin, command);
        if (command == "")
            continue;
        parsedCommand = split(command, ' ');

        if (command == "uci")
            cout << "id name Prelude" << endl;
            cout << "id author Quinniboi10" << endl;
            cout << "uciok" << endl;
        else if (command == "isready")
            cout << "readyok"
        else if (parsedCommand[0] == "position") {
            if (parsedCommand[1] == "startpos")
                board.reset();
            else if (parsedCommand[1] == "fen")
                board.loadFromFEN(command.substr(13));
        }
        // ****** NON-UCI ******
        else if (command == "d") {
            board.display();
        }
    }
    return 0;
}