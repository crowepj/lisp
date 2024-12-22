#include "vmbinding.h"
#include <stdlib.h>
#include <string.h>

int vmbinding_compare(const void *a, const void *b, void *udata) {
  const struct VmBindingEntry* ae = a;
  const struct VmBindingEntry* be = b;
  return strcmp(ae->key, be->key);
}

uint64_t vmbinding_hash(const void *item, uint64_t seed0, uint64_t seed1) {
  const struct VmBindingEntry* entry = item;
  return hashmap_sip(entry->key, strlen(entry->key), seed0, seed1);
}

struct VmBinding* vmbinding_new() {
  struct VmBinding* ret = malloc(sizeof(struct VmBinding));
  if (ret == NULL)
    return NULL;

  ret->parent = NULL;
  ret->map = hashmap_new(sizeof(struct VmBindingEntry), 0, 0, 0, vmbinding_hash, vmbinding_compare, NULL, NULL);
  return ret;
}

void vmbinding_free(struct VmBinding* bindings) {
  hashmap_free(bindings->map);
  free(bindings);
}

int vmbinding_set(struct VmBinding* bindings, const char* key, struct Expr* value) {
  hashmap_set(bindings->map, &(struct VmBindingEntry){.key = key, .value = value});
  return 0;
}

int vmbinding_get(struct VmBinding* bindings, const char* key, struct Expr** out) {
  const struct VmBindingEntry* item = hashmap_get(bindings->map, &(struct VmBindingEntry){.key = key});
  if (item == NULL) {
    if (bindings->parent != NULL)
      return vmbinding_get(bindings->parent, key, out);
    else {
      return 1;
    }
  }

  *out = item->value;
  return 0;
}
