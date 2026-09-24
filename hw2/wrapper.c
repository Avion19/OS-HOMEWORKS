/*
 * Author: Avien Ramirez
 * Date: September 23, 2026
 * Class: CS 452-2
 * Description: Routes malloc() and free() through the buddy allocator.
 */

#include <string.h>

#include "balloc.h"

static Balloc bp=0;

#include <stdio.h>

/* Lazily create the fixed-size pool, then serve the request from that pool. */
extern void *malloc(size_t size) {
  bp=bp ? bp : bcreate(4096,4,12);
  return balloc(bp,size);
}

/* Return the block to the allocator that owns it. */
extern void free(void *ptr) {
  bfree(bp,ptr);
}

/* Resize by allocating first, copying the old contents, then freeing the old block. */
extern void *realloc(void *ptr, size_t size) {
  size_t min(size_t x, size_t y) { return x<y ? x : y; }
  void *new=malloc(size);
  if (!ptr)
    return new;
  memcpy(new,ptr,min(size,bsize(bp,ptr)));
  free(ptr);
  return new;
}
