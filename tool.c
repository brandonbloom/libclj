// Reads Clojure forms from stdin and formats them to stdout.

#include "clj.h"
#include <stdlib.h>

clj_Reader reader;
clj_Printer printer;
clj_Node current_node;

int consume(clj_Node *node) {
  *node = current_node;
  return 0;
}

int emit(const clj_Node *node) {
  current_node = *node;
  clj_print(&printer);
  return 0;
}

int main(int argc, char **argv) {
  clj_Result result;

  reader.emit = emit;
  printer.consume = consume;

  // Connect stdin to stdout.
  reader.getwchar = getwchar;
  printer.putwchar = putwchar;

  // Read all forms.
  while (1) {
    result = clj_read(&reader);
    switch (result) {
    case CLJ_MORE:
      break;
    case CLJ_EOF:
      return EXIT_SUCCESS;
    default:
      fprintf(stderr, "ERROR: clj_read_result:%d at (%d:%d)\n",
          result, reader.line, reader.column);
      return EXIT_FAILURE;
    }
  }
}
