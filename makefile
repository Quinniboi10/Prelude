# Compiler and flags
CXX := g++
CXXFLAGS := -O3 -march=native -std=c++20

# Default target executable name
EXE ?= Prelude

# Default evaluation file path
EVALFILE ?= ./nnue.bin

# Source and object files
SRCS := Prelude.cpp
OBJS := $(SRCS:.cpp=.o)

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

# Debugging info
.PHONY: print
print:
	@echo "EXE: $(EXE)"
	@echo "SRCS: $(SRCS)"
	@echo "OBJS: $(OBJS)"
	@echo "EVALFILE: $(EVALFILE)"