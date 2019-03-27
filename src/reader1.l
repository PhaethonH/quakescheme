; v1 Reader
; Build upon v1 S-Expression Parser to construct a Scheme reader.

; Values understood by v1 S-Expression Parser (reader source restricted to these types):
;  * integer
;  * list (proper only)
;  * string
;  * boolean (#t, #f)
;  * name (everything else: null, symbol, character, floats, quotes, #-specials, etc.)

; Forms understood by the minimalist CESK interpreter:
;  * (lambda a b)
;  * (set! var expr)    ; var is not list; expr is lambda/closure for procedure.
;  * (if e T F)
;  * (let ((v e)) b)    ; note only one set of binds allowed.
;  * (letrec ((v0 e0) (v1 e1) ...) b)
;  * (call/cc p)

; Operations are provided by Primitives (see qsprims_1.c)
;
; Other value types may be constructed programmatically from primitives:
;  * vector: (make-vector k fill)
;  * character: (integer->character k)
;  * float: (inexact i) (/ (inexact i) 10)
;  * bytevector: (make-bytevector k fillbyte)
;  * improper list: (cons x y)
;  * empty list/null: (make-null)

; (read PORT) => returns one value


; What is being written is one large S-Expression representing the reader.


(set! read-from-port
 (lambda (port)
  (let ((ch (character->integer (port-read-u8 port))))
   (if (= ch 34)
    (read-string)
))))
