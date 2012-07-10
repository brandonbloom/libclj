// Reads Clojure forms from stdin and formats them to stdout.

#include "clj.h"
#include <stdlib.h>

clj_Reader reader;
clj_Printer printer;

extern void print(const clj_Reader *r, const clj_Node *n) {
  clj_print(&printer, n);
}

extern void tool_failure(clj_Reader *r, const char *msg) {
  fprintf(stderr, "ERROR: %s at line %d, column %d\n",
          msg, reader.line, reader.column);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  clj_Result result;

  reader.emit = print;

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
      tool_failure(&reader, "unexpected end of file");
    case CLJ_UNMATCHED_DELIMITER:
      tool_failure(&reader, "unmatched delimiter");
    case CLJ_NOT_IMPLEMENTED:
      tool_failure(&reader, "unsupported form");
    case CLJ_UNREADABLE:
      tool_failure(&reader, "unreadable form");
    default:
      tool_failure(&reader, "unexpected error");
    }
  }
}
