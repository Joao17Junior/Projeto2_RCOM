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
.PHONY: run
run: $(TARGET) $(TEST_DIR)
	@echo "Running download application..."
	./$(TARGET) ftp://demo:password@test.rebex.net/readme.txt

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
	@echo "  make all         - Build the download application"
	@echo "  make run         - Run with default test URL"
	@echo "  make clean       - Remove compiled files"
	@echo "  make help        - Show this help message"