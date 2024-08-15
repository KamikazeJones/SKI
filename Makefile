# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE
DEPFLAGS = -MMD -MP

# Source files
SRCS = ski-start.c ski-parse.c

# Object files
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

# Executable name
EXEC = ski-start

# Default target
all: $(EXEC)

# Rule to build the executable
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to build object files
%.o: %.c
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

# Include der Abhängigkeitsdateien
-include $(DEPS)

# Clean up build files
clean:
	rm -f $(OBJS) $(EXEC) $(DEPS)

debug: clean
debug: CFLAGS += -g -DDEBUG
debug: all

# Phony targets
.PHONY: all clean

