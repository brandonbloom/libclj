#ifndef CLJ_H
#define CLJ_H

#include <wchar.h>
#include <setjmp.h>  //TODO: Only needed privately?

#ifdef __cplusplus
extern "C" {
#endif

enum clj_read_result {
  CLJ_SUCCESS = 0,
  CLJ_UNMATCHED_DELIMITER,
  CLJ_NOT_IMPLEMENTED,
};

//struct clj_named {
//  const wchar_t *ns;
//  const wchar_t *name;
//};

enum clj_type {
  CLJ_ERROR = -1,
  // Atomic values
  CLJ_NUMBER = 1,
  CLJ_CHARACTER,
  CLJ_STRING,
  CLJ_KEYWORD,
  CLJ_SYMBOL,
  // Push collection
  CLJ_MAP,
  CLJ_LIST,
  CLJ_SET,
  CLJ_VECTOR,
  // Pop collection
  CLJ_END,
};

struct clj_node {
  enum clj_type type;
  const wchar_t *value;
};

struct clj_parser {
  // Read/write
  wint_t (*getwchar)(void);
  int (*emit)(const struct clj_node*);
  // Read-only
  int line;
  int column;
  // Private
  wint_t _readback;
  wint_t _readback_column;
  jmp_buf _fail;
};

enum clj_read_result clj_read(struct clj_parser*);

struct clj_printer {
  wint_t (*putwchar)(wchar_t c);
  int (*consume)(struct clj_node*);
};

int clj_print(struct clj_printer*);

#ifdef __cplusplus
}
#endif

#endif /* CLJ_H */
