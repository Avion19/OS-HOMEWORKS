/*
 * Author: Avien Ramirez
 * Date: September 23, 2026
 * Class: CS 452-2
 * Description: Declares memory mapping, bit, and size utility functions.
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

/** Number of bits in one byte. */
static const int bitsperbyte=8;

/**
 * Creates a private, read/write anonymous memory mapping.
 *
 * The returned mapping must be released with mmfree(), using the same size
 * that was passed to this function.  On failure, mmap()'s MAP_FAILED value is
 * returned.
 *
 * @param size Number of bytes to map.
 * @return Address of the new mapping, or MAP_FAILED on failure.
 */
extern void *mmalloc(size_t size);

/**
 * Releases a memory mapping created by mmalloc().
 *
 * Passing 0 is allowed and has no effect.  The size must match the size
 * used when the mapping was created.
 *
 * @param p    Address of the mapping to release.
 * @param size Number of bytes in the mapping.
 */
extern void mmfree(void *p, size_t size);

/**
 * Performs integer division rounded up.
 *
 * The divisor must be non-zero.
 *
 * @param n Dividend.
 * @param d Divisor.
 * @return ceil(n / d).
 */
extern size_t divup(size_t n, size_t d);

/**
 * Converts a number of bits to bytes using bitsperbyte.
 *
 * @param bits Number of bits.
 * @return The number of bytes represented by bits.
 */
extern size_t bits2bytes(size_t bits);

/**
 * Converts a base-two exponent to its corresponding size.
 *
 * @param e Base-two exponent.
 * @return 2 raised to the power e, or 0 if e is outside the size_t range.
 */
extern size_t e2size(int e);

/**
 * Converts a size to the smallest base-two exponent whose size is at least
 * the requested size.
 *
 * @param size Number of bytes.
 * @return ceil(log2(size)); returns 0 for sizes 0 and 1, or -1 if the
 *         required power of two cannot be represented as a size_t.
 */
extern int size2e(size_t size);

/**
 * Sets one bit in the storage unit pointed to by p.
 *
 * Bit numbering starts at zero for the least-significant bit.  The caller
 * must provide a valid storage unit and a bit index supported by that unit.
 *
 * @param p   Address of the storage unit to modify.
 * @param bit Zero-based bit index.
 */
extern void bitset(void *p, int bit);

/**
 * Clears one bit in the storage unit pointed to by p.
 *
 * @param p   Address of the storage unit to modify.
 * @param bit Zero-based bit index.
 */
extern void bitclr(void *p, int bit);

/**
 * Toggles one bit in the storage unit pointed to by p.
 *
 * @param p   Address of the storage unit to modify.
 * @param bit Zero-based bit index.
 */
extern void bitinv(void *p, int bit);

/**
 * Tests one bit in the storage unit pointed to by p.
 *
 * @param p   Address of the storage unit to inspect.
 * @param bit Zero-based bit index.
 * @return Non-zero if the bit is set; zero otherwise.
 */
extern int  bittst(void *p, int bit);

#endif
