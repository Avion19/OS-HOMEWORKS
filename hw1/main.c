  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>

  #include "deq.h"


  static void test_rem_scenario(void) {
    Deq q = deq_new();

    char *a = strdup("a");
    char *b = strdup("b");
    char *c = strdup("c");

    deq_head_put(q, a);
    deq_head_put(q, b);
    deq_tail_put(q, c);

    char *s = deq_str(q, 0);
    printf("rem scenario: %s\n", s ? s : "(null)");
    free(s);

    char *rem = deq_head_rem(q, b);
    printf("rem scenario: rem=%s len=%d\n", rem ? rem : "(null)", deq_len(q));

    char *after_rem = deq_str(q, 0);
    printf("rem scenario: %s\n", after_rem ? after_rem : "(null)");
    free(after_rem);
    free(rem);

    deq_del(q, free);
  }

  static void test_put_scenario(void) {
    Deq q = deq_new();
    deq_head_put(q, strdup("a"));
    deq_head_put(q, strdup("b"));
    deq_tail_put(q, strdup("c"));

    char *s = deq_str(q, 0);
    printf("put scenario: %s\n", s ? s : "(null)");
    free(s);

    deq_del(q, free);
  }

  static void test_get_scenario(void) {
    Deq q = deq_new();
    deq_head_put(q, strdup("a"));
    deq_head_put(q, strdup("b"));
    deq_tail_put(q, strdup("c"));
    char *s = deq_str(q, 0);
    printf("get scenario: %s\n", s ? s : "(null)");
    free(s);

    char *got = deq_head_get(q);
    printf("get scenario: got=%s len=%d\n", got ? got : "(null)", deq_len(q));

    char *after_get = deq_str(q, 0);
    printf("get scenario: %s\n", after_get ? after_get : "(null)");
    free(after_get);
    free(got);

    deq_del(q, free);
  }

  int main() {
    test_put_scenario();
    printf("\n");
    test_get_scenario();
    printf("\n");
    test_rem_scenario();
    return 0;
  }
