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

typedef struct string_buffer {
  wchar_t *chars;
  int length;
  int capacity;
} StringBuffer;

void strbuf_init(StringBuffer *strbuf, int capacity) {
  strbuf->chars = xmalloc(sizeof(wchar_t) * (capacity + 1));
  strbuf->chars[0] = L'\0';
  strbuf->length = 0;
  strbuf->capacity = capacity;
}

void strbuf_resize(StringBuffer *strbuf, int capacity) {
  strbuf->chars = xrealloc(strbuf->chars, sizeof(wchar_t) * (capacity + 1));
  strbuf->capacity = capacity;
}

void strbuf_append(StringBuffer *strbuf, wchar_t c) {
  if (strbuf->length == strbuf->capacity) {
    strbuf_resize(strbuf, strbuf->capacity * 2);
  }
  strbuf->chars[strbuf->length] = c;
  strbuf->length++;
  strbuf->chars[strbuf->length] = L'\0';
}

void strbuf_free(StringBuffer *strbuf) {
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

wint_t pop_char(clj_Reader *r) {
  wint_t c;
  if (r->_readback == 0) {
    c = r->getwchar();
  } else {
    c = r->_readback;
    r->_readback = 0;
  }
  if (ends_line(c)) {
    r->line++;
    r->_readback_column = r->column;
    r->column = 0;
  } else {
    r->column++;
  }
  return c;
}

void push_char(clj_Reader *r, wint_t c) {
  assert(r->_readback == 0);
  r->_readback = c;
  if (ends_line(c)) {
    r->line--;
    r->column = r->_readback_column;
  } else {
    r->column--;
  }
}

wint_t peek_char(clj_Reader *r) {
  wchar_t c = pop_char(r);
  push_char(r, c);
  return c;
}


// Read forms

void reader_error(clj_Reader *r, clj_ReadResult error) {
  longjmp(r->_fail, error);
}

#define CLJ_NOT_IMPLEMENTED_READ \
  reader_error(r, CLJ_NOT_IMPLEMENTED);

int at_number(clj_Reader *r, wint_t c) {
  return iswdigit(c) || (is_sign(c) && iswdigit(peek_char(r)));
}

typedef void (*form_reader)(clj_Reader *r);

form_reader get_macro_reader(wint_t c);

int is_macro_terminating(wint_t c) {
  return c != L'#'
      && c != L'\''
      && c != L':'
      && get_macro_reader(c);
}

void read_string(clj_Reader *r) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_token(clj_Reader *r, clj_Type type) {
  wint_t c;
  clj_Node node;
  StringBuffer strbuf;
  node.type = type;
  strbuf_init(&strbuf, 40); // grand-foo-bar-frobulator-factory-factory
  while (1) {
    c = pop_char(r);
    if (WEOF == c || is_clj_whitespace(c) || is_macro_terminating(c)) {
      push_char(r, c);
      node.value = strbuf.chars;
      r->emit(&node);
      strbuf_free(&strbuf);
      break;
    } else {
      strbuf_append(&strbuf, c);
    }
  }
}

void read_keyword(clj_Reader *r) {
  read_token(r, CLJ_KEYWORD);
}

void read_symbol(clj_Reader *r) {
  read_token(r, CLJ_SYMBOL);
}

void read_number(clj_Reader *r) {
  wint_t c;
  clj_Node node;
  StringBuffer strbuf;
  node.type = CLJ_NUMBER;
  strbuf_init(&strbuf, 20); // MAX_LONG
  while (1) {
    c = pop_char(r);
    if (WEOF == c || is_clj_whitespace(c) || get_macro_reader(c)) {
      push_char(r, c);
      node.value = strbuf.chars;
      r->emit(&node);
      strbuf_free(&strbuf);
      break;
    } else {
      strbuf_append(&strbuf, c);
    }
  }
}

void read_comment(clj_Reader *r) {
  wint_t c;
  do {
    c = pop_char(r);
  } while (!ends_line(c) && c != WEOF);
}

void read_quote(clj_Reader *r) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_deref(clj_Reader *r) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_meta(clj_Reader *r) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_syntax_quote(clj_Reader *r) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_unquote(clj_Reader *r) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_list(clj_Reader *r) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_unmatched_delimiter(clj_Reader *r) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_vector(clj_Reader *r) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_map(clj_Reader *r) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_char(clj_Reader *r) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_lambda_arg(clj_Reader *r) {
  CLJ_NOT_IMPLEMENTED_READ
}

void read_dispatch(clj_Reader *r) {
  CLJ_NOT_IMPLEMENTED_READ
}

void not_implemented(clj_Reader *r) {
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

void read_form(clj_Reader *r) {
  clj_Node node;
  form_reader macro_reader;
  wint_t c;
  while (WEOF != (c = pop_char(r))) {
    if (is_clj_whitespace(c)) {
      // ignored
    } else if ((macro_reader = get_macro_reader(c))) {
      macro_reader(r);
    } else if (at_number(r, c)) {
      push_char(r, c);
      read_number(r);
    } else {
      push_char(r, c);
      read_symbol(r);
    }
  }
};

clj_ReadResult clj_read(clj_Reader *r) {
  clj_ReadResult error;
  r->line = 1;
  r->column = 0;
  r->_readback = 0;
  if (!(error = setjmp(r->_fail))) {
    read_form(r);
  }
  return error;
}


// Print forms

void print_string(clj_Printer *printer, const wchar_t *s) {
  for (const wchar_t *i = s; *i != L'\0'; i++) {
    printer->putwchar(*i);
  };
}

int clj_print(clj_Printer *printer) {
  clj_Node node;
  printer->consume(&node);
  switch (node.type) {

    case CLJ_KEYWORD:
      printer->putwchar(L':');
      // Intentional fallthrough
    case CLJ_NUMBER:
    case CLJ_SYMBOL:
      print_string(printer, node.value);
      break;

    default:
      fatal("unexpected node type");
  }
  printer->putwchar(L'\n');
  return 0;
}
