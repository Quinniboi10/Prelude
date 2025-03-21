# Prelude Chess Engine

> UCI chess engine with NNUE evaluation

Prelude is a UCI-compatible chess engine utilizing advanced search\* algorithms and a neural network (NNUE) for evaluation. Designed for high performance\*\* and configurability, Prelude is *not* ideal for chess enthusiasts and developers.

*depends on a person's definition of advanced  
**performance is unlikely

## Feature List:

### CLI Commands:

1. Functional UCI implementation with custom commands:
   - **`d`**: Display the current board position.
   - **`move <move>`**: Applies a move in long algebraic notation.
   - **`bulk <depth>`**: Starts a perft test from the current position using bulk counting.
   - **`perft <depth>`**: Performs a perft test from the current position.
   - **`perftsuite <suite>`**: Executes a suite of perft tests*.
   - **`bench <depth>`**: Benchmarks engine performance on test positions.
   - **`position kiwipete`**: Loads the "Kiwipete" position, commonly used for debugging.
   - **`debug.gamestate`**: Displays the current board state and game metadata.
   - **`debug.moves`**: Lists **all moves**\*\* for the current position.
   - **`debug.eval`**: Outputs the evaluation of the current position.
   - **`debug.popcnt`**: Displays piece counts for both sides.  
*Uses bulk counting  
\*\*All pseudolegal moves  

### Board Representation:

1. Bitboard implementation.
2. Incremental zobrist hashing for fast position recognition.
3. Efficient NNUE updates.

### Search Algorithms:

1. Fail-soft Principal Variation Search (PVS) with alpha-beta pruning.
2. Transposition Table (TT) used for move ordering and cutoffs.
3. Null move pruning, reverse futility pruning, with more to come.
4. Iterative deepening with aspiration windows.
5. Quiescence search.

### Evaluation:

1. NNUE evaluation trained with [bullet](https://github.com/jw1912/bullet)
2. Uses QA of 255, QB of 64, and eval scale of 400
3. SCReLU activation function
4. (768->256)x2->1x8

### Move Ordering:

Moves will always be ordered  
TT move, all captures, all quiets

1. TT best move prioritization.
2. Most valuable victim - least valuable attacker (MVVLVA) prioritization for captures.
3. Quiet move ordering using history tables

## Installation

### Prerequisites

- **Make**: Uses make to build the executable.
- **Clang++ Compiler**: Requires support for C++20 or later.
- **CPU Architecture**: Requires support for AVX2 or later.
- **Neural Network File**: Provide an NNUE and update the code to load it (included nnue.bin is recommended).

### Compilation

1. Clone the repository:

   ```bash
   git clone https://github.com/Quinniboi10/Prelude.git
   cd Prelude
   ```

2. Compile using make:

   ```bash
   make
   ```

3. Run the engine:

   ```bash
   ./Prelude
   ```

## Usage

Prelude uses the UCI protocol but supports custom debugging and testing commands. Compatible with GUIs like Cute Chess, En Croissant, or any UCI-compatible interface. See above commands.

## Configuration

Prelude supports customizable options via the `setoption` command:

- **`Threads`**: Number of threads to use (1 to 1024). Default: 1.
- **`Hash`**: Configurable hash table size (1 to 4096 MB). Default: 16 MB.
- **`Move Overhead`**: Adjusts time overhead per move (0 to 1000 ms). Default: 20 ms.

## Neural Network

Ensure a NNUE file is correctly placed. Update its path in the code if necessary:

```cpp
#define EVALFILE "./pathToYourNNUE.bin"
```

## Special Thanks

- **Vast**: Help hunting for bugs and explaining concepts
- **Ciekce**: Lots of guidance and test NNUEs
- **Shawn\_xu**: Explaining NNUEs and many other things
- **A\_randomnoob**: Fixing obvious mistakes
- **Matt**: Providing help and allowing me to use his git instance
- **jw**: Helping me with NNUE training