# Makefile for building Prelude.cpp with Clang and maximum optimizations

# Compiler to use
CXX := clang++

# Compiler flags:
# -O3       : Enable high-level optimizations
# -march=native : Optimize code for the local machine architecture
CXXFLAGS := -O3 -march=native -std=c++20

# Target executable name
TARGET := Prelude

# Source files
SRCS := Prelude.cpp

# Object files (generated by replacing .cpp with .o)
OBJS := $(SRCS:.cpp=.o)

# Default target when `make` is run without arguments
all: $(TARGET)

# Rule to link object files into the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target to remove generated files
.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)
