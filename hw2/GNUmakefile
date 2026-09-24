# Build and test the buddy allocator and its deque-wrapper integration.
CC := gcc
CPPFLAGS := -D_GNU_SOURCE -I. -Ideq
CFLAGS := -g -Wall -Wextra
LDLIBS := -lm

ALLOCATOR_SOURCES := balloc.c bbm.c bm.c freelist.c utils.c
ALLOCATOR_TEST := tests/buddy_test.c
DEQ_SOURCES := deq/deq.c
DEQ_TEST := tests/mainDeq.c

.PHONY: all test test-buddy test-deq clean

all: buddy_tests deq_tests

buddy_tests: $(ALLOCATOR_SOURCES) $(ALLOCATOR_TEST)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(ALLOCATOR_TEST) $(ALLOCATOR_SOURCES) $(LDLIBS)

# Build the user's original deque test suite without changing its source or deq/.
deq_tests: $(DEQ_TEST) $(DEQ_SOURCES) deq/deq.h deq/error.h wrapper.c $(ALLOCATOR_SOURCES)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(DEQ_TEST) $(DEQ_SOURCES) \
		wrapper.c $(ALLOCATOR_SOURCES) $(LDLIBS)

test: test-buddy test-deq

test-buddy: buddy_tests
	./buddy_tests

# Run unbuffered so stdio does not request a buffer from the allocator wrapper.
test-deq: deq_tests
	stdbuf -o0 ./deq_tests

clean:
	rm -f buddy_tests deq_tests hw2_tests deq_wrapper_tests deq-wrapper-tests deq-wrapper-tests.d \
		balloc.o balloc.d bbm.o bbm.d bm.o bm.d freelist.o freelist.d \
		utils.o utils.d wrapper.o wrapper.d
