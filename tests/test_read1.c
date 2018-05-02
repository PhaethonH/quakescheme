#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"
#include "qsmach.h"

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

    qsptr_t ch;
    qsptr_t str;
    qsptr_t ofs;
    qsptr_t nextofs;
    qsptr_t nextnode;
    qsptr_t pending;
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

    qsptr_t add_tok;
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

  s = qsstr_inject(heap1, "ch", 0);
  sym.ch = qssymbol_make(heap1, s);
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

  s = qsstr_inject(heap1, "add_tok", 0);
  sym.add_tok = qssymbol_make(heap1, s);
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
  ws_p = qsclosure_make(heap1, QSNIL, lam);

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
  eos_p = qsclosure_make(heap1, QSNIL, lam);

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
  qsptr_t clo = qsclosure_make(heap1, QSNIL, lam);
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
  qsptr_t clo = qsclosure_make(heap1, QSNIL, lam);
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
  qsptr_t clo = qsclosure_make(heap1, QSNIL, lam);
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
  qsptr_t clo = qsclosure_make(heap1, QSNIL, lam);
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
  qsptr_t clo = qsclosure_make(heap1, QSNIL, lam);
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
  qsptr_t clause0 = qsimmlist_injectl(heap1,
//    sym._let, QSBOL, QSBOL, sym.nextnode, QSBOL, sym.cons, sym.ch, QSNIL, QSEOL, QSEOL, QSEOL,
//    QSBOL, sym._let, QSBOL, QSBOL, sym._, QSBOL, sym.obj_setq_ptr, sym.pending, QSINT(3), sym.nextnode, QSEOL, QSEOL, QSEOL,
//    sym.nextnode, QSEOL, QSEOL, QSEOL,
    sym.cons,
    QSEOL);
  qsptr_t param = qsimmlist_injectl(heap1, sym.ch, sym.pending, QSEOL);
  qsptr_t lam = qslambda_make(heap1, param, clause0);
  qsptr_t clo = qsclosure_make(heap1, QSNIL, lam);
  return clo;
}





int lim_run0 (qsptr_t e0, qsptr_t exp)
{
  int n = 0;
  scheme1->E = e0;
  qs_inject_exp(scheme1, exp);
  while (! scheme1->halt && n++ < 1000)
    qs_step(scheme1);
//  ck_assert_int_lt(n, 1000);
  return n;
}

#define lim_run(a,b) ck_assert_int_lt(lim_run0(a,b), 1000)

START_TEST(test_reader1)
{
  init();

  qsptr_t ws_p = build_ws_p();
  qsptr_crepr(heap1, ws_p, buf, sizeof(buf));

  qsptr_t e0 = scheme1->E = qsenv_setq(heap1, scheme1->E, sym.ws_p, ws_p);

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
  e0 = qsenv_setq(heap1, e0, sym.eos_p, eos_p);

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
  qsptr_t dummyroot = qspair_make(heap1, QSCHAR(' '), QSNIL);
  exp = qsimmlist_injectl(heap1, sym.add_tok, dummyroot, dummyroot, QSEOL);
  lim_run(e0, exp);
}
END_TEST

START_TEST(test_test2)
{
  init();
}
END_TEST


TESTCASE(reader1,
  TFUNC(test_reader1)
  TFUNC(test_test2)
  )

TESTSUITE(suite1,
  TCASE(reader1)
  )

int main ()
{
  RUNSUITE(suite1);
}

