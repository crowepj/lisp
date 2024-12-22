#include "expr.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

struct Expr* expr_new(struct Expr* l, struct Expr* r) {
  struct Expr* expr = malloc(sizeof(struct Expr));
  if (expr == NULL)
    return NULL;

  expr->type = E_LIST;
  expr->l = l;
  expr->r = r;
  return expr;
}

struct Expr* expr_atom(const char* str) {
  struct Expr* expr = malloc(sizeof(struct Expr));
  if (expr == NULL)
    return NULL;

  expr->type = E_ATOM;
  expr->data.s = malloc(strlen(str) + 1);
  if (expr->data.s == NULL) {
    free(expr);
    return NULL;
  }
  strcpy(expr->data.s, str);
  return expr;
}

struct Expr* expr_num(double num) {
  struct Expr* expr = malloc(sizeof(struct Expr));
  if (expr == NULL)
    return NULL;

  expr->type = E_NUMBER;
  expr->data.num = num;
  return expr;
}

struct Expr* expr_arg(struct Expr* expr, int* arg) {
  if (expr == NULL)
    return NULL;
  else if (expr->type != E_LIST)
    return NULL;
  else if (*arg <= 0)
    return expr->l;
  else {
    (*arg)--;
    return expr_arg(expr->r, arg);
  }
}

int expr_count(struct Expr* expr, int start) {
  if (expr == NULL || expr->l == NULL)
    return start;

  start++;
  if (expr->r == NULL)
    return start;

  return expr_count(expr->r, start);
}

void expr_free(struct Expr* expr, int recursive) {
  if (expr == NULL)
    return;

  if ((expr->type == E_LIST || expr->type == E_LAMBDA) && recursive) {
    if (expr->l != NULL)
      expr_free(expr->l, 1);
    if (expr->r != NULL)
      expr_free(expr->r, 1);
  }
  free(expr);
}

void expr_dump_pretty(struct Expr* expr, int space) {
  if (expr == NULL)
    return;

  if (expr->type == E_LIST || expr->type == E_LAMBDA) {
    if (expr->l == NULL) {
      expr_dump_pretty(expr->r, space);
      return;
    }

    if (expr->l->type == E_LIST) {
      printf("(");
      expr_dump_pretty(expr->l, space);
      printf(")");
    }
    else
      expr_dump_pretty(expr->l, space);

    if (expr->r != NULL)
      printf(" ");
    
    expr_dump_pretty(expr->r, space);
  }
  else {
    switch (expr->type) {
    case E_COMPILED:
      printf("[compiled fun]");
      break;
    case E_ATOM:
    case E_STRING:
      printf("%s", expr->data.s);
      break;
    case E_NUMBER:
      printf("%.3f", expr->data.num);
      break;

    default:
      break;
    }
  }
}
