#ifndef CLJ_H
#define CLJ_H

#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

struct clj_named {
  const wchar_t *ns;
  const wchar_t *name;
};

struct clj_node {
  enum {
    // Atomic values
    CLJ_INTEGER = 1,
    CLJ_RATIO,
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
    wint_t character;
    const wchar_t *string;
    struct clj_named keyword;
    struct clj_named symbol;
  } value;
};

struct clj_parser {
  wint_t (*getwchar)(void);
  int (*emit)(const struct clj_node*);
  const wint_t readback;
};

int clj_read(struct clj_parser*);

struct clj_printer {
  wint_t (*putwchar)(wchar_t c);
  int (*consume)(struct clj_node*);
};

int clj_print(struct clj_printer*);

#ifdef __cplusplus
}
#endif

#endif /* CLJ_H */
