#include <algorithm>
#include <bitset>
#include <atomic>
#include <thread>
#include <string>

#include "board.h"
#include "move.h"
#include "nnue.h"
#include "types.h"
#include "movegen.h"
#include "search.h"
#include "ttable.h"
#include "datagen.h"
#include "searcher.h"

#ifndef EVALFILE
    #define EVALFILE "./nnue.bin"
#endif

#ifdef _MSC_VER
    #define MSVC
    #pragma push_macro("_MSC_VER")
    #undef _MSC_VER
#endif

#include "../external/incbin.h"

#ifdef MSVC
    #pragma pop_macro("_MSC_VER")
    #undef MSVC
#endif

#if !defined(_MSC_VER) || defined(__clang__)
INCBIN(EVAL, EVALFILE);
#endif


usize MOVE_OVERHEAD = 20;
NNUE  nnue;
bool  chess960 = false;

// ****** MAIN ENTRY POINT, HANDLES UCI ******
int main(int argc, char* argv[]) {
    auto loadDefaultNet = [&]([[maybe_unused]] bool warnMSVC = false) {
#if defined(_MSC_VER) && !defined(__clang__)
        nnue.loadNetwork(EVALFILE);
        if (warnMSVC)
            cerr << "WARNING: This file was compiled with MSVC, this means that an nnue was NOT embedded into the exe." << endl;
#else
        nnue = *reinterpret_cast<const NNUE*>(gEVALData);
#endif
    };

    loadDefaultNet(true);

    Board board;

    string              command;
    std::vector<string> tokens;

    Movegen::initializeAllDatabases();
    Board::fillZobristTable();
    Search::fillLmrTable();

    board.reset();

    Searcher searcher;

    // *********** ./Prelude <ARGS> ************
    if (argc > 1) {
        string arg1 = argv[1];
        if (arg1 == "bench")
            Search::bench();
        else if (arg1 == "datagen") {
            usize threads = 1;
            if (argc >= 2)
                threads = std::stoi(argv[2]);
            Datagen::run(threads);
        }
        return 0;
    }

    auto exists            = [&](const string& sub) { return command.find(" " + sub + " ") != string::npos; };
    auto index             = [&](const string& sub, int offset = 0) { return findIndexOf(tokens, sub) + offset; };
    auto getValueFollowing = [&](const string& value, int defaultValue) { return exists(value) ? std::stoi(tokens[index(value, 1)]) : defaultValue; };

    // ************ UCI ************

    cout << "Prelude ready and awaiting commands" << endl;
    while (true) {
        std::getline(std::cin, command);
        Stopwatch<std::chrono::milliseconds> commandTime;
        if (command == "")
            continue;
        tokens = split(command, ' ');

        if (command == "uci") {
            cout << "id name Prelude"
                #ifdef GIT_HEAD_COMMIT_ID
                 << " (" << GIT_HEAD_COMMIT_ID << ")"
                #endif
                << endl;
            cout << "id author Quinniboi10" << endl;
            cout << "option name Threads type spin default 1 min 1 max 512" << endl;
            cout << "option name Hash type spin default 16 min 1 max 524288" << endl;
            cout << "option name Move Overhead type spin default 20 min 0 max 1000" << endl;
            cout << "option name EvalFile type string default internal" << endl;
            cout << "option name UCI_Chess960 type check default false" << endl;
            cout << "uciok" << endl;
        }
        else if (command == "ucinewgame")
            searcher.reset();
        else if (command == "isready")
            cout << "readyok" << endl;
        else if (tokens[0] == "position") {
            if (tokens[1] == "startpos") {
                board.reset();
                if (tokens.size() > 2 && tokens[2] == "moves")
                    for (usize i = 3; i < tokens.size(); i++)
                        board.move(tokens[i]);
            }
            else if (tokens[1] == "kiwipete")
                board.loadFromFEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
            else if (tokens[1] == "fen") {
                board.loadFromFEN(command.substr(13));
                if (tokens.size() > 8 && tokens[8] == "moves")
                    for (usize i = 9; i < tokens.size(); i++)
                        board.move(tokens[i]);
            }
        }
        else if (tokens[0] == "go") {
            searcher.stop();

            usize depth = getValueFollowing("depth", MAX_PLY);

            usize maxNodes  = getValueFollowing("nodes", 0);
            usize softNodes = getValueFollowing("softnodes", 0);

            usize mtime = getValueFollowing("movetime", 0);
            usize wtime = getValueFollowing("wtime", 0);
            usize btime = getValueFollowing("btime", 0);

            usize winc = getValueFollowing("winc", 0);
            usize binc = getValueFollowing("binc", 0);

            usize mate = getValueFollowing("mate", 0);

            searcher.start(board, Search::SearchParams(commandTime, depth, maxNodes, softNodes, mtime, wtime, btime, winc, binc, mate));
        }
        else if (tokens[0] == "setoption") {
            if (tokens[2] == "Hash") {
                searcher.resizeTT(std::stoi(tokens[findIndexOf(tokens, "value") + 1]));
            }
            else if (tokens[2] == "EvalFile") {
                string value = tokens[findIndexOf(tokens, "value") + 1];
                if (value == "internal")
                    loadDefaultNet();
                else
                    nnue.loadNetwork(value);
            }
            else if (tokens[2] == "Move" && tokens[3] == "Overhead")
                MOVE_OVERHEAD = std::stoi(tokens[findIndexOf(tokens, "value") + 1]);
            else if (tokens[2] == "Threads")
                searcher.makeThreads(std::stoi(tokens[findIndexOf(tokens, "value") + 1]));
            else if (tokens[2] == "UCI_Chess960")
                chess960 = tokens[findIndexOf(tokens, "value") + 1] == "true";
        }
        else if (command == "stop")
            searcher.stop();
        else if (command == "quit") {
            searcher.stop();
            return 0;
        }


        // ************ NON-UCI ************


        else if (command == "d")
            board.display();
        else if (tokens[0] == "perft") {
            if (tokens.size() < 2) {
                cout << "Usage: perft <depth>" << endl;
                continue;
            }
            Movegen::perft(board, std::stoi(tokens[1]), false);
        }
        else if (tokens[0] == "bulk") {
            if (tokens.size() < 2) {
                cout << "Usage: bulk <depth>" << endl;
                continue;
            }
            Movegen::perft(board, std::stoi(tokens[1]), true);
        }
        else if (tokens[0] == "perftsuite")
            Movegen::perftSuite(tokens[1]);
        else if (tokens[0] == "move") {
            board.move(Move(tokens[1], board));
        }
        else if (command == "bench")
            Search::bench();
        else if (tokens[0] == "datagen")
            Datagen::run(tokens.size() > 1 ? std::stoi(tokens[1]) : 1);
        else if (command == "debug.eval") {
            cout << "Raw eval: " << nnue.forwardPass(&board) << endl;
            nnue.showBuckets(&board);
        }
        else if (command == "debug.moves") {
            MoveList moves = Movegen::generateMoves<ALL_MOVES>(board);
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
            cout << "Half move clock: " << board.halfMoveClock << endl;
            cout << "Castling rights: { ";
            cout << squareToAlgebraic(board.castling[castleIndex(WHITE, true)]) << ", ";
            cout << squareToAlgebraic(board.castling[castleIndex(WHITE, false)]) << ", ";
            cout << squareToAlgebraic(board.castling[castleIndex(BLACK, true)]) << ", ";
            cout << squareToAlgebraic(board.castling[castleIndex(BLACK, false)]);
            cout << " }" << endl;
        }
        else if (command == "debug.incheck")
            cout << "Stm is " << (board.inCheck() ? "in check" : "NOT in check") << endl;
        else if (tokens[0] == "debug.islegal")
            cout << tokens[1] << " is " << (board.isLegal(Move(tokens[1], board)) ? "" : "not ") << "legal" << endl;
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