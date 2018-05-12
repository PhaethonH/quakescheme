#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test  closure, lambda, env */

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


START_TEST(test_env1)
{
  init();

  qsptr_t e0 = qsenv_make(heap1, QSNIL);
  ck_assert_int_ne(e0, QSNIL);

  qsptr_t strs[32] = { QSNIL, };
  strs[0] = qsstr_inject(heap1, "foo", 0);
  strs[1] = qsstr_inject(heap1, "bar", 0);
  strs[2] = qsstr_inject(heap1, "baz", 0);
  strs[3] = qsstr_inject(heap1, "lorem_ipsum", 0);

  qsptr_t syms[32] = { QSNIL, };
  int i;
  for (i = 0; i < 4; i++)
    {
      qsptr_t y = qssymbol_make(heap1, strs[i]);
      ck_assert_int_ne(y, QSNIL);
      syms[i] = y;
    }

  qsenv_setq(heap1, e0, syms[0], QSINT(10));

  /* Frame 0. */
  qsptr_t q0 = qsenv_ref(heap1, e0, syms[0]);
  ck_assert_int_eq(q0, QSINT(10));
  /* check newly added binding. */
  q0 = qsenv_ref(heap1, e0, syms[1]);
  ck_assert_int_eq(q0, QSERROR_INVALID);

  qsenv_crepr(heap1, e0, buf, sizeof(buf));
  ck_assert_str_eq(buf, "(ENV foo:10)");


  /* Frame 1: check resolving down frames. */
  qsptr_t e1 = qsenv_make(heap1, e0);
  qsenv_setq(heap1, e1, syms[1], QSINT(11));
  /* check newly added binding. */
  q0 = qsenv_ref(heap1, e1, syms[1]);
  ck_assert_int_eq(q0, QSINT(11));
  /* check resolve down frames. */
  q0 = qsenv_ref(heap1, e1, syms[0]);
  ck_assert_int_eq(q0, QSINT(10));

  /* multiple added to most-recent frame. */
  qsenv_setq(heap1, e1, syms[2], QSINT(12));
  qsenv_setq(heap1, e1, syms[3], QSINT(13));
  /* check last insert. */
  q0 = qsenv_ref(heap1, e1, syms[3]);
  ck_assert_int_eq(q0, QSINT(13));
  /* re-check first insert this frame. */
  q0 = qsenv_ref(heap1, e1, syms[1]);
  ck_assert_int_eq(q0, QSINT(11));
  /* re-check resolve down frames. */
  q0 = qsenv_ref(heap1, e1, syms[0]);
  ck_assert_int_eq(q0, QSINT(10));


  qsword cnt = qsenv_length(heap1, e1);
  ck_assert_int_eq(cnt, 3);


  /* Frame 2: overload bindings. */
  qsptr_t e2 = qsenv_make(heap1, e1);
  qsenv_setq(heap1, e2, syms[0], QSINT(110));
  /* check in most recent frame. */
  q0 = qsenv_ref(heap1, e2, syms[0]);
  ck_assert_int_eq(q0, QSINT(110));
  /* check in oldest frame. */
  q0 = qsenv_ref(heap1, e0, syms[0]);
  ck_assert_int_eq(q0, QSINT(10));

  qsenv_crepr(heap1, e1, buf, sizeof(buf));
  ck_assert_str_eq(buf, "(ENV lorem_ipsum:13 baz:12 bar:11)");
}
END_TEST

START_TEST(test_env2)
{
  init();

  qsptr_t e0 = qsenv_make(heap1, QSNIL);
  ck_assert_int_ne(e0, QSNIL);

  qsptr_t strs[32] = { QSNIL, };
  strs[0] = qsstr_inject(heap1, "foo", 0);
  strs[1] = qsstr_inject(heap1, "bar", 0);
  strs[2] = qsstr_inject(heap1, "baz", 0);
  strs[3] = qsstr_inject(heap1, "lorem_ipsum", 0);

  qsptr_t syms[32] = { QSNIL, };
  int i;
  for (i = 0; i < 4; i++)
    {
      qsptr_t y = qssymbol_make(heap1, strs[i]);
      ck_assert_int_ne(y, QSNIL);
      syms[i] = y;
    }

  qsenv_setq(heap1, e0, syms[0], QSINT(10));
  qsenv_setq(heap1, e0, syms[1], QSINT(20));
  qsenv_setq(heap1, e0, syms[2], QSINT(30));
  qsenv_setq(heap1, e0, syms[3], QSINT(40));
  ck_assert_int_eq(qsenv_length(heap1, e0), 4);

  /* Frame 0. */
  qsptr_t q0;
  q0 = qsenv_ref(heap1, e0, syms[0]);
  ck_assert_int_eq(q0, QSINT(10));
  q0 = qsenv_ref(heap1, e0, syms[1]);
  ck_assert_int_eq(q0, QSINT(20));
  q0 = qsenv_ref(heap1, e0, syms[2]);
  ck_assert_int_eq(q0, QSINT(30));
  q0 = qsenv_ref(heap1, e0, syms[3]);
  ck_assert_int_eq(q0, QSINT(40));
}
END_TEST

START_TEST(test_lambda1)
{
  init();

  qsptr_t sym_x = qssymbol_make(heap1, qsstr_inject(heap1, "x", 0));
  qsptr_t sym_y = qssymbol_make(heap1, qsstr_inject(heap1, "y", 0));
  qsptr_t sym_z = qssymbol_make(heap1, qsstr_inject(heap1, "z", 0));
  ck_assert_int_ne(QSNIL, sym_x);
  ck_assert_int_ne(QSNIL, sym_y);
  ck_assert_int_ne(QSNIL, sym_z);

  /* The identity function: (lambda (x) x) */
  qsptr_t parm0 = qspair_make(heap1, sym_x, QSNIL);
  ck_assert_int_ne(parm0, QSNIL);
  qsptr_t body0 = qspair_make(heap1, sym_x, QSNIL);
  ck_assert_int_ne(body0, QSNIL);
  qsptr_t lam0 = qslambda_make(heap1, parm0, body0);
  ck_assert_int_ne(lam0, QSNIL);

  ck_assert_int_eq(qslambda_ref_param(heap1, lam0), parm0);
  ck_assert_int_eq(qslambda_ref_body(heap1, lam0), body0);


  // TODO: longer parameter list
}
END_TEST


START_TEST(test_closure1)
{
  init();

  qsptr_t sym_x = qssymbol_make(heap1, qsstr_inject(heap1, "x", 0));
  qsptr_t sym_y = qssymbol_make(heap1, qsstr_inject(heap1, "y", 0));
  qsptr_t sym_z = qssymbol_make(heap1, qsstr_inject(heap1, "z", 0));
  ck_assert_int_ne(QSNIL, sym_x);
  ck_assert_int_ne(QSNIL, sym_y);
  ck_assert_int_ne(QSNIL, sym_z);

  /* The identity function: (lambda (x) x) */
  qsptr_t parm0 = qspair_make(heap1, sym_x, QSNIL);
  qsptr_t body0 = qspair_make(heap1, sym_x, QSNIL);
  qsptr_t lam0 = qslambda_make(heap1, parm0, body0);
  ck_assert_int_ne(lam0, QSNIL);

  qsptr_t e0 = qsenv_make(heap1, QSNIL);
  ck_assert_int_ne(e0, QSNIL);

  qsptr_t sym_foo = qssymbol_make(heap1, qsstr_inject(heap1, "foo", 0));
  qsptr_t sym_bar = qssymbol_make(heap1, qsstr_inject(heap1, "bar", 0));
  qsenv_setq(heap1, e0, sym_foo, QSINT(1000));
  qsenv_setq(heap1, e0, sym_bar, QSINT(2000));

  qsptr_t q0 = QSNIL;
  qsptr_t clo0 = qsclosure_make(heap1, e0, lam0);
  q0 = qsclosure_ref_env(heap1, clo0);
  ck_assert_int_eq(q0, e0);
  q0 = qsclosure_ref_lambda(heap1, clo0);
  ck_assert_int_eq(q0, lam0);
}
END_TEST


TESTCASE(closure1,
  TFUNC(test_env1)
  TFUNC(test_env2)
  TFUNC(test_lambda1)
  TFUNC(test_closure1)
  )

TESTSUITE(suite1,
  TCASE(closure1)
  )

int main ()
{
  RUNSUITE(suite1);
}

