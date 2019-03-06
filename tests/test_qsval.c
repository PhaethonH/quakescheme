#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsstore.h"
#include "qsobj.h"
#include "qsval.h"
#include "qsmach.h"

/* Unit test: qsvalue, values manipulation */

qsmachine_t _machine, *machine=&_machine;

char buf[4096];


void init ()
{
  qsmachine_init(machine);
}


START_TEST(test_test1)
{
  init();

  /* Directly-encoded values. */

  qsptr p;
  int n;


  /* float31 */
  p = qsfloat_make(machine, 0);
  n = qsfloat_crepr(machine, p, buf, sizeof(buf));
  ck_assert(qsfloat_p(machine, p));
  ck_assert_str_eq(buf, "0");

  p = qsfloat_make(machine, 1);
  n = qsfloat_crepr(machine, p, buf, sizeof(buf));
  ck_assert(qsfloat_p(machine, p));
  ck_assert_str_eq(buf, "1");

  p = qsfloat_make(machine, 1.5);
  n = qsfloat_crepr(machine, p, buf, sizeof(buf));
  ck_assert(qsfloat_p(machine, p));
  ck_assert_str_eq(buf, "1.5");


  /* int30 */
  p = qsint_make(machine, 0);
  n = qsint_crepr(machine, p, buf, sizeof(buf));
  ck_assert(qsint_p(machine, p));
  ck_assert_str_eq(buf, "0");

  p = qsint_make(machine, 42);
  n = qsint_crepr(machine, p, buf, sizeof(buf));
  ck_assert(qsint_p(machine, p));
  ck_assert_str_eq(buf, "42");

  p = qsint_make(machine, -1048576);
  n = qsint_crepr(machine, p, buf, sizeof(buf));
  ck_assert(qsint_p(machine, p));
  ck_assert_str_eq(buf, "-1048576");


  /* char24 */
  p = qschar_make(machine, 65);
  n = qschar_crepr(machine, p, buf, sizeof(buf));
  ck_assert(qschar_p(machine, p));
  ck_assert_str_eq(buf, "#\\A");

  p = qschar_make(machine, 32);
  n = qschar_crepr(machine, p, buf, sizeof(buf));
  ck_assert(qschar_p(machine, p));
  ck_assert_str_eq(buf, "#\\space");

  p = qschar_make(machine, 9);
  n = qschar_crepr(machine, p, buf, sizeof(buf));
  ck_assert(qschar_p(machine, p));
  ck_assert_str_eq(buf, "#\\tab");

  p = qschar_make(machine, 10);
  n = qschar_crepr(machine, p, buf, sizeof(buf));
  ck_assert(qschar_p(machine, p));
  ck_assert_str_eq(buf, "#\\newline");


  /* const20 */
  p = QSNIL;
  n = qsconst_crepr(machine, p, buf, sizeof(buf));
  ck_assert(qsconst_p(machine, p));
  ck_assert_str_eq(buf, "()");

  p = QSTRUE;
  n = qsconst_crepr(machine, p, buf, sizeof(buf));
  ck_assert(qsconst_p(machine, p));
  ck_assert_str_eq(buf, "#t");

  p = QSFALSE;
  n = qsconst_crepr(machine, p, buf, sizeof(buf));
  ck_assert(qsconst_p(machine, p));
  ck_assert_str_eq(buf, "#f");

  p = qsconst_make(machine, 123456);
  n = qsconst_crepr(machine, p, buf, sizeof(buf));
  ck_assert(qsconst_p(machine, p));
  ck_assert_str_eq(buf, "#<const 123456>");
}
END_TEST

START_TEST(test_test2)
{
  init();

  /* Generalized printing. */
  qsptr p;
  int n;

  p = qsint_make(machine, 17);
  n = qsptr_crepr(machine, p, buf, sizeof(buf));
  ck_assert_str_eq(buf, "17");
}
END_TEST

START_TEST(test_pairs)
{
  init();

  qsptr p;
  int n;

  /* pairs (improper list) */
  p = qspair_make(machine, QSINT(1), QSINT(1));
  ck_assert(qspair_p(machine, p));
  ck_assert_int_ne(p, QSNIL);
  ck_assert_int_eq(qspair_car(machine, p), QSINT(1));
  ck_assert_int_eq(qspair_cdr(machine, p), QSINT(1));

  p = qspair_make(machine, QSINT(2), QSINT(3));
  ck_assert(qspair_p(machine, p));
  ck_assert_int_ne(p, QSNIL);
  ck_assert_int_eq(qspair_car(machine, p), QSINT(2));
  ck_assert_int_eq(qspair_cdr(machine, p), QSINT(3));

  n = qspair_crepr(machine, p, buf, sizeof(buf));
  ck_assert_str_eq(buf, "(2 . 3)");

  /* proper list. */
  qsptr pp[8];
  pp[3] = qspair_make(machine, QSINT(30), QSNIL);
  pp[2] = qspair_make(machine, QSINT(20), pp[3]);
  pp[1] = qspair_make(machine, QSINT(10), pp[2]);
  ck_assert(qspair_p(machine, pp[1]));
  ck_assert(qspair_p(machine, pp[2]));
  ck_assert(qspair_p(machine, pp[3]));
  n = qspair_crepr(machine, pp[1], buf, sizeof(buf));
  ck_assert_str_eq(buf, "(10 20 30)");
}
END_TEST

START_TEST(test_vectors)
{
  init();

  qsptr p;
  qserr err;
  int n;

  /* vector of 5. */
  p = qsvector_make(machine, 5, QSNIL);
  ck_assert(qsvector_p(machine, p));
  n = qsvector_crepr(machine, p, buf, sizeof(buf));
  ck_assert_str_eq(buf, "#(() () () () ())");

  ck_assert_int_eq(qsvector_length(machine, p), 5);

  err = qsvector_setq(machine, p, 0, QSINT(100));
  err = qsvector_setq(machine, p, 1, QSINT(200));
  err = qsvector_setq(machine, p, 2, QSINT(300));
  err = qsvector_setq(machine, p, 3, QSINT(400));
  err = qsvector_setq(machine, p, 4, QSINT(500));
  ck_assert_int_eq(err, p);
  err = qsvector_setq(machine, p, 5, QSINT(500));
  ck_assert_int_eq(err, QSERR_FAULT);
  n = qsvector_crepr(machine, p, buf, sizeof(buf));
  ck_assert_str_eq(buf, "#(100 200 300 400 500)");


  p = qsvector_make(machine, 13, QSINT(13));
  ck_assert(qsvector_p(machine, p));
  ck_assert_int_eq(qsvector_length(machine, p), 13);
  ck_assert_int_eq(qsvector_ref(machine, p, 0), QSINT(13));
  ck_assert_int_eq(qsvector_ref(machine, p, 10), QSINT(13));
  n = qsvector_crepr(machine, p, buf, sizeof(buf));
  ck_assert_str_eq(buf, "#(13 13 13 13 13 13 13 13 13 13 13 13 13)");
}
END_TEST

START_TEST(test_widenums)
{
  init();

  qsptr p;
  qserr err;
  int n;

  /* longs */
  p = qslong_make(machine, 8111222333);
  ck_assert(qslong_p(machine, p));  /* test predicate */
  long l = 0;
  l = qslong_get(machine, p); /* test get(). */
  ck_assert(l == 8111222333);
  l = 0;
  n = qslong_fetch(machine, p, &l); /* test fetch(). */
  ck_assert_int_ne(n, 0);
  ck_assert(l == 8111222333);
  n = qslong_crepr(machine, p, buf, sizeof(buf));  /* test stringify. */
  ck_assert_str_eq(buf, "8111222333");


  /* doubles. */
  p = qsdouble_make(machine, 3.141592653589L);
  ck_assert(qsdouble_p(machine, p)); /* test predicate. */
  double d = 0;
  d = qsdouble_get(machine, p); /* test get(). */
  ck_assert( (d - 3.141592653589L) <= 0.0000001 );
  d = 0;
  n = qsdouble_fetch(machine, p, &d); /* test fetch(). */
  ck_assert_int_ne(n, 0);
  ck_assert( (d - 3.141592653589L) <= 0.0000001 );
  n = qsdouble_crepr(machine, p, buf, sizeof(buf)); /* test stringify. */
  ck_assert_str_eq(buf, "3.14159");

  p = qsdouble_make(machine, 3141592653589.79L);
  n = qsdouble_crepr(machine, p, buf, sizeof(buf));
  ck_assert_str_eq(buf, "3.14159e+12");
}
END_TEST

START_TEST(test_bytevecs)
{
  init();

  qsptr p;
  qserr err;
  int n;
  int b;

  /* byte vectors. */
  p = qsbytevec_make(machine, 12, 0);
  ck_assert(qsbytevec_p(machine, p)); /* test predicate */
  n = qsbytevec_crepr(machine, p, buf, sizeof(buf));
  ck_assert_str_eq(buf, "#u8(0 0 0 0 0 0 0 0 0 0 0 0)");

  n = qsbytevec_length(machine, p);
  ck_assert_int_eq(n, 12);
  b = qsbytevec_ref(machine, p, 7); /* test ref(). */
  ck_assert_int_eq(b, 0);
  qsbytevec_setq(machine, p, 6, 27); /* test setq(). */
  b = qsbytevec_ref(machine, p, 6);
  ck_assert_int_eq(b, 27);
  n = qsbytevec_crepr(machine, p, buf, sizeof(buf));
  ck_assert_str_eq(buf, "#u8(0 0 0 0 0 0 27 0 0 0 0 0)");
}
END_TEST


TESTCASE(case1,
  TFUNC(test_test1)
  TFUNC(test_pairs)
  TFUNC(test_vectors)
  TFUNC(test_widenums)
  TFUNC(test_bytevecs)
  TFUNC(test_test2)
  )

TESTSUITE(suite1,
  TCASE(case1)
  )

int main ()
{
  RUNSUITE(suite1);
}
