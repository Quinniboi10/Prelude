# Compiler and performance flags
CXX      := clang++
CXXFLAGS := -O3 -march=native -flto -ffast-math -funroll-loops -std=c++20 -static -DNDEBUG

# Default target executable name and evaluation file path
EXE      ?= Prelude
EVALFILE ?= ./nnue.bin

# Source and object files
SRCS     := Prelude.cpp
OBJS     := $(SRCS:.cpp=.o)

# Default target
all: $(EXE)

# Link the executable
$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) -DEVALFILE_PATH=\"$(EVALFILE)\" -o $@ $^

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -DEVALFILE_PATH=\"$(EVALFILE)\" -c $< -o $@

# Force rebuild
.PHONY: force
force: clean all

# Clean up
.PHONY: clean
clean:
	rm -f $(EXE) $(OBJS)
