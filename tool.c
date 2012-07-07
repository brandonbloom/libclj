// Reads Clojure forms from stdin and formats them to stdout.

#include "clj.h"
#include <stdlib.h>

struct clj_parser parser;
struct clj_printer printer;
struct clj_node current_node;

int consume(struct clj_node *node) {
  *node = current_node;
  return 0;
}

int emit(const struct clj_node *node) {
  printf("emitting: %d\n", node->type);
  current_node = *node;
  clj_print(&printer, consume);
  return 0;
}

int main(int argc, char **argv) {

  // Connect stdin to stdout.
  parser.getwchar = getwchar;
  printer.putwchar = putwchar;

  clj_read(&parser, emit);

  return 0;
}
