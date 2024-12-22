gcc ffi/funutil.c core/vm/*.c core/*.c core/hashmap/*.c -I . -I core -ldl -rdynamic -g3 -Wall -o lisp
