#pragma once
#include <expr.h>
#include "vmbinding.h"
#include <mem.h>

struct LispVm {
  struct MemoryManager* manager;
  struct VmBinding* bindings;
  struct {
    int n;
    void** d;
  } foreign;
};

struct LispVm* vm_new(struct MemoryManager* manager);
void vm_free(struct LispVm* vm);

int vm_loadlib(struct LispVm* vm, const char* path);
int vm_exec(const char* path);
int vm_eval(struct LispVm* vm, struct Expr* arg, int left, struct Expr** out);
