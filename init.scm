(define cadr (lambda (obj) (car (cdr obj))))
(define cddr (lambda (obj) (cdr (cdr obj))))
(define caddr (lambda (obj) (car (cddr obj))))
(define cdddr (lambda (obj) (cdr (cddr obj))))

(define map
  (lambda (f lst)
    (if (null? lst)
	'()
	(cons (f (car lst))
	      (map f (cdr lst))))))

(define close-syntax (lambda (form env) (make-syntactic-closure env '() form)))

(define sc-macro-transformer
  (lambda (f)
    (lambda (expr use-env mac-env)
      (make-syntactic-closure mac-env '() (f expr use-env)))))

(define-syntax let
  (lambda (expr use-env mac-env)
    ((lambda (vars vals rlambda)
       (cons (cons rlambda (cons vars (cddr expr))) vals))
     (map car  (cadr expr))
     (map cadr (cadr expr))
     (make-syntactic-closure mac-env '() 'lambda))))


