/*
 * Author: Avien Ramirez
 * Date: September 23, 2026
 * Class: CS 452-2
 * Description: Implements memory mapping, bit, and size utility functions.
 */

#include "utils.h"
#include <math.h>
#include <sys/mman.h>

/** @copydoc mmalloc */
extern void *mmalloc(size_t size) {
    return mmap(0,size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
}

/** @copydoc mmfree */
extern void mmfree(void *p, size_t size) {
    if (p == 0 || p == MAP_FAILED) return;
    munmap(p,size);
}

/** @copydoc divup */
extern size_t divup(size_t n, size_t d) {
    if (d == 0)
        return 0;  

    return n / d + (n % d != 0);
}

/** @copydoc bits2bytes */
extern size_t bits2bytes(size_t bits) {
    return  divup(bits,bitsperbyte);
}

/** @copydoc e2size */
extern size_t e2size(int e) {
    return (size_t)pow(2, e);
}

/** @copydoc size2e */
extern int size2e(size_t size) {
    return (int)ceil(log2(size));
}

/** @copydoc bitset */
extern void bitset(void *p, int bit) {
    unsigned char *byte = p;
    *byte |= (unsigned char)(1u << bit);
}

/** @copydoc bitclr */
extern void bitclr(void *p, int bit) {
    unsigned char *byte = p;
    *byte &= (unsigned char)~(1u << bit);
}

/** @copydoc bitinv */
extern void bitinv(void *p, int bit) {
    unsigned char *byte = p;
    *byte ^= (unsigned char)(1u << bit);
}

/** @copydoc bittst */
extern int bittst(void *p, int bit) {
    unsigned char *byte = p;
    return *byte & (unsigned char)(1u << bit);
}
