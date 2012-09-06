(define cadr (lambda (obj) (car (cdr obj))))
(define cddr (lambda (obj) (cdr (cdr obj))))
(define caddr (lambda (obj) (car (cddr obj))))
(define cdddr (lambda (obj) (cdr (cddr obj))))
(define caadr (lambda (obj) (car (cadr obj))))
(define cdadr (lambda (obj) (cdr (cadr obj))))
(define cadadr (lambda (obj) (car (cdadr obj))))
(define caaadr (lambda (obj) (car (caadr obj))))
(define cdaadr (lambda (obj) (cdr (caadr obj))))
(define cadaadr (lambda (obj) (car (cdaadr obj))))

(define list
  (lambda args args))

(define map
  (lambda (f lst)
    (if (null? lst)
	'()
	(cons (f (car lst))
	      (map f (cdr lst))))))

(define append
  (lambda (lst1 lst2)
    (if (null? lst1)
	lst2
	(cons (car lst1)
	      (append (cdr lst1) lst2)))))

