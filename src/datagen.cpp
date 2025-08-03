#include "datagen.h"
#include "board.h"
#include "movegen.h"
#include "search.h"
#include "constants.h"
#include "thread.h"

#include "../external/fmt/fmt/format.h"

#include <atomic>
#include <vector>
#include <random>
#include <thread>
#include <cstdlib>
#include <fstream>
#include <filesystem>

struct ScoredMove {
    u16 move;
    i16 score;  // As white

    ScoredMove(Move m, i16 s) {
        move  = *reinterpret_cast<u16*>(&m);
        score = s;
    }
};

struct MarlinFormat {
    u64         occupancy;
    U4Array<32> pieces;
    u8          epSquare;  // Also stores side to move
    u8          halfmoveClock;
    u16         fullmoveClock;
    i16         eval;  // As white
    u8          wdl;   // 0 = white loss, 1 = draw, 2 = white win
    u8          extraByte;

    MarlinFormat() = default;

    MarlinFormat(const Board& b) {
        constexpr PieceType UNMOVED_ROOK = PieceType(6);  // Piece type

        const u8 stm = b.stm == BLACK ? (1 << 7) : 0;

        occupancy     = b.pieces();
        epSquare      = stm | (b.epSquare == NO_SQUARE ? 64 : toSquare(b.stm == BLACK ? RANK3 : RANK6, fileOf(b.epSquare)));
        halfmoveClock = b.halfMoveClock;
        fullmoveClock = b.fullMoveClock;
        eval          = 0;
        wdl           = 0;
        extraByte     = 0;

        u64   occ   = occupancy;
        usize index = 0;

        while (occ) {
            Square sq = Square(ctzll(occ));
            int    pt = b.getPiece(sq);

            if (pt == ROOK
                && ((sq == a1 && b.canCastle(WHITE, false))      // White queenside
                    || (sq == h1 && b.canCastle(WHITE, true))    // White kingside
                    || (sq == a8 && b.canCastle(BLACK, false))   // Black queenside
                    || (sq == h8 && b.canCastle(BLACK, true))))  // Black kingside
                pt = UNMOVED_ROOK;

            assert(pt < 7);

            pt |= ((1ULL << sq & b.pieces(BLACK)) ? 1ULL << 3 : 0);

            pieces.set(index++, pt);

            occ &= occ - 1;
        }
    }
};

struct Game {
    MarlinFormat            header;
    std::vector<ScoredMove> moves;

    Game(MarlinFormat h, std::vector<ScoredMove> m) {
        header = h;
        moves  = m;
    }
};

void makeRandomMove(Board& board) {
    MoveList moves = Movegen::generateLegalMoves(board);
    assert(moves.length > 0);

    std::random_device rd;

    std::mt19937_64 engine(rd());

    std::uniform_int_distribution<int> dist(0, moves.length - 1);

    Board testBoard;
    Move  m = moves.moves[dist(engine)];

    board.move(m);
}

string makeFileName() {
    std::random_device rd;

    std::mt19937_64 engine(rd());

    std::uniform_int_distribution<int> dist(0, INF_INT);

    // Get current time
    auto now = std::chrono::system_clock::now();

    // Convert to time_t
    std::time_t t = std::chrono::system_clock::to_time_t(now);

    // Convert to tm structure
    std::tm tm;

#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    string randomStr = std::to_string(dist(engine));

    return "data-" + fmt::format("{:04}-{:02}-{:02}", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday) + "-" + randomStr + ".preludedata";
}

void writeToFile(std::ofstream& outFile, const std::vector<Game>& data) {
    i32 nullBytes = 0;
    for (const Game& game : data) {
        outFile.write(reinterpret_cast<const char*>(&game.header), sizeof(game.header));                          // Header
        outFile.write(reinterpret_cast<const char*>(game.moves.data()), sizeof(ScoredMove) * game.moves.size());  // Moves
        outFile.write(reinterpret_cast<const char*>(&nullBytes), 4);                                              // Null move and eval to show end of game
    }
    outFile.flush();
}


void runThread(int id) {
    string filePath = "./data/" + makeFileName();

    if (!std::filesystem::is_directory("./data/"))
        std::filesystem::create_directory("./data/");

    std::ofstream outFile(filePath, std::ios::app | std::ios::binary);

    if (!outFile.is_open()) {
        cerr << "Error: Could not open the file " << filePath << " for writing." << endl;
        exit(-1);
    }

    i64 cachedPositions = 0;

    Board                   board;
    std::vector<ScoredMove> gameBuffer;
    MarlinFormat            startingPos;

    std::vector<Game>                    outputBuffer;
    TranspositionTable                   TT;
    std::atomic<bool>                    exitFlag(false);
    std::unique_ptr<Search::ThreadInfo>  thisThread = std::make_unique<Search::ThreadInfo>(Search::ThreadType::SECONDARY, TT, exitFlag);
    Stopwatch<std::chrono::milliseconds> time;
    Stopwatch<std::chrono::milliseconds> totalTime;
    Search::SearchParams                 sp(time, MAX_PLY, Datagen::HARD_NODES, Datagen::SOFT_NODES, 0, 0, 0, 0, 0, 0);

    gameBuffer.reserve(256);
    outputBuffer.reserve(Datagen::OUTPUT_BUFFER_GAMES);

    std::random_device                 rd;
    std::mt19937_64                    engine(rd());
    std::uniform_int_distribution<int> dist(0, 1);
    auto                               randBool = [&]() { return dist(engine); };

mainLoop:
    while (true) {
        thisThread->reset();
        board.reset();
        gameBuffer.clear();
        TT.clear();

        usize randomMoves = Datagen::RAND_MOVES + randBool();

        for (usize i = 0; i < randomMoves; i++) {
            makeRandomMove(board);
            if (board.isGameOver())
                goto mainLoop;
        }

        startingPos = MarlinFormat(board);

        while (!board.isGameOver()) {
            MoveEvaluation move = Search::iterativeDeepening(board, *thisThread.get(), sp);
            assert(!move.move.isNull());
            gameBuffer.emplace_back(move.move, board.stm == WHITE ? move.eval : -move.eval);

            board.move(move.move);
            cachedPositions++;
        }

        if (board.isDraw() || !board.inCheck())
            startingPos.wdl = 1;
        else if (board.stm == WHITE)
            startingPos.wdl = 0;
        else
            startingPos.wdl = 2;

        outputBuffer.emplace_back(startingPos, gameBuffer);

        gameBuffer.clear();

        if (outputBuffer.size() >= Datagen::OUTPUT_BUFFER_GAMES) {
            writeToFile(outFile, outputBuffer);
            outputBuffer.clear();
            cout << "Thread " << id << " wrote " << formatNum(cachedPositions) << " positions at " << fmt::format("{:.1f}", cachedPositions * 1000 / (double) time.elapsed()) << " pos/s" << endl;
            cout << "Elapsed time: " << formatTime(totalTime.elapsed()) << endl;
            cachedPositions = 0;
            time.reset();
        }
    }
}

void Datagen::run(usize threadCount) {
    std::vector<std::thread> threads;

    for (usize i = 0; i < threadCount - 1; i++)
        threads.emplace_back(runThread, i);

    runThread(threadCount - 1);
}

void Datagen::genFens(u64 numFens, u64 seed) {
    std::mt19937                       boolEng(seed);
    std::uniform_int_distribution<int> dist(0, 1);
    auto                               randBool = [&]() { return dist(boolEng); };

    std::mt19937 moveEng(seed);

    u64 fens = 0;
    while (fens < numFens) {
        Board board;
        board.reset();
        usize randomMoves = Datagen::RAND_MOVES + randBool();
        for (usize i = 0; i < randomMoves; i++) {
            if (board.isGameOver())
                break;
            MoveList moves = Movegen::generateLegalMoves(board);
            std::uniform_int_distribution<int> dist(0, moves.length - 1);
            board.move(moves.moves[dist(moveEng)]);
        }
        if (board.isGameOver())
            continue;

        cout << "info string genfens " << board.fen() << endl;
        fens++;
    }
}