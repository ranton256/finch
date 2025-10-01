# Makefile for Finch Examples
#
# Builds all example programs that demonstrate Finch graphics library features

CC = gcc
FLAGS = -Wall -g -std=c99
LFLAGS = -lm

# Parent directory contains the Finch library
FINCH_DIR = ..
FINCH_LIB = $(FINCH_DIR)/libfinch.a

# Get SDL flags if available
-include $(FINCH_DIR)/Makefile.sdlflags

# Example programs
EXAMPLES = bounce fern text_demo

# Default target - build all examples
all: $(EXAMPLES)

# Build the Finch library if needed
$(FINCH_LIB):
	@echo "Building Finch library..."
	@$(MAKE) -C $(FINCH_DIR) $(notdir $(FINCH_LIB))

# Pattern rule for building examples
%: %.c $(FINCH_LIB)
	@echo "Building $@..."
	$(CC) $(FLAGS) -I$(FINCH_DIR) $< $(FINCH_LIB) -o $@ $(SDL_LFLAGS)

# Clean all built examples
clean:
	rm -f $(EXAMPLES)

# Run a specific example (make run EXAMPLE=bounce)
run: $(EXAMPLE)
	./$(EXAMPLE)

.PHONY: all clean run
