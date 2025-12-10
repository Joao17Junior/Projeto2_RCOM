# Makefile for FTP Download Application
# Computer Networks - Lab 2

# Compiler and flags
CC = gcc
CFLAGS = -Wall

# Directories
SRC = src
HEADER = header

# Target executable
TARGET = download

# Source files
SOURCES = $(SRC)/download.c
HEADERS = $(HEADER)/download.h

# ===============================================
# Main target
# ===============================================
.PHONY: all
all: $(TARGET)

# Build the download application
$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -I$(HEADER) -o $@ $(SOURCES)
	@echo "✓ Compiled successfully: $(TARGET)"

# ===============================================
# Run targets (tests only)
# ===============================================
.PHONY: run
run: $(TARGET)
	@echo "Running download application..."
	$(TARGET) ftp://ftp.netlab.fe.up.pt/pub/README

# Run with custom URL
.PHONY: run-custom
run-custom: $(TARGET)
	@echo "Usage: make run-custom URL=ftp://..."
	@if [ -z "$(URL)" ]; then \
		echo "Error: Please specify URL"; \
		echo "Example: make run-custom URL=ftp://ftp.netlab.fe.up.pt/pub/README"; \
	else \
		$(TARGET) $(URL); \
	fi

# ===============================================
# Clean targets
# ===============================================
.PHONY: clean
clean:
	rm -f $(TARGET)
	@echo "✓ Cleaned build files"

# ===============================================
# Help target
# ===============================================
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  make all         - Build the download application"
	@echo "  make run         - Run with default test URL"
	@echo "  make run-custom  - Run with custom URL (make run-custom URL=...)"
	@echo "  make clean       - Remove compiled files"
	@echo "  make help        - Show this help message"