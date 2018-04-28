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

char buf[131072];


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


TESTCASE(mach1,
  TFUNC(test_inject1)
  TFUNC(test_atomeval1)
  )

TESTSUITE(suite1,
  TCASE(mach1)
  )

int main ()
{
  RUNSUITE(suite1);
}

