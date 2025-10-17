# Compiler
CC := gcc

# Flags for this directory
CFLAGS := -Wall -g

# Phony targets
.PHONY: all clean

# Default target for standalone testing
all: driver

# Rule to build the rbtree object file (called by the root Makefile)
rbtree.o: rbtree.c rbtree.h
	$(CC) $(CFLAGS) -c -o $@ rbtree.c

# Rule to build the driver object file
driver.o: driver.c rbtree.h
	$(CC) $(CFLAGS) -c -o $@ driver.c

# Rule to link the standalone driver executable
driver: driver.o rbtree.o
	$(CC) $(CFLAGS) -o $@ $^

# Rule to clean up this directory
clean:
	rm -f driver *.o