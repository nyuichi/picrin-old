(define cadr (lambda (obj) (car (cdr obj))))
(define cddr (lambda (obj) (cdr (cdr obj))))
(define caddr (lambda (obj) (car (cddr obj))))
(define cdddr (lambda (obj) (cdr (cddr obj))))
(define caadr (lambda (obj) (car (cadr obj))))


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

(define close-syntax
  (lambda (form env)
    (if (pair? form)
	(cons (close-syntax (car form) env)
	      (Close-syntax (cdr form) env))
	(if (symbol? form)
	    (make-syntactic-closure env '() form)
	    form))))

(define sc-macro-transformer
  (lambda (f)
    (lambda (expr use-env mac-env)
      (close-syntax (f expr use-env) mac-env))))


(define-syntax and
  (sc-macro-transformer
   (lambda (form env)
     (if (null? (cdr form))
	 #t
	 (list 'if (close-syntax (cadr form) env)
	       (cons 'and (close-syntax (cddr form) env))
	       #f)))))

; (and a b c)
; 
; #s(if #sa (if #sb (if #sc #t #f) #f )


(define cdadr (lambda (obj) (cdr (cadr obj))))
(define cadadr (lambda (obj) (car (cdadr obj))))
(define caaadr (lambda (obj) (car (caadr obj))))
(define cdaadr (lambda (obj) (cdr (caadr obj))))
(define cadaadr (lambda (obj) (car (cdaadr obj))))

(define-syntax quasiquote
  (sc-macro-transformer
   (lambda (form env)
     (if (and (pair? (cadr form))
	      (eq? (caadr form) 'unquote))
	 (close-syntax (cadadr form) env)	       ; (quasiquote (unquote a)) => a
	 (if (and (pair? (cadr form))
		  (eq? (caadr form) 'quasiquote)) 
	     (list 'quote (list 'quasiquote (cadadr form))) ; (quasiquote (quasiquote a)) => (quote (quasiquote a))
	     (if (and (pair? (cadr form))
		      (pair? (caadr form))
		      (eq? (caaadr form) 'unquote-splicing))
		 (list 'append
		       (close-syntax (cadaadr form) env)
		       (list 'quasiquote (cdadr form))) ; (quasiquote ((unquote-splicing a) . b)) => (append a (quasiquote b))
		 (if (pair? (cadr form))
		     (list 'cons
			   (list 'quasiquote (caadr form))
			   (list 'quasiquote (cdadr form))) ; (quasiquote (a . b)) => (cons (quasiquote a) (quasiquote b))
		     (list 'quote (cadr form)))))))))

(define-syntax cond
  (sc-macro-transformer
   (lambda (form env)
     (if (null? (cdr form))
	 #f
	 `(if ,(close-syntax (caadr form) env)
	      (begin ,@(map (lambda (x) (close-syntax x env)) (cdadr form)))
	      (cond ,@(map (lambda (x) (close-syntax x env)) (cdadr form))))))))

; (cond (test . body) ...)

(define-syntax let
  (lambda (expr use-env mac-env)
    (cons (cons (close-syntax 'lambda mac-env)
		(cons (map car (cadr form))
		      (cddr form)))
	  (map cadr (cadr form)))))

