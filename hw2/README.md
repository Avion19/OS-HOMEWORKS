## Info
Name: Avien Ramirez  
Date: 9/23/26  
Class: CS 452-2  
Assignment: hw-2

## Intro

This project implements a buddy-system memory allocator in C. It manages a
memory pool using power-of-two blocks, splits larger blocks to satisfy
allocations, and coalesces free buddy blocks when possible. The project also
integrates the allocator with a double-ended queue by routing the deque's
`malloc()` and `free()` calls through the allocator.

## Description

The main files are:

- `balloc.c` and `balloc.h`: public allocator implementation and interface
- `freelist.c` and `freelist.h`: per-order free lists, splitting, and coalescing
- `bbm.c` and `bbm.h`: buddy-pair bitmap operations and buddy address helpers
- `bm.c` and `bm.h`: general-purpose bitmap implementation and interface
- `utils.c` and `utils.h`: memory mapping, bit operations, and size conversions
- `wrapper.c`: `malloc()` and `free()` wrapper backed by the buddy allocator
- `deq/deq.c` and `deq/deq.h`: double-ended deque implementation and interface
- `tests/buddy_test.c`: consolidated unit and integration tests for buddy modules
- `main.c`: original deque test suite, built unchanged with the allocator wrapper
- `GNUmakefile`: build, test, and clean targets

The `*-deep-dive.md` files explain the allocator modules and their data
structures in more detail.

## How to Run

Build the buddy and deque test programs:

```sh
make
```

Build and run the buddy tests and the original deque suite:

```sh
make test
```

Run only the buddy allocator tests:

```sh
make test-buddy
```

Run only the original deque tests through the allocator wrapper:

```sh
make test-deq
```

Remove generated build files:

```sh
make clean
```

## Discussion

Understanding the supplied bitmap modules was a difficult first step. I started
by documenting `bm` and `bbm` so I could understand what each one was supposed
to do. I was a little rusty with bit manipulation, so following how the buddy
address helpers use the block order took some time. It helped once I understood
that `baddrset` selects the higher-addressed buddy, `baddrclr` selects the
lower-addressed buddy, and `baddrinv` selects the other buddy. When two buddies
merge, the resulting block starts at the lower address.

The free-list code was where I had the most difficulty. I kept its interface
unchanged, and `freelistfree()` does not receive the largest order `u`. I had to
figure out how to stop coalescing safely after reaching that order. I decided to
put a sentinel `OrderList` after the real orders. Its bitmap is zero, so the
free-list code can recognize the end of the array without reading past it. Each
real `OrderList` holds a head pointer and a bitmap for one order. The free
blocks store their next pointer at the beginning of the block, which means the
allocator uses free pool memory for the list links. When a larger block is
needed, the allocator splits it, keeps the lower-addressed half, and adds the
upper half to the smaller-order free list. When a block is freed, it merges
with its buddy only if that buddy is on the same-order free list.

After working through the free lists, I found it easier to understand how
`balloc.c` connects them to the public allocator functions. `bcreate()` maps the
pool and its bookkeeping, then divides the pool into aligned blocks that fit
within the configured orders. `balloc()` rounds a request up to a block order
and asks the free-list code for memory. `bfree()` and `bsize()` check the
pointer and use the free lists and bitmaps to find its actual allocation order.
This separation lets `balloc.c` manage the allocator and lets `freelist.c` focus
on splitting and merging blocks.

Testing the allocator through the deque was useful because it showed the
allocator handling regular `malloc()` and `free()` calls. The wrapper defines
those functions and sends them to `balloc()` and `bfree()`. The deque source and
its original test suite stay unchanged; `make test-deq` links them with the
wrapper, and `make test` runs those tests along with the buddy module tests.


## Testing

The consolidated test suite in `tests/buddy_test.c` covers:

- Utility arithmetic and bit operations
- Generic bitmap indexing across byte boundaries
- Buddy address calculations and pair bitmap semantics
- Free-list splitting, merging, allocation-size lookup, and exhaustion
- Public allocator rounding, data preservation, invalid requests, and exhaustion
- Double-free rejection and rejection of a misaligned free
- Non-power-of-two pools, pools larger than the maximum block, and independent pools
- Small (64-byte), medium (4 KiB), and large (1 MiB) pools filled to exhaustion,
  freed, and checked for full coalescing

`make test-deq` compiles the existing `main.c` deque suite with the unchanged
deque source and `wrapper.c`, so deque allocations use the buddy allocator.

## Results

`make test` builds without compiler warnings and passes both suites. The buddy
tests intentionally print diagnostics while checking invalid requests, frees,
and exhausted pools.

```text
gcc -D_GNU_SOURCE -I. -Ideq -g -Wall -Wextra -o buddy_tests tests/buddy_test.c balloc.c bbm.c bm.c freelist.c utils.c -lm
./buddy_tests
balloc: request exceeds the largest block size
bsize: pointer is not a live allocation
bsize: pointer is not a live allocation
balloc: no suitable free block remains
bsize: pointer is not a live allocation
bsize: pointer is not a live allocation
balloc: no suitable free block remains
bsize: pointer is not a live allocation
bfree: pointer is not a live allocation
bfree: pointer is not aligned to its allocation size
balloc: no suitable free block remains
balloc: no suitable free block remains
balloc: no suitable free block remains
balloc: no suitable free block remains
balloc: no suitable free block remains
balloc: no suitable free block remains
bcreate: invalid pool size or block-order range
bcreate: invalid pool size or block-order range
bcreate: invalid pool size or block-order range
bcreate: pool is smaller than one usable free block
balloc: allocator is invalid or request size is zero
All buddy allocator tests passed.
gcc -D_GNU_SOURCE -I. -Ideq -g -Wall -Wextra -o deq_tests main.c deq/deq.c \
	wrapper.c balloc.c bbm.c bm.c freelist.c utils.c -lm
stdbuf -o0 ./deq_tests
PASS: test_empty
PASS: test_null_behavior
PASS: test_put_and_ith
PASS: test_get
PASS: test_rem
PASS: test_duplicate_pointer_rem
PASS: test_string_after_operations
PASS: test_map
PASS: test_head_put_only
PASS: test_tail_put_only
PASS: test_head_get_only
PASS: test_tail_get_only
PASS: test_head_ith_only
PASS: test_tail_ith_only
PASS: test_head_rem_only
PASS: test_tail_rem_only
PASS: test_operations_in_tandem
All deque tests passed.
```
