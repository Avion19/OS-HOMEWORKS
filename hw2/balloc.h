/*
 * Author: Avien Ramirez
 * Date: September 23, 2026
 * Class: CS 452-2
 * Description: Declares the public buddy allocator interface.
 */

#ifndef BALLOC_H
#define BALLOC_H

typedef void *Balloc;

/**
 * Creates and returns a new allocator.
 *
 * The smallest allocation size is 2^l bytes, and the largest allocation
 * size is 2^u bytes. The allocator manages a total of `size` bytes.
 *
 * @param size Total number of bytes managed by the allocator.
 * @param l    Exponent defining the smallest allocation size.
 * @param u    Exponent defining the largest allocation size.
 * @return A handle to the newly created allocator.
 */
extern Balloc bcreate(unsigned int size, int l, int u);

/**
 * Deletes an allocator.
 *
 * @param pool Allocator to delete.
 */
extern void   bdelete(Balloc pool);

/**
 * Requests a block of memory from an allocator.
 *
 * @param pool Allocator from which to request memory.
 * @param size Number of bytes requested.
 * @return A pointer to the allocated memory block.
 */
extern void *balloc(Balloc pool, unsigned int size);

/**
 * Deallocates a block of memory.
 *
 * @param pool Allocator that owns the memory block.
 * @param mem  Memory block to deallocate.
 */
extern void  bfree(Balloc pool, void *mem);

/**
 * Returns the size of an allocation.
 *
 * The returned size is the actual allocation size, not the original
 * requested size.
 *
 * @param pool Allocator that owns the allocation.
 * @param mem  Memory block whose allocation size is requested.
 * @return Actual size of the allocation in bytes.
 */
extern unsigned int bsize(Balloc pool, void *mem);

/**
 * Prints a textual representation of an allocator to standard output.
 *
 * @param pool Allocator to print.
 */
extern void bprint(Balloc pool);

#endif
