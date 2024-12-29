# Compiler and flags
CXX := clang++
CXXFLAGS := -O3 -march=native -std=c++20

# Default target executable name
EXE ?= Prelude

# Source and object files
SRCS := Prelude.cpp
OBJS := $(SRCS:.cpp=.o)

# Default target
all: $(EXE)

# Link the executable
$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

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
