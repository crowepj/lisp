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

Average of a list of numbers
```
(define avg (lambda (l s n) (
  (if (null? l)
    (/ s n)
    (avg (cdr l) (+ s (car l)) (+ n 1)))
)))
(print "Average: " (avg (list 1 2 3 5 10 9) 0 0))
```
