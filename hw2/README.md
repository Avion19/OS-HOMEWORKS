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
- `tests/buddy_test.c`: tests for utilities, free lists, and the public allocator
- `tests/mainDeq.c`: deque test suite, linked with the allocator wrapper
- `GNUmakefile`: build, test, and clean targets

## How to Run

Build the buddy allocator and deque test programs:

```sh
make
```

Build and run the utility, free-list, allocator, and deque tests:

```sh
make test
```

Run only the utility, free-list, and allocator tests:

```sh
make test-buddy
```

Run only the deque tests through the allocator wrapper:

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
`balloc.c` connects them to the public allocator functions. Building reasonably
sized blocks in `bcreate()` was a little challenging, especially for pools
whose size is not a power of two. I eventually figured out that I could track
how much of the pool I had used with an offset. At each step, I choose the
largest block that fits in the remaining space, add it to the matching free
list, and increase the offset by that block's size. This let me fill the pool
with aligned blocks and represent the usable leftover space with smaller
orders.

`bcreate()` also maps the pool and its bookkeeping. `balloc()` rounds a request
up to a block order and asks the free-list code for memory. `bfree()` and
`bsize()` check the pointer and use the free lists and bitmaps to find its
actual allocation order. This separation lets `balloc.c` manage the allocator
and lets `freelist.c` focus on splitting and merging blocks.

Testing the allocator through the deque was useful because it showed the
allocator handling regular `malloc()` and `free()` calls. The wrapper defines
those functions and sends them to `balloc()` and `bfree()`. The deque source and
its original test suite stay unchanged; `make test-deq` links them with the
wrapper, and `make test` runs those tests along with the buddy module tests.

## Testing

The buddy test suite in `tests/buddy_test.c` covers `utils`, `freelist`, and
`balloc`:

Run this suite with `make test-buddy`. It builds and runs `buddy_tests` without
running the deque suite; use `make test-deq` for deque tests or `make test` to
run both suites.

- Utility arithmetic and bit operations
- Free-list splitting, merging, allocation-size lookup, and exhaustion
- Public allocator rounding, data preservation, invalid requests, and exhaustion
- Double-free rejection and rejection of a misaligned free
- Non-power-of-two pools, pools larger than the maximum block, and independent pools
- Small (64-byte), medium (4 KiB), and large (1 MiB) pools filled to exhaustion,
  freed, and checked for full coalescing

There are no standalone `bm` or `bbm` tests. The free-list and allocator tests
exercise those bitmap modules through their normal use.

`make test-deq` runs the original deque suite with allocator-backed
`malloc()` and `free()`.

## Results

`make test` runs both the buddy module tests and the deque suite. The buddy
tests intentionally print descriptive diagnostics while checking invalid
requests, frees, and exhausted pools. A successful run ends with `All buddy
allocator tests passed.` and `All deque tests passed.`

```
make test
./buddy_tests
balloc: 33-byte request exceeds the maximum block size of 32 bytes
bsize: pointer 0x70543aac4020 is not the start of a live allocation (it may be freed or invalid)
bsize: pointer 0x70543aac4030 is not the start of a live allocation (it may be freed or invalid)
balloc: no free block can satisfy the 8-byte request (rounded to 8 bytes)
bsize: pointer 0x70543aac4000 is not the start of a live allocation (it may be freed or invalid)
bsize: pointer 0x70543aac4008 is not the start of a live allocation (it may be freed or invalid)
balloc: no free block can satisfy the 8-byte request (rounded to 8 bytes)
bsize: pointer 0x70543aac4000 is not the start of a live allocation (it may be freed or invalid)
bfree: pointer 0x70543aac4000 is not the start of a live allocation (it may be freed or invalid)
bfree: pointer 0x70543aac4009 is not aligned to its 8-byte allocation block
balloc: no free block can satisfy the 8-byte request (rounded to 8 bytes)
balloc: no free block can satisfy the 8-byte request (rounded to 8 bytes)
balloc: no free block can satisfy the 8-byte request (rounded to 8 bytes)
balloc: no free block can satisfy the 8-byte request (rounded to 8 bytes)
balloc: no free block can satisfy the 256-byte request (rounded to 256 bytes)
balloc: no free block can satisfy the 4096-byte request (rounded to 4096 bytes)
bcreate: pool size must be greater than zero
bcreate: invalid block-order range -1 through 5; expected supported orders with l <= u
bcreate: invalid block-order range 5 through 3; expected supported orders with l <= u
bcreate: pool size (4 bytes) is smaller than the minimum usable block (8 bytes)
balloc: request size must be greater than zero
All buddy allocator tests passed.
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
