gcc -I . -I core -shared -fPIC -rdynamic $1 -o "${1%.c}.so"
