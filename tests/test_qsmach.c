#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test  machine */

#define SPACELEN 20000

uint8_t _heap1[sizeof(qsheap_t) + SPACELEN*sizeof(qsobj_t)];
qsheap_t *heap1 = (qsheap_t*)&_heap1;

qs_t _scheme1, *scheme1 = &_scheme1;

//char buf[131072];
char buf[65536];


void init ()
{
  qsheap_init(heap1, SPACELEN);
  qs_init(scheme1, heap1);

  //heap_dump(heap1, 0);
}


START_TEST(test_inject1)
{
  init();

  // body0 = (&&)  ; (crash)
  qsptr_t str_op_crash = qsstr_inject(heap1, "&&", 0);
  qsptr_t sym_op_crash = qssymbol_make(heap1, str_op_crash);
  qsptr_t body0 = qspair_make(heap1, sym_op_crash, QSNIL);

  qs_inject_exp(scheme1, body0);

  ck_assert_int_ne(QSNIL, scheme1->C);
  ck_assert_int_eq(scheme1->C, body0);
  ck_assert(qsenv_p(heap1, scheme1->E));
  ck_assert(scheme1->store);
  ck_assert_int_eq(scheme1->K, QSNIL);
}
END_TEST

START_TEST(test_atomeval1)
{
  init();

  /* Evaluate integer. */
  qsptr_t a0 = qs_atomic_eval(scheme1, QSINT(99));
  ck_assert_int_eq(a0, QSINT(99));

  /* Evaluate boolean. */
  qsptr_t a1 = qs_atomic_eval(scheme1, QSFALSE);
  ck_assert_int_eq(a1, QSFALSE);

  /* Evaluate variable (symbol). */
  qsptr_t name1 = qsstr_inject(heap1, "foo", 0);
  qsptr_t sym1 = qssymbol_make(heap1, name1);
  qsptr_t env = qsenv_make(heap1, QSNIL);
  scheme1->E = env = qsenv_setq(heap1, env, sym1, QSINT(3));
  qsptr_t a2 = qs_atomic_eval(scheme1, sym1);
  ck_assert_int_eq(a2, QSINT(3));

  /* Evaluate lambda. */
  qsptr_t lam_name = qsstr_inject(heap1, "lambda", 0);
  qsptr_t lam_sym = qssymbol_make(heap1, lam_name);
  qsptr_t name_x = qsstr_inject(heap1, "x", 0);
  qsptr_t sym_x = qssymbol_make(heap1, name_x);
  qsptr_t body = qspair_make(heap1, CINT30(0), QSNIL);
  qsptr_t param = qspair_make(heap1, sym_x, QSNIL);
  qsptr_t lam1 = qspair_make(heap1, lam_sym,
		  qspair_make(heap1, param,
		    qspair_make(heap1, body, QSNIL)));
  qsptr_t a3 = qs_atomic_eval(scheme1, lam1);
  ck_assert_int_ne(QSNIL, a3);
  ck_assert(qsclosure_p(heap1, a3));

  /* Evaluate primitive operation. */
  qsptr_t str_crash = qsstr_inject(heap1, "&.", 0);
  qsptr_t sym_crash = qssymbol_make(heap1, str_crash);
  qsptr_t body0 = qspair_make(heap1, sym_crash, QSNIL);
  qsptr_t a4 = qs_atomic_eval(scheme1, body0);
  ck_assert_int_eq(a4, QSTRUE);
  ck_assert(scheme1->halt);

  /* Evalute to self. */
  /** vector */
  qsptr_t v0 = qsvector_make(heap1, 1, QSNIL);
  qsptr_t a5 = qs_atomic_eval(scheme1, v0);
  ck_assert_int_eq(v0, a5);
  /** string */
  char temp[100];
  qsptr_t a6 = qs_atomic_eval(scheme1, name1);
  ck_assert(!ISERROR16(qsstr_extract(heap1, a6, temp, sizeof(temp))));
  ck_assert_str_eq(temp, "foo");
}
END_TEST

START_TEST(test_step1)
{
  /* Atomic Evaluation as step. */
  init();

  /* Evaluate integer. */
  qs_inject_exp(scheme1, QSINT(99));
  qs_step(scheme1);
  qsptr_t a0 = scheme1->A;
  ck_assert_int_eq(a0, QSINT(99));

  /* Evaluate boolean. */
  qs_inject_exp(scheme1, QSFALSE);
  qs_step(scheme1);
  qsptr_t a1 = scheme1->A;
  ck_assert_int_eq(a1, QSFALSE);

  /* Evaluate variable (symbol). */
  qsptr_t name1 = qsstr_inject(heap1, "foo", 0);
  qsptr_t sym1 = qssymbol_make(heap1, name1);
  qsptr_t env = qsenv_make(heap1, QSNIL);
  scheme1->E = env = qsenv_setq(heap1, env, sym1, QSINT(3));
  qs_inject_exp(scheme1, sym1);
  qs_step(scheme1);
  qsptr_t a2 = scheme1->A;

  /* Evaluate lambda. */
  qsptr_t lam_name = qsstr_inject(heap1, "lambda", 0);
  qsptr_t lam_sym = qssymbol_make(heap1, lam_name);
  qsptr_t name_x = qsstr_inject(heap1, "x", 0);
  qsptr_t sym_x = qssymbol_make(heap1, name_x);
  qsptr_t body = qspair_make(heap1, CINT30(0), QSNIL);
  qsptr_t param = qspair_make(heap1, sym_x, QSNIL);
  qsptr_t lam1 = qspair_make(heap1, lam_sym,
		  qspair_make(heap1, param,
		    qspair_make(heap1, body, QSNIL)));
  qs_inject_exp(scheme1, lam1);
  qs_step(scheme1);
  qsptr_t a3 = scheme1->A;
  ck_assert_int_ne(QSNIL, a3);
  ck_assert(qsclosure_p(heap1, a3));

  /* Evaluate primitive operation. */
  qsptr_t str_crash = qsstr_inject(heap1, "&.", 0);
  qsptr_t sym_crash = qssymbol_make(heap1, str_crash);
  qsptr_t body0 = qspair_make(heap1, sym_crash, QSNIL);
  qs_inject_exp(scheme1, body0);
  qs_step(scheme1);
  qsptr_t a4 = scheme1->A;
  ck_assert_int_eq(a4, QSTRUE);
  ck_assert(scheme1->halt);

  /* Evaluate to self. */
  /** vector */
  qsptr_t v0 = qsvector_make(heap1, 1, QSNIL);
  qs_inject_exp(scheme1, v0);
  qs_step(scheme1);
  qsptr_t a5 = scheme1->A;
  ck_assert_int_eq(v0, a5);
  /** string */
  char temp[100];
  memset(temp, 100, 0);
  qs_inject_exp(scheme1, name1);
  qs_step(scheme1);
  qsptr_t a6 = scheme1->A;
  ck_assert(!ISERROR16(qsstr_extract(heap1, a6, temp, sizeof(temp))));
  ck_assert_str_eq(temp, "foo");

  /* Use iterator object as list. */
  qsptr_t img1[] = { lam_sym, QSBOL, sym_x, QSEOL, QSINT(97), QSBOL };
  qsptr_t l1 = qsimmlist_inject(heap1, img1, 6);
  qs_inject_exp(scheme1, l1);
  qs_step(scheme1);
  qsptr_t a7 = scheme1->A;
  ck_assert(qsclosure_p(heap1, a7));
}
END_TEST

START_TEST(test_step2)
{
  init();

  qsptr_t q0 = QSNIL;

  /* test condition. */
  qsptr_t name_if = qsstr_inject(heap1, "if", 0);
  qsptr_t sym_if = qssymbol_make(heap1, name_if);
  qsptr_t cond1 = qsimmlist_injectl(heap1, sym_if, QSTRUE, QSINT(42), QSINT(17), QSEOL);
  qs_inject_exp(scheme1, cond1);
  qs_step(scheme1);  // once to select a code branch.
  qs_step(scheme1);  // again to evaluate the branched code.
  q0 = scheme1->A;
  ck_assert_int_eq(q0, QSINT(42));

  qsptr_t cond2 = qsimmlist_injectl(heap1, sym_if, QSFALSE, QSINT(42), QSINT(17), QSEOL);
  qs_inject_exp(scheme1, cond2);
  qs_step(scheme1);  // once to select a code branch.
  qs_step(scheme1);  // again to evaluate the branched code.
  q0 = scheme1->A;
  ck_assert_int_eq(q0, QSINT(17));

  /* test let. */
  qsptr_t name_let = qsstr_inject(heap1, "let", 0);
  qsptr_t sym_let = qssymbol_make(heap1, name_let);
  qsptr_t name_x = qsstr_inject(heap1, "x", 0);
  qsptr_t sym_x = qssymbol_make(heap1, name_x);
  // (let ((x 101)) x)
  qsptr_t let1 = qsimmlist_injectl(heap1, sym_let, QSBOL, QSBOL, sym_x, QSINT(101), QSEOL, QSEOL, sym_x, QSEOL);
  qs_inject_exp(scheme1, let1);
  qs_step(scheme1);
  ck_assert(!ISNIL(scheme1->C));
  qs_step(scheme1);
  qs_step(scheme1);
  q0 = scheme1->A;
  ck_assert_int_eq(q0, QSINT(101));

  /* test mutate. */
  qsptr_t name_setq = qsstr_inject(heap1, "set!", 0);
  qsptr_t sym_setq = qssymbol_make(heap1, name_setq);
  qsptr_t mutate1 = qsimmlist_injectl(heap1, sym_setq, sym_x, QSINT(102), QSEOL);
  qs_inject_exp(scheme1, mutate1);
  qs_step(scheme1);
  q0 = qsenv_ref(heap1, scheme1->E, sym_x);
  ck_assert_int_eq(q0, QSINT(102));

  /* test letrec. */

  /* test call/cc. */
  qsptr_t name_lambda = qsstr_inject(heap1, "lambda", 0);
  qsptr_t sym_lambda = qssymbol_make(heap1, name_lambda);
  qsptr_t name_f = qsstr_inject(heap1, "f", 0);
  qsptr_t sym_f = qssymbol_make(heap1, name_f);
  qsptr_t name_k = qsstr_inject(heap1, "k", 0);
  qsptr_t sym_k = qssymbol_make(heap1, name_k);
  qsptr_t name_callcc = qsstr_inject(heap1, "call/cc", 0);
  qsptr_t sym_callcc = qssymbol_make(heap1, name_callcc);
  /*
     (define f (lambda (k) (k 5)))
     (call/cc f)
  */
  qsptr_t param1 = qspair_make(heap1, sym_k, QSNIL);
  qsptr_t body1 = qsimmlist_injectl(heap1, sym_k, QSINT(5), QSEOL);
  qsptr_t lam1 = qslambda_make(heap1, param1, body1);
  qsptr_t clo1 = qsclosure_make(heap1, qsenv_make(heap1,QSNIL), lam1);
  scheme1->E = qsenv_setq(heap1, scheme1->E, sym_f, clo1);

  qsptr_t callcc1 = qsimmlist_injectl(heap1, sym_callcc, sym_f, QSEOL);

  qs_inject_exp(scheme1, callcc1);
  qs_step(scheme1);
  qs_step(scheme1);
  q0 = scheme1->A;
  ck_assert_int_eq(q0, QSINT(5));

}
END_TEST


TESTCASE(mach1,
  TFUNC(test_inject1)
  )
TESTCASE(mach2,
  TFUNC(test_atomeval1)
  )
TESTCASE(mach3,
  TFUNC(test_step1)
  )
TESTCASE(mach4,
  TFUNC(test_step2)
  )


TESTSUITE(suite1,
  TCASE(mach1)
  TCASE(mach2)
  TCASE(mach3)
  TCASE(mach4)
  )

int main ()
{
  RUNSUITE(suite1);
}

