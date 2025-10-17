# This Makefile is a convenience wrapper.
# The main build logic is in the root Makefile (../Makefile).

.PHONY: all test valgrind clean

# All targets are passed up to the parent Makefile.
# For example, 'make test' in this directory will run 'make -C .. test'.
all test valgrind clean:
	$(MAKE) -C .. $@