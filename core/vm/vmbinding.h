#pragma once
#include <expr.h>
#include <hashmap/hashmap.h>

struct VmBindingEntry {
  const char* key;
  struct Expr* value;
};

struct VmBinding {
  struct hashmap* map;
  struct VmBinding* parent;
};

struct VmBinding* vmbinding_new();
void vmbinding_free(struct VmBinding* bindings);
int vmbinding_set(struct VmBinding* bindings, const char* key, struct Expr* value);
int vmbinding_get(struct VmBinding* bindings, const char* key, struct Expr** out);
