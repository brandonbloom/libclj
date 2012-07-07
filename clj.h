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

struct clj_named {
  const wchar_t *ns;
  const wchar_t *name;
};

struct clj_node {
  enum {
    CLJ_ERROR = -1,
    // Atomic values
    CLJ_INTEGER = 1,
    CLJ_RATIO,
    CLJ_FLOATING,
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
  } type;
  union {
    long integer;
    struct {
      long numerator;
      long denominator;
    } ratio;
    double floating;
    wint_t character;
    const wchar_t *string;
    struct clj_named keyword;
    struct clj_named symbol;
  } value;
};

struct clj_parser {
  wint_t (*getwchar)(void);
  int (*emit)(const struct clj_node*);
  //TODO: int line;
  //TODO: int column;
  wint_t _readback;
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
