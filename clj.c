#include "clj.h"
#include <stdlib.h>

int clj_read(struct clj_parser *parser, clj_emit emit) {
  struct clj_node node;
  node.type = CLJ_INTEGER;
  node.value.integer = 5;
  emit(&node);
  return 0;
}

int clj_print(struct clj_printer *printer, clj_consume consume) {
  struct clj_node node;
  consume(&node);
  switch (node.type) {

    case CLJ_INTEGER:
      //TODO: Numbers greater than 10!
      printer->putwchar(L'0' + node.value.integer % 10);
      break;

    default:
      fprintf(stderr, "Unexpected node type: %d\n", node.type);
      exit(1);
  }
  return 0;
}
