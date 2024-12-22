#pragma once
#include "vm/vmbinding.h"
#include "expr.h"

struct MemoryManagerNode {
  void* addr;
  struct MemoryManagerNode* next;
};

struct MemoryManager {
  struct MemoryManagerNode* exprs;
  struct MemoryManagerNode* constants;
};

struct MemoryManager* memmanager_init();
struct Expr* mem_allocExpr(struct MemoryManager* manager);
char* mem_allocConstant(struct MemoryManager* manager, int size);
char* mem_allocConstantCopy(struct MemoryManager* manager, const char* str);
void mem_sweep(struct MemoryManager* manager, struct VmBinding* bindings);
