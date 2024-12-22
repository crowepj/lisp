# lisp
A LISP interpreter written in C

# Compiling lisp
To compile the main interpreter, run the script `build.sh` whilst within the lisp/ directory. To compile the standard library file, run `buildffi.sh stdlib/stdlib.c`.

# Usage
`lisp [file]`
If a file is passed as an argument, the interpreter will load the file and run it expression by expression. If no file is passed as an argument, the interpreter will run as REPL.

# Examples
Factorial function
```
(define fact (lambda (n) (
  (if (= n 1)
    1
    (* n (fact (- n 1)))))))

(print "6 factorial is equal to " (fact 6) "\n")
```
