#pragma once
#include "core/expr.h"
#include "core/vm/vm.h"

extern int funutil_expect(struct Expr* expr, const char* fname, int num, ...);
extern int funutil_getArgs(struct Expr* expr, int num, ...);

struct FFIDescriptor {
  // Function name (so it can be referenced in the program)
  char* fname;

  // Pointer to the function
  CompiledFun fpointer;
};
