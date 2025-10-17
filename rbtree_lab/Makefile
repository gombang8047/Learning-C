# Compiler
CC := gcc

# Directories
SRC_DIR := src
TEST_DIR := test

# Flags: All paths are relative to this root directory.
CFLAGS := -Wall -g -DSENTINEL -I$(SRC_DIR) -I$(TEST_DIR)/unity

# Final executable will be created in the 'test' directory.
TARGET := $(TEST_DIR)/test-rbtree

# Object files needed for the final executable
OBJS_RBTREE := $(SRC_DIR)/rbtree.o
OBJS_TEST := $(TEST_DIR)/test-main.o $(TEST_DIR)/unity/unity.o

# Phony targets
.PHONY: all test valgrind clean

# Default target
all: $(TARGET)

# Rule to link the final executable
$(TARGET): $(OBJS_RBTREE) $(OBJS_TEST)
	$(CC) $(CFLAGS) -o $@ $(OBJS_RBTREE) $(OBJS_TEST)

# Rule to build rbtree.o by calling 'make' in the src directory
$(OBJS_RBTREE):
	$(MAKE) -C $(SRC_DIR) rbtree.o

# Rule to build test-main.o from the test directory
$(TEST_DIR)/test-main.o: $(TEST_DIR)/test-main.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to build unity.o from the test/unity directory
$(TEST_DIR)/unity/unity.o: $(TEST_DIR)/unity/unity.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to run the tests
test: $(TARGET)
	./$(TARGET)

# Rule to run tests with Valgrind
valgrind: $(TARGET)
	valgrind --leak-check=full ./$(TARGET)

# Rule to clean up the entire project
clean:
	# Remove object files and the target from the test directory
	rm -f $(TARGET) $(OBJS_TEST)
	# Call the clean rule in the src directory
	$(MAKE) -C $(SRC_DIR) clean