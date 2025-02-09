#include "board.h"
#include "move.h"

// ****** MAIN ENTRY POINT ******
int main(int argc, char* argv[]) {
    Board board;
    board.reset();
    board.display();
    board.move(Move(e2, e4, STANDARD_MOVE));
    board.display();
    return 0;
}