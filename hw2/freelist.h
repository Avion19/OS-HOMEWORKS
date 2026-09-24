/*
 * Author: Avien Ramirez
 * Date: September 23, 2026
 * Class: CS 452-2
 * Description: Declares the buddy-system free-list interface.
 */

#ifndef FREELIST_H
#define FREELIST_H

#include <stdio.h>

/** Opaque handle for a sentinel-terminated array of buddy free-list records. */
typedef void *FreeList;

/**
 * Creates the bookkeeping for a set of buddy-system free lists.
 *
 * `size` is the total number of bytes in the managed memory pool.  It is not
 * an allocation request, the size of this metadata object, or the maximum
 * block size.  It is used to determine how many buddy pairs must be tracked
 * at each block order.  The smallest order is `l` (blocks of 2^l bytes) and
 * the largest usable order is `u` (blocks of 2^u bytes).
 *
 * This function creates empty list and bitmap bookkeeping only.  It has no
 * `base` address for the managed pool, so it cannot place any pool block on a
 * free list; the caller that owns the pool must initialize those blocks after
 * creation.  Internally, the handle points directly to an ascending per-order
 * metadata array, from l through u.  A trailing sentinel with a zero-valued bitmap
 * follows the order-u entry, allowing operations to find the upper boundary.
 * The handle does not store l or u.  Calls that turn an order into an array
 * index must use the same l supplied at creation; traversal operations use
 * the sentinel to find the upper boundary.
 *
 * @param size Total number of bytes managed by the allocator.
 * @param l    Smallest block order.
 * @param u    Largest usable block order.
 * @return A free-list handle, or 0 if validation or metadata allocation
 *         fails.
 */
extern FreeList freelistcreate(size_t size, int l, int u);

/**
 * Releases all bitmaps and metadata owned by a free-list handle.
 *
 * The l and u parameters must be the values supplied to freelistcreate(), so
 * this function can delete the correct number of bitmaps and unmap the exact
 * array size.  The managed pool is not released here because its mapping is
 * owned by the allocator object.  Passing 0, MAP_FAILED, or invalid bounds
 * has no effect.
 *
 * @param f Free-list handle returned by freelistcreate().
 * @param l Smallest block order supplied at creation.
 * @param u Largest block order supplied at creation.
 */
extern void     freelistdelete(FreeList f, int l, int u);

/**
 * Allocates one order-e block from a buddy free list.
 *
 * The function first searches order e and larger orders.  If a larger block
 * is found, it is split repeatedly; the lower-addressed half is returned and
 * each upper half is placed on the appropriate smaller-order free list.  A
 * pair bitmap bit is set whenever a block is removed because a set bit means
 * one or both buddies in that pair are unavailable.
 *
 * @param f    Free-list bookkeeping created by freelistcreate().
 * @param base Starting address of the managed memory pool.
 * @param e    Requested block order; must be in f's configured range.
 * @param l    Smallest block order supplied when f was created.
 * @return A block of 2^e bytes, or 0 if no suitable free block exists.
 */
extern void *freelistalloc(FreeList f, void *base, int e, int l);

/**
 * Returns an order-e block and coalesces it with free buddies when possible.
 *
 * A buddy is merged only when its exact address appears on the same-order
 * free list.  This preserves client memory (only free blocks contain list
 * links) and avoids treating a set occupancy bit as proof that a buddy is
 * allocated.  The final merged block is added to one free list.
 *
 * This operation may also be used while initializing a pool: pass an
 * aligned, initially free block as mem.  It need not have been returned by
 * freelistalloc() in that case.
 *
 * @param f    Free-list bookkeeping created by freelistcreate().
 * @param base Starting address of the managed memory pool.
 * @param mem  Address of an aligned order-e block to return or initialize.
 * @param e    Actual block order; must be in f's configured range.
 * @param l    Smallest block order supplied when f was created.
 */
extern void  freelistfree(FreeList f, void *base, void *mem, int e, int l);

/**
 * Finds the actual order, and therefore the block size, of a live allocation.
 *
 * Free lists are checked first to reject addresses that belong to an
 * available block, including blocks that have coalesced to a larger order.
 * Then bitmap bits are examined from l upward.  The first set bit for mem is
 * its allocation order; bits at larger orders may remain set because its
 * source block was split.  mem must be a valid currently allocated block.
 *
 * @param f    Free-list bookkeeping created by freelistcreate().
 * @param base Starting address of the managed memory pool.
 * @param mem  Address of a currently allocated block.
 * @param l    Smallest block order supplied when f was created.
 * @param u    Largest block order supplied when f was created.
 * @return The allocation order, or -1 when the arguments are invalid or no
 *         allocation order can be found.
 */
extern int freelistsize(FreeList f, void *base, void *mem, int l, int u);

/**
 * Prints the free-list and bitmap state for debugging.
 *
 * Each order from l through u is displayed with the addresses of its free
 * blocks and its buddy bitmap.  The function is read-only and allocates no
 * memory; it traverses only links already stored in free blocks.
 *
 * @param f Free-list bookkeeping created by freelistcreate().
 * @param l Smallest block order for f.
 * @param u Largest block order for f.
 */
extern void freelistprint(FreeList f, int l, int u);

#endif
