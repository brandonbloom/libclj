// Reads Clojure forms from stdin and formats them to stdout.

#include "clj.h"
#include <stdlib.h>

clj_Reader reader;
clj_Printer printer;

void emit(const clj_Node *node) {
  clj_print(&printer, node);
}

int main(int argc, char **argv) {
  clj_Result result;

  reader.emit = emit;

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
    case CLJ_UNMATCHED_DELIMITER:
      fprintf(stderr, "ERROR: unmatched delimiter at line %d, column %d\n",
              reader.line, reader.column);
      return EXIT_FAILURE;
    default:
      fprintf(stderr, "ERROR: clj_read_result:%d at line %d, column %d\n",
              result, reader.line, reader.column);
      return EXIT_FAILURE;
    }
  }
}
