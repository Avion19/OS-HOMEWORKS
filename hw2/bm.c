/*
 * Author: Avien Ramirez
 * Date: September 23, 2026
 * Class: CS 452-2
 * Description: Implements the general-purpose bitmap module.
 */

#include <stdlib.h>
#include <string.h>

#include "bm.h"
#include "utils.h"

/* The bit count is stored in the size_t immediately before the public BM. */
static size_t bmbits(BM b) { size_t *bits=b; return *--bits; }

/* Convert the stored bit count to the number of backing bytes. */
static size_t bmbytes(BM b) { return bits2bytes(bmbits(b)); }

/* Bitmap operations reject indices outside the range recorded at creation. */
static void ok(BM b, size_t i) {
  if (i<bmbits(b))
    return;
  fprintf(stderr,"bitmap index out of range\n");
  exit(1);
}         

extern BM bmcreate(size_t bits) {
  size_t bytes=bits2bytes(bits);
  /* The mapping layout is: | stored bit count | bitmap bytes |. */
  size_t *p=mmalloc(sizeof(size_t)+bytes);
  if ((long)p==-1)
    return 0;
  /* Preserve the bit count for bounds checks and later deletion. */
  *p=bits;
  /* Return a handle to the bytes, not to the private size field. */
  BM b=++p;
  memset(b,0,bytes);
  return b;
}

extern void bmdelete(BM b) {
  size_t *p=b;
  /* Step back from the public bytes to the private size field. */
  p--;
  mmfree(p,sizeof(size_t)+bits2bytes(*p));
}

extern void bmset(BM b, size_t i) {
  /* Select the containing byte, then set the bit within that byte. */
  ok(b,i); bitset(b+i/bitsperbyte,i%bitsperbyte);
}

extern void bmclr(BM b, size_t i) {
  /* Select the containing byte, then clear the bit within that byte. */
  ok(b,i); bitclr(b+i/bitsperbyte,i%bitsperbyte);
}

extern int bmtst(BM b, size_t i) {
  /* Select the containing byte, then test the bit within that byte. */
  ok(b,i); return bittst(b+i/bitsperbyte,i%bitsperbyte);
}

extern void bmprt(BM b) {
  for (int byte=bmbytes(b)-1; byte>=0; byte--)
    printf("%02x%s",((char *)b)[byte],(byte ? " " : "\n"));
}
