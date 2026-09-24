/*
 * Author: Avien Ramirez
 * Date: September 2, 2026
 * Class: CS 452-1
 * Description: Test suite for the double-ended doubly-linked deque.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deq.h"

/** Verifies operations on an empty deque return the documented empty values. */
static void test_empty(void)
{
  Deq q = deq_new();

  assert(deq_len(q) == 0);
  assert(deq_head_get(q) == 0);
  assert(deq_tail_get(q) == 0);
  assert(deq_head_ith(q, 0) == 0);
  assert(deq_tail_ith(q, 0) == 0);
  assert(deq_head_ith(q, -1) == 0);
  assert(deq_tail_ith(q, -1) == 0);
  assert(deq_head_rem(q, (Data) "missing") == 0);
  assert(deq_tail_rem(q, (Data) "missing") == 0);

  deq_del(q, free);
  printf("PASS: test_empty\n");
}

/** Verifies 0 results and 0 search arguments leave the deque unchanged. */
static void test_null_behavior(void)
{
  Deq q = deq_new();
  char *item = strdup("item");
  char *s;

  assert(item != 0);
  deq_head_put(q, item);

  assert(deq_head_ith(q, -1) == 0);
  assert(deq_tail_ith(q, 1) == 0);
  assert(deq_head_rem(q, 0) == 0);
  assert(deq_tail_rem(q, 0) == 0);
  assert(deq_len(q) == 1);
  assert(deq_head_get(q) == item);
  assert(deq_tail_get(q) == 0);

  free(item);
  s = deq_str(q, 0);
  assert(strcmp(s, "") == 0);
  free(s);
  deq_del(q, free);
  printf("PASS: test_null_behavior\n");
}

/** Verifies insertion at both ends and indexed access from both endpoints. */
static void test_put_and_ith(void)
{
  Deq q = deq_new();
  char *a = strdup("a");
  char *b = strdup("b");
  char *c = strdup("c");
  char *d = strdup("d");

  //make sure what we are inserting isn't null
  assert(a != 0);
  assert(b != 0);
  assert(c != 0);
  assert(d != 0);
  
  deq_head_put(q, a);
  // current queue = a
  deq_tail_put(q, b);
  // current queue = ab
  deq_head_put(q, c);
  // current queue = cab
  deq_tail_put(q, d);
  // current queue = cabd

  assert(deq_len(q) == 4);
  assert(deq_head_ith(q, 0) == c);
  assert(deq_head_ith(q, 1) == a);
  assert(deq_head_ith(q, 2) == b);
  assert(deq_head_ith(q, 3) == d);
  assert(deq_tail_ith(q, 0) == d);
  assert(deq_tail_ith(q, 1) == b);
  assert(deq_tail_ith(q, 2) == a);
  assert(deq_tail_ith(q, 3) == c);
  assert(deq_head_ith(q, -1) == 0);
  assert(deq_tail_ith(q, 4) == 0);

  deq_del(q, free);
  printf("PASS: test_put_and_ith\n");
}

/** Verifies removal from both ends and ownership of returned data. */
static void test_get(void)
{
  Deq q = deq_new();
  char *a = strdup("a");
  char *b = strdup("b");
  char *c = strdup("c");

  deq_head_put(q, a);
  // current queue = a
  deq_tail_put(q, b);
  // current queue = ab
  deq_tail_put(q, c);
  // current queue = abc
  assert(deq_head_get(q) == a);
  // current queue = bc
  assert(deq_len(q) == 2);
  assert(deq_tail_get(q) == c);
  // current queue = b
  assert(deq_len(q) == 1);
  assert(deq_head_get(q) == b);
  // current queue = empty
  assert(deq_len(q) == 0);
  assert(deq_tail_get(q) == 0);

  free(a);
  free(b);
  free(c);
  deq_del(q, free);
  printf("PASS: test_get\n");
}

/** Verifies matching removal, missing-data handling, and deque length updates. */
static void test_rem(void)
{
  Deq q = deq_new();
  char *a = strdup("a");
  char *b = strdup("b");
  char *c = strdup("c");
  char *d = strdup("d");
  char *missing = strdup("missing");

  assert(a != 0);
  assert(b != 0);
  assert(c != 0);
  assert(d != 0);

  deq_head_put(q, a);
  // current queue = a
  deq_tail_put(q, b);
  // current queue = ab
  deq_head_put(q, c);
  // current queue = cab
  deq_tail_put(q, d);         
  // current queue = cabd

  assert(deq_head_rem(q, a) == a);
  // current queue = cbd
  free(a);
  assert(deq_len(q) == 3);
  assert(deq_head_ith(q, 0) == c);
  assert(deq_head_ith(q, 1) == b);
  assert(deq_head_ith(q, 2) == d);

  assert(deq_tail_rem(q, b) == b);
  // current queue = cd
  free(b);
  assert(deq_len(q) == 2);
  assert(deq_head_rem(q, c) == c);
  // current queue = d
  free(c);
  assert(deq_tail_rem(q, d) == d);
  // current queue = empty
  free(d);
  assert(deq_len(q) == 0);
  assert(deq_head_rem(q, missing) == 0);
  free(missing);

  deq_del(q, free);
  printf("PASS: test_rem\n");
}

/** Verifies duplicate entries containing the same data pointer are removable. */
static void test_duplicate_pointer_rem(void)
{
  Deq q = deq_new();
  char *shared = strdup("shared");

  deq_head_put(q, shared);
  // current queue = shared
  deq_tail_put(q, shared);
  // current queue = shared shared
  assert(deq_len(q) == 2);
  assert(deq_head_rem(q, shared) == shared);
  // current queue = shared
  assert(deq_len(q) == 1);
  assert(deq_tail_rem(q, shared) == shared);
  // current queue = empty
  assert(deq_len(q) == 0);

  free(shared);
  deq_del(q, free);
  printf("PASS: test_duplicate_pointer_rem\n");
}

/** Verifies string conversion before and after a removal. */
static void test_string_after_operations(void)
{
  Deq q = deq_new();
  char *a = strdup("a");
  char *b = strdup("b");
  char *c = strdup("c");
  char *s;

  deq_head_put(q, a);
  // current queue = a
  deq_head_put(q, b);
  // current queue = ba
  deq_tail_put(q, c);
  // current queue = bac
  s = deq_str(q, 0);
  assert(strcmp(s, "b a c") == 0);
  free(s);

  assert(deq_head_rem(q, a) == a);
  // current queue = bc
  free(a);
  s = deq_str(q, 0);
  assert(strcmp(s, "b c") == 0);
  free(s);
  deq_del(q, free);
  printf("PASS: test_string_after_operations\n");
}

static Data mapped_data[3];
static int mapped_count;

/** Records each data pointer visited by deq_map. */
static void record_mapping(Data d)
{
  mapped_data[mapped_count++] = d;
}

/** Verifies callback count, traversal order, and unchanged deque contents. */
static void test_map(void)
{
  Deq q = deq_new();
  char *a = strdup("a");
  char *b = strdup("b");
  char *c = strdup("c");

  deq_tail_put(q, a);
  deq_tail_put(q, b);
  deq_tail_put(q, c);

  mapped_count = 0;
  deq_map(q, record_mapping);

  assert(mapped_count == 3);
  assert(mapped_data[0] == a);
  assert(mapped_data[1] == b);
  assert(mapped_data[2] == c);
  assert(deq_len(q) == 3);
  assert(deq_head_ith(q, 0) == a);
  assert(deq_head_ith(q, 1) == b);
  assert(deq_head_ith(q, 2) == c);

  deq_del(q, free);
  printf("PASS: test_map\n");
}

/** Verifies repeated insertion at the head. */
static void test_head_put_only(void)
{
  Deq q = deq_new();
  char *a = strdup("a");
  char *b = strdup("b");

  deq_head_put(q, a);
  // current queue = a
  deq_head_put(q, b);
  // current queue = ba
  assert(deq_len(q) == 2);
  assert(deq_head_ith(q, 0) == b);
  assert(deq_head_ith(q, 1) == a);
  deq_del(q, free);
  printf("PASS: test_head_put_only\n");
}

/** Verifies repeated insertion at the tail. */
static void test_tail_put_only(void)
{
  Deq q = deq_new();
  char *a = strdup("a");
  char *b = strdup("b");

  deq_tail_put(q, a);
  // current queue = a
  deq_tail_put(q, b);
  // current queue = ab
  assert(deq_len(q) == 2);
  assert(deq_tail_ith(q, 0) == b);
  assert(deq_tail_ith(q, 1) == a);
  deq_del(q, free);
  printf("PASS: test_tail_put_only\n");
}

/** Verifies head removal from a single-entry deque. */
static void test_head_get_only(void)
{
  Deq q = deq_new();
  char *item = strdup("head");

  deq_head_put(q, item);
  assert(deq_head_get(q) == item);
  assert(deq_len(q) == 0);
  free(item);
  deq_del(q, free);
  printf("PASS: test_head_get_only\n");
}

/** Verifies tail removal from a single-entry deque. */
static void test_tail_get_only(void)
{
  Deq q = deq_new();
  char *item = strdup("tail");

  deq_tail_put(q, item);
  assert(deq_tail_get(q) == item);
  assert(deq_len(q) == 0);
  free(item);
  deq_del(q, free);
  printf("PASS: test_tail_get_only\n");
}

/** Verifies indexed traversal from the head. */
static void test_head_ith_only(void)
{
  Deq q = deq_new();
  char *a = strdup("a");
  char *b = strdup("b");

  deq_tail_put(q, a);
  // current queue = a
  deq_tail_put(q, b);
  // current queue = ab
  assert(deq_head_ith(q, 0) == a);
  assert(deq_head_ith(q, 1) == b);
  deq_del(q, free);
  printf("PASS: test_head_ith_only\n");
}

/** Verifies indexed traversal from the tail. */
static void test_tail_ith_only(void)
{
  Deq q = deq_new();
  char *a = strdup("a");
  char *b = strdup("b");

  deq_head_put(q, a);
  // current queue = a
  deq_head_put(q, b);
  // current queue = ba
  assert(deq_tail_ith(q, 0) == a);
  assert(deq_tail_ith(q, 1) == b);
  deq_del(q, free);
  printf("PASS: test_tail_ith_only\n");
}

/** Verifies matching removal searched from the head. */
static void test_head_rem_only(void)
{
  Deq q = deq_new();
  char *a = strdup("a");
  char *b = strdup("b");

  deq_tail_put(q, a);
  deq_tail_put(q, b);
  assert(deq_head_rem(q, a) == a);
  assert(deq_len(q) == 1);
  free(a);
  deq_del(q, free);
  printf("PASS: test_head_rem_only\n");
}

/** Verifies matching removal searched from the tail. */
static void test_tail_rem_only(void)
{
  Deq q = deq_new();
  char *a = strdup("a");
  char *b = strdup("b");

  deq_head_put(q, a);
  deq_head_put(q, b);
  assert(deq_tail_rem(q, a) == a);
  assert(deq_len(q) == 1);
  free(a);
  deq_del(q, free);
  printf("PASS: test_tail_rem_only\n");
}

/** Verifies a sequence combining insertion, indexed access, get, and rem. */
static void test_operations_in_tandem(void)
{
  Deq q = deq_new();
  char *a = strdup("a");
  char *b = strdup("b");
  char *c = strdup("c");

  deq_head_put(q, a);
  // current queue = a
  deq_tail_put(q, b);
  // current queue = ab
  assert(deq_head_ith(q, 1) == b);
  assert(deq_tail_ith(q, 1) == a);
  deq_head_put(q, c);
  // current queue = cab
  assert(deq_head_get(q) == c);
  // current queue = ab
  assert(deq_tail_rem(q, a) == a);
  // current queue = b
  assert(deq_tail_get(q) == b);
  // current queue = empty
  assert(deq_len(q) == 0);

  free(a);
  free(b);
  free(c);
  deq_del(q, free);
  printf("PASS: test_operations_in_tandem\n");
}

/** Runs the complete deque test suite. */
int main(void)
{
  test_empty();
  test_null_behavior();
  test_put_and_ith();
  test_get();
  test_rem();
  test_duplicate_pointer_rem();
  test_string_after_operations();
  test_map();
  test_head_put_only();
  test_tail_put_only();
  test_head_get_only();
  test_tail_get_only();
  test_head_ith_only();
  test_tail_ith_only();
  test_head_rem_only();
  test_tail_rem_only();
  test_operations_in_tandem();

  printf("All deque tests passed.\n");
  return EXIT_SUCCESS;
}
