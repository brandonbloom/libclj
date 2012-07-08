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

void reader_error(clj_Reader *r, clj_Result error) {
  longjmp(r->_fail, error);
}

// Emits a node which demarks a complete object (ie. not a begin bracket)
void emit_complete(clj_Reader *r, clj_Node *n) {
  r->emit(n);
  if (r->_depth == 0) {
    r->_depth = -1;
  }
}

#define CLJ_NOT_IMPLEMENTED_READ \
  fprintf(stderr, "%s not implemented\n", __func__); \
  reader_error(r, CLJ_NOT_IMPLEMENTED); \
  return 0;

int at_number(clj_Reader *r, wint_t c) {
  return iswdigit(c) || (is_sign(c) && iswdigit(peek_char(r)));
}

typedef clj_Result (*form_reader)(clj_Reader *r, wint_t initch);

form_reader get_macro_reader(wint_t c);

int is_macro_terminating(wint_t c) {
  return c != L'#' && c != L'\'' && c != L':' && get_macro_reader(c);
}

clj_Result ok_read(wint_t c) {
  return (c == WEOF ? CLJ_EOF : CLJ_MORE);
}

clj_Result read_form(clj_Reader*);

clj_Result read_string(clj_Reader *r, wint_t initch) {
  CLJ_NOT_IMPLEMENTED_READ
}

clj_Result read_token(clj_Type type, clj_Reader *r, wint_t initch) {
  wint_t c;
  clj_Node node;
  StringBuffer strbuf;
  node.type = type;
  strbuf_init(&strbuf, 40); // grand-foo-bar-frobulator-factory-factory
  strbuf_append(&strbuf, initch);
  while (1) {
    c = pop_char(r);
    if (WEOF == c || is_clj_whitespace(c) || is_macro_terminating(c)) {
      push_char(r, c);
      node.value = strbuf.chars;
      emit_complete(r, &node);
      strbuf_free(&strbuf);
      break;
    } else {
      strbuf_append(&strbuf, c);
    }
  }
  return ok_read(c);
}

clj_Result read_keyword(clj_Reader *r, wint_t initch) {
  return read_token(CLJ_KEYWORD, r, initch);
}

clj_Result read_symbol(clj_Reader *r, wint_t initch) {
  return read_token(CLJ_SYMBOL, r, initch);
}

clj_Result read_number(clj_Reader *r, wint_t initch) {
  wint_t c;
  clj_Node node;
  StringBuffer strbuf;
  node.type = CLJ_NUMBER;
  strbuf_init(&strbuf, 20); // MAX_LONG
  strbuf_append(&strbuf, initch);
  while (1) {
    c = pop_char(r);
    if (WEOF == c || is_clj_whitespace(c) || get_macro_reader(c)) {
      push_char(r, c);
      node.value = strbuf.chars;
      emit_complete(r, &node);
      strbuf_free(&strbuf);
      break;
    } else {
      strbuf_append(&strbuf, c);
    }
  }
  return ok_read(c);
}

clj_Result read_comment(clj_Reader *r, wint_t initch) {
  wint_t c;
  do {
    c = pop_char(r);
  } while (!ends_line(c) && c != WEOF);
  return ok_read(c);
}

clj_Result read_quote(clj_Reader *r, wint_t initch) {
  CLJ_NOT_IMPLEMENTED_READ
}

clj_Result read_deref(clj_Reader *r, wint_t initch) {
  CLJ_NOT_IMPLEMENTED_READ
}

clj_Result read_meta(clj_Reader *r, wint_t initch) {
  CLJ_NOT_IMPLEMENTED_READ
}

clj_Result read_syntax_quote(clj_Reader *r, wint_t initch) {
  CLJ_NOT_IMPLEMENTED_READ
}

clj_Result read_unquote(clj_Reader *r, wint_t initch) {
  CLJ_NOT_IMPLEMENTED_READ
}

clj_Result read_unmatched_delimiter(clj_Reader *r, wint_t initch) {
  CLJ_NOT_IMPLEMENTED_READ
}

clj_Result read_delimited(clj_Type type, clj_Reader *r, wint_t terminator) {
  return CLJ_MORE;
}

clj_Result read_list(clj_Reader *r, wint_t initch) {
  return read_delimited(CLJ_LIST, r, L')');
}

clj_Result read_vector(clj_Reader *r, wint_t initch) {
  return read_delimited(CLJ_VECTOR, r, L']');
}

clj_Result read_map(clj_Reader *r, wint_t initch) {
  return read_delimited(CLJ_MAP, r, L'}');
}

clj_Result read_set(clj_Reader *r, wint_t initch) {
  return read_delimited(CLJ_SET, r, L'}');
}

clj_Result read_char(clj_Reader *r, wint_t initch) {
  CLJ_NOT_IMPLEMENTED_READ
}

clj_Result read_lambda_arg(clj_Reader *r, wint_t initch) {
  CLJ_NOT_IMPLEMENTED_READ
}

clj_Result read_dispatch(clj_Reader *r, wint_t initch) {
  CLJ_NOT_IMPLEMENTED_READ
}

clj_Result not_implemented(clj_Reader *r, wint_t initch) {
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

clj_Result read_form(clj_Reader *r) {
  clj_Result result;
  clj_Node node;
  form_reader macro_reader;
  wint_t c;
  while (WEOF != (c = pop_char(r))) {
    if (is_clj_whitespace(c)) {
      continue;
    } else if ((macro_reader = get_macro_reader(c))) {
      result = macro_reader(r, c);
      if (r->_depth == -1) {
        return result;
      }
    } else if (at_number(r, c)) {
      return read_number(r, c);
    } else {
      return read_symbol(r, c);
    }
  }
  return CLJ_EOF;
};

clj_Result clj_read(clj_Reader *r) {
  clj_Result error;
  r->line = 1;
  r->column = 0;
  r->_depth = 0;
  r->_readback = 0;
  if ((error = setjmp(r->_fail))) {
    return error;
  } else {
    return read_form(r);
  }
}


// Print forms

void print_node(clj_Printer*, clj_Node*);

void print_string(clj_Printer *p, const wchar_t *s) {
  for (const wchar_t *i = s; *i != L'\0'; i++) {
    p->putwchar(*i);
  };
}

void print_delimited(clj_Printer *p, const wchar_t *begin, wint_t end) {
  print_string(p, begin);
  //p->putwchar(end);
}

void print_node(clj_Printer *p, clj_Node *node) {
  switch (node->type) {

    case CLJ_NUMBER:
    case CLJ_SYMBOL:
    case CLJ_KEYWORD:
      print_string(p, node->value);
      break;

    default:
      fatal("unexpected node type");
  }
  p->putwchar(L'\n');
}

int clj_print(clj_Printer *p) {
  clj_Node node;
  p->consume(&node);
  print_node(p, &node);
  return 0;
}
