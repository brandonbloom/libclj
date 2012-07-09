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

void strbuf_appends(StringBuffer *strbuf, const wchar_t *s) {
  for (const wchar_t *i = s; *i != L'\0'; i++) {
    strbuf_append(strbuf, *i);
  }
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

void emit(clj_Reader *r, clj_Node *n) {
  if (!r->_discard) {
    r->emit(n);
  }
}

#define CLJ_NOT_IMPLEMENTED_READ \
  fprintf(stderr, "%s not implemented\n", __func__); \
  reader_error(r, CLJ_NOT_IMPLEMENTED); \
  return 0;

wint_t skip_whitespace(clj_Reader *r) {
  wint_t c;
  while (is_clj_whitespace(c = pop_char(r)));
  return c;
}

int at_number(clj_Reader *r, wint_t c) {
  return iswdigit(c) || (is_sign(c) && iswdigit(peek_char(r)));
}

typedef clj_Result (*form_reader)(clj_Reader *r, wint_t initch);
typedef int (*char_pred)(wint_t c);

form_reader get_macro_reader(wint_t c);

int is_macro_terminating(wint_t c) {
  return c != L'#' && c != L'\'' && c != L':' && get_macro_reader(c);
}

clj_Result ok_read(wint_t c) {
  return (c == WEOF ? CLJ_EOF : CLJ_MORE);
}

clj_Result read_form(clj_Reader*);

clj_Result read_typed_string(clj_Type type, const wchar_t *prefix,
                             clj_Reader *r) {
  wint_t c;
  clj_Node node;
  StringBuffer strbuf;
  int escape = 0;
  node.type = type;
  strbuf_init(&strbuf, 80); // C'mon now, how big is your terminal?
  strbuf_appends(&strbuf, prefix);
  while (1) {
    c = pop_char(r);
    switch (c) {
      case WEOF:
        strbuf_free(&strbuf);
        reader_error(r, CLJ_UNEXPECTED_EOF);
      case L'\\':
        strbuf_append(&strbuf, c);
        escape = !escape;
        break;
      case L'"':
        strbuf_append(&strbuf, c);
        if (escape) {
          escape = 0;
          break;
        } else {
          node.value = strbuf.chars;
          emit(r, &node);
          strbuf_free(&strbuf);
          return CLJ_MORE;
        }
      default:
        escape = 0;
        strbuf_append(&strbuf, c);
    }
  }
}

clj_Result read_string(clj_Reader *r, wint_t initch) {
  return read_typed_string(CLJ_STRING, L"\"", r);
}

clj_Result read_regex(clj_Reader *r, wint_t initch) {
  return read_typed_string(CLJ_REGEX, L"#\"", r);
}

clj_Result read_token(clj_Type type, clj_Reader *r, wint_t initch,
                      size_t initial_capacity, char_pred terminates) {
  wint_t c;
  clj_Node node;
  StringBuffer strbuf;
  node.type = type;
  strbuf_init(&strbuf, initial_capacity);
  strbuf_append(&strbuf, initch);
  while (1) {
    c = pop_char(r);
    if (WEOF == c || is_clj_whitespace(c) || terminates(c)) {
      push_char(r, c);
      node.value = strbuf.chars;
      emit(r, &node);
      strbuf_free(&strbuf);
      break;
    } else {
      strbuf_append(&strbuf, c);
    }
  }
  return ok_read(c);
}

clj_Result read_keyword(clj_Reader *r, wint_t initch) {
  return read_token(CLJ_KEYWORD, r, initch,
                    25, // :some-very-interesting-key
                    is_macro_terminating);
}

clj_Result read_symbol(clj_Reader *r, wint_t initch) {
  return read_token(CLJ_SYMBOL, r, initch,
                    40, // grand-foo-bar-frobulator-factory-factory
                    is_macro_terminating);
}

clj_Result read_number(clj_Reader *r, wint_t initch) {
  return read_token(CLJ_NUMBER, r, initch,
                    20 /* MAX_LONG */, (char_pred)get_macro_reader);
}

clj_Result read_comment(clj_Reader *r, wint_t initch) {
  wint_t c;
  do {
    c = pop_char(r);
  } while (!ends_line(c) && c != WEOF);
  return ok_read(c);
}

clj_Result read_wrapped(clj_Reader *r, const wint_t *sym) {
  clj_Node node;
  clj_Result result;
  // Begin list
  node.type = CLJ_LIST;
  emit(r, &node);
  // Invoked form
  node.type = CLJ_SYMBOL;
  node.value = sym;
  emit(r, &node);
  // Argument
  result = read_form(r);
  // End list
  node.type = CLJ_LIST | CLJ_END;
  emit(r, &node);
  return result;
}

clj_Result read_quote(clj_Reader *r, wint_t initch) {
  return read_wrapped(r, L"quote");
}

clj_Result read_deref(clj_Reader *r, wint_t initch) {
  //TODO: Prepend core namespace
  return read_wrapped(r, L"deref");
}

clj_Result read_syntax_quote(clj_Reader *r, wint_t initch) {
  CLJ_NOT_IMPLEMENTED_READ
}

clj_Result read_unquote(clj_Reader *r, wint_t initch) {
  CLJ_NOT_IMPLEMENTED_READ
}

clj_Result read_unmatched_delimiter(clj_Reader *r, wint_t initch) {
  reader_error(r, CLJ_UNMATCHED_DELIMITER);
  return 0;
}

clj_Result read_delimited(clj_Type type, clj_Reader *r, wint_t terminator) {
  wint_t c;
  clj_Node node;
  form_reader macro_reader;
  node.type = type;
  emit(r, &node);
  r->_depth++;
  while (1) {
    c = skip_whitespace(r);
    if (c == terminator) {
      node.type = type | CLJ_END;
      r->_depth--;
      emit(r, &node);
      return CLJ_MORE;
    } else if ((macro_reader = get_macro_reader(c))) {
      macro_reader(r, c);
    } else if (c == WEOF) {
      reader_error(r, CLJ_UNEXPECTED_EOF);
    } else {
      push_char(r, c);
      read_form(r);
    }
  }
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
  return read_token(CLJ_CHARACTER, r, initch,
                    10 /* \backspace */, is_macro_terminating);
}

clj_Result read_lambda_arg(clj_Reader *r, wint_t initch) {
  CLJ_NOT_IMPLEMENTED_READ
}

clj_Result read_unreadable(clj_Reader *r, wint_t initch) {
  reader_error(r, CLJ_UNREADABLE);
  return 0;
}

clj_Result read_discard(clj_Reader *r, wint_t initch) {
  clj_Result result;
  r->_discard++;
  result = read_form(r);
  r->_discard--;
  return result;
}

clj_Result read_meta(clj_Reader *r, wint_t initch) {
  //TODO: Don't discard metadata.
  return read_discard(r, initch);
}

form_reader get_dispatch_reader(wint_t c) {
  switch (c) {
    case L'{': return read_set;
    case L'<': return read_unreadable;
    case L'"': return read_regex;
    case L'!': return read_comment;
    case L'_': return read_discard;
    default:   return 0;
  }
}

clj_Result read_dispatch(clj_Reader *r, wint_t initch) {
  form_reader dispatch_reader;
  wint_t c = pop_char(r);
  if ((dispatch_reader = get_dispatch_reader(c))) {
    return dispatch_reader(r, c);
  } else {
    assert(0); //TODO tagged types and unknown dispatch macros
  }
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
      return macro_reader(r, c);
    } else if (at_number(r, c)) {
      return read_number(r, c);
    } else {
      return read_symbol(r, c);
    }
  }
  if (r->_depth > 0) {
    reader_error(r, CLJ_UNEXPECTED_EOF);
  }
  return CLJ_EOF;
};

clj_Result clj_read(clj_Reader *r) {
  clj_Result error;
  r->line = 1;
  r->column = 0;
  r->_depth = 0;
  r->_discard = 0;
  r->_readback = 0;
  if ((error = setjmp(r->_fail))) {
    return error;
  } else {
    return read_form(r);
  }
}


// Print forms

void print_string(clj_Printer *p, const wchar_t *s) {
  for (const wchar_t *i = s; *i != L'\0'; i++) {
    p->putwchar(*i);
  };
}

void clj_print(clj_Printer *p, const clj_Node *node) {
  switch (node->type) {

    case CLJ_NUMBER:
    case CLJ_STRING:
    case CLJ_REGEX:
    case CLJ_SYMBOL:
    case CLJ_KEYWORD:
    case CLJ_CHARACTER:
      print_string(p, node->value);
      break;

    case CLJ_LIST:
      p->putwchar(L'(');
      break;
    case CLJ_LIST | CLJ_END:
      p->putwchar(L')');
      break;

    case CLJ_VECTOR:
      p->putwchar(L'[');
      break;
    case CLJ_VECTOR | CLJ_END:
      p->putwchar(L']');
      break;

    case CLJ_MAP:
      p->putwchar(L'{');
      break;
    case CLJ_MAP | CLJ_END:
      p->putwchar(L'}');
      break;

    case CLJ_SET:
      print_string(p, L"#{");
      break;
    case CLJ_SET | CLJ_END:
      p->putwchar(L'}');
      break;

    default:
      fatal("unexpected node type");
  }
  p->putwchar(L'\n');
}
