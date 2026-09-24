/*
 * Author: Avien Ramirez
 * Date: September 23, 2026
 * Class: CS 452-2
 * Description: Declares buddy bitmap operations and buddy address helpers.
 */

/**
 * A buddy-system bitmap.
 *
 * Each bit represents one pair of buddy blocks at a particular order.  A
 * clear bit means neither block in that pair is allocated from that order's
 * free list.  A set bit means that one or both blocks in the pair are
 * allocated from that order's free list.  This is an occupancy bitmap, not
 * an XOR or "different states" bitmap.
 *
 * The bitmap does not contain block addresses.  The base address, a block
 * address, and the order identify the pair whose bit is to be used.
 */

#ifndef BBM_H
#define BBM_H

#include <stdio.h>

typedef void *BBM;

/**
 * Creates a buddy bitmap for a memory region.
 *
 * The bitmap contains one bit for each pair of buddy blocks of size 2^e
 * bytes.  The number of bits is rounded up when size is not an exact
 * multiple of the block size or when the number of blocks is odd.  Bits for
 * a final partial pair only provide bookkeeping space; they do not make
 * bytes outside the managed region usable.
 *
 * @param size Number of bytes in the managed memory region.
 * @param e    Order of the buddy blocks; each block is 2^e bytes.
 * @return A newly created buddy bitmap, or 0 if bitmap allocation fails.
 */
extern BBM  bbmcreate(size_t size, int e);

/**
 * Deletes a buddy bitmap.
 *
 * @param b Bitmap returned by bbmcreate().
 */
extern void bbmdelete(BBM b);

/**
 * Records that the buddy pair containing mem has an allocation at order e.
 *
 * After this operation the pair bit is set, meaning one or both buddies are
 * allocated from the order-e free list.
 *
 * @param b    Buddy bitmap to modify.
 * @param base Starting address of the managed memory region.
 * @param mem  Address of either block in the buddy pair.
 * @param e    Order of the blocks; each block is 2^e bytes.
 */
extern void bbmset(BBM b, void *base, void *mem, int e);

/**
 * Records that the buddy pair containing mem has no order-e allocations.
 *
 * Callers must clear this bit only after determining that both buddies in the
 * pair are free.  Clearing it merely changes the bitmap; it does not inspect
 * the free list.
 *
 * @param b    Buddy bitmap to modify.
 * @param base Starting address of the managed memory region.
 * @param mem  Address of either block in the buddy pair.
 * @param e    Order of the blocks; each block is 2^e bytes.
 */
extern void bbmclr(BBM b, void *base, void *mem, int e);

/**
 * Tests the bit representing the buddy pair containing mem.
 *
 * @param b    Buddy bitmap to inspect.
 * @param base Starting address of the managed memory region.
 * @param mem  Address of either block in the buddy pair.
 * @param e    Order of the blocks; each block is 2^e bytes.
 * @return Zero when neither buddy in the pair is allocated at order e, or
 *         non-zero when one or both buddies are allocated at that order.
 */
extern  int bbmtst(BBM b, void *base, void *mem, int e);

/**
 * Prints the buddy bitmap.
 *
 * @param b Buddy bitmap to print.
 */
extern void bbmprt(BBM b);

/**
 * Returns the address of the right buddy.
 *
 * The right buddy is the higher-addressed block in the pair.  This sets bit
 * e in mem's offset from base, leaving the other offset bits unchanged.
 *
 * @param base Starting address of the managed memory region.
 * @param mem  Address of an order-e block in the managed region.
 * @param e    Order of the blocks; each block is 2^e bytes.
 * @return Address of the right buddy.
 */
extern void *baddrset(void *base, void *mem, int e);

/**
 * Returns the address of the left buddy.
 *
 * The left buddy is the lower-addressed block in the pair.  This clears bit
 * e in mem's offset from base, leaving the other offset bits unchanged.
 *
 * @param base Starting address of the managed memory region.
 * @param mem  Address of an order-e block in the managed region.
 * @param e    Order of the blocks; each block is 2^e bytes.
 * @return Address of the left buddy.
 */
extern void *baddrclr(void *base, void *mem, int e);

/**
 * Returns the address of the other buddy.
 *
 * This toggles bit e in mem's offset from base.  If mem is the left buddy,
 * the returned address is the right buddy; if mem is the right buddy, the
 * returned address is the left buddy.
 *
 * @param base Starting address of the managed memory region.
 * @param mem  Address of an order-e block in the managed region.
 * @param e    Order of the blocks; each block is 2^e bytes.
 * @return Address of the other buddy.
 */
extern void *baddrinv(void *base, void *mem, int e);

/**
 * Tests whether mem is the left or right buddy.
 *
 * The e-th bit of mem's offset from base identifies the side of the pair.
 * A zero result means mem is the left buddy; a non-zero result means mem is
 * the right buddy.  For a right buddy, the returned value is the mask value
 * 2^e rather than necessarily the integer 1.
 *
 * @param base Starting address of the managed memory region.
 * @param mem  Address of an order-e block in the managed region.
 * @param e    Order of the blocks; each block is 2^e bytes.
 * @return Zero for the left buddy, or a non-zero value for the right buddy.
 */
extern int   baddrtst(void *base, void *mem, int e);

#endif
