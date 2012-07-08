// Reads Clojure forms from stdin and formats them to stdout.

#include "clj.h"
#include <stdlib.h>

clj_Reader reader;
clj_Printer printer;

void emit(const clj_Node *node) {
  clj_print(&printer, node);
}

void print_reader_error(clj_Reader *r, const char *msg) {
  fprintf(stderr, "ERROR: %s at line %d, column %d\n",
          msg, reader.line, reader.column);
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
    case CLJ_UNEXPECTED_EOF:
      print_reader_error(&reader, "unexpected end of file");
      break;
    case CLJ_UNMATCHED_DELIMITER:
      print_reader_error(&reader, "unmatched delimiter");
      break;
    case CLJ_NOT_IMPLEMENTED:
      print_reader_error(&reader, "unsupported form");
      break;
    default:
      print_reader_error(&reader, "unexpected error");
    }
    return EXIT_FAILURE;
  }
}
