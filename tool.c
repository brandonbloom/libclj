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
  clj_print(&printer);
  return 0;
}

int main(int argc, char **argv) {
  enum clj_read_result result;

  //TODO: Maybe some kind of parser init function? or read_top_level

  parser.emit = emit;
  printer.consume = consume;

  // Connect stdin to stdout.
  parser.getwchar = getwchar;
  printer.putwchar = putwchar;

  //TODO: Read forms until end of file
  result = clj_read(&parser);
  switch (result) {
  case 0:
    return 0;
  default:
    fprintf(stderr, "Unknown clj_read_result: %d\n", result);
  }
  return 1;
}
