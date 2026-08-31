#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deq.h"

int main() {
  Deq q=deq_new();
  deq_head_put(q,strdup("a"));
  deq_head_put(q,strdup("b"));
  deq_tail_put(q,strdup("c"));

  char *s=deq_str(q,0);
  printf("%s\n",s);
  free(s);

  deq_del(q,0);
  return 0;
}
