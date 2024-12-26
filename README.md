# Prelude Chess Engine

> UCI chess engine with NNUE evaluation

Prelude is a UCI-compatible chess engine utilizing advanced search algorithms and a neural network (NNUE) for evaluation. Designed for high performance and configurability, Prelude is ideal for chess enthusiasts and developers.

## Feature List:

### CLI Commands:

1. Functional UCI implementation with custom commands:
   - **`debug.gamestate`**: Displays the current board state and game metadata.
   - **`perft <depth>`**: Performs a perft test from the current position.
   - **`perftsuite <suite>`**: Executes a suite of perft tests.
   - **`bench <depth>`**: Benchmarks engine performance on test positions.
   - **`position kiwipete`**: Loads the famous "Kiwipete" position.
   - **`move <move>`**: Applies a move in long algebraic notation.
   - **`debug.moves`**: Lists all legal moves for the current position.
   - **`debug.eval`**: Outputs the evaluation of the current position.
   - **`debug.popcnt`**: Displays piece counts for both sides.

### Board Representation:

1. Efficient bitboard implementation.
2. Zobrist hashing for fast position recognition.
3. Incremental NNUE updates.
4. Repetition detection.

### Search Algorithms:

1. Fail-soft Principal Variation Search (PVS) with alpha-beta pruning.
2. Transposition Table (TT) with configurable size.
3. Null move pruning, reverse futility pruning, with more to come.
4. Iterative deepening.
5. Quiescence search for tactical clarity.

### Evaluation:

1. NNUE evaluation for accurate and deep positional analysis
2. Currently uses a 1024HL perspective net trained by Ciekce

### Move Ordering:

1. TT best move prioritization.
2. Most valuable victim - least valuable attacker prioritization for captures.

## Installation

### Prerequisites

- **C++ Compiler**: Requires support for C++17 or later.
- **Neural Network File**: Provide an NNUE and update the code to load it.

### Compilation

1. Clone the repository:

   ```bash
   git clone https://github.com/Quinniboi10/Prelude.git
   cd Prelude
   ```

2. Compile using a C++ compiler:

   ```bash
   g++ -std=c++17 -O3 -pthread main.cpp -o Prelude
   ```

3. Run the engine:

   ```bash
   ./Prelude
   ```

## Usage

Prelude uses the UCI protocol but supports custom debugging and testing commands. Compatible with GUIs like Cute Chess, En Croissant, or any UCI-compatible interface.

### Custom Commands:

- **`debug.gamestate`**: Inspect board state and legal moves.
- **`perft <depth>`**: Test move generation accuracy to a specified depth.
- **`bench <depth>`**: Evaluate engine speed across predefined positions.
- **`move <move>`**: Apply a move to the current board state.
- **`debug.eval`**: Analyze position evaluation score.

## Configuration

Prelude supports customizable options via the `setoption` command:

- **`Hash`**: Configurable hash table size (1 to 4096 MB). Default: 16 MB.
- **`Move Overhead`**: Adjusts time overhead per move (0 to 1000 ms). Default: 20 ms.

## Neural Network

Ensure a NNUE file is correctly placed. Update its path in the code if necessary:

```cpp
nn.loadNet("path/to/your.nnue");
```

## Special Thanks

- **Vast**: Help hunting for bugs and explaining concepts
- **Ciekce**: Guidance and test NNUEs
- **Shawn\_xu**: Explaining NNUEs and many other things
- **A\_randomnoob**: Fixing obvious mistakes
- **Matt**: Providing help and allowing me to use his git instance

## License

Prelude is released under the GNU 3 License. See `LICENSE` for details.

---
