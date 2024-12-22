#include "mem.h"
#include <stdlib.h>
#include <string.h>

struct MemoryManager* memmanager_init() {
  struct MemoryManager* manager = malloc(sizeof(struct MemoryManager));
  manager->exprs = malloc(sizeof(struct MemoryManagerNode));
  manager->constants = malloc(sizeof(struct MemoryManagerNode));

  if (manager->exprs == NULL || manager->constants == NULL)
    return NULL;

  *manager->exprs = (struct MemoryManagerNode){.addr = NULL, .next = NULL};
  *manager->constants = (struct MemoryManagerNode){.addr = NULL, .next = NULL};

  return manager;
}

void memmanager_free(struct MemoryManager* manager) {
  struct MemoryManagerNode* index = manager->exprs;
  struct MemoryManagerNode* prev = index;

  while (index != NULL) {
    if (index->addr != NULL)
      free(index->addr);

    prev = index;
    index = index->next;
    free(prev);
  }

  index = manager->constants;
  prev = index;
  while (index != NULL) {
    if (index->addr != NULL)
      free(index->addr);

    prev = index;
    index = index->next;
    free(prev);
  }
  free(manager);
}

struct Expr* mem_allocExpr(struct MemoryManager* manager) {
  struct Expr* expr = malloc(sizeof(struct Expr));
  struct MemoryManagerNode* node = malloc(sizeof(struct MemoryManagerNode));
  if (expr == NULL || node == NULL)
    return NULL;

  *expr = (struct Expr){0};
  expr->freeable = 1;

  node->addr = expr;
  node->next = manager->exprs->next;
  manager->exprs->next = node;
  return expr;
}

char* mem_allocConstant(struct MemoryManager* manager, int size) {
  char* str = malloc(size + 1);
  struct MemoryManagerNode* node = malloc(sizeof(struct MemoryManagerNode));
  if (str == NULL || node == NULL)
    return NULL;

  str[size] = '\0';
  node->addr = str;
  node->next = manager->constants->next;

  manager->constants->next = node;
  return str;
}

char* mem_allocConstantCopy(struct MemoryManager* manager, const char* str) {
  char* strAllocd = mem_allocConstant(manager, strlen(str));
  if (strAllocd == NULL)
    return NULL;

  strcpy(strAllocd, str);
  return strAllocd;
}

void mem_mark(struct Expr* expr, int freeable) {
  if (expr == NULL)
    return;

  expr->freeable = freeable;
  if (expr->type == E_LIST || expr->type == E_LAMBDA) {
    mem_mark(expr->l, freeable);
    mem_mark(expr->r, freeable);
  }
}

void mem_sweep(struct MemoryManager* manager, struct VmBinding* bindings) {
  struct VmBindingEntry* item;
  size_t iter = 0;
  while (hashmap_iter(bindings->map, &iter, (void**)&item))
    mem_mark(item->value, 0);

  struct MemoryManagerNode* prev = manager->exprs;
  struct MemoryManagerNode* exprs = prev->next;
  while (exprs != NULL) {
    if (exprs->addr != NULL) {
      if (((struct Expr*)exprs->addr)->freeable) {
        struct MemoryManagerNode* tofree = exprs;

        exprs = exprs->next;
        prev->next = exprs;

        free(tofree->addr);
        free(tofree);
        continue;
      }
    }

    prev = exprs;
    exprs = exprs->next;
  }
}
