# Makefile for compiling PiBench project with multiple source files

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

# Directories
SRC_DIR := .
BUILD_DIR := .
SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

# Output file
OUT := $(BUILD_DIR)/pibench_libretro.so

# Default target
all: $(OUT)

# Main shared library target
$(OUT): $(OBJECTS)
	@echo "Linking $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Pattern rule for compiling source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)/*.so $(BUILD_DIR)/*.o

.PHONY: all clean