#include "funutil.h"
#include <stdarg.h>
#include <stdio.h>

int funutil_expect(struct Expr* expr, const char* fname, int num, ...) {
  if (expr == NULL) {
    if (num == 0) {
      fprintf(stderr, "[ERROR] Function %s expected %i argument(s), got 0\n", fname, num);
      return 1;
    }
    return 0;
  }

  va_list valist;
  va_start(valist, num);

  enum ExprType type;
  for (int i = 0; i < num; i++) {
    type = va_arg(valist, enum ExprType);

    // Not enough arguments
    if (expr == NULL || expr->l == NULL) {
      fprintf(stderr, "[ERROR] Function %s expected %i argument(s), got %i\n", fname, num, i);
      va_end(valist);
      return 1;
    }

    // Argument of incorrect type
    else if (type != E_ANY && expr->l->type != type) {
      fprintf(stderr, "[ERROR] Function %s expected argument %i of type %s, got type %s\n", fname, i, exprTypeName[type], exprTypeName[expr->l->type]);
      va_end(valist);
      return 1;
    }
    expr = expr->r;
  }

  // Too many arguments
  if (expr != NULL && expr->l != NULL) {
    fprintf(stderr, "[ERROR] Function %s expected %i argument(s), got %i\n", fname, num, expr_count(expr, num));
    va_end(valist);
    return 1;
  }

  va_end(valist);
  return 0;
}

// TODO combine getArgs and expect
// This function assumes all arguments are present, so call funutil_expect before using lisp_args
int funutil_getArgs(struct Expr* expr, int num, ...) {
  va_list valist;
  va_start(valist, num);

  for (int i = 0; i < num; i++) {
    struct Expr** arg = va_arg(valist, struct Expr**);
    *arg = expr->l;
    expr = expr->r;
  }

  va_end(valist);
  return 0;
}
