(define sumr (lambda (l s) ((if (null? (cdr l)) (+ s (car l)) (sumr (cdr l) (+ s (car l)))))))
(define sum (lambda (l) ((sumr l 0))))

(define lenr (lambda (l s) ((if (null? (cdr l)) (+ s 1) (lenr (cdr l) (+ s 1))))))
(define len (lambda (l) ((lenr l 0))))

(define avg (lambda (l) ((/ (sum l) (len l)))))

(define numbers (list 1 2 3 5 9 10))

(define fact (lambda (n) (
  (if (= n 1)
    1
    (* n (fact (- n 1)))))))

(define printn (lambda (l n) (
  (if (= n 0)
  (print (car l))
  (printn (cdr l) (- n 1)))
)))

(define printall (lambda (l) (
  (if (null? l)
    (quote ())
    ((lambda () ((print (car l) " _ ") (printall (cdr l)))))
  )
)))

(print "The factorial of 5 is " (fact 6) "\n")
(print "The sum of (" numbers ") is " (sum numbers) "\n")
(print "The average of (" numbers ") is " (avg numbers) "\n")
(printall numbers)
