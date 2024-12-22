#pragma once
#include "expr.h"

int funutil_expect(struct Expr* expr, const char* fname, int num, ...);
int funutil_getArgs(struct Expr* expr, int num, ...);
