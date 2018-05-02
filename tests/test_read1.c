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
    qsptr_t _lambda;
    qsptr_t _setq;
    qsptr_t _if;
    qsptr_t _let;
    qsptr_t _letrec;
    qsptr_t _callcc;

    qsptr_t ch;
    qsptr_t char_equal_p;

    qsptr_t is_sp;
    qsptr_t is_tab;
    qsptr_t is_cr;
    qsptr_t is_lf;

    qsptr_t ws_p;
} sym;

void init ()
{
  qsheap_init(heap1, SPACELEN);
  qs_init(scheme1, heap1);

  scheme1->E = qsenv_make(heap1, QSNIL);

  //heap_dump(heap1, 0);
  qsptr_t s = QSNIL;
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

  s = qsstr_inject(heap1, "&c=?", 0);
  sym.char_equal_p = qssymbol_make(heap1, s);

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
     (eq? ofs lim))))

  (qstokenize_str
   (lambda (str ofs pending)
    (let ((eos (eos? str ofs))) (if eos (cons (+ 1 ofs) (cons #\" pending))
     (let ((ch (&s@ str ofs)))
      (let ((cp (&c>i ch)))
       (let ((is_dq (eq? cp 34))) (if is_dq (cons (+ 1 ofs) (cons ch pending))
        (let ((is_bksl (eq? cp 92))) (if is_bksl
         (let ((nextch (&s@ str (+ 1 ofs))))
          (qstokenize_str str (+ 2 ofs) (cons nextch pending)))
         (qstokenize_str str (+ 1 ofs) (cons ch pending))))))))))))

  (qstokenize_atom
   (lambda (str ofs pending)
    (let ((eos (eos? str ofs))) (if eos (cons ofs pending)
     (let ((ch (&s@ str ofs)))
      (let ((cp (&c>i ch)))
       (let ((is_ws (ws? ch))) (if is_ws (cons ofs pending)
        (let ((is_op (eq? cp 40))) (if is_op (cons ofs (cons ch pending))
         (let ((is_cl (eq? cp 41))) (if is_cl (cons ofs (cons ch pending))
          (let ((is_dq (eq? cp 34))) (if is_dq (cons ofs pending)
           (qstokenize_atom str (+ 1 ofs) (cons ch pending))))))))))))))))

  (qssubtokenize
   (lambda (str ofs pending)
    (let ((eos (eos? str ofs))) (if eos (cons ofs pending)
     (let ((ch (&s@ str ofs)))
      (let ((is_ws (ws? ch))) (if is_ws (qstokenize str (+ 1 ofs) pending)
       (let ((cp (&c>i ch)))
        (let ((is_op (eq? cp 40))) (if is_op (cons ofs (cons ch pending))
         (let ((is_cl (eq? cp 41))) (if is_cl (cons ofs (cons ch pending))
          (let ((is_dq (eq? cp 34))) (if is_dq
           (qstokenize_str str (+ 1 ofs) (cons ch pending))
           (qstokenize_atom str ofs pending)))))))))))))))

   (qstokenize
    (lambda (str)
     (let ((res (qssubtokenize str 0 '())))
      (cons (+ 1 (car res)) (list->string (reverse (cdr res)))))))
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

