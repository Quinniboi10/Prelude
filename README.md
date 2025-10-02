# Prelude Chess Engine

> A superhuman UCI chess engine with NNUE evaluation

Prelude is a UCI-compatible chess engine utilizing advanced search\* algorithms and a neural network (NNUE) for evaluation. Designed for high performance\*\* and configurability, Prelude is *not* ideal for chess enthusiasts and developers.

*depends on a person's definition of advanced  
**performance is unlikely

## Feature List:

### CLI Commands:

1. Functional UCI implementation with custom commands:
   - **`d`**: Display the current board position.
   - **`move <move>`**: Applies a move in long algebraic notation.
   - **`bulk <depth>`**: Starts a perft test from the current position using semi-bulk counting.
   - **`perft <depth>`**: Performs a perft test from the current position.
   - **`perftsuite <suite>`**: Executes a suite of perft tests with multithreading.
   - **`bench <depth>`**: Benchmarks engine performance on test positions.
   - **`datagen <threads>`**: Starts datagen with the given number of threads.
   - **`position kiwipete`**: Loads the "Kiwipete" position, commonly used for debugging.
   - Some other commands are supported, but are mostly for debugging. See Prelude.cpp for the full list.

### Evaluation:

1. NNUE evaluation trained with [bullet](https://github.com/jw1912/bullet)
2. Uses QA of 255, QB of 64, and eval scale of 400
3. SCReLU activation function
4. (768->1024)x2->1x8

More information can be found in `config.h`

## Build From Source

### Prerequisites

- **Make**: Uses make to build the executable.
- **Clang++ Compiler**: Requires support for C++20 or later.
- **CPU Architecture**: AVX2 or later recommended.
- **Neural Network File**: Provide an NNUE and update the code to load it. If no network is given, it will download the latest Prelude network.

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
- **`EvalFile`**: Path to the NNUE file. Default: internal.
- **`SyzygyPath`**: Path to the Syzygy tablebases. Default: <empty>
- **`SyzygyProbeDepth`**: Minimal internal search depth to probe the TBs. Default: 1
- **`SyzygyProbeLimit`**: Max number of pieces to probe the TBs for. Default: 32*
- **`UCI_Chess960`**: Represents moves as king takes rook. Default: false.
- **`Softnodes`**: Only check if the node count has been reached after completing a depth. Default: false

*Currently the largest TB is only 7 pieces

## Neural Network

Ensure a NNUE file is placed relative to this file, as a .nnue will be if you build with `make`.

When building run:
```cpp
make EVALFILE=<path to your file>
```

In the case of a different architecture, modifications may be made to the NNUE section of the config:

In config.h:
```cpp
// ************ NNUE ************
constexpr i16    QA             = 255;
constexpr i16    QB             = 64;
constexpr i16    EVAL_SCALE     = 400;
...
```

## Special Thanks

- **Swedishchef**: Teaching me about CPUs, high performance programming, and much more
- **Dan**: Allowing me to use his OpenBench instance while MattBench was down
- **Vast**: Help hunting for bugs and explaining concepts when I was new
- **Fury**: Integral is a big source of inspiration, and the fantastic comments are a great source
- **Ciekce**: Lots of guidance and test NNUEs
- **Shawn\_xu**: Explaining NNUEs and other common concepts
- **A\_randomnoob**: Hunting for bugs and explaining concepts
- **Matt**: Providing help and allowing me to use his git instance, as well as sharing OpenBench
- **Kan**: Making the Prelude TCEC logo
- **jw**: Helping me with NNUE training and developer of [bullet](https://github.com/jw1912/bullet)
- **Cosmo**: Creator of Viri binpacks, and the person who added them to bullet
- **All other members of the Stockfish discord**: So many people who I didn't name have helped with this project, and even those who I did mention by name certainly did more than I listed here. Thanks to everyone who contributed!