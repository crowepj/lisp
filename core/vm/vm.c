#include "vm.h"
#include "ffi/funutil.h"
#include "ffi/ffi.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

struct LispVm* vm_new(struct MemoryManager* manager) {
  struct LispVm* ret = malloc(sizeof(struct LispVm));
  if (ret == NULL)
    return NULL;

  ret->bindings = vmbinding_new();
  if (ret->bindings == NULL)
    return NULL;

  ret->manager = manager;
  ret->foreign.d = NULL;
  ret->foreign.n = 0;
  return ret;
}

void vm_free(struct LispVm* vm) {
  for (int i = 0; i < vm->foreign.n; i++) {
    dlclose(vm->foreign.d[i]);
  }
  free(vm->foreign.d);
  vmbinding_free(vm->bindings);
  free(vm);
}

int vm_loadlib(struct LispVm* vm, const char* path) {
  void* handle = dlopen(path, RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "%s\n", dlerror());
    return 1;
  }

  // Clear errors
  dlerror();

  char* error;
  struct FFIDescriptor* (*methodsFun)(void) = dlsym(handle, "methods");
  if ((error = dlerror()) != NULL)  {
    fprintf(stderr, "%s\n", error);
    return 1;
  }

  struct FFIDescriptor* methods = methodsFun();
  if (methods == NULL) {
    fprintf(stderr, "[ERROR] FFI Library methods() function returned NULL\n");
    return 1;
  }

  int i = 0;
  while (methods[i].fname != NULL) {
    struct Expr* funE = mem_allocExpr(vm->manager);
    funE->type = E_COMPILED;
    funE->data.compiled = methods[i].fpointer;
    vmbinding_set(vm->bindings, methods[i].fname, funE);
    i++;
  }
  void** temp = realloc(vm->foreign.d, (vm->foreign.n + 1) * sizeof(void*));
  if (temp == NULL)
    return 1;

  vm->foreign.d = temp;
  vm->foreign.d[vm->foreign.n++] = handle;
  return 0;
}

// Evaluate the definition of a lambda function
int vm_evalLambdaDef(struct LispVm* vm, struct Expr* arg, struct Expr** out) {
  if (funutil_expect(arg->r, "lambda", 2, E_LIST, E_ANY))
    return 1;

  struct Expr* lambdaArgs = arg->r->l;
  while (lambdaArgs != NULL) {
    if (lambdaArgs->l != NULL && lambdaArgs->l->type != E_ATOM) {
      fprintf(stderr, "[ERROR] Ill-formed syntax: first argument of lambda expects list of atoms, got list containing %s\n", exprTypeName[lambdaArgs->l->type]);
      return 1;
    }

    lambdaArgs = lambdaArgs->r;
  }

  *out = arg->r;
  (*out)->type = E_LAMBDA;
  return 0;
}

int vm_evalDefine(struct LispVm* vm, struct Expr* arg, struct Expr** out) {
  if (funutil_expect(arg->r, "define", 2, E_ATOM, E_ANY))
    return 1;

  struct Expr* argname;
  struct Expr* value;
  funutil_getArgs(arg->r, 2, &argname, &value);

  struct Expr* valueEval = mem_allocExpr(vm->manager);
  if (vm_eval(vm, value, 1, &valueEval))
    return 1;

  *out = argname;
  return vmbinding_set(vm->bindings, argname->data.s, valueEval);
}

int vm_evalIf(struct LispVm* vm, struct Expr* arg, struct Expr** out) {
  if (funutil_expect(arg->r, "if", 3, E_ANY, E_ANY, E_ANY))
    return 1;

  struct Expr* cond;
  struct Expr* trueRes;
  struct Expr* falseRes;
  funutil_getArgs(arg->r, 3, &cond, &trueRes, &falseRes);

  struct Expr* condEval;
  if (vm_eval(vm, cond, 1, &condEval))
    return 1;

  if (condEval->type != E_NUMBER) {
    fprintf(stderr, "[ERROR] Function if expected argument 0 of type Number, got %s\n", exprTypeName[cond->type]);
    return 1;
  }

  if (condEval->data.num)
    return vm_eval(vm, trueRes, 1, out);
  else
    return vm_eval(vm, falseRes, 1, out);
}

// Evaluate/call a lambda function
int vm_evalLambda(struct LispVm* vm, struct Expr* arg, struct Expr** out) {
  // List of argument names that the lambda takes
  struct Expr* arglistp = arg->l->l;
  int arglistCount = expr_count(arglistp, 0);

  // Arguments passed to the lambda function
  struct Expr* argp = arg->r;
  int argpassedCount = expr_count(argp, 0);

  if (argpassedCount != arglistCount) {
    fprintf(stderr, "[ERROR] Function [lambda] expected %i argument(s), got %i\n", arglistCount, argpassedCount);
    return 1;
  }

  // Body of lambda to be executed
  struct Expr* bodyp = arg->l->r->l;

  struct VmBinding* lambdaBinding = vmbinding_new();
  lambdaBinding->parent = vm->bindings;

  while (arglistp != NULL && arglistp->l != NULL) {
    if (arglistp->l->type != E_ATOM) {
      fprintf(stderr, "[ERROR] Internal Error - Lambda Argument List contained non-atom expression\n");
      return 1;
    }
    if (vmbinding_set(lambdaBinding, arglistp->l->data.s, argp->l))
      return 1;
    argp = argp->r;
    arglistp = arglistp->r;
  }

  vm->bindings = lambdaBinding;

  struct Expr* ret;
  while (bodyp != NULL && bodyp->l != NULL) {
    if (vm_eval(vm, bodyp->l, 1, &ret))
      return 1;
    bodyp = bodyp->r;
  }

  *out = ret;

  vm->bindings = lambdaBinding->parent;
  vmbinding_free(lambdaBinding);
  return 0;
}

int vm_eval(struct LispVm* vm, struct Expr* arg, int left, struct Expr** out) {
  if (vm == NULL)
    return 1;

  // Null argument does not indicate error
  if (arg == NULL) {
    *out = arg;
    return 0;
  }

  // Number/string (i.e. a constant value) evaluates to itself
  if (arg->type == E_NUMBER || arg->type == E_STRING) {
    *out = arg;
    return 0;
  }

  // Atom evaluates to its bound value
  else if (arg->type == E_ATOM) {
    if (vmbinding_get(vm->bindings, arg->data.s, out)) {
      fprintf(stderr, "[ERROR] Could not find symbol %s\n", arg->data.s);
      return 1;
    }
    return 0;
  }

  else if (arg->type == E_LIST) {
    struct Expr* ret = mem_allocExpr(vm->manager);
    if (ret == NULL)
      return 1;

    ret->type = E_LIST;

    // If left == 1, it means we're the left hand child of a list
    // This means it needs to be evaluated - the left child is the function to apply, the right side is the list of arguments
    if (left && arg->l != NULL) {

      // Special cases (syntactic keywords)
      if (arg->l->type == E_ATOM) {
        if (strcmp(arg->l->data.s, "lambda") == 0)
          return vm_evalLambdaDef(vm, arg, out);

        // Define needs its 1st argument unevaluated (the name of the variable being defined)
        else if (strcmp(arg->l->data.s, "define") == 0)
          return vm_evalDefine(vm, arg, out);

        // If needs its arguments conditionally evaluated (Evaluate the condition first, if true, evaluate true branch, if false, evaluate false branch)
        else if (strcmp(arg->l->data.s, "if") == 0)
          return vm_evalIf(vm, arg, out);

        else if (strcmp(arg->l->data.s, "quote") == 0) {
          *out = arg->r;
          return 0;
        }
      }

      // Not a special case/syntactic keyword, so it's okay to evaluate the left and right hand side
      if (vm_eval(vm, arg->l, 1, &ret->l) || vm_eval(vm, arg->r, 0, &ret->r))
        return 1;

      // Compiled function (i.e. set from within C)
      if (ret->l->type == E_COMPILED)
        return ret->l->data.compiled(vm, ret->r, out);

      // Lambda function (defined from within the program)
      else if (ret->l->type == E_LAMBDA)
        return vm_evalLambda(vm, ret, out);

      // Attempting to apply non-function value
      else {
        printf("ERR\n");
        return 1;
      }
    }

    else if (vm_eval(vm, arg->l, 1, &ret->l) || vm_eval(vm, arg->r, 0, &ret->r))
      return 1;

    *out = ret;
  }

  // This shouldn't happen
  else
    return 1;

  return 0;
}
