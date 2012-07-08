#include "clj.h"
#include <stdlib.h>
#include <assert.h>


// Core Utilities

void fatal(char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

void *xmalloc(size_t size)
{
  void *value = malloc(size);
  if (value == 0) {
    fatal("virtual memory exhausted");
  }
  return value;
}

void *xrealloc(void *ptr, size_t size)
{
  void *value = realloc(ptr, size);
  if (value == 0) {
    fatal("virtual memory exhausted");
  }
  return value;
}


// String Buffer

struct strbuf {
  wchar_t *chars;
  int length;
  int capacity;
};

void strbuf_init(struct strbuf *strbuf, int capacity) {
  strbuf->chars = xmalloc(sizeof(wchar_t) * (capacity + 1));
  strbuf->chars[0] = L'\0';
  strbuf->length = 0;
  strbuf->capacity = capacity;
}

void strbuf_resize(struct strbuf *strbuf, int capacity) {
  strbuf->chars = xrealloc(strbuf->chars, sizeof(wchar_t) * (capacity + 1));
  strbuf->capacity = capacity;
}

void strbuf_append(struct strbuf *strbuf, wchar_t c) {
  if (strbuf->length == strbuf->capacity) {
    strbuf_resize(strbuf, strbuf->capacity * 2);
  }
  strbuf->chars[strbuf->length] = c;
  strbuf->length++;
  strbuf->chars[strbuf->length] = L'\0';
}

void strbuf_free(struct strbuf *strbuf) {
  free(strbuf->chars);
};


// Character classification

int is_clj_whitespace(wint_t c) {
  return iswspace(c) || c == L',';
}

int is_sign(wint_t c) {
  return c == L'+' || c == L'-';
}

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


// Read forms

void reader_error(struct clj_parser *parser, enum clj_read_result error) {
  longjmp(parser->_fail, error);
}

#define CLJ_NOT_IMPLEMENTED_READ \
  reader_error(parser, CLJ_NOT_IMPLEMENTED);

int at_number(struct clj_parser *parser, wint_t c) {
  return iswdigit(c) || (is_sign(c) && iswdigit(peek_char(parser)));
}

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
  wint_t c;
  struct clj_node node;
  struct strbuf strbuf;
  strbuf_init(&strbuf, 20); // MAX_LONG string length
  while (1) {
    c = pop_char(parser);
    if (WEOF == c || is_clj_whitespace(c) || get_macro_reader(c)) {
      push_char(parser, c);
      node.type = CLJ_NUMBER;
      node.value = strbuf.chars;
      parser->emit(&node);
      strbuf_free(&strbuf);
      break;
    } else {
      strbuf_append(&strbuf, c);
    }
  }
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
      push_char(parser, c);
      read_number(parser);
    } else {
      push_char(parser, c);
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

void print_string(struct clj_printer *printer, const wchar_t *s) {
  for (const wchar_t *i = s; *i != L'\0'; i++) {
    printer->putwchar(*i);
  };
}

int clj_print(struct clj_printer *printer) {
  struct clj_node node;
  printer->consume(&node);
  switch (node.type) {

    case CLJ_NUMBER:
      //TODO: Numbers greater than 10!
      print_string(printer, node.value);
      break;

    default:
      fatal("unexpected node type");
  }
  return 0;
}
