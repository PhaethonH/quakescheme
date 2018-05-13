
;(define src '(set! a 10))
(define src '(lambda (x) (+ x 1)))

(define body-qstokenize-atom
  '(_let ((eos (eos_p str ofs))) (_if eos (cons ofs root)
    (_let ((ch (string_ref str ofs)))
     (_let ((is_ws (ws_p ch))) (_if is_ws (cons ofs root)
      (_let ((is_op (op_p ch))) (_if is_op (cons ofs root)
       (_let ((is_cl (cl_p ch))) (_if is_cl (cons ofs root)
        (_let ((is_dq (dq_p ch))) (_if is_dq (cons ofs root)
         (_let ((nextofs (int_add 1 ofs)))
          (_let ((nextpending (add_tok ch pending)))
           (qstokenize_atom str nextofs root nextpending)))))))))))))))

(define (eos_p str ofs) (>= ofs (string-length str)))
(define (ws_p ch) (eq? ch #\space))
(define (op_p ch) (eq? ch #\x28))
(define (cl_p ch) (eq? ch #\x29))
(define (dq_p ch) (eq? ch #\x22))
(define string_ref string-ref)
(define int_add +)
(define (add_tok ch pending)
    (let ((nextnode (cons ch '())))
     (let ((_ (set-cdr! pending nextnode)))
      nextnode)))


(define qstokenize_atom
 (lambda (str ofs root pending)
  (let ((eos (eos_p str ofs))) (if eos (cons ofs root)
   (let ((ch (string_ref str ofs)))
    (let ((is_ws (ws_p ch))) (if is_ws (cons ofs root)
     (let ((is_op (op_p ch))) (if is_op (cons ofs root)
      (let ((is_cl (cl_p ch))) (if is_cl (cons ofs root)
       (let ((is_dq (dq_p ch))) (if is_dq (cons ofs root)
        (let ((nextofs (int_add 1 ofs)))
         (let ((nextpending (add_tok ch pending)))
          (qstokenize_atom str nextofs root nextpending))))))))))))))))


;(define (transcribe p)
; (if
;  (list? p)
;  (begin (display "QSBOL, ") (letrec ((helper (lambda (q) (if (pair? q) (begin (transcribe (car q)) (helper (cdr q))))))) (helper p)) (display "QSEOL, "))
;  (if (integer? p)
;   (begin (display "QSINT(") (display p) (display "), "))
;   (if (symbol? p)
;    (begin (display "sym.") (display p) (display ", "))
;    (if (not (null? p))
;     (begin (display p) (display ", ")))))))

(define (transcribe-raw p pending)
 (if
  (null? p)
  pending
  (let ((elt (car p)))
   (if (pair? elt)
    (transcribe-raw (cdr p) (cons "QSEOL" (transcribe-raw elt (cons "QSBOL" pending))))
    (if (number? elt)
     (transcribe-raw (cdr p) (cons (string-append "QSINT(" (string-append (number->string elt) ")")) pending))
     (if (symbol? elt)
      (transcribe-raw (cdr p) (cons (string-append "sym." (symbol->string elt)) pending))))))))

(define (c-transcribe c-name l-object)
 (let ((transcribed (reverse (cons "QSEOL" (transcribe-raw l-object '())))))
  (begin
   (map display `("qsptr_t " ,c-name "[" ,(length transcribed) "] = {"))
   (newline)
   (display "  ")
   (letrec ((helper (lambda (transcribed)
    (if (not (null? transcribed))
     (begin
      (display (car transcribed))
      (if (not (null? (cdr transcribed)))
       (display ", "))
      (helper (cdr transcribed)))))))
    (helper transcribed))
   (newline)
   (display "};") (newline)
)))


;(display (reverse (transcribe-raw src '())))
;(display (transcribe-raw src '()))
;(c-transcribe "qstokenize_atom" src-qstokenize-atom)
;(c-transcribe "qstokenize_atom" src)
;(c-transcribe "img_qstokenize_atom" body-qstokenize-atom)
(define temp '(#\space))
(display (qstokenize_atom "foobar bletch" 0 temp temp)) (newline)

