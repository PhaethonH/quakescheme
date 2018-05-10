#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"
#include "qsmach.h"
#include "qssexpr.h"

/* Unit test  reader1 */

#define SPACELEN 20000

uint8_t _heap1[sizeof(qsheap_t) + SPACELEN*sizeof(qsobj_t)];
qsheap_t *heap1 = (qsheap_t*)&_heap1;

qs_t _scheme1, *scheme1 = &_scheme1;

char buf[131072];


struct {
    qsptr_t _;

    qsptr_t _lambda;
    qsptr_t _setq;
    qsptr_t _if;
    qsptr_t _let;
    qsptr_t _letrec;
    qsptr_t _callcc;
    qsptr_t cons;

    qsptr_t char_equal_p;
    qsptr_t string_ref;
    qsptr_t string_length;
    qsptr_t int_gt_p;
    qsptr_t int_eq_p;
    qsptr_t obj_make;
    qsptr_t obj_ref_ptr;
    qsptr_t obj_setq_ptr;
    qsptr_t int_add;

    qsptr_t ch;
    qsptr_t str;
    qsptr_t ofs;
    qsptr_t nextofs;
    qsptr_t nextnode;
    qsptr_t pending;
    qsptr_t nextpending;
    qsptr_t root;
    qsptr_t obj;
    qsptr_t x;
    qsptr_t y;

    qsptr_t is_sp;
    qsptr_t is_tab;
    qsptr_t is_cr;
    qsptr_t is_lf;

    qsptr_t ws_p;

    qsptr_t lim;

    qsptr_t eos_p;
    qsptr_t dq_p;
    qsptr_t bksl_p;
    qsptr_t op_p;
    qsptr_t cl_p;

    qsptr_t eos;
    qsptr_t is_ws;
    qsptr_t is_dq;
    qsptr_t is_op;
    qsptr_t is_cl;

    qsptr_t add_tok;
    qsptr_t qstokenize_atom;
} sym;

void init ()
{
  qsheap_init(heap1, SPACELEN);
  qs_init(scheme1, heap1);

  scheme1->E = qsenv_make(heap1, QSNIL);

  //heap_dump(heap1, 0);
  qsptr_t s = QSNIL;
  s = qsstr_inject(heap1, "_", 0);
  sym._ = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "lambda", 0);
  sym._lambda = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "set!", 0);
  sym._setq = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "if", 0);
  sym._if = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "let", 0);
  sym._let = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "letrec", 0);
  sym._letrec = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "callcc", 0);
  sym._callcc = qssymbol_make(heap1, s);

  s = qsstr_inject(heap1, "str", 0);
  sym.str = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "ofs", 0);
  sym.ofs = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "nextofs", 0);
  sym.nextofs = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "nextnode", 0);
  sym.nextnode = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "pending", 0);
  sym.pending = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "nextpending", 0);
  sym.nextpending = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "root", 0);
  sym.root = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "obj", 0);
  sym.obj = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "cons", 0);
  sym.cons = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "x", 0);
  sym.x = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "y", 0);
  sym.y = qssymbol_make(heap1, s);

  s = qsstr_inject(heap1, "&i>?", 0);
  sym.int_gt_p = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "&i=?", 0);
  sym.int_eq_p = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "&c=?", 0);
  sym.char_equal_p = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "&o*", 0);
  sym.obj_make = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "&o@p", 0);
  sym.obj_ref_ptr = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "&o!p", 0);
  sym.obj_setq_ptr = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "&+", 0);
  sym.int_add = qssymbol_make(heap1, s);

  s = qsstr_inject(heap1, "ch", 0);
  sym.ch = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "eos", 0);
  sym.eos = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "is_sp", 0);
  sym.is_sp = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "is_tab", 0);
  sym.is_tab = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "is_cr", 0);
  sym.is_cr = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "is_lf", 0);
  sym.is_lf = qssymbol_make(heap1, s);

  s = qsstr_inject(heap1, "ws?", 0);
  sym.ws_p = qssymbol_make(heap1, s);

  s = qsstr_inject(heap1, "&s@", 0);
  sym.string_ref = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "&s#", 0);
  sym.string_length = qssymbol_make(heap1, s);

  s = qsstr_inject(heap1, "lim", 0);
  sym.lim = qssymbol_make(heap1, s);

  s = qsstr_inject(heap1, "eos?", 0);
  sym.eos_p = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "dq?", 0);
  sym.dq_p = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "bksl?", 0);
  sym.bksl_p = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "op?", 0);
  sym.op_p = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "cl?", 0);
  sym.cl_p = qssymbol_make(heap1, s);

  s = qsstr_inject(heap1, "eos", 0);
  sym.eos = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "is_ws", 0);
  sym.is_ws = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "is_dq", 0);
  sym.is_dq = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "is_op", 0);
  sym.is_op = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "is_cl", 0);
  sym.is_cl = qssymbol_make(heap1, s);

  s = qsstr_inject(heap1, "add_tok", 0);
  sym.add_tok = qssymbol_make(heap1, s);
  s = qsstr_inject(heap1, "qstokenize_atom", 0);
  sym.qstokenize_atom = qssymbol_make(heap1, s);
}


/*
(define &c>i char->integer)
(define &s# string-length)
(define &s@ string-ref)
(define &c=? char=?)


(letrec
 ((ws? 
   (lambda (ch)
    (let ((is_sp (char=? ch #\space))) (if is_sp #t
     (let ((is_tab (char=? ch #\tab))) (if is_tab #t
      (let ((is_cr (char=? ch #\cr))) (if is_cr #t
       (let ((is_lf (char=? ch #\lf))) (if is_lf #t #f))))))))))

  (eos?
   (lambda (str ofs)
    (let ((lim (&s# str)))
     (let ((eq (= ofs lim)))
      (if eq #t (> ofs lim))))))

  (dq?  (lambda (ch) (let ((is_dq (&c=? ch #\x22))) is_dq)))

  (bksl?  (lambda (ch) (let ((is_bksl (&c=? ch #\x5c))) is_bksl)))

  (op?  (lambda (ch) (let ((is_op (&c=? ch #\x28))) is_op)))

  (cl?  (lambda (ch) (let ((is_cl (&c=? ch #\x29))) is_cl)))

  (add-tok
   (lambda (ch pending)
    (let ((nextnode (cons ch '())))
     (let ((_ (set-cdr! pending nextnode)))
      nextnode))))

  (qstokenize_str
   (lambda (str ofs root pending)
    (let ((nextofs (+ 1 ofs)))
     (let ((eos (eos? str ofs))) (if eos
      (let ((_ (add-tok #\x22 pending))) (cons nextofs root))
      (let ((ch (&s@ str ofs)))
        (let ((is_dq (dq? ch))) (if is_dq (cons nextofs root)
         (let ((is_bksl (bksl? ch))) (if is_bksl
          (let ((nextch (&s@ str nextofs)))
           (qstokenize_str str (+ 2 ofs) root (add-tok nextch pending)))
          (qstokenize_str str nextofs root (add-tok ch pending))))))))))))

  (qstokenize_atom
   (lambda (str ofs root pending)
    (let ((eos (eos? str ofs))) (if eos (cons ofs root)
     (let ((ch (&s@ str ofs)))
      (let ((is_ws (ws? ch))) (if is_ws (cons ofs root)
       (let ((is_op (op? ch))) (if is_op (cons ofs root)
        (let ((is_cl (cl? ch))) (if is_cl (cons ofs root)
         (let ((is_dq (dq? ch))) (if is_dq (cons ofs root)
          (let ((nextofs (+ 1 ofs)))
           (qstokenize_atom str nextofs root (add-tok ch pending))))))))))))))))

  (qssubtokenize
   (lambda (str ofs root pending)
    (let ((eos (eos? str ofs))) (if eos (cons ofs root)
     (let ((nextofs (+ 1 ofs)))
      (let ((ch (&s@ str ofs)))
       (let ((is_ws (ws? ch))) (if is_ws (qstokenize str nextofs root pending)
        (let ((cp (&c>i ch)))
         (let ((is_op (op? ch))) (if is_op (cons ofs root)
          (let ((is_cl (cl? ch))) (if is_cl (cons ofs root)
           (let ((is_dq (dq? ch))) (if is_dq
            (qstokenize_str str nextofs root (add-tok ch pending))
            (qstokenize_atom str ofs root pending))))))))))))))))

   (qstokenize
    (lambda (str)
     (let ((start (cons #\space '())))
      (let ((res (qssubtokenize str 0 start start)))
       (cons (+ 1 (car res)) (list->string (cdr res)))))))
  )

 (begin (display (list
  (qstokenize "98 def")
  (qstokenize "foobar")
  (qstokenize "\"the quoted rest\"not_this")
  (qstokenize "(ab");
  (qstokenize "c )");
  )) (newline)))

*/


qsptr_t build_ws_p ()
{
/*
 (define ws? 
   (lambda (ch)
    (let ((is_sp (char=? ch #\space))) (if is_sp #t
     (let ((is_tab (char=? ch #\tab))) (if is_tab #t
      (let ((is_cr (char=? ch #\cr))) (if is_cr #t
       (let ((is_lf (char=? ch #\lf))) (if is_lf #t #f))))))))))
*/
  qsptr_t ws_p = QSNIL;

  qsptr_t clause0 = qsimmlist_injectl(heap1,
    sym._let,
    QSBOL,
      QSBOL,
        sym.is_lf,
	QSBOL,
	  sym.char_equal_p,
	  sym.ch,
	  QSCHAR('\n'),
	  QSEOL,
        QSEOL,
      QSEOL,
    QSBOL,
      sym._if,
      sym.is_lf,
      QSTRUE,
      QSFALSE,
      QSEOL,
    QSEOL);
  qsptr_t clause1 = qsimmlist_injectl(heap1,
    sym._let,
    QSBOL,
      QSBOL,
        sym.is_cr,
	QSBOL,
	  sym.char_equal_p,
	  sym.ch,
	  QSCHAR('\r'),
	  QSEOL,
        QSEOL,
      QSEOL,
    QSBOL,
      sym._if,
      sym.is_cr,
      QSTRUE,
      clause0,
      QSEOL,
    QSEOL);
  qsptr_t clause2 = qsimmlist_injectl(heap1,
    sym._let,
    QSBOL,
      QSBOL,
        sym.is_tab,
	QSBOL, sym.char_equal_p, sym.ch, QSCHAR('\t'), QSEOL,
        QSEOL,
      QSEOL,
    QSBOL,
      sym._if, sym.is_tab, QSTRUE, clause1, QSEOL,
    QSEOL);
  qsptr_t clause3 = qsimmlist_injectl(heap1,
    sym._let,
    QSBOL,
      QSBOL,
        sym.is_sp,
	QSBOL, sym.char_equal_p, sym.ch, QSCHAR(' '), QSEOL,
        QSEOL,
      QSEOL,
    QSBOL,
      sym._if, sym.is_sp, QSTRUE, clause2, QSEOL,
    QSEOL);

  qsptr_t lam = qslambda_make(heap1,
		       qsimmlist_injectl(heap1, sym.ch, QSEOL),
		       clause3);
  ws_p = qsclosure_make(heap1, scheme1->E, lam);

  return ws_p;
}

qsptr_t build_eos_p ()
{
/*
  (eos?
   (lambda (str ofs)
    (let ((lim (&s# str)))
     (let ((eq (= ofs lim)))
      (if eq #t (> ofs lim))))))
*/
  qsptr_t eos_p = QSNIL;

  qsptr_t clause0 = qsimmlist_injectl(heap1,
    sym._let,
    QSBOL, QSBOL, sym.lim, QSBOL, sym.string_length, sym.str, QSEOL, QSEOL, QSEOL,

    QSBOL, sym._let, QSBOL, QSBOL, sym._, QSBOL, sym.int_eq_p, sym.ofs, sym.lim, QSEOL, QSEOL, QSEOL,
    QSBOL, sym._if, sym._, QSTRUE, QSBOL, sym.int_gt_p, sym.ofs, sym.lim, QSEOL, QSEOL, QSEOL, QSEOL, QSEOL,
    QSEOL);
  qsptr_t lam = qslambda_make(heap1,
			      qsimmlist_injectl(heap1, sym.str, sym.ofs, QSEOL),
			      clause0);
  eos_p = qsclosure_make(heap1, scheme1->E, lam);

  return eos_p;
}


/*
  (dq?  (lambda (ch) (let ((is_dq (&c=? ch #\x22))) is_dq)))

  (bksl?  (lambda (ch) (let ((is_bksl (&c=? ch #\x5c))) is_bksl)))

  (op?  (lambda (ch) (let ((is_op (&c=? ch #\x28))) is_op)))

  (cl?  (lambda (ch) (let ((is_cl (&c=? ch #\x29))) is_cl)))
*/
qsptr_t build_dq_p ()
{
  qsptr_t clause0 = qsimmlist_injectl(heap1,
    sym._let,
    QSBOL, QSBOL, sym._, QSBOL, sym.char_equal_p, sym.ch, QSCHAR(0x22), QSEOL, QSEOL, QSEOL,
    sym._,
    QSEOL);
  qsptr_t param = qspair_make(heap1, sym.ch, QSNIL);
  qsptr_t lam = qslambda_make(heap1, param, clause0);
  qsptr_t clo = qsclosure_make(heap1, scheme1->E, lam);
  return clo;
}
qsptr_t build_bksl_p ()
{
  qsptr_t clause0 = qsimmlist_injectl(heap1,
    sym._let,
    QSBOL, QSBOL, sym._, QSBOL, sym.char_equal_p, sym.ch, QSCHAR(0x5c), QSEOL, QSEOL, QSEOL,
    sym._,
    QSEOL);
  qsptr_t param = qspair_make(heap1, sym.ch, QSNIL);
  qsptr_t lam = qslambda_make(heap1, param, clause0);
  qsptr_t clo = qsclosure_make(heap1, scheme1->E, lam);
  return clo;
}
qsptr_t build_op_p ()
{
  qsptr_t clause0 = qsimmlist_injectl(heap1,
    sym._let,
    QSBOL, QSBOL, sym._, QSBOL, sym.char_equal_p, sym.ch, QSCHAR(0x28), QSEOL, QSEOL, QSEOL,
    sym._,
    QSEOL);
  qsptr_t param = qspair_make(heap1, sym.ch, QSNIL);
  qsptr_t lam = qslambda_make(heap1, param, clause0);
  qsptr_t clo = qsclosure_make(heap1, scheme1->E, lam);
  return clo;
}
qsptr_t build_cl_p ()
{
  qsptr_t clause0 = qsimmlist_injectl(heap1,
    sym._let,
    QSBOL, QSBOL, sym._, QSBOL, sym.char_equal_p, sym.ch, QSCHAR(0x29), QSEOL, QSEOL, QSEOL,
    sym._,
    QSEOL);
  qsptr_t param = qspair_make(heap1, sym.ch, QSNIL);
  qsptr_t lam = qslambda_make(heap1, param, clause0);
  qsptr_t clo = qsclosure_make(heap1, scheme1->E, lam);
  return clo;
}

/*
   (set! cons
    (lambda (x y)
     (let ((obj (&o* 1 #f)))
      (let ((_ (&o* 1 0)))
       (let ((_ (&o!p 2 x)))
        (let ((_ (&o!p 3 y)))
         o))))))
*/
qsptr_t build_cons ()
{
  qsptr_t clause0 = qsimmlist_injectl(heap1,
    sym._let, QSBOL, QSBOL, sym.obj, QSBOL, sym.obj_make, QSINT(1), QSFALSE, QSEOL, QSEOL, QSEOL,
    QSBOL, sym._let, QSBOL, QSBOL, sym._, QSBOL, sym.obj_setq_ptr, sym.obj, QSINT(1), QSNIL, QSEOL, QSEOL, QSEOL,
    QSBOL, sym._let, QSBOL, QSBOL, sym._, QSBOL, sym.obj_setq_ptr, sym.obj, QSINT(2), sym.x, QSEOL, QSEOL, QSEOL,
    QSBOL, sym._let, QSBOL, QSBOL, sym._, QSBOL, sym.obj_setq_ptr, sym.obj, QSINT(3), sym.y, QSEOL, QSEOL, QSEOL,
    sym.obj, QSEOL, QSEOL, QSEOL,
    QSEOL);
  qsptr_t param = qsimmlist_injectl(heap1, sym.x, sym.y, QSEOL);
  qsptr_t lam = qslambda_make(heap1, param, clause0);
  qsptr_t clo = qsclosure_make(heap1, scheme1->E, lam);
  return clo;
}

/*
  (add-tok
   (lambda (ch pending)
    (let ((nextnode (cons ch '())))
     (let ((_ (&o!p pending 2 nextnode)))   ; (set-cdr! pending nextnode)
      nextnode))))
*/
qsptr_t build_add_tok ()
{
  qsptr_t img_clause0[] = {
      sym._let,
        QSBOL, QSBOL, sym.nextnode, QSBOL, sym.cons, sym.ch, QSNIL, QSEOL, QSEOL, QSEOL,
      QSBOL, sym._let, QSBOL, QSBOL, sym._, QSBOL, sym.obj_setq_ptr, sym.pending, QSINT(2), sym.nextnode, QSEOL, QSEOL, QSEOL,
      sym.nextnode, QSEOL, QSEOL, QSEOL,
      QSEOL
  };
  qsptr_t clause0 = qsimmlist_inject(heap1, img_clause0, 28);
  qsptr_t param = qsimmlist_injectl(heap1, sym.ch, sym.pending, QSEOL);
  qsptr_t lam = qslambda_make(heap1, param, clause0);
  qsptr_t clo = qsclosure_make(heap1, scheme1->E, lam);
  return clo;
}

qsptr_t Xbuild_qstokenize_atom ()
{
  /*
  (qstokenize_atom
   (lambda (str ofs root pending)
    (let ((eos (eos? str ofs))) (if eos (cons ofs root)
     (let ((ch (&s@ str ofs)))
      (let ((is_ws (ws? ch))) (if is_ws (cons ofs root)
       (let ((is_op (op? ch))) (if is_op (cons ofs root)
        (let ((is_cl (cl? ch))) (if is_cl (cons ofs root)
         (let ((is_dq (dq? ch))) (if is_dq (cons ofs root)
          (let ((nextofs (+ 1 ofs)))
	   (let ((nextpending (add-tok ch pending)))
            (qstokenize_atom str nextofs root nextpending))))))))))))))))
	   */
  qsptr_t img_clause0[] = {
      sym.qstokenize_atom, sym.str, sym.nextofs, sym.root, sym.nextpending, QSEOL };
  qsptr_t clause0 = qsimmlist_inject(heap1, img_clause0, 0);

  qsptr_t img_clause1[] = {
      sym._let, QSBOL, QSBOL, sym.nextpending, QSBOL, sym.add_tok, sym.ch, sym.pending, QSEOL, QSEOL, QSEOL, clause0, QSEOL
  };
  qsptr_t clause1 = qsimmlist_inject(heap1, img_clause1, 0);

  qsptr_t img_clause2[] = {
      sym._let, QSBOL, QSBOL, sym.nextofs, QSBOL, sym.int_add, QSINT(1), sym.ofs, QSEOL, QSEOL, QSEOL, clause1, QSEOL
  };
  qsptr_t clause2 = qsimmlist_inject(heap1, img_clause2, 0);

  qsptr_t img_clause3[] = {
      sym._let, QSBOL, QSBOL, sym.is_dq, QSBOL, sym.dq_p, sym.ch, QSEOL, QSEOL, QSEOL,
      QSBOL, sym._if, sym.is_dq, QSBOL, sym.cons, sym.ofs, sym.root, QSEOL, clause2, QSEOL, QSEOL,
  };
  qsptr_t clause3 = qsimmlist_inject(heap1, img_clause3, 0);

  qsptr_t img_clause4[] = {
      sym._let, QSBOL, QSBOL, sym.is_cl, QSBOL, sym.cl_p, sym.ch, QSEOL, QSEOL, QSEOL,
      QSBOL, sym._if, sym.is_cl, QSBOL, sym.cons, sym.ofs, sym.root, QSEOL, clause3, QSEOL, QSEOL,
  };
  qsptr_t clause4 = qsimmlist_injectl(heap1, img_clause4, 0);

  qsptr_t img_clause5[] = {
      sym._let, QSBOL, QSBOL, sym.is_op, QSBOL, sym.op_p, sym.ch, QSEOL, QSEOL, QSEOL,
      QSBOL, sym._if, sym.is_op, QSBOL, sym.cons, sym.ofs, sym.root, QSEOL, clause4, QSEOL, QSEOL,
  };
  qsptr_t clause5 = qsimmlist_injectl(heap1, img_clause5, 0);

  qsptr_t img_clause6[] = {
      sym._let, QSBOL, QSBOL, sym.is_ws, QSBOL, sym.ws_p, sym.ch, QSEOL, QSEOL, QSEOL, QSEOL,
      QSBOL, sym._if, sym.is_ws, QSBOL, sym.cons, sym.ofs, sym.root, QSEOL, clause5, QSEOL
  };
  qsptr_t clause6 = qsimmlist_injectl(heap1, img_clause6, 0);

  qsptr_t img_clause7[] = {
      sym._let, QSBOL, QSBOL, sym.ch, QSBOL, sym.string_ref, sym.str, sym.ofs, QSEOL, QSEOL, QSEOL, clause6, QSEOL, QSEOL,
  };
  qsptr_t clause7 = qsimmlist_injectl(heap1, img_clause7, 0);

  qsptr_t img_clause8[] = {
      sym._let, QSBOL, QSBOL, sym.eos, QSBOL, sym.eos_p, sym.str, sym.ofs, QSEOL, QSEOL, QSEOL,
      QSBOL, sym._if, sym.eos, QSBOL, sym.cons, sym.ofs, sym.root, QSEOL,
      clause7, QSEOL, QSEOL,
  };
  qsptr_t clause8 = qsimmlist_injectl(heap1, img_clause8, 0);

  qsptr_t param = qsimmlist_injectl(heap1, sym.str, sym.ofs, sym.root, sym.pending, QSEOL);
  qsptr_t body = clause8;
  qsptr_t lambda = qslambda_make(heap1, param, body);
  qsptr_t clo = qsclosure_make(heap1, scheme1->E, lambda);

  return clo;
}

qsptr_t build_qstokenize_atom ()
{
  /*
  (qstokenize_atom
   (lambda (str ofs root pending)
    (let ((eos (eos? str ofs))) (if eos (cons ofs root)
     (let ((ch (&s@ str ofs)))
      (let ((is_ws (ws? ch))) (if is_ws (cons ofs root)
       (let ((is_op (op? ch))) (if is_op (cons ofs root)
        (let ((is_cl (cl? ch))) (if is_cl (cons ofs root)
         (let ((is_dq (dq? ch))) (if is_dq (cons ofs root)
          (let ((nextofs (+ 1 ofs)))
	   (let ((nextpending (add-tok ch pending)))
            (qstokenize_atom str nextofs root nextpending))))))))))))))))
	   */
  qsptr_t img_qstokenize_atom[151] = {
      sym._let, QSBOL, QSBOL, sym.eos, QSBOL, sym.eos_p, sym.str, sym.ofs,
      QSEOL, QSEOL, QSEOL, QSBOL, sym._if, sym.eos, QSBOL, sym.cons, sym.ofs,
      sym.root, QSEOL, QSBOL, sym._let, QSBOL, QSBOL, sym.ch, QSBOL,
      sym.string_ref, sym.str, sym.ofs, QSEOL, QSEOL, QSEOL, QSBOL, sym._let,
      QSBOL, QSBOL, sym.is_ws, QSBOL, sym.ws_p, sym.ch, QSEOL, QSEOL, QSEOL,
      QSBOL, sym._if, sym.is_ws, QSBOL, sym.cons, sym.ofs, sym.root, QSEOL,
      QSBOL, sym._let, QSBOL, QSBOL, sym.is_op, QSBOL, sym.op_p, sym.ch, QSEOL,
      QSEOL, QSEOL, QSBOL, sym._if, sym.is_op, QSBOL, sym.cons, sym.ofs,
      sym.root, QSEOL, QSBOL, sym._let, QSBOL, QSBOL, sym.is_cl, QSBOL,
      sym.cl_p, sym.ch, QSEOL, QSEOL, QSEOL, QSBOL, sym._if, sym.is_cl, QSBOL,
      sym.cons, sym.ofs, sym.root, QSEOL, QSBOL, sym._let, QSBOL, QSBOL,
      sym.is_dq, QSBOL, sym.dq_p, sym.ch, QSEOL, QSEOL, QSEOL, QSBOL, sym._if,
      sym.is_dq, QSBOL, sym.cons, sym.ofs, sym.root, QSEOL, QSBOL, sym._let,
      QSBOL, QSBOL, sym.nextofs, QSBOL, sym.int_add, QSINT(1), sym.ofs, QSEOL,
      QSEOL, QSEOL, QSBOL, sym._let, QSBOL, QSBOL, sym.nextpending, QSBOL,
      sym.add_tok, sym.ch, sym.pending, QSEOL, QSEOL, QSEOL, QSBOL,
      sym.qstokenize_atom, sym.str, sym.nextofs, sym.root, sym.nextpending,
      QSEOL, QSEOL, QSEOL, QSEOL, QSEOL, QSEOL, QSEOL, QSEOL, QSEOL, QSEOL,
      QSEOL, QSEOL, QSEOL, QSEOL
  };
  qsptr_t body = qsimmlist_inject(scheme1->store, img_qstokenize_atom, 151);
  qsptr_t param = qsimmlist_injectl(heap1, sym.str, sym.ofs, sym.root, sym.pending, QSEOL);
  qsptr_t lambda = qslambda_make(heap1, param, body);
  qsptr_t clo = qsclosure_make(heap1, scheme1->E, lambda);

  return clo;
}







int spam = 0;
const int run_limit = 1000;
int lim_run0 (qsptr_t e0, qsptr_t exp)
{
  int n = 0;
  scheme1->E = e0;
  qs_inject_exp(scheme1, exp);
  while (! scheme1->halt && n++ < run_limit)
    {
      if (spam) { puts("--STEP--"); qs_dump(scheme1); }
      qs_step(scheme1);
    }
//  ck_assert_int_lt(n, 1000);
  return n;
}

#define lim_run(a,b) ck_assert_int_lt(lim_run0(a,b), 1000)

START_TEST(test_reader1)
{
  init();

  qsptr_t ws_p = build_ws_p();
  qsptr_crepr(heap1, ws_p, buf, sizeof(buf));

  ck_assert(qssymbol_p(heap1, sym.ws_p));
  qsptr_t e0 = scheme1->E = qsenv_setq(heap1, scheme1->E, sym.ws_p, ws_p);
  ck_assert_int_eq(qsenv_length(heap1, e0), 1);

  int n = 0;
  qsptr_t exp = QSNIL;

  exp = qsimmlist_injectl(heap1, sym.ws_p, QSCHAR('\n'), QSEOL);
  qs_inject_exp(scheme1, exp);
  qs_step(scheme1);
  while (! scheme1->halt && n < 1000)
    {
      qs_step(scheme1);
      n++;
    }
  ck_assert_int_lt(n, 1000);

  n = 0;
  scheme1->E = e0;
  exp = qsimmlist_injectl(heap1, sym.ws_p, QSCHAR('\r'), QSEOL);
  qs_inject_exp(scheme1, exp);
  while (! scheme1->halt && n++ < 1000)
    qs_step(scheme1);
  ck_assert_int_eq(scheme1->A, QSTRUE);

  n = 0;
  scheme1->E = e0;
  exp = qsimmlist_injectl(heap1, sym.ws_p, QSCHAR(' '), QSEOL);
  qs_inject_exp(scheme1, exp);
  while (! scheme1->halt && n++ < 1000)
    qs_step(scheme1);
  ck_assert_int_eq(scheme1->A, QSTRUE);

  n = 0;
  scheme1->E = e0;
  exp = qsimmlist_injectl(heap1, sym.ws_p, QSCHAR('\a'), QSEOL);
  qs_inject_exp(scheme1, exp);
  while (! scheme1->halt && n++ < 1000)
    qs_step(scheme1);
  ck_assert_int_eq(scheme1->A, QSFALSE);

  n = 0;
  scheme1->E = e0;
  exp = qsimmlist_injectl(heap1, sym.ws_p, QSINT(3), QSEOL);
  qs_inject_exp(scheme1, exp);
  while (! scheme1->halt && n++ < 1000)
    qs_step(scheme1);
  ck_assert_int_eq(scheme1->A, QSFALSE);


  qsptr_t eos_p = build_eos_p();
  ck_assert_int_eq(qsenv_length(heap1, e0), 1);
  ck_assert(qssymbol_p(heap1, sym.eos_p));
  ck_assert(qsenv_p(heap1, e0));
  e0 = qsenv_setq(heap1, e0, sym.eos_p, eos_p);

  ck_assert_int_eq(qsenv_length(heap1, e0), 2);

  qsptr_t s1 = qsstr_inject(heap1, "lorem ipsum", 0);

  exp = qsimmlist_injectl(heap1, sym.eos_p, s1, QSINT(3), QSEOL);
  lim_run(e0, exp);
  ck_assert_int_eq(scheme1->A, QSFALSE);

  exp = qsimmlist_injectl(heap1, sym.eos_p, s1, QSINT(11), QSEOL);
  lim_run(e0, exp);
  ck_assert_int_eq(scheme1->A, QSTRUE);

  exp = qsimmlist_injectl(heap1, sym.eos_p, s1, QSINT(30), QSEOL);
  lim_run(e0, exp);
  ck_assert_int_eq(scheme1->A, QSTRUE);


  qsptr_t dq_p = build_dq_p();
  qsptr_t bksl_p = build_bksl_p();
  qsptr_t op_p = build_op_p();
  qsptr_t cl_p = build_cl_p();
  e0 = qsenv_setq(heap1, e0, sym.dq_p, dq_p);
  e0 = qsenv_setq(heap1, e0, sym.bksl_p, bksl_p);
  e0 = qsenv_setq(heap1, e0, sym.op_p, op_p);
  e0 = qsenv_setq(heap1, e0, sym.cl_p, cl_p);


  exp = qsimmlist_injectl(heap1, sym.dq_p, QSCHAR('"'), QSEOL);
  lim_run(e0, exp);
  ck_assert_int_eq(scheme1->A, QSTRUE);

  exp = qsimmlist_injectl(heap1, sym.dq_p, QSCHAR('x'), QSEOL);
  lim_run(e0, exp);
  ck_assert_int_eq(scheme1->A, QSFALSE);


  exp = qsimmlist_injectl(heap1, sym.bksl_p, QSCHAR('\\'), QSEOL);
  lim_run(e0, exp);
  ck_assert_int_eq(scheme1->A, QSTRUE);

  exp = qsimmlist_injectl(heap1, sym.bksl_p, QSCHAR('x'), QSEOL);
  lim_run(e0, exp);
  ck_assert_int_eq(scheme1->A, QSFALSE);


  exp = qsimmlist_injectl(heap1, sym.op_p, QSCHAR('('), QSEOL);
  lim_run(e0, exp);
  ck_assert_int_eq(scheme1->A, QSTRUE);

  exp = qsimmlist_injectl(heap1, sym.op_p, QSCHAR('x'), QSEOL);
  lim_run(e0, exp);
  ck_assert_int_eq(scheme1->A, QSFALSE);


  exp = qsimmlist_injectl(heap1, sym.cl_p, QSCHAR(')'), QSEOL);
  lim_run(e0, exp);
  ck_assert_int_eq(scheme1->A, QSTRUE);

  exp = qsimmlist_injectl(heap1, sym.cl_p, QSCHAR('x'), QSEOL);
  lim_run(e0, exp);
  ck_assert_int_eq(scheme1->A, QSFALSE);


  qsptr_t cons = build_cons();
  e0 = qsenv_setq(heap1, e0, sym.cons, cons);
  exp = qsimmlist_injectl(heap1, sym.cons, QSINT(1), QSINT(2), QSEOL);
  lim_run(e0, exp);
  ck_assert(qspair_p(heap1, scheme1->A));
  ck_assert_int_eq(qspair_ref_a(heap1, scheme1->A), QSINT(1));
  ck_assert_int_eq(qspair_ref_d(heap1, scheme1->A), QSINT(2));


  qsptr_t add_tok = build_add_tok();
  e0 = qsenv_setq(heap1, e0, sym.add_tok, add_tok);
//  ck_assert_int_eq(qsenv_length(heap1, e0), 2); 
  qsptr_t dummyroot = qspair_make(heap1, QSCHAR(' '), QSNIL);
  e0 = qsenv_setq(heap1, e0, sym._, dummyroot);
  exp = qsimmlist_injectl(heap1, sym.add_tok, sym._, sym._, QSEOL);
  spam=1;
  lim_run(e0, exp);
//  printf("A = %08x\n", scheme1->A);
//  puts(qsobj_typeof(heap1, scheme1->A));

  qsptr_t qstokenize_atom = build_qstokenize_atom();
  e0 = qsenv_setq(heap1, e0, sym.qstokenize_atom, qstokenize_atom);
  qsptr_t in1 = qsstr_inject(heap1, "foobar bletch", 0);
  dummyroot = qspair_make(heap1, QSCHAR(' '), QSNIL);
  e0 = qsenv_setq(heap1, e0, sym._, dummyroot);
  exp = qsimmlist_injectl(heap1, sym.qstokenize_atom, in1, QSINT(0), dummyroot, dummyroot, QSEOL);
  lim_run(e0, exp);

  /*
  qsptr_t handoff = qspair_ref_a(heap1, scheme1->A);
  qsptr_t token = qspair_ref_d(heap1, scheme1->A);
  ck_assert(qspair_p(heap1, token));
  qsstr_extract(heap1, token, buf, sizeof(buf));
  ck_assert_str_eq(buf, "foobar");
  */

  puts("--END--");
  qs_dump(scheme1);
}
END_TEST

START_TEST(test_sexpr1)
{
  init();
  qsptr_t se0 = qssexpr_parse0_str(heap1, "hello", NULL);

  qsptr_crepr(heap1, se0, buf, sizeof(buf));
  ck_assert_str_eq(buf, "hello");

  qsptr_t se1 = qssexpr_parse0_str(heap1, "\"hi\"", NULL);
  qsptr_crepr(heap1, se1, buf, sizeof(buf));
  ck_assert_str_eq(buf, "\"hi\"");

  qsptr_t se2 = qssexpr_parse0_str(heap1, "\"hi", NULL);
  qsptr_crepr(heap1, se2, buf, sizeof(buf));
  ck_assert_str_eq(buf, "\"hi\"");

  qsptr_t se3 = qssexpr_parse0_str(heap1, "(baz)", NULL);
  qsptr_crepr(heap1, se3, buf, sizeof(buf));
  ck_assert_str_eq(buf, "(baz )");

  qsptr_t se4 = qssexpr_parse0_str(heap1, "(foo bar)", NULL);
  qsptr_crepr(heap1, se4, buf, sizeof(buf));
  ck_assert_str_eq(buf, "(foo bar )");

  qsptr_t se5 = qssexpr_parse0_str(heap1, "((quux))", NULL);
  qsptr_crepr(heap1, se5, buf, sizeof(buf));
  ck_assert_str_eq(buf, "((quux ) )");

  qsptr_t se6 = qssexpr_parse0_str(heap1, "(foo bar(foobar)quux ())", NULL);
  qsptr_crepr(heap1, se6, buf, sizeof(buf));
  ck_assert_str_eq(buf, "(foo bar (foobar ) quux '() )");
}
END_TEST


TESTCASE(reader1,
  TFUNC(test_reader1)
  TFUNC(test_sexpr1)
  )

TESTSUITE(suite1,
  TCASE(reader1)
  )

int main ()
{
  RUNSUITE(suite1);
}

