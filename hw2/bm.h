/*
 * Author: Avien Ramirez
 * Date: September 23, 2026
 * Class: CS 452-2
 * Description: Declares the general-purpose bitmap interface.
 */

/**
 * A general-purpose, zero-initialized bitmap.
 *
 * BM has no knowledge of buddy allocation or block addresses.  It provides
 * exactly `bits` zero-based bit positions; clients give those positions their
 * own meaning.  The number of bits is retained privately so bmdelete() can
 * release the mapping created by bmcreate().
 */

#ifndef BM_H
#define BM_H

#include <stdio.h>

/** Opaque handle to a bitmap created by bmcreate(). */
typedef void *BM;

/**
 * Creates a bitmap containing `bits` usable bit positions.
 *
 * Every bit is initially clear.  Returns 0 when its backing mapping cannot
 * be created.
 */
extern BM   bmcreate(size_t bits);

/**
 * Deletes a bitmap created by bmcreate().
 *
 * The argument must be a valid, nonzero bitmap handle that has not already
 * been deleted.
 */
extern void bmdelete(BM b);

/** Sets bit `i`.  `i` must be less than the bitmap's bit count. */
extern void bmset(BM b, size_t i);

/** Clears bit `i`.  `i` must be less than the bitmap's bit count. */
extern void bmclr(BM b, size_t i);

/**
 * Tests bit `i`.  `i` must be less than the bitmap's bit count.
 *
 * Returns zero for a clear bit and non-zero for a set bit.  An out-of-range
 * index is treated as a programming error: this module prints an error and
 * terminates the process.
 */
extern int  bmtst(BM b, size_t i);

/**
 * Prints the bitmap's backing bytes from the highest-addressed byte to the
 * lowest-addressed byte.  This is a raw debugging view, not a buddy bitmap
 * interpretation.
 */
extern void bmprt(BM b);

#endif
