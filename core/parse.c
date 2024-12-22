
#include "parse.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include "expr.h"

struct Token {
  enum {
    // Token types to coincide with Expr types
    T_ATOM = 0,
    T_STRING = 2,
    T_NUMBER = 3,
    T_OPEN,
    T_CLOSE,
  } type;
  union ExprData data;
};

uint8_t isatom(char c) {
  if (isalpha(c) != 0 || c == '+' || c == '-' || c == '/' || c == '*' || c == '=' || c == '>' || c == '<' || c == '?')
    return 1;
  return 0;
}

void token_free(struct Token* tokens, int len) {
  for (int i = 0; i < len; i++) {
    struct Token tok = tokens[i];
    if (tok.type == T_ATOM || tok.type == T_STRING)
      free(tok.data.s);
  }
  free(tokens);
}

struct Token* lex(const char* str, int* len) {
  int tokensLen = 0;
  int tokenI = 0;
  struct Token* tokens = malloc(sizeof(struct Token) * tokensLen);
  if (tokens == NULL)
    return NULL;

  int nesting = 0;
  int i = 0;
  while (i < strlen(str)) {
    char c = str[i];
    struct Token tok;

    if (c == '(') {
      tok.type = T_OPEN;
      nesting++;
      i++;
    }
    else if (c == ')') {
      tok.type = T_CLOSE;
      nesting--;
      if (nesting < 0) {
        fprintf(stderr, "[ERROR] Mismatched \')\' at index %i\n", i);
        token_free(tokens, tokenI);
        *len = 0;
        return NULL;
      }
      i++;
    }
    else if ((isdigit(c) != 0) || ((c == '+' || c == '-') && isdigit(str[i + 1]) != 0)) {
      char* endptr;
      double num = strtod(str + i, &endptr);
      if (endptr == NULL) {
      	// Error
      	printf("[ERROR] Failed to parse number beginning at index %i\n", i);
        token_free(tokens, tokenI);
        *len = 0;
        return NULL;
      }
      else {
      	tok.type = T_NUMBER;
      	tok.data.num = num;
      	i = endptr - str;
      }
    }
    else if (c == '\"') {
      int strStart = i++;
      int esc = 0;

      tok.type = T_STRING;
      tok.data.s = malloc(0);
      int stri = 0;

      char app = '\0';
      while ((str[i] != '\"' || esc) && i < strlen(str)) {
        if (esc) {
          if (str[i] == '\\')
            app = '\\';
          else if (str[i] == 'n')
            app = '\n';
          else if (str[i] == 't')
            app = '\t';
          else
            app = str[i];
          esc = 0;
        }
        else if (str[i] == '\\') {
          esc = 1;
          i++;
          continue;
        }
        else
          app = str[i];

        void* temp = realloc(tok.data.s, stri + 2);
        if (temp == NULL)
          return NULL;

        tok.data.s = temp;
        tok.data.s[stri++] = app;
        i++;
      }
      i++;
      if (str[i - 1] != '\"') {
        fprintf(stderr, "[ERROR] Unterminated string starting at index %i\n", strStart);
        *len = 0;
        return NULL;
      }

      tok.data.s[stri] = '\0';
    }
    else if (isatom(c)) {
      int atomStart = i;
      while (isatom(str[i]) || isdigit(str[i]) != 0) i++;

      tok.type = T_ATOM;
      tok.data.s = malloc(i - atomStart + 1);
      tok.data.s[i - atomStart] = '\0';
      strncpy(tok.data.s, str + atomStart, i - atomStart);
    }
    else if (isspace(c) != 0) {
      i++;
      continue;
    }
    else {
      fprintf(stderr, "[ERROR] Unexpected character [%c] at index %i\n", c, i);
      token_free(tokens, tokenI);
      *len = 0;
      return NULL;
    }

    if (tokenI + 1 > tokensLen) {
      tokensLen++;
      void* temp = realloc(tokens, tokensLen * sizeof(struct Token));
      if (temp == NULL) {
      	free(tokens);
      	return NULL;
      }
      tokens = (struct Token*)temp;
    }
    tokens[tokenI++] = tok;
  }

  if (nesting != 0) {
    fprintf(stderr, "[ERROR] Expected \')\' at index %i\n", i - 1);
    token_free(tokens, tokenI);
    *len = 0;
    return NULL;
  }

  *len = tokenI;
  return tokens;
}

struct Expr* parse_r(struct MemoryManager* manager, const struct Token* tokens, int tokenLen, int i, int* contin) {
  struct Expr* expr = mem_allocExpr(manager);
  *expr = (struct Expr){.l = NULL, .r = NULL, .type = E_LIST};

  int head = 0;
  while (i < tokenLen) {
    struct Token tok = tokens[i++];
    struct Expr* child = NULL;

    if (tok.type == T_OPEN)
      child = parse_r(manager, tokens, tokenLen, i, &i);

    else if (tok.type == T_CLOSE)
      break;

    else {
      child = mem_allocExpr(manager);
      child->type = tok.type;
      if (tok.type == T_ATOM || tok.type == T_STRING) {
        child->data.s = mem_allocConstantCopy(manager, tok.data.s);
      }
      else if (tok.type == T_NUMBER) {
        child->data.num = tok.data.num;
      }
    }

    if (!head) {
      expr->l = child;
      head = 1;
    }
    else {
      struct Expr* r = mem_allocExpr(manager);
      r->type = E_LIST;
      r->l = child;
      r->r = parse_r(manager, tokens, tokenLen, i, &i);
      expr->r = r;
      break;
    }
  }

  *contin = i;
  return expr;
}

struct Expr* parse(struct MemoryManager* manager, const char* str) {
  int len = 0;
  struct Token* tokens = lex(str, &len);
  if (tokens == NULL)
    return NULL;

  int contin = 0;
  struct Expr* expr = parse_r(manager, tokens, len, 0, &contin);
  token_free(tokens, len);

  if (expr == NULL || contin == -1)
    return NULL;

  return expr;
}
