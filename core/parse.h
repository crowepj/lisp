#pragma once
#include "expr.h"
#include "vm/vm.h"
#include "mem.h"

struct Expr* parse(struct MemoryManager* manager, const char* str);
