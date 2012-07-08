#ifndef CLJ_H
#define CLJ_H

#include <wchar.h>
#include <setjmp.h>  //TODO: Only needed privately?

#ifdef __cplusplus
extern "C" {
#endif

typedef enum clj_read_result {
  CLJ_SUCCESS = 0,
  CLJ_UNMATCHED_DELIMITER,
  CLJ_NOT_IMPLEMENTED,
} clj_ReadResult;

//typedef struct clj_named {
//  const wchar_t *ns;
//  const wchar_t *name;
//} clj_Named;

typedef enum clj_type {
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
} clj_Type;

typedef struct clj_node {
  clj_Type type;
  const wchar_t *value;
} clj_Node;

typedef struct clj_reader {
  // Read/write
  wint_t (*getwchar)(void);
  int (*emit)(const clj_Node*);
  // Read-only
  int line;
  int column;
  // Private
  wint_t _readback;
  wint_t _readback_column;
  jmp_buf _fail;
} clj_Reader;

clj_ReadResult clj_read(clj_Reader*);

typedef struct clj_printer {
  wint_t (*putwchar)(wchar_t c);
  int (*consume)(clj_Node*);
} clj_Printer;

int clj_print(clj_Printer*);

#ifdef __cplusplus
}
#endif

#endif /* CLJ_H */
