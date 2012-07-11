// Reads Clojure forms from stdin and formats them to stdout.

#include "clj.h"
#include <stdlib.h>

clj_Reader reader;
clj_Printer printer;

static wint_t reader_getwchar(const clj_Reader *r) {
  return getwchar();
}

extern void print(const clj_Reader *r, const clj_Node *n) {
  clj_print(&printer, n);
}

extern void tool_failure(clj_Reader *r, const char *msg) {
}

int main(int argc, char **argv) {
  clj_Result result;
  char message[200];

  reader.emit = print;

  // Connect stdin to stdout.
  reader.getwchar = reader_getwchar;
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
      clj_read_error(message, &reader, result);
      fprintf(stderr, "%s\n", message);
      return EXIT_FAILURE;
    }
  }
}
