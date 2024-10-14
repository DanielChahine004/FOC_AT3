# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Werror -ansi -Wextra -std=c99

# Source files
SOURCES = main.c huffman_compression.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Executable name
ifeq ($(OS),Windows_NT)
    EXECUTABLE = main.exe
else
    EXECUTABLE = main
endif

# Default target
all: $(EXECUTABLE)

# Link the program
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
ifeq ($(OS),Windows_NT)
	del /Q *.o $(EXECUTABLE)
else
	rm -f *.o $(EXECUTABLE)
endif

# Phony targets
.PHONY: all clean