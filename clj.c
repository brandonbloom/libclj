#include "clj.h"
#include <stdlib.h>

#define CLJ_NOT_IMPLEMENTED \
  fprintf(stderr, "%s is not implemented.\n", __func__);

void clj_read_string(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED
}

void clj_read_keyword(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED
}

void clj_read_comment(struct clj_parser *parser) {
  wint_t c;
  do {
    c = parser->getwchar();
  } while (c != L'\n' && c != L'\r' && c != WEOF);
}

void clj_read_quote(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED
}

void clj_read_deref(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED
}

void clj_read_meta(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED
}

void clj_read_syntax_quote(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED
}

void clj_read_unquote(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED
}

void clj_read_list(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED
}

void clj_read_unmatched_delimiter(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED
}

void clj_read_vector(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED
}

void clj_read_map(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED
}

void clj_read_char(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED
}

void clj_read_lambda_arg(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED
}

void clj_read_dispatch(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED
}

int clj_read(struct clj_parser *parser) {
  struct clj_node node;
  wint_t c;
  while (WEOF != (c = parser->getwchar())) {
    if (iswspace(c)) {
      // ignored
    } else {
      switch (c) {
      case L'"':
        clj_read_string(parser);
        break;
      case L':':
        clj_read_keyword(parser);
        break;
      case L';':
        clj_read_comment(parser);
        break;
      case L'\'':
        clj_read_quote(parser);
        break;
      case L'@':
        clj_read_deref(parser);
        break;
      case L'^':
        clj_read_meta(parser);
        break;
      case L'`':
        clj_read_syntax_quote(parser);
        break;
      case L'~':
        clj_read_unquote(parser);
        break;
      case L'(':
        clj_read_list(parser);
        break;
      case L')':
        clj_read_unmatched_delimiter(parser);
        break;
      case L'[':
        clj_read_vector(parser);
        break;
      case L']':
        clj_read_unmatched_delimiter(parser);
        break;
      case L'{':
        clj_read_map(parser);
        break;
      case L'}':
        clj_read_unmatched_delimiter(parser);
        break;
      case L'\\':
        clj_read_char(parser);
        break;
      case L'%':
        clj_read_lambda_arg(parser);
        break;
      case L'#':
        clj_read_dispatch(parser);
        break;
      default:
        fprintf(stderr, "OMG"); //TODO TODO
        //node.type = CLJ_INTEGER;
        //node.value.integer = 5;
        //parse->emit(&node);
      }
    }
  }
  return 0;
}

int clj_print(struct clj_printer *printer) {
  struct clj_node node;
  printer->consume(&node);
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
