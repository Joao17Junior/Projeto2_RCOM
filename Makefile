# Makefile for FTP Download Application
# Computer Networks - Lab 2

# Compiler and flags
CC = gcc
CFLAGS = -Wall

# Directories
SRC = src
INCLUDE = include
TEST_DIR = test_folder

# Target executable
TARGET = download

# Source files
SOURCES = $(SRC)/download.c
INCLUDES = $(INCLUDE)/download.h

# ===============================================
# Main target
# ===============================================
.PHONY: all
all: $(TEST_DIR) $(TARGET)

# Create test directory if it doesn't exist
$(TEST_DIR):
	@mkdir -p $(TEST_DIR)
	@echo "✓ Test directory ready: $(TEST_DIR)"

# Build the download application
$(TARGET): $(SOURCES) $(INCLUDES)
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $@ $(SOURCES)
	@echo "✓ Compiled successfully: $(TARGET)"

# ===============================================
# Run targets (tests only)
# ===============================================
.PHONY: run_1
run_1: $(TARGET) $(TEST_DIR)
	@echo "Running test 1: Arch Linux ISO..."
	./$(TARGET) ftp://ftp.up.pt/pub/archlinux/archive/iso/arch-0.8-base-i686.iso

.PHONY: run_2
run_2: $(TARGET) $(TEST_DIR)
	@echo "Running test 2: Rebex readme.txt..."
	./$(TARGET) ftp://demo:password@test.rebex.net/readme.txt

.PHONY: run_3
run_3: $(TARGET) $(TEST_DIR)
	@echo "Running test 3: 100MB speed test..."
	./$(TARGET) ftp://anonymous:anonymous@ftp.bit.nl/speedtest/100mb.bin

# ===============================================
# Clean targets
# ===============================================
.PHONY: clean
clean:
	rm -f $(TARGET)
	rm -rf $(TEST_DIR)
	@echo "✓ Cleaned build files"

# ===============================================
# Help target
# ===============================================
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  make all       - Build the download application"
	@echo "  make run_1     - Test 1: Arch Linux ISO"
	@echo "  make run_2     - Test 2: Rebex readme.txt"
	@echo "  make run_3     - Test 3: 100MB speed test"
	@echo "  make clean     - Remove compiled files"
	@echo "  make help      - Show this help message"