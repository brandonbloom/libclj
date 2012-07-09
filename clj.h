#ifndef CLJ_H
#define CLJ_H

#include <wchar.h>
#include <setjmp.h>  //TODO: Only needed privately?

#ifdef __cplusplus
extern "C" {
#endif

typedef enum clj_result {
  CLJ_EOF  = 1,
  CLJ_MORE = 2,
  CLJ_UNEXPECTED_EOF      = -1,
  CLJ_UNMATCHED_DELIMITER = -2,
  CLJ_NOT_IMPLEMENTED     = -3,
  CLJ_UNREADABLE          = -4,
} clj_Result;

//typedef struct clj_named {
//  const wchar_t *ns;
//  const wchar_t *name;
//} clj_Named;

typedef enum clj_type {
  CLJ_ERROR = -1,
  // Atomic values
  CLJ_NUMBER    = 0x01,
  CLJ_CHARACTER = 0x02,
  CLJ_STRING    = 0x03,
  CLJ_KEYWORD   = 0x04,
  CLJ_SYMBOL    = 0x05,
  CLJ_REGEX     = 0x06,
  // Composites
  CLJ_MAP        = 0x010,
  CLJ_LIST       = 0x020,
  CLJ_SET        = 0x030,
  CLJ_VECTOR     = 0x040,
  // Pop collection bit flag
  CLJ_END        = 0x100,
} clj_Type;

typedef struct clj_node {
  clj_Type type;
  const wchar_t *value;
} clj_Node;

typedef struct clj_reader {
  // Read/write
  wint_t (*getwchar)(void);
  void (*emit)(const clj_Node*);
  // Read-only
  int line;
  int column;
  // Private
  int _depth; // composite type nesting depth, but uses -1 as "got a top-level"
  wint_t _readback;
  wint_t _readback_column;
  jmp_buf _fail;
} clj_Reader;

clj_Result clj_read(clj_Reader*);

typedef struct clj_printer {
  wint_t (*putwchar)(wchar_t c);
  //TODO: line/column/depth
} clj_Printer;

void clj_print(clj_Printer*, const clj_Node*);

#ifdef __cplusplus
}
#endif

#endif /* CLJ_H */
