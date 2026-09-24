/*
 * Author: Avien Ramirez
 * Date: September 23, 2026
 * Class: CS 452-2
 * Description: Implements buddy-system free lists, allocation, and coalescing.
 */

#include <sys/mman.h>
#include <stdint.h>

#include "freelist.h"
#include "bbm.h"
#include "utils.h"

/*
 * Terminology used in this file:
 *
 * - A FreeList is the opaque handle for the complete OrderList array.  It
 *   points directly to that array's first element, orders[0].
 * - orders is the ascending OrderList array for one allocator.
 * - An OrderList is one array element: the list head and bitmap for one
 *   particular block order.
 * - A block is a managed memory block.  A buddy is that block's other half.
 *
 * There is one OrderList for every usable block order.  The orders array is
 * always ascending from the minimum order l to the maximum order u; e is
 * only an order being requested or processed.  Their positions correspond to:
 *
 *     orders[0]       -> order l
 *     orders[1]       -> order l + 1
 *     orders[e - l]   -> requested/current order e
 *
 * A free block uses its first word to store the next free block's address.
 * Therefore, an OrderList's head points directly to a block; there are no
 * separately allocated list nodes.
 *
 * The OrderLists for orders l through u are usable.  A final sentinel slot
 * has a zero-valued bitmap; it marks the end of orders but is not a valid order.
 * Because no private header stores the bounds, callers that convert an order
 * to an array index must pass the l value used when the FreeList was created.
 * The sentinel supplies the upper boundary while traversing the array.
 */
typedef struct
{
    BBM bitmap;  /* One buddy-pair bitmap for this block order. */
    void *head;  /* First free block of this order, or 0. */
} OrderList;

/* Put a free block at the front of one order's in-block linked list. */
static void freelistinsert(OrderList *list, void *block)
{
    /* Store the old head in the newly free block's first word. */
    *(void **)block = list->head;
    /* The newly free block becomes this order's new head. */
    list->head = block;
}

/*
 * Remove block from list if it is present.  This searches only free blocks,
 * so it never reads the first word of a block owned by an allocator client.
 */
static int freelistremove(OrderList *list, void *block)
{
    /* link points to the list head, then to each free block's next pointer. */
    void **link = &list->head;

    while (*link != 0)
    {
        if (*link == block)
        {
            /* Skip this free block by linking to the following free block. */
            *link = *(void **)*link;
            return 1;
        }

        /* Advance to the next-pointer stored in the current free block. */
        link = (void **)*link;
    }

    return 0;
}

/* Test list membership without modifying its in-block links. */
static int freelisthas(OrderList *list, void *block)
{
    /* Start at this order's first free block. */
    void *current = list->head;

    while (current != 0)
    {
        if (current == block)
            return 1;

        /* Move through the next pointer stored in the current free block. */
        current = *(void **)current;
    }

    return 0;
}

/*
 * Allocate metadata for orders l through u for a pool containing `size`
 * managed bytes.  `size` describes the pool itself and is passed to BBM so
 * each order gets enough buddy-pair bits; it is unrelated to the allocation
 * size of the OrderList-array allocation below.
 *
 * The free lists begin empty.  This function receives no pool base address,
 * so it cannot safely place the initial pool blocks on a list.  The caller
 * that owns the pool must do that after this metadata has been created.  The
 * allocation also includes a trailing sentinel so an upward search can stop
 * without treating the real order-u list as special.
 */
extern FreeList freelistcreate(size_t size, int l, int u)
{
    /* The order range must be non-empty and orders must be non-negative. */
    if (size == 0 || l < 0 || u < l)
        return 0;

    /* One OrderList for every usable order, plus one sentinel slot. */
    size_t ordercount = (size_t)u - (size_t)l + 1;
    size_t arraycount = ordercount + 1;
    if (arraycount <= ordercount ||
        arraycount > (size_t)-1 / sizeof(OrderList))
        return 0;

    /* The opaque handle returned below points directly at orders[0]. */
    OrderList *orders = mmalloc(sizeof(*orders) * arraycount);
    if (orders == MAP_FAILED)
        return 0;

    /* Each array position i represents the order l + i. */
    /* Create one empty list and one bitmap for every real order. */
    for (size_t i = 0; i < ordercount; i++)
    {
        orders[i].head = 0;
        orders[i].bitmap = bbmcreate(size, l + (int)i);
        if (orders[i].bitmap == 0)
        {
            /* Release only the bitmaps that were created before this failure. */
            for (size_t j = 0; j < i; j++)
                bbmdelete(orders[j].bitmap);

            mmfree(orders, sizeof(*orders) * arraycount);
            return 0;
        }
    }

    /* The sentinel is not an order; it has no blocks and no buddy bitmap. */
    orders[ordercount].head = 0;
    orders[ordercount].bitmap = 0;

    /* No memory block is put on a list here because base is not available. */
    return orders;
}

extern void freelistdelete(FreeList f, int l, int u)
{
    OrderList *orders = f;

    /* Without a private header, deletion relies on the caller's original bounds. */
    if (orders == 0 || orders == MAP_FAILED || l < 0 || u < l)
        return;

    /* Include the sentinel when calculating the size of the original mapping. */
    size_t ordercount = (size_t)u - (size_t)l + 1;
    size_t arraycount = ordercount + 1;
    if (arraycount <= ordercount ||
        arraycount > (size_t)-1 / sizeof(OrderList))
        return;

    /* Every real order has a bitmap; the trailing sentinel does not. */
    for (size_t i = 0; i < ordercount; i++)
        bbmdelete(orders[i].bitmap);

    mmfree(orders, sizeof(*orders) * arraycount);
}

extern void *freelistalloc(FreeList f, void *base, int e, int l)
{
    OrderList *orders = f;
    OrderList *list;
    void *block;
    int order;

    /* The caller must provide the minimum order used to create this list. */
    if (orders == 0 || base == 0)
        return 0;

    /* balloc() has already guaranteed l <= e <= u for this FreeList. */
    list = &orders[e - l];

    order = e;
    //go up an order until we find an order's freelist that isn't empty
    while (list->head == 0)
    {
        /* The following array slot is the sentinel, so no larger block exists. */
        if ((list + 1)->bitmap == 0)
            return 0;

        /* Move upward through orders until a larger free block is found. */
        list++;
        order++;
    }

   
    /* Remove the first free block from the selected order's list. */
    block = list->head;
    list->head = *(void **)block;
    /* The block is becoming client-owned; discard its old free-list link. */
    *(void **)block = 0;
    bbmset(list->bitmap, base, block, order);

    /*
     * Split until block has the requested order.  Keep the lower half for
     * the caller and link the upper half into the next smaller free list.
     * The retained half is unavailable at every newly created child order.
     */
    while (order > e)
    {
        void *right;

        /* A split creates two blocks at the next smaller order. */
        order--;
        list--;
        /* Keep the lower half and return the upper half to the free list. */
        right = (char *)block + e2size(order);

        bbmset(list->bitmap, base, block, order);
        freelistinsert(list, right);
    }

    return block;
}

extern void freelistfree(FreeList f, void *base, void *mem, int e, int l)
{
    OrderList *orders = f;
    OrderList *list;
    int order = e;

    /* The caller must supply the minimum order used to create this list. */
    if (orders == 0 || base == 0 || mem == 0)
        return;

    /* bcreate() and bfree() pass an order in the configured range. */
    list = &orders[order - l];

    /*
     * Merge only with a buddy that is actually on this order's free list.
     * The occupancy bit cannot distinguish one unavailable buddy from two,
     * so it is not enough to decide whether a merge is safe.  The loop stops
     * when no free buddy exists or when this is the largest supported order.
     */
    for (;;)
    {
        /* The buddy is the same-sized block on the other side of this pair. */
        void *buddy = baddrinv(base, mem, order);

        /* Order u has no larger list.  Keep both top-order buddies free. */
        if ((list + 1)->bitmap == 0)
        {
            if (freelisthas(list, buddy))
                bbmclr(list->bitmap, base, mem, order);

            freelistinsert(list, mem);
            return;
        }

        if (!freelistremove(list, buddy))
        {
            /* The buddy is allocated, split, or outside the managed pool. */
            freelistinsert(list, mem);
            return;
        }

        /* Both buddies are free at this order, so their pair is clear. */
        bbmclr(list->bitmap, base, mem, order);
        /* The merged block starts at the lower buddy address. */
        mem = baddrclr(base, mem, order);
        /* Continue with the next larger order and its next array position. */
        order++;
        list++;
    }
}

extern int freelistsize(FreeList f, void *base, void *mem, int l, int u)
{
    OrderList *list = f;
    uintptr_t address = (uintptr_t)mem;

    if (list == 0 || base == 0 || mem == 0 || l < 0 || u < l)
        return -1;

    /*
     * A bitmap bit says that at least one block in a buddy pair is
     * unavailable; it cannot tell whether this particular block is still
     * allocated. Check every free list first, including larger orders where
     * this address may now be part of a coalesced free block. This rejects
     * both duplicate frees and bsize() queries on freed memory.
     */
    OrderList *free_list = list;
    for (int e = l; e <= u && free_list->bitmap != 0; e++, free_list++)
    {
        size_t block_size = e2size(e);

        for (void *block = free_list->head; block != 0;
             block = *(void **)block)
        {
            uintptr_t start = (uintptr_t)block;

            if (address >= start && address - start < block_size)
                return -1;
        }
    }

    /*
     * The lowest set bit is the allocation's order.  Higher-order bits can
     * also be set because this block was obtained by splitting a parent.
     */
    for (int e = l; e <= u && list->bitmap != 0; e++, list++)
    {
        /* Check the pair bit for mem at one order, starting at the smallest. */
        if (bbmtst(list->bitmap, base, mem, e))
            return e;
    }

    return -1;
}

extern void freelistprint(FreeList f, int l, int u)
{
    OrderList *list = f;

    if (list == 0 || list == MAP_FAILED || l < 0 || u < l)
        return;

    /*
     * This is read-only: it follows links already stored in free blocks and
     * prints each already-created bitmap.  It does not allocate any memory.
     */
    for (int e = l; e <= u && list->bitmap != 0; e++, list++)
    {
        /* A list head is either the first free block or 0 for an empty list. */
        void *block = list->head;

        printf("order %d free blocks:", e);
        while (block != 0)
        {
            printf(" %p", block);
            /* Free blocks store the address of the next free block first. */
            block = *(void **)block;
        }

        /* Print this order's existing bitmap after its free-block addresses. */
        printf("\norder %d bitmap: ", e);
        bbmprt(list->bitmap);
    }
}
