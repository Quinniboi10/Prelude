# Detect Operating System
ifeq ($(OS),Windows_NT)
    # Windows settings
    RM := del /F /Q
    EXE_EXT := .exe
else
    # Unix/Linux settings
    RM := rm -f
    EXE_EXT :=
endif

# Short commit id of HEAD (thanks to Weiss for this!)
GIT_HEAD_COMMIT_ID_RAW := $(shell git rev-parse --short HEAD)
ifneq ($(GIT_HEAD_COMMIT_ID_RAW),)
GIT_HEAD_COMMIT_ID_DEF := -DGIT_HEAD_COMMIT_ID=\""$(GIT_HEAD_COMMIT_ID_RAW)"\"
else
GIT_HEAD_COMMIT_ID_DEF :=
endif

# Compiler and flags
CXX      := clang++
CXXFLAGS := -O3 -march=native -ffast-math -fno-finite-math-only -funroll-loops -flto -fuse-ld=lld -std=c++20 -static -DNDEBUG

# Default target executable name and evaluation file path
EXE      ?= Prelude$(EXE_EXT)
EVALFILE ?= ./nnue.bin

# Source and object files
SRCS     := $(wildcard ./src/*.cpp)
OBJS     := $(SRCS:.cpp=.o)

# Default target
all: $(EXE)

# Link the executable
$(EXE): $(SRCS)
	$(CXX) $(CXXFLAGS) -DEVALFILE=\"$(EVALFILE)\" $(GIT_HEAD_COMMIT_ID_DEF) $(SRCS) ./external/fmt/format.cc -o $@

# Debug build
.PHONY: debug
debug: clean
debug: CXXFLAGS = -march=native -std=c++20 -O3 -DDEBUG -fsanitize=address -fsanitize=undefined -fno-finite-math-only -fno-omit-frame-pointer -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -Wall -Wextra
debug: all

# Debug build
.PHONY: profile
profile: clean
profile: CXXFLAGS = -O2 -g -march=native -ffast-math -fno-finite-math-only -funroll-loops -flto -fuse-ld=lld -std=c++20 -fno-omit-frame-pointer -static -DNDEBUG
profile: all

# Force rebuild
.PHONY: force
force: clean all

# Clean up
.PHONY: clean
clean:
	$(RM) $(EXE)
	$(RM) Prelude.exp
	$(RM) Prelude.lib
	$(RM) Prelude.pdb
