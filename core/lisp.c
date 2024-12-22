#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include "expr.h"
#include "parse.h"
#include "ffi/funutil.h"
#include "vm/vm.h"
#include "mem.h"

static volatile int running = 1;
const char* exitmsg = "Press return to exit\n";

void intHandler(int s) {
  write(1, exitmsg, strlen(exitmsg));
  running = 0;
}

void repl(struct LispVm* env, struct MemoryManager* manager) {
  char buf[2048];
  printf("lisp> ");
  if (fgets(buf, 2048, stdin) == NULL)
    return;

  if (!running)
    return;

  struct Expr* expr = parse(manager, buf);
  if (expr == NULL)
    return;

  struct Expr* res = mem_allocExpr(manager);
  int err = vm_eval(env, expr->l, 1, &res);
  if (!err) {
    printf("\n");
    expr_dump_pretty(res, 1);
    printf("\n\n");
  }

}

int main(int argc, char* argv[]) {
  signal(SIGINT, intHandler);

  struct MemoryManager* manager = memmanager_init();
  struct LispVm* vm = vm_new(manager);
  vm_loadlib(vm, "stdlib/stdlib.so");

  // REPL
  if (argc == 1) {
    while (running) {
      repl(vm, manager);
      mem_sweep(manager, vm->bindings);
    }
  }

  //  Read file
  else if (argc == 2) {
    FILE* f = fopen(argv[1], "r");
    if (f == NULL) {
      fprintf(stderr, "[ERROR] Could not find file %s\n", argv[1]);
      return 1;
    }

    fseek(f, 0, SEEK_END);
    int end = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buf = malloc(end + 1);
    if (buf == 0) {
      fprintf(stderr, "[ERROR] Failed to allocate file buffer\n");
      return 1;
    }
    buf[end] = '\0';

    if (fread(buf, 1, end, f) < end) {
      fprintf(stderr, "[ERROR] Failed to read whole file\n");
      return 1;
    }

    struct Expr* expr = parse(manager, buf);
    if (expr == NULL)
      return 1;

    struct Expr* res = mem_allocExpr(manager);
    while (expr != NULL) {
      vm_eval(vm, expr->l, 1, &res);
      expr = expr->r;
    }
    fclose(f);
    free(buf);
  }
  else {

  }

  vm_free(vm);
  memmanager_free(manager);
}
