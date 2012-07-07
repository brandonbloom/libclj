#include "clj.h"
#include <stdlib.h>
#include <assert.h>


int ends_line(wint_t c) {
  return c == L'\n' || c == L'\r';
}


// Position-tracking Pushback Reader

wint_t pop_char(struct clj_parser *parser) {
  wint_t c;
  if (parser->_readback == 0) {
    c = parser->getwchar();
  } else {
    c = parser->_readback;
    parser->_readback = 0;
  }
  if (ends_line(c)) {
    parser->line++;
    parser->_readback_column = parser->column;
    parser->column = 0;
  } else {
    parser->column++;
  }
  return c;
}

void push_char(struct clj_parser *parser, wint_t c) {
  assert(parser->_readback == 0);
  parser->_readback = c;
  if (ends_line(c)) {
    parser->line--;
    parser->column = parser->_readback_column;
  } else {
    parser->column--;
  }
}

wint_t peek_char(struct clj_parser *parser) {
  wchar_t c = pop_char(parser);
  push_char(parser, c);
  return c;
}


// Utilities

void reader_error(struct clj_parser *parser, enum clj_read_result error) {
  longjmp(parser->_fail, error);
}

#define CLJ_NOT_IMPLEMENTED_READ \
  reader_error(parser, CLJ_NOT_IMPLEMENTED);

int is_clj_whitespace(wint_t c) {
  return iswspace(c) || c == L',';
}

int is_sign(wint_t c) {
  return c == L'+' || c == L'-';
}

int at_number(struct clj_parser *parser, wint_t c) {
  return iswdigit(c) || (is_sign(c) && iswdigit(peek_char(parser)));
}


// Read forms

typedef void (*form_reader)(struct clj_parser *parser);

form_reader get_macro_reader(wint_t c);

void read_string(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_keyword(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_symbol(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_number(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED_READ
  //node.type = CLJ_INTEGER;
  //node.value.integer = 5;
  //parse->emit(&node);
}

void read_comment(struct clj_parser *parser) {
  wint_t c;
  do {
    c = pop_char(parser);
  } while (!ends_line(c) && c != WEOF);
}

void read_quote(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_deref(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_meta(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_syntax_quote(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_unquote(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_list(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_unmatched_delimiter(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_vector(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_map(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_char(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_lambda_arg(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_dispatch(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED_READ
}

void not_implemented(struct clj_parser *parser) {
  CLJ_NOT_IMPLEMENTED_READ
}

form_reader get_macro_reader(wint_t c) {
  switch (c) {
  case L'"':  return read_string;
  case L':':  return read_keyword;
  case L';':  return read_comment; // never hit this
  case L'\'': return read_quote;
  case L'@':  return read_deref;
  case L'^':  return read_meta;
  case L'`':  return read_syntax_quote;
  case L'~':  return read_unquote;
  case L'(':  return read_list;
  case L')':  return read_unmatched_delimiter;
  case L'[':  return read_vector;
  case L']':  return read_unmatched_delimiter;
  case L'{':  return read_map;
  case L'}':  return read_unmatched_delimiter;
  case L'\\': return read_char;
  case L'%':  return read_lambda_arg;
  case L'#':  return read_dispatch;
  default:    return 0;
  }
}

void read_form(struct clj_parser *parser) {
  struct clj_node node;
  form_reader macro_reader;
  wint_t c;
  while (WEOF != (c = pop_char(parser))) {
    if (is_clj_whitespace(c)) {
      // ignored
    } else if ((macro_reader = get_macro_reader(c))) {
      macro_reader(parser);
    } else if (at_number(parser, c)) {
      read_number(parser);
    } else {
      read_symbol(parser);
    }
  }
};

enum clj_read_result clj_read(struct clj_parser *parser) {
  enum clj_read_result error;
  parser->line = 1;
  parser->column = 0;
  parser->_readback = 0;
  if (!(error = setjmp(parser->_fail))) {
    read_form(parser);
  }
  return error;
}


// Print forms

int clj_print(struct clj_printer *printer) {
  struct clj_node node;
  printer->consume(&node);
  switch (node.type) {

    case CLJ_INTEGER:
      //TODO: Numbers greater than 10!
      printer->putwchar(L'0' + node.value.integer % 10);
      break;

    default:
      //TODO: return error, don't print
      fprintf(stderr, "Unexpected node type: %d\n", node.type);
      exit(1);
  }
  return 0;
}
