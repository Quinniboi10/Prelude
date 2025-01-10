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

# Compiler and performance flags
CXX      := clang++
CXXFLAGS := -O3 -march=native -ffast-math -funroll-loops -std=c++20 -static -DNDEBUG

# Default target executable name and evaluation file path
EXE      ?= Prelude$(EXE_EXT)
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

# Debug Build
.PHONY: debug
debug: clean
debug: CXXFLAGS += -DDEBUG
debug: all

# Force rebuild
.PHONY: force
force: clean all

# Clean up
.PHONY: clean
clean:
	$(RM) $(EXE) $(OBJS)
