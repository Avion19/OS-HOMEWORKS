/*
 * Author: Avien Ramirez
 * Date: September 23, 2026
 * Class: CS 452-2
 * Description: Implements buddy bitmap operations and buddy address helpers.
 */

#include "bbm.h"
#include "bm.h"
#include "utils.h"

/* Each bitmap position describes one pair of same-order buddy blocks. */

static size_t mapsize(size_t size, int e) {
  /* Count order-e blocks, then round up again to count their pairs. */
  size_t blocksize=e2size(e);
  size_t blocks=divup(size,blocksize);
  size_t buddies=divup(blocks,2);
  return buddies;
}

/*
 * Return the pair index for mem.  First normalize mem to the left block of
 * its pair; consecutive left/right blocks then share one index.
 */
static size_t bitaddr(void *base, void *mem, int e) {
  size_t addr=baddrclr(base,mem,e)-base;
  size_t blocksize=e2size(e);
  return addr/blocksize/2;
}

/* Create occupancy bits for every order-e buddy pair in the region. */
extern BBM bbmcreate(size_t size, int e) {
  return bmcreate(mapsize(size,e));
}

/* Destroy the underlying general-purpose bitmap. */
extern void bbmdelete(BBM b) {
  bmdelete(b);
}

/* Mark this pair as containing one or more order-e allocations. */
extern void bbmset(BBM b, void *base, void *mem, int e) {
  bmset(b,bitaddr(base,mem,e));
}

/* Mark this pair as containing no order-e allocations. */
extern void bbmclr(BBM b, void *base, void *mem, int e) {
  bmclr(b,bitaddr(base,mem,e));
}

/* Test whether this pair contains any order-e allocation. */
extern int bbmtst(BBM b, void *base, void *mem, int e) {
  return bmtst(b,bitaddr(base,mem,e));
}

/* Print the underlying occupancy bits for debugging. */
extern void bbmprt(BBM b) { bmprt(b); }

/* Set offset bit e to select the higher-addressed buddy. */
extern void *baddrset(void *base, void *mem, int e) {
  unsigned int mask=1<<e;
  return base+((mem-base)|mask);
}

/* Clear offset bit e to select the lower-addressed buddy. */
extern void *baddrclr(void *base, void *mem, int e) {
  unsigned int mask=~(1<<e);
  return base+((mem-base)&mask);
}

/* Toggle offset bit e to select the other buddy. */
extern void *baddrinv(void *base, void *mem, int e) {
  unsigned int mask=1<<e;
  return base+((mem-base)^mask);
}

/* A clear offset bit e denotes the left buddy; a set bit denotes the right. */
extern int baddrtst(void *base, void *mem, int e) {
  unsigned int mask=1<<e;
  return (mem-base)&mask;
}
