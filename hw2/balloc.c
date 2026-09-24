/*
 * Author: Avien Ramirez
 * Date: September 23, 2026
 * Class: CS 452-2
 * Description: Implements the public buddy allocator interface.
 */

#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>

#include "balloc.h"
#include "freelist.h"
#include "utils.h"

/*
 * Balloc is opaque outside this file.  This private structure owns the pool
 * mapping and the free-list metadata needed to manage that pool.
 */
typedef struct
{
    void *base;       /* First byte of the managed memory pool. */
    size_t size;      /* Number of bytes mapped for the pool. */
    int l;            /* Smallest usable block order. */
    int u;            /* Largest usable block order. */
    FreeList freelist;
} BallocData;

/*
 * The supplied bitmap code shifts a signed int by e, so e must leave its
 * sign bit untouched.  This also keeps 2^e representable as a size_t.
 */
static int validorder(int e)
{
    if (e < 0 || e >= (int)(sizeof(int) * CHAR_BIT - 1))
        return 0;

    if (e >= (int)(sizeof(size_t) * CHAR_BIT))
        return 0;

    return 1;
}

/* Print a short diagnostic for a failed public allocator operation. */
static void allocatorerror(const char *function, const char *message)
{
    fprintf(stderr, "%s: %s\n", function, message);
}

/* Check that mem names a byte inside this allocator's pool mapping. */
static int inpool(BallocData *data, void *mem)
{
    uintptr_t base = (uintptr_t)data->base;
    uintptr_t address = (uintptr_t)mem;

    return address >= base && address - base < data->size;
}

extern Balloc bcreate(unsigned int size, int l, int u)
{
    BallocData *data;
    size_t minimum;
    size_t offset;

    /* Validate the order range before calculating any powers of two. */
    if (size == 0)
    {
        allocatorerror("bcreate", "pool size must be greater than zero");
        return 0;
    }

    if (!validorder(l) || !validorder(u) || u < l)
    {
        allocatorerror("bcreate", "invalid block-order range; require supported orders with l <= u");
        return 0;
    }

    minimum = e2size(l);
    /* A free block must be large enough to store its next-list pointer. */
    if (minimum < sizeof(void *) || size < minimum)
    {
        allocatorerror("bcreate", "pool is smaller than the minimum usable block");
        return 0;
    }

    /* Map the private allocator record first; it owns the later mappings. */
    data = mmalloc(sizeof(*data));
    if (data == MAP_FAILED)
    {
        allocatorerror("bcreate", "could not map allocator metadata");
        return 0;
    }

    /* Map every byte of the managed pool during creation. */
    data->base = mmalloc(size);
    if (data->base == MAP_FAILED)
    {
        allocatorerror("bcreate", "could not map the memory pool");
        mmfree(data, sizeof(*data));
        return 0;
    }

    /* Create every per-order list and bitmap before returning the allocator. */
    data->freelist = freelistcreate(size, l, u);
    if (data->freelist == 0)
    {
        allocatorerror("bcreate",
                       "could not allocate free-list metadata and per-order bitmaps");
        mmfree(data->base, size);
        mmfree(data, sizeof(*data));
        return 0;
    }

    data->size = size;
    data->l = l;
    data->u = u;

    /*
     * Seed the empty lists with a binary partition of the pool.  At each
     * offset, choose the largest allowed block that fits in the remaining
     * bytes.  Because blocks are chosen largest first, every offset remains
     * aligned relative to base.  Bytes smaller than the minimum block remain
     * mapped but cannot be allocated.
     */
    offset = 0;
    while ((size_t)size - offset >= minimum)
    {
        int order = u;
        size_t blocksize = e2size(order);

        while (order > l && blocksize > (size_t)size - offset)
        {
            order--;
            blocksize = e2size(order);
        }

        freelistfree(data->freelist, data->base,
                     (char *)data->base + offset, order, data->l);
        offset += blocksize;
    }

    return data;
}

extern void bdelete(Balloc pool)
{
    BallocData *data = pool;

    if (data == 0 || data == MAP_FAILED)
        return;

    /* Delete bookkeeping first because it still refers to the pool bounds. */
    freelistdelete(data->freelist, data->l, data->u);
    /* The pool and private allocator record are separate mmap allocations. */
    mmfree(data->base, data->size);
    mmfree(data, sizeof(*data));
}

extern void *balloc(Balloc pool, unsigned int size)
{
    BallocData *data = pool;
    int order;

    if (data == 0 || data == MAP_FAILED)
    {
        allocatorerror("balloc", "invalid allocator handle");
        return 0;
    }

    if (size == 0)
    {
        allocatorerror("balloc", "request size must be greater than zero");
        return 0;
    }

    /* Round the byte request up to a power-of-two buddy block order. */
    order = size2e(size);
    if (order < 0 || order > data->u)
    {
        allocatorerror("balloc", "request exceeds the maximum block size");
        return 0;
    }

    if (order < data->l)
        order = data->l;

    void *block = freelistalloc(data->freelist, data->base, order, data->l);
    if (block == 0)
        allocatorerror("balloc", "no free block can satisfy the request");

    return block;
}

extern void bfree(Balloc pool, void *mem)
{
    BallocData *data = pool;
    int order;

    if (data == 0 || data == MAP_FAILED || mem == 0)
        return;

    if (!inpool(data, mem))
    {
        allocatorerror("bfree", "pointer is outside this allocator's pool");
        return;
    }

    /* Recover the actual order; callers do not need to supply an allocation size. */
    order = freelistsize(data->freelist, data->base, mem, data->l, data->u);
    if (order < data->l)
    {
        allocatorerror("bfree", "pointer is not the start of a live allocation");
        return;
    }

    if (((uintptr_t)mem - (uintptr_t)data->base) % e2size(order) != 0)
    {
        allocatorerror("bfree", "pointer is not aligned to its allocation size");
        return;
    }

    /* Return the block at its actual order so it can merge with its buddy. */
    freelistfree(data->freelist, data->base, mem, order, data->l);
}

extern unsigned int bsize(Balloc pool, void *mem)
{
    BallocData *data = pool;
    int order;

    if (data == 0 || data == MAP_FAILED || mem == 0)
        return 0;

    if (!inpool(data, mem))
    {
        allocatorerror("bsize", "pointer is outside this allocator's pool");
        return 0;
    }

    /* Use the same bitmap lookup as bfree to find the allocation's order. */
    order = freelistsize(data->freelist, data->base, mem, data->l, data->u);
    if (order < data->l)
    {
        allocatorerror("bsize", "pointer is not the start of a live allocation");
        return 0;
    }

    if (((uintptr_t)mem - (uintptr_t)data->base) % e2size(order) != 0)
    {
        allocatorerror("bsize", "pointer is not aligned to its allocation size");
        return 0;
    }

    return (unsigned int)e2size(order);
}

extern void bprint(Balloc pool)
{
    BallocData *data = pool;

    if (data == 0 || data == MAP_FAILED)
    {
        allocatorerror("bprint", "invalid allocator handle");
        return;
    }

    /* Print the allocator's pool information, then its existing freelists. */
    printf("pool base: %p\n", data->base);
    printf("pool size: %zu\n", data->size);
    printf("orders: %d through %d\n", data->l, data->u);
    freelistprint(data->freelist, data->l, data->u);
}
