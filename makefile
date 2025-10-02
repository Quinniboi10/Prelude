DEFAULT_NETWORK = Prelude_09.nnue

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
CXXFLAGS := -O3 -fno-finite-math-only -funroll-loops -flto -std=c++20 -DNDEBUG

ifeq ($(OS),Windows_NT)
  ARCH := $(PROCESSOR_ARCHITECTURE)
else
  ARCH := $(shell uname -m)
endif

IS_ARM := $(filter ARM arm64 aarch64 arm%,$(ARCH))

ifeq ($(IS_ARM),)
  LINKFLAGS := -fuse-ld=lld -pthread
  ARCHFLAGS := -march=native
else
  LINKFLAGS :=
  ARCHFLAGS := -mcpu=native
endif


# Default target executable name and evaluation file path
EXE      ?= Prelude$(EXE_EXT)
EVALFILE ?= $(DEFAULT_NETWORK)

# Source and object files
SRCS     := $(wildcard ./src/*.cpp)
SRCS     += ./external/fmt/format.cpp
SRCS     += ./external/Pyrrhic/tbprobe.cpp
OBJS     := $(SRCS:.cpp=.o)
DEPS     := $(OBJS:.o=.d)

# Default target
all: download
all: $(EXE)

# Build the objects
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(ARCHFLAGS) $(GIT_HEAD_COMMIT_ID_DEF) -DEVALFILE="\"$(EVALFILE)\"" -c $< -o $@

CXXFLAGS += -MMD -MP
-include $(DEPS)

# Link the executable
$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LINKFLAGS) -o $@

# Net repo URL
NET_BASE_URL := https://git.nocturn9x.space/Quinniboi10/Prelude-Nets/raw/branch/main

# Files for make clean
CLEAN_STUFF := $(EXE) Prelude.exp Prelude.lib Prelude.pdb $(OBJS) $(DEPS) $(DEFAULT_NETWORK)
ifeq ($(OS),Windows_NT)
    CLEAN_STUFF := $(subst /,\\,$(CLEAN_STUFF))
endif

# Download if the net is not specified
ifeq ($(EVALFILE),$(DEFAULT_NETWORK))
download: $(DEFAULT_NETWORK)
else
download:
	@echo "EVALFILE is set to '$(EVALFILE)', skipping download."
endif

$(DEFAULT_NETWORK):
	curl -L -o $@ $(NET_BASE_URL)/$@

# Debug build
.PHONY: debug
debug: clean
debug: CXXFLAGS = -march=native -std=c++20 -O3 -fsanitize=address,undefined -fno-finite-math-only -fno-omit-frame-pointer -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -Wall -Wextra
debug: all

# Debug build
.PHONY: profile
profile: clean
profile: CXXFLAGS = -O3 -ggdb -fno-finite-math-only -funroll-loops -flto -std=c++20 -fno-omit-frame-pointer -DNDEBUG
profile: all

# Force rebuild
.PHONY: force
force: clean
force: all

# Clean up
.PHONY: clean
clean:
	$(RM) $(CLEAN_STUFF)