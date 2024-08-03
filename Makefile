# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c11

# Source files
SRCS = ski-start.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
EXEC = ski-start

# Default target
all: $(EXEC)

# Rule to build the executable
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to build object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(EXEC)

# Phony targets
.PHONY: all clean

