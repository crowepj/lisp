#pragma once

enum ExprType {
  E_ATOM = 0,
  E_LIST = 1,
  E_STRING = 2,
  E_NUMBER = 3,
  E_COMPILED,
  E_LAMBDA,

  // Only used in auxiliary functions
  E_ANY,
};

const static char* exprTypeName[] = {
  "Atom",
  "List",
  "String",
  "Number",
  "Compiled",
  "Lambda",
  "Any [If you see this, an error occured]"
};

struct LispVm;
struct Expr;
typedef int (*CompiledFun)(struct LispVm*, struct Expr*, struct Expr**);

union ExprData {
    char* s;
    double num;

    // Pointer to compiled function
    CompiledFun compiled;
};

struct Expr {
  int freeable: 1;
  enum ExprType type;
  union ExprData data;
  struct Expr* l;
  struct Expr* r;
};

struct Expr* expr_new(struct Expr* l, struct Expr* r);
struct Expr* expr_atom(const char* str);
struct Expr* expr_num(double num);

struct Expr* expr_arg(struct Expr* expr, int* arg);
int expr_count(struct Expr* expr, int start);

void expr_dump(struct Expr* expr);
void expr_dump_pretty(struct Expr* expr, int space);
void expr_free(struct Expr* expr, int recursive);
