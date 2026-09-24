# Use the shared Onyx compiler and object rules, with explicit source lists for
# this project’s two separate test programs.
.DEFAULT_GOAL := all

prog := buddy_tests
override objs := tests/buddy_test.o balloc.o bbm.o bm.o freelist.o utils.o

defines += -I. -Ideq
ccflags += -Wextra
ldflags += -lm

include ../GNUmakefile

.PHONY: all test test-buddy test-deq

all: buddy_tests deq_tests

deq_tests: tests/mainDeq.o deq/deq.o wrapper.o balloc.o bbm.o bm.o freelist.o utils.o
	$(ld) -o $@ $^ $(ldflags)

test: test-buddy test-deq

test-buddy: buddy_tests
	./buddy_tests

# Run unbuffered so stdio does not request a buffer from the allocator wrapper.
test-deq: deq_tests
	stdbuf -o0 ./deq_tests

clean::
	rm -f deq_tests tests/*.o tests/*.d deq/*.o deq/*.d

-include tests/*.d deq/*.d
