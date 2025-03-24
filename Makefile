# Makefile for compiling pibench_libretro.c within PiBench directory

# Compiler
CC := gcc

# Debug flag
DEBUG := 0

# Adjust compiler flags based on debug mode
ifeq ($(DEBUG), 1)
   	CFLAGS := -fPIC -Wall -Wextra -O0 -g -DDEBUG -march=armv8-a
else
	CFLAGS := -fPIC -O3 -march=armv8-a -ftree-vectorize -fomit-frame-pointer -pipe
endif

# Flags for linking
LDFLAGS := -shared
CFLAGS += -Ilibretro-common/include

# Source and build directories
SRC_DIR := .
BUILD_DIR := .

# Source file
SRC := $(SRC_DIR)/pibench_libretro.c

# Output file
OUT := $(BUILD_DIR)/pibench_libretro.so

# Default target
all: $(OUT)

# Target for building the output
$(OUT): $(SRC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

# Target for cleaning the build directory
clean:
	rm -rf $(BUILD_DIR)/*.so

.PHONY: all clean
