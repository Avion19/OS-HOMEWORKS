#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deq.h"
#include "error.h"

// indices and size of array of node pointers
typedef enum
{
  Head,
  Tail,
  Ends
} End;

typedef struct Node
{
  struct Node *np[Ends]; // next/prev neighbors
  Data data;
} *Node;

typedef struct
{
  Node ht[Ends]; // head/tail nodes
  int len;
} *Rep;

static Rep rep(Deq q)
{
  if (!q)
    ERROR("zero pointer");
  return (Rep)q;
}

static void put(Rep r, End e, Data d)
{
  r->len++;
  struct Node *added = (Node)malloc(sizeof(struct Node));
  added->data = d;
  added->np[Head] = 0;
  added->np[Tail] = 0;

  // empty list case
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
}

static Data ith(Rep r, End e, int i) { return 0; }
static Data get(Rep r, End e)
{
  if (r->len == 0)
    return 0;

  Data d = r->ht[e]->data;

  if (r->len == 1)
  {
    r->ht[Head] = 0;
    r->ht[Tail] = 0;
    free(r->ht[e]);
  }
  else
  {
    if (e == Head)
    {
      Node next = r->ht[e]->np[Tail];
      free(r->ht[e]);
      r->ht[Head] = next;
      next->np[Head] = 0;
    }
    else if (e == Tail)
    {
      Node prev = r->ht[e]->np[Head];
      free(r->ht[e]);
      r->ht[Tail] = prev;
      prev->np[Tail] = 0;
    }
  }
  r->len--;
  return d;
}
static Data rem(Rep r, End e, Data d) { 
  if (r->len == 0) return 0;
  
  //edge case head or tail has the data
  // if (r->ht[e]->data == d) {
  //   return get(r, e);
  // }

  End step = (e == Head) ? Tail : Head;
  Node curr = r->ht[e];

  
    while (curr != 0) {
      if (curr->data == d) {
        //edge head or tail case
        if (curr == r->ht[e]) {
          return get(r, e);
        }
        Node prev = curr->np[Head];
        Node next = curr->np[Tail];
        prev->np[Tail] = next;
        next->np[Head] = prev;
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
    asprintf(&t, "%s%s%s", s, (*s ? " " : ""), d);
    free(s);
    s = t;
    if (f)
      free(d);
  }
  return s;
}
