/*
 * Author: Avien Ramirez
 * Date: September 2, 2026
 * Class: CS 452-1
 * Description: Public interface for a double-ended doubly-linked deque.
 */

#ifndef DEQ_H
#define DEQ_H

typedef void *Deq;
typedef void *Data;

/**
 * Creates an empty deque.
 *
 * @return a new deque handle; an allocation failure causes an error
 */
extern Deq deq_new();

/**
 * Returns the number of data items in a deque.
 *
 * @param q deque to inspect
 * @return number of stored items
 */
extern int deq_len(Deq q);

/**
 * Stores the caller's data pointer at the head. The data is not copied.
 * A null data pointer causes an error.
 *
 * @param q deque to modify
 * @param d data pointer to store
 */
extern void deq_head_put(Deq q, Data d);

/**
 * Removes the data pointer at the head.
 *
 * @param q deque to modify
 * @return removed data pointer, owned by the caller, or 0 if q is empty
 */
extern Data deq_head_get(Deq q);

/**
 * Returns the data pointer at a zero-based index measured from the head.
 *
 * @param q deque to inspect
 * @param i zero-based index
 * @return data pointer at index i, or 0 if i is invalid
 */
extern Data deq_head_ith(Deq q, int i);

/**
 * Removes the first data pointer equal to d, searching from the head.
 *
 * @param q deque to modify
 * @param d data pointer to find
 * @return removed data pointer, owned by the caller, or 0 if d is not found
 *         or is 0
 */
extern Data deq_head_rem(Deq q, Data d);

/**
 * Stores the caller's data pointer at the tail. The data is not copied.
 * A null data pointer causes an error.
 *
 * @param q deque to modify
 * @param d data pointer to store
 */
extern void deq_tail_put(Deq q, Data d);

/**
 * Removes the data pointer at the tail.
 *
 * @param q deque to modify
 * @return removed data pointer, owned by the caller, or 0 if q is empty
 */
extern Data deq_tail_get(Deq q);

/**
 * Returns the data pointer at a zero-based index measured from the tail.
 *
 * @param q deque to inspect
 * @param i zero-based index
 * @return data pointer at index i, or 0 if i is invalid
 */
extern Data deq_tail_ith(Deq q, int i);

/**
 * Removes the first data pointer equal to d, searching from the tail.
 *
 * @param q deque to modify
 * @param d data pointer to find
 * @return removed data pointer, owned by the caller, or 0 if d is not found
 *         or is 0
 */
extern Data deq_tail_rem(Deq q, Data d);

typedef char *Str;
typedef void (*DeqMapF)(Data d);
typedef Str  (*DeqStrF)(Data d);

/**
 * Applies a function to every stored data pointer from head to tail.
 *
 * @param q deque to traverse
 * @param f function called once for each stored data pointer
 * @return no value
 */
extern void deq_map(Deq q, DeqMapF f);

/**
 * Deletes every node in q and then deletes q itself. If f is not null,
 * it is called on each stored data pointer before the nodes are freed.
 *
 * @param q deque to delete
 * @param f optional function used to release stored data
 * @return no value
 */
extern void deq_del(Deq q, DeqMapF f);

/**
 * Builds a string representation of q from head to tail. If f is provided,
 * it converts each data pointer into a newly allocated string, which this
 * function frees after using it.
 *
 * @param q deque to convert
 * @param f optional data-to-string conversion function
 * @return newly allocated string representation; the caller must free it
 */
extern Str deq_str(Deq q, DeqStrF f);

#endif
