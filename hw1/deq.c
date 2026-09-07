/*
 * Author: Avien Ramirez
 * Date: September 2, 2026
 * Class: CS 452-1
 * Description: Implements a double-ended doubly-linked deque.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deq.h"
#include "error.h"

/** Identifies a deque endpoint and the corresponding node-link index. */
typedef enum
{
  Head,
  Tail,
  Ends
} End;

typedef struct Node
{
  struct Node *np[Ends]; /* links toward the head and tail neighbors */
  Data data;
} *Node;

typedef struct
{
  Node ht[Ends]; /* pointers to the head and tail nodes */
  int len;
} *Rep;

/**
 * Validates a public deque handle and converts it to the internal
 * representation.
 *
 * @param q deque handle to validate
 * @return the internal deque representation
 */
static Rep rep(Deq q)
{
  if (!q)
    ERROR("deque handle cannot be null");
  return (Rep)q;
}

/**
 * Adds data to the specified endpoint of the deque.
 *
 * @param r internal deque representation to modify
 * @param e endpoint at which to insert the data
 * @param d data pointer to insert
 * @return no value; the function exits with an error for null data or
 *         allocation failure
 */
static void put(Rep r, End e, Data d)
{
  if (!d)
    ERROR("put: data pointer cannot be null");

  Node added = malloc(sizeof *added);
  if (!added)
    ERROR("put: unable to allocate node");

  added->data = d;
  added->np[Head] = 0;
  added->np[Tail] = 0;

  /* For the first node, both deque endpoints refer to the new node. */
  if (r->ht[e] == 0)
  {
    r->ht[Head] = added;
    r->ht[Tail] = added;
  }
  else
  {
    End opp = (e == Head) ? Tail : Head;
    added->np[opp] = r->ht[e];
    r->ht[e]->np[e] = added;
    r->ht[e] = added;
  }

  r->len++;
}

/**
 * Returns data at a zero-based index measured from endpoint e.
 *
 * @param r internal deque representation to inspect
 * @param e endpoint from which to measure
 * @param i zero-based index
 * @return data pointer at i, or 0 if i is outside the deque
 */
static Data ith(Rep r, End e, int i)
{
  /* Invalid indexes do not modify the deque and return 0. */
  if (i < 0 || i >= r->len)
    return 0;

  /* Traverse toward the opposite endpoint from the selected endpoint. */
  End step = (e == Head) ? Tail : Head;
  Node curr = r->ht[e];

  /* Advance until the node at the requested index is reached. */
  for (int j = 0; j < i; j++)
  {
    curr = curr->np[step];
  }

  Data d = curr->data;
  /* Stored null data is treated as an unsuccessful lookup. */
  if (!d)
    return 0;
  return d;
}

/**
 * Removes and returns data at the specified endpoint.
 *
 * @param r internal deque representation to modify
 * @param e endpoint from which to remove
 * @return removed data pointer, or 0 if the deque is empty
 */
static Data get(Rep r, End e)
{
  /* Removing from an empty deque returns 0. */
  if (r->len == 0)
    return 0;

  Node removed = r->ht[e];
  Data data = removed->data;
  End opp = (e == Head) ? Tail : Head;

  r->ht[e] = removed->np[opp];

  if (r->len == 1)
    r->ht[opp] = 0;
  else
    r->ht[e]->np[e] = 0;

  free(removed);
  r->len--;

  if (!data)
    return 0;
  return data;
}

/**
 * Removes and returns the first node whose data pointer equals d.
 *
 * @param r internal deque representation to modify
 * @param e endpoint from which to search
 * @param d data pointer to find
 * @return removed data pointer, or 0 if d is not found or is 0
 */
static Data rem(Rep r, End e, Data d)
{
  if (!d || r->len == 0)
    return 0;

  /* Search toward the opposite endpoint from the selected endpoint. */
  End step = (e == Head) ? Tail : Head;
  Node curr = r->ht[e];

  while (curr != 0)
  {
    if (curr->data == d)
    {
      /* Endpoint matches use the shared endpoint-removal helper. */
      if (curr == r->ht[e])
      {
        return get(r, e);
      }
      Node prev = curr->np[Head];
      Node next = curr->np[Tail];
      if (prev)
        prev->np[Tail] = next;
      else
        r->ht[Head] = next;
      if (next)
        next->np[Head] = prev;
      else
        r->ht[Tail] = prev;
      curr->np[Head] = 0;
      curr->np[Tail] = 0;
      free(curr);
      r->len--;
      return d;
    }
    curr = curr->np[step];
  }
  return 0;
}

extern Deq deq_new()
{
  Rep r = (Rep)malloc(sizeof(*r));
  if (!r)
    ERROR("malloc() failed");
  r->ht[Head] = 0;
  r->ht[Tail] = 0;
  r->len = 0;
  return r;
}

extern int deq_len(Deq q) { return rep(q)->len; }

extern void deq_head_put(Deq q, Data d) { put(rep(q), Head, d); }
extern Data deq_head_get(Deq q) { return get(rep(q), Head); }
extern Data deq_head_ith(Deq q, int i) { return ith(rep(q), Head, i); }
extern Data deq_head_rem(Deq q, Data d) { return rem(rep(q), Head, d); }

extern void deq_tail_put(Deq q, Data d) { put(rep(q), Tail, d); }
extern Data deq_tail_get(Deq q) { return get(rep(q), Tail); }
extern Data deq_tail_ith(Deq q, int i) { return ith(rep(q), Tail, i); }
extern Data deq_tail_rem(Deq q, Data d) { return rem(rep(q), Tail, d); }

extern void deq_map(Deq q, DeqMapF f)
{
  for (Node n = rep(q)->ht[Head]; n; n = n->np[Tail])
    f(n->data);
}

extern void deq_del(Deq q, DeqMapF f)
{
  if (f)
    deq_map(q, f);
  Node curr = rep(q)->ht[Head];
  while (curr)
  {
    Node next = curr->np[Tail];
    free(curr);
    curr = next;
  }
  free(q);
}

extern Str deq_str(Deq q, DeqStrF f)
{
  char *s = strdup("");
  for (Node n = rep(q)->ht[Head]; n; n = n->np[Tail])
  {
    char *d = f ? f(n->data) : n->data;
    char *t;
    if (d)
      asprintf(&t, "%s%s%s", s, (*s ? " " : ""), d);
    else
      asprintf(&t, "%s%s(null)", s, (*s ? " " : ""));
    free(s);
    s = t;
    if (f && d)
      free(d);
  }
  return s;
}
