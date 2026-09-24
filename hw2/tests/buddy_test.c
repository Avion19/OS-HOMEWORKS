/*
 * Author: Avien Ramirez
 * Date: September 23, 2026
 * Class: CS 452-2
 * Description: Unit and integration tests for all buddy allocator modules.
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

#include "balloc.h"
#include "bbm.h"
#include "bm.h"
#include "freelist.h"
#include "utils.h"

/* Verify integer conversions and primitive byte-level bit operations. */
static void test_utils(void)
{
    unsigned char byte = 0;

    assert(divup(0, 8) == 0);
    assert(divup(8, 8) == 1);
    assert(divup(9, 8) == 2);
    assert(divup(17, 8) == 3);
    assert(bits2bytes(0) == 0);
    assert(bits2bytes(8) == 1);
    assert(bits2bytes(9) == 2);
    assert(e2size(0) == 1);
    assert(e2size(3) == 8);
    assert(size2e(1) == 0);
    assert(size2e(8) == 3);
    assert(size2e(9) == 4);

    bitset(&byte, 0);
    bitset(&byte, 7);
    assert(byte == 0x81);
    assert(bittst(&byte, 0) != 0);
    assert(bittst(&byte, 7) != 0);
    assert(bittst(&byte, 3) == 0);
    bitinv(&byte, 0);
    assert(byte == 0x80);
    bitclr(&byte, 7);
    assert(byte == 0);
}

/* Exercise bitmap operations across byte boundaries. */
static void test_bitmap(void)
{
    BM bits = bmcreate(17);

    assert(bits != 0);
    for (size_t i = 0; i < 17; i++)
        assert(bmtst(bits, i) == 0);

    bmset(bits, 0);
    bmset(bits, 7);
    bmset(bits, 8);
    bmset(bits, 16);
    assert(bmtst(bits, 0) != 0);
    assert(bmtst(bits, 7) != 0);
    assert(bmtst(bits, 8) != 0);
    assert(bmtst(bits, 16) != 0);
    assert(bmtst(bits, 15) == 0);

    bmclr(bits, 7);
    bmclr(bits, 16);
    assert(bmtst(bits, 7) == 0);
    assert(bmtst(bits, 16) == 0);
    assert(bmtst(bits, 0) != 0);
    assert(bmtst(bits, 8) != 0);
    bmdelete(bits);
}

/* Verify buddy-address arithmetic and shared pair-bit indexing. */
static void test_buddy_bitmap(void)
{
    unsigned char region[64] = {0};
    void *base = region;
    BBM bits = bbmcreate(sizeof(region), 3);

    assert(bits != 0);
    assert(baddrset(base, base, 3) == region + 8);
    assert(baddrclr(base, region + 8, 3) == base);
    assert(baddrinv(base, region, 3) == region + 8);
    assert(baddrinv(base, region + 8, 3) == base);
    assert(baddrtst(base, base, 3) == 0);
    assert(baddrtst(base, region + 8, 3) != 0);

    /* Both order-3 buddies share one bit; the next pair has another bit. */
    assert(bbmtst(bits, base, region, 3) == 0);
    assert(bbmtst(bits, base, region + 8, 3) == 0);
    assert(bbmtst(bits, base, region + 16, 3) == 0);
    bbmset(bits, base, region + 8, 3);
    assert(bbmtst(bits, base, region, 3) != 0);
    assert(bbmtst(bits, base, region + 8, 3) != 0);
    assert(bbmtst(bits, base, region + 16, 3) == 0);
    bbmclr(bits, base, region, 3);
    assert(bbmtst(bits, base, region + 8, 3) == 0);

    bbmset(bits, base, region + 24, 3);
    assert(bbmtst(bits, base, region + 16, 3) != 0);
    assert(bbmtst(bits, base, region + 24, 3) != 0);
    bbmdelete(bits);
}

/* Exercise initialization, splitting, lookup, and coalescing. */
static void test_freelist(void)
{
    const size_t pool_size = 64;
    void *base = mmalloc(pool_size);
    FreeList lists;
    void *small;
    void *large_a;
    void *large_b;

    assert(base != MAP_FAILED);
    lists = freelistcreate(pool_size, 3, 5);
    assert(lists != 0);

    /* Seed two maximum-order blocks, as bcreate() does for its pool. */
    freelistfree(lists, base, base, 5, 3);
    freelistfree(lists, base, (char *)base + 32, 5, 3);

    small = freelistalloc(lists, base, 3, 3);
    assert(small != 0);
    assert(freelistsize(lists, base, small, 3, 5) == 3);

    /* Returning the split block coalesces its smaller free buddies. */
    freelistfree(lists, base, small, 3, 3);
    assert(freelistsize(lists, base, small, 3, 5) == -1);

    large_a = freelistalloc(lists, base, 5, 3);
    large_b = freelistalloc(lists, base, 5, 3);
    assert(large_a != 0 && large_b != 0 && large_a != large_b);
    /* The old free-list link is cleared before client ownership is returned. */
    assert(*(void **)large_a == 0);
    assert(freelistalloc(lists, base, 5, 3) == 0);
    assert(freelistsize(lists, base, large_a, 3, 5) == 5);
    assert(freelistsize(lists, base, large_b, 3, 5) == 5);

    freelistfree(lists, base, large_a, 5, 3);
    freelistfree(lists, base, large_b, 5, 3);
    freelistdelete(lists, 3, 5);
    mmfree(base, pool_size);
}

/* Check request rounding, client writes, allocation sizes, and freeing. */
static void test_allocator_rounding_and_memory(void)
{
    Balloc pool = bcreate(64, 3, 5);
    unsigned char *small;
    unsigned char *medium;

    assert(pool != 0);
    small = balloc(pool, 1);
    medium = balloc(pool, 9);
    assert(small != 0 && medium != 0 && small != medium);
    assert(bsize(pool, small) == 8);
    assert(bsize(pool, medium) == 16);

    small[0] = 0x5a;
    medium[0] = 0xa5;
    assert(small[0] == 0x5a && medium[0] == 0xa5);
    assert(balloc(pool, 33) == 0);

    bfree(pool, small);
    assert(bsize(pool, small) == 0);
    bfree(pool, medium);
    assert(bsize(pool, medium) == 0);
    bdelete(pool);
}

/* Confirm buddies coalesce into a larger allocation and then exhaust. */
static void test_coalescing_and_exhaustion(void)
{
    Balloc pool = bcreate(16, 3, 4);
    void *left;
    void *right;
    void *merged;

    assert(pool != 0);
    left = balloc(pool, 8);
    right = balloc(pool, 8);
    assert(left != 0 && right != 0 && left != right);
    assert(balloc(pool, 8) == 0);

    bfree(pool, left);
    bfree(pool, right);
    /* Both stale pointers now lie inside the coalesced 16-byte free block. */
    assert(bsize(pool, left) == 0);
    assert(bsize(pool, right) == 0);
    merged = balloc(pool, 16);
    assert(merged != 0);
    assert(bsize(pool, merged) == 16);
    assert(balloc(pool, 8) == 0);

    bfree(pool, merged);
    bdelete(pool);
}

/* Ensure invalid and duplicate frees do not corrupt free-list state. */
static void test_repeated_and_invalid_free(void)
{
    Balloc pool = bcreate(16, 3, 4);
    void *first;
    void *second;
    void *after;

    assert(pool != 0);
    first = balloc(pool, 8);
    second = balloc(pool, 8);
    assert(first != 0 && second != 0);

    bfree(pool, first);
    assert(bsize(pool, first) == 0);
    bfree(pool, first); /* A repeated free must not duplicate the list node. */
    bfree(pool, (char *)second + 1); /* Misaligned pointer must be rejected. */
    assert(bsize(pool, second) == 8);

    after = balloc(pool, 8);
    assert(after == first);
    assert(balloc(pool, 8) == 0); /* The free block is present only once. */
    bfree(pool, after);
    bfree(pool, second);
    bdelete(pool);
}

/* Cover irregular, oversized, and independent pool configurations. */
static void test_pool_shapes_and_independence(void)
{
    Balloc larger = bcreate(160, 3, 5);
    Balloc irregular = bcreate(104, 3, 5);
    Balloc first = bcreate(32, 3, 5);
    Balloc second = bcreate(32, 3, 5);
    void *blocks[5];
    void *tail;
    void *a;
    void *b;

    assert(larger != 0 && irregular != 0);
    for (size_t i = 0; i < 5; i++)
    {
        blocks[i] = balloc(larger, 32);
        assert(blocks[i] != 0);
        assert(bsize(larger, blocks[i]) == 32);
    }
    assert(balloc(larger, 8) == 0);
    for (size_t i = 0; i < 5; i++)
        bfree(larger, blocks[i]);

    /* 104 bytes holds three 32-byte blocks and one 8-byte block. */
    for (size_t i = 0; i < 3; i++)
    {
        blocks[i] = balloc(irregular, 32);
        assert(blocks[i] != 0);
    }
    tail = balloc(irregular, 8);
    assert(tail != 0 && bsize(irregular, tail) == 8);
    assert(balloc(irregular, 8) == 0);
    for (size_t i = 0; i < 3; i++)
        bfree(irregular, blocks[i]);
    bfree(irregular, tail);

    assert(first != 0 && second != 0);
    a = balloc(first, 32);
    b = balloc(second, 32);
    assert(a != 0 && b != 0 && a != b);
    bfree(first, a);
    bfree(second, b);
    bdelete(larger);
    bdelete(irregular);
    bdelete(first);
    bdelete(second);
}

/* Fill, exhaust, free, and coalesce small, medium, and large pools. */
static void test_pool_scales(void)
{
    Balloc small = bcreate(64, 3, 6);
    Balloc medium = bcreate(4096, 3, 12);
    Balloc large = bcreate(1u << 20, 3, 20);
    void *small_blocks[8];
    void *medium_blocks[16];
    void *large_blocks[256];
    void *whole;

    assert(small != 0 && medium != 0 && large != 0);

    /* Small: fill 64 bytes with eight minimum-size allocations. */
    for (size_t i = 0; i < 8; i++)
    {
        small_blocks[i] = balloc(small, 8);
        assert(small_blocks[i] != 0);
    }
    assert(balloc(small, 8) == 0);
    for (size_t i = 0; i < 8; i++)
        bfree(small, small_blocks[i]);
    whole = balloc(small, 64);
    assert(whole != 0 && bsize(small, whole) == 64);
    bfree(small, whole);

    /* Medium: fill 4 KiB with sixteen 256-byte allocations. */
    for (size_t i = 0; i < 16; i++)
    {
        medium_blocks[i] = balloc(medium, 256);
        assert(medium_blocks[i] != 0);
    }
    assert(balloc(medium, 256) == 0);
    for (size_t i = 0; i < 16; i++)
        bfree(medium, medium_blocks[i]);
    whole = balloc(medium, 4096);
    assert(whole != 0 && bsize(medium, whole) == 4096);
    bfree(medium, whole);

    /* Large: fill 1 MiB with 256 4 KiB allocations, then fully coalesce. */
    for (size_t i = 0; i < 256; i++)
    {
        large_blocks[i] = balloc(large, 4096);
        assert(large_blocks[i] != 0);
    }
    assert(balloc(large, 4096) == 0);
    for (size_t i = 0; i < 256; i++)
        bfree(large, large_blocks[i]);
    whole = balloc(large, 1u << 20);
    assert(whole != 0 && bsize(large, whole) == (1u << 20));
    bfree(large, whole);

    bdelete(small);
    bdelete(medium);
    bdelete(large);
}

/* Check rejected pool configurations, zero-size requests, and zero pointer arguments. */
static void test_invalid_inputs(void)
{
    Balloc pool;

    assert(bcreate(0, 3, 5) == 0);
    assert(bcreate(64, -1, 5) == 0);
    assert(bcreate(64, 5, 3) == 0);
    assert(bcreate(4, 3, 5) == 0);

    pool = bcreate(64, 3, 5);
    assert(pool != 0);
    assert(balloc(pool, 0) == 0);
    assert(bsize(pool, 0) == 0);
    bfree(pool, 0);
    bdelete(pool);
}

int main(void)
{
    test_utils();
    test_bitmap();
    test_buddy_bitmap();
    test_freelist();
    test_allocator_rounding_and_memory();
    test_coalescing_and_exhaustion();
    test_repeated_and_invalid_free();
    test_pool_shapes_and_independence();
    test_pool_scales();
    test_invalid_inputs();
    puts("All buddy allocator tests passed.");
    return 0;
}
