#include "ffi/ffi.h"
#include "ffi/funutil.h"
#include <stdio.h>

int std_add(struct LispVm* vm, struct Expr* expr, struct Expr** out) {
  if (funutil_expect(expr, "+", 2, E_NUMBER, E_NUMBER))
    return 1;

  struct Expr* n1;
  struct Expr* n2;

  if (funutil_getArgs(expr, 2, &n1, &n2))
    return 1;

  *out = mem_allocExpr(vm->manager);
  (*out)->type = E_NUMBER;
  (*out)->data.num = n1->data.num + n2->data.num;
  return 0;
}

int std_sub(struct LispVm* vm, struct Expr* expr, struct Expr** out) {
  if (funutil_expect(expr, "-", 2, E_NUMBER, E_NUMBER))
    return 1;

  struct Expr* n1;
  struct Expr* n2;

  if (funutil_getArgs(expr, 2, &n1, &n2))
    return 1;

  *out = mem_allocExpr(vm->manager);
  (*out)->type = E_NUMBER;
  (*out)->data.num = n1->data.num - n2->data.num;
  return 0;
}

int std_mul(struct LispVm* vm, struct Expr* expr, struct Expr** out) {
  if (funutil_expect(expr, "*", 2, E_NUMBER, E_NUMBER))
    return 1;

  struct Expr* n1;
  struct Expr* n2;

  if (funutil_getArgs(expr, 2, &n1, &n2))
    return 1;

  *out = mem_allocExpr(vm->manager);
  (*out)->type = E_NUMBER;
  (*out)->data.num = n1->data.num * n2->data.num;
  return 0;
}

int std_div(struct LispVm* vm, struct Expr* expr, struct Expr** out) {
  if (funutil_expect(expr, "/", 2, E_NUMBER, E_NUMBER))
    return 1;

  struct Expr* n1;
  struct Expr* n2;

  if (funutil_getArgs(expr, 2, &n1, &n2))
    return 1;

  *out = mem_allocExpr(vm->manager);
  (*out)->type = E_NUMBER;
  (*out)->data.num = n1->data.num / n2->data.num;
  return 0;
}

int std_eq(struct LispVm* vm, struct Expr* expr, struct Expr** out) {
  if (funutil_expect(expr, "=", 2, E_NUMBER, E_NUMBER))
    return 1;

  struct Expr* n1;
  struct Expr* n2;

  if (funutil_getArgs(expr, 2, &n1, &n2))
    return 1;

  *out = mem_allocExpr(vm->manager);
  (*out)->type = E_NUMBER;
  (*out)->data.num = n1->data.num == n2->data.num;
  return 0;
}

int std_less(struct LispVm* vm, struct Expr* expr, struct Expr** out) {
  if (funutil_expect(expr, "<", 2, E_NUMBER, E_NUMBER))
    return 1;

  struct Expr* n1;
  struct Expr* n2;

  if (funutil_getArgs(expr, 2, &n1, &n2))
    return 1;

  *out = mem_allocExpr(vm->manager);
  (*out)->type = E_NUMBER;
  (*out)->data.num = n1->data.num < n2->data.num;
  return 0;
}

int std_greater(struct LispVm* vm, struct Expr* expr, struct Expr** out) {
  if (funutil_expect(expr, ">", 2, E_NUMBER, E_NUMBER))
    return 1;

  struct Expr* n1;
  struct Expr* n2;

  if (funutil_getArgs(expr, 2, &n1, &n2))
    return 1;

  *out = mem_allocExpr(vm->manager);
  (*out)->type = E_NUMBER;
  (*out)->data.num = n1->data.num > n2->data.num;
  return 0;
}

int std_list(struct LispVm* vm, struct Expr* expr, struct Expr** out) {
  *out = expr;
  return 0;
}

int std_cons(struct LispVm* vm, struct Expr* expr, struct Expr** out) {
  if (funutil_expect(expr, "cons", 2, E_ANY, E_ANY))
    return 1;

  struct Expr* n1;
  struct Expr* n2;

  if (funutil_getArgs(expr, 2, &n1, &n2))
    return 1;

  *out = mem_allocExpr(vm->manager);
  (*out)->l = n1;
  (*out)->r = n2;
  (*out)->type = E_LIST;

  return 0;
}

int std_car(struct LispVm* vm, struct Expr* expr, struct Expr** out) {
  if (funutil_expect(expr, "car", 1, E_LIST))
    return 1;
  *out = expr->l->l;
  return 0;
}

int std_cdr(struct LispVm* vm, struct Expr* expr, struct Expr** out) {
  if (funutil_expect(expr, "car", 1, E_LIST))
    return 1;

  if (expr->l->r == NULL)
    *out = expr->r;
  else
    *out = expr->l->r;
  return 0;
}

int std_null(struct LispVm* vm, struct Expr* expr, struct Expr** out) {
  if (funutil_expect(expr, "null?", 1, E_ANY))
    return 1;

  *out = mem_allocExpr(vm->manager);
  (*out)->type = E_NUMBER;
  (*out)->data.num = (expr->l->l == NULL && expr->r->r == NULL);
  return 0;
}

int std_print(struct LispVm* vm, struct Expr* expr, struct Expr** out) {
  struct Expr* s;
  //expr = expr->r;
  while (expr != NULL && expr->l != NULL) {
    expr_dump_pretty(expr->l, 0);
    expr = expr->r;
  }

  *out = NULL;

  return 0;
}

static const struct FFIDescriptor descriptor[] = {
  // Arithmetic operations
  {.fname = "+", .fpointer = std_add},
  {.fname = "-", .fpointer = std_sub},
  {.fname = "*", .fpointer = std_mul},
  {.fname = "/", .fpointer = std_div},
  {.fname = "=", .fpointer = std_eq},
  {.fname = "<", .fpointer = std_less},
  {.fname = ">", .fpointer = std_greater},

  // List operations
  {.fname = "list", .fpointer = std_list},
  {.fname = "car", .fpointer = std_car},
  {.fname = "cdr", .fpointer = std_cdr},
  {.fname = "cons", .fpointer = std_cons},
  {.fname = "null?", .fpointer = std_null},
  {.fname = "print", .fpointer = std_print},
  {.fname = NULL, .fpointer = NULL}
};

const struct FFIDescriptor* methods() {
  return descriptor;
}
