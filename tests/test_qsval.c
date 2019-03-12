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

START_TEST(test_voidptrs)
{
  init();

  qsptr p;
  qserr err;
  int n;
  int b;

  /* C pointers (void pointers). */
  void * q = NULL;
  p = qscptr_make(machine, NULL);
  ck_assert(qscptr_p(machine, p));
  q = qscptr_get(machine, p);
  ck_assert(q == NULL);
  q = NULL;
  n = qscptr_fetch(machine, p, &q);
  ck_assert(q == NULL);
  n = qscptr_crepr(machine, p, buf, sizeof(buf));
  ck_assert_str_eq(buf, "#<(void*)00000000>");

  void * nn = &n;
  p = qscptr_make(machine, nn);
  ck_assert(qscptr_p(machine, p));
  q = qscptr_get(machine, p);
  ck_assert(q == nn);
  q = NULL;
  n = qscptr_fetch(machine, p, &q);
  ck_assert(q == nn);
  n = qscptr_crepr(machine, p, buf, sizeof(buf));
  ck_assert_str_ne(buf, "#<(void*)00000000>");
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

START_TEST(test_arrays)
{
  init();

  qsptr p;
  qserr err;
  int n;
  int b;
  qsptr it;
  qsptr probe;

  /* arrays. */

  /* Test manual setup and iteration. */
  p = qsarray_make(machine, 12);
  /* 12 ptrs need 3 boundaries, +1 for header => 4 boundaries total. */
  ck_assert_int_eq( qsarray_length(machine,p), 4*(sizeof(qsobj_t)/sizeof(qsptr)) );
  qsarray_setq(machine, p, 0, QSINT(101));
  qsarray_setq(machine, p, 1, QSINT(102));
  qsarray_setq(machine, p, 2, QSINT(103));
  qsarray_setq(machine, p, 3, QSINT(104));
  qsarray_setq(machine, p, 4, QSINT(105));
  qsarray_setq(machine, p, 5, QSINT(106));
  qsarray_setq(machine, p, 6, QSINT(107));
  qsarray_setq(machine, p, 7, QSEOL);
  ck_assert(ISOBJ26(p));
  ck_assert(qsarray_p(machine, p));

  it = qsarray_iter(machine, p);
  /* should be boundary after object's start. */
  ck_assert_int_eq( CITER28(it), (COBJ26(p)+1)<<2 );
  ck_assert(ISITER28(it));
  probe = qsiter_head(machine, it);
  ck_assert_int_eq(probe, QSINT(101));

  it = qsiter_tail(machine, it);
  ck_assert(ISITER28(it));
  probe = qsiter_head(machine, it);
  ck_assert_int_eq(probe, QSINT(102));

  it = qsiter_tail(machine, it);
  ck_assert(ISITER28(it));
  probe = qsiter_head(machine, it);
  ck_assert_int_eq(probe, QSINT(103));

  it = qsiter_tail(machine, it);
  ck_assert(ISITER28(it));
  probe = qsiter_head(machine, it);
  ck_assert_int_eq(probe, QSINT(104));

  it = qsiter_tail(machine, it);
  ck_assert(ISITER28(it));
  probe = qsiter_head(machine, it);
  ck_assert_int_eq(probe, QSINT(105));

  it = qsiter_tail(machine, it);
  ck_assert(ISITER28(it));
  probe = qsiter_head(machine, it);
  ck_assert_int_eq(probe, QSINT(106));

  it = qsiter_tail(machine, it);
  ck_assert(ISITER28(it));
  probe = qsiter_head(machine, it);
  ck_assert_int_eq(probe, QSINT(107));

  it = qsiter_tail(machine, it);
  ck_assert_int_eq(it, QSNIL);


  /* Test iteration from injection call. */
  p = qsarray_inject(machine, QSINT(1001), QSINT(1002), QSINT(1003), QSEOL);
  /* 1 bounds for payload, plus 1 for header, rounded to 2 bounds total. */
  ck_assert_int_eq( qsarray_length(machine,p), 2*(sizeof(qsobj_t)/sizeof(qsptr)) );
  it = qsarray_iter(machine, p);
  ck_assert(ISITER28(it));
  probe = qsiter_head(machine, it);
  ck_assert_int_eq(probe, QSINT(1001));

  it = qsiter_tail(machine, it);
  ck_assert(ISITER28(it));
  probe = qsiter_head(machine, it);
  ck_assert_int_eq(probe, QSINT(1002));

  it = qsiter_tail(machine, it);
  ck_assert(ISITER28(it));
  probe = qsiter_head(machine, it);
  ck_assert_int_eq(probe, QSINT(1003));

  it = qsiter_tail(machine, it);
  ck_assert(ISNIL(it));


  /* Test iterating with nested arrays. */
  p = qsarray_inject(machine, QSINT(2001), QSBOL, QSINT(2101), QSINT(2102),
		     QSEOL, QSINT(2003), QSEOL);
  ck_assert_int_eq( qsarray_length(machine,p), 4*(sizeof(qsobj_t)/sizeof(qsptr)) );
  it = qsarray_iter(machine, p);
  ck_assert(ISITER28(it));
  probe = qsiter_head(machine, it);
  ck_assert_int_eq(probe, QSINT(2001));

  it = qsiter_tail(machine, it);
  ck_assert(ISITER28(it));
  probe = qsiter_head(machine, it);
  ck_assert(ISITER28(probe));

    {
      /* iterate nested list. */

      qsptr it2 = probe;
      probe = qsiter_head(machine, it2);
      ck_assert_int_eq(probe, QSINT(2101));

      it2 = qsiter_tail(machine, it2);
      probe = qsiter_head(machine, it2);
      ck_assert_int_eq(probe, QSINT(2102));

      it2 = qsiter_tail(machine, it2);
      ck_assert_int_eq(it2, QSNIL);
    }

  it = qsiter_tail(machine, it);
  ck_assert(ISITER28(it));
  probe = qsiter_head(machine, it);
  ck_assert_int_eq(probe, QSINT(2003));


  /* Test more-deeply nested array. */
  p = qsarray_inject(machine, QSBOL, QSBOL, QSINT(3000), QSEOL, QSEOL, QSEOL);
  it = qsarray_iter(machine, p);
  probe = qsiter_head(machine, it);
  ck_assert(ISITER28(probe));

  it = qsiter_tail(machine, it);
  ck_assert_int_eq(it, QSNIL);
}
END_TEST

START_TEST(test_iters)
{
  init();

  qsptr p;
  qserr err;
  int n;
  int b;
  qsptr it;
  qsptr probe;

  /* iterators. */

  qsptr pairs[16];

  pairs[4] = qspair_make(machine, QSINT(4004), QSNIL);
  pairs[3] = qspair_make(machine, QSINT(4003), pairs[4]);
  pairs[2] = qspair_make(machine, QSINT(4002), pairs[3]);
  pairs[1] = qspair_make(machine, QSINT(4001), pairs[2]);
  pairs[0] = qspair_make(machine, QSINT(4000), pairs[1]);

  ck_assert(qspair_p(machine, pairs[0]));

  it = qspair_iter(machine, pairs[0]);
  ck_assert(qsiter_p(machine, it));
  probe = qsiter_head(machine, it);
  ck_assert_int_eq(probe, QSINT(4000));

  it = qsiter_tail(machine, it);
  ck_assert(qsiter_p(machine, it));
  probe = qsiter_head(machine, it);
  ck_assert_int_eq(probe, QSINT(4001));

  it = qsiter_tail(machine, it);
  ck_assert(qsiter_p(machine, it));
  probe = qsiter_head(machine, it);
  ck_assert_int_eq(probe, QSINT(4002));

  it = qsiter_tail(machine, it);
  ck_assert(qsiter_p(machine, it));
  probe = qsiter_head(machine, it);
  ck_assert_int_eq(probe, QSINT(4003));

  it = qsiter_tail(machine, it);
  ck_assert(qsiter_p(machine, it));
  probe = qsiter_head(machine, it);
  ck_assert_int_eq(probe, QSINT(4004));

  it = qsiter_tail(machine, it);
  ck_assert_int_eq(it, QSNIL);
}
END_TEST

START_TEST(test_utf8)
{
  init();

  qsptr p;
  qserr err;
  qsptr probe;

  /* UTF-8 encoded strings. */

  p = qsutf8_make(machine, 8, (char)0);
  ck_assert_int_eq(machine->S.smem.freelist, 2*sizeof(qsobj_t));

  err = qsutf8_setq(machine, p, 0, 'H');
  ck_assert_int_eq(err, p);
  err = qsutf8_setq(machine, p, 1, 'e');
  ck_assert_int_eq(err, p);
  err = qsutf8_setq(machine, p, 2, 'l');
  ck_assert_int_eq(err, p);
  err = qsutf8_setq(machine, p, 3, 'l');
  ck_assert_int_eq(err, p);
  err = qsutf8_setq(machine, p, 4, 'o');
  ck_assert_int_eq(err, p);
  err = qsutf8_setq(machine, p, 5, '!');
  ck_assert_int_eq(err, p);
  err = qsutf8_setq(machine, p, 6, '!');
  ck_assert_int_eq(err, p);
  err = qsutf8_setq(machine, p, 7, '!');
  ck_assert_int_eq(err, p);

  err = qsutf8_setq(machine, p, 8, '8');
  ck_assert(ISERR20(err));

  qsutf8_crepr(machine, p, buf, sizeof(buf));
  ck_assert_str_eq(buf, "\"Hello!!!\"");


  p = qsutf8_inject_charp(machine, "Hello, world.");
  ck_assert_int_eq(machine->S.smem.freelist, (2+2)*sizeof(qsobj_t));
  ck_assert_int_eq(qsutf8_length(machine, p), 13);
}
END_TEST

START_TEST(test_symbols)
{
  init();

  qsptr p, q;
  qserr err;
  int n;
  int b;

  /* symbols (names). */
  ck_assert_int_eq(machine->Y, QSNIL);

  p = qssymbol_intern_c(machine, "foobar");
  ck_assert(qssymbol_p(machine, p));
  n = qssymbol_crepr(machine, p, buf, sizeof(buf));
  ck_assert_str_eq(buf, "foobar");

  ck_assert_int_ne(machine->Y, QSNIL);

  /* check intern returns the same object instance. */
  q = qssymbol_intern_c(machine, "foobar");
  ck_assert(qssymbol_p(machine, q));
  ck_assert_int_eq(q, p);

  /* populate with useless symbols. */
  qssymbol_intern_c(machine, "foo");
  qssymbol_intern_c(machine, "bar");
  p = qssymbol_intern_c(machine, "baz");
  qssymbol_intern_c(machine, "quux");

  q = qssymbol_intern_c(machine, "baz");
  ck_assert_int_eq(q, p);
}
END_TEST

START_TEST(test_envs)
{
  init();

  qsptr p, q;
  qserr err;
  int n;
  int b;

  /* environment (variable bindings). */

  /* ( (foo . 1) (bar . 2) (baz . 3) ) */
  qsptr env = qsenv_make(machine, QSNIL);

  qsptr y_foo = qssymbol_intern_c(machine, "foo");
  qsptr y_bar = qssymbol_intern_c(machine, "bar");
  qsptr y_baz = qssymbol_intern_c(machine, "baz");
  qsptr y_quux = qssymbol_intern_c(machine, "quux");

  env = qsenv_insert(machine, env, y_foo, QSINT(1));
  ck_assert_int_ne(env, QSNIL);
  env = qsenv_insert(machine, env, y_bar, QSINT(2));
  env = qsenv_insert(machine, env, y_baz, QSINT(3));

  p = qsenv_lookup(machine, env, y_foo);
  ck_assert(ISINT30(p));
  ck_assert_int_eq(p, QSINT(1));

  p = qsenv_lookup(machine, env, y_bar);
  ck_assert_int_eq(p, QSINT(2));

  p = qsenv_lookup(machine, env, y_baz);
  ck_assert_int_eq(p, QSINT(3));

  p = qsenv_lookup(machine, env, y_quux);
  ck_assert_int_eq(p, QSERR_UNBOUND);


  /* New frame, ( (foo . 222) (quux . 444) ) */
  env = qsenv_make(machine, env);

  env = qsenv_insert(machine, env, y_foo, QSINT(222));
  env = qsenv_insert(machine, env, y_quux, QSINT(444));

  p = qsenv_lookup(machine, env, y_foo);
  ck_assert_int_eq(p, QSINT(222));

  p = qsenv_lookup(machine, env, y_bar);
  ck_assert_int_eq(p, QSINT(2));

  p = qsenv_lookup(machine, env, y_baz);
  ck_assert_int_eq(p, QSINT(3));

  p = qsenv_lookup(machine, env, y_quux);
  ck_assert_int_eq(p, QSINT(444));
}
END_TEST


TESTCASE(case1,
  TFUNC(test_test1)
  TFUNC(test_pairs)
  TFUNC(test_vectors)
  TFUNC(test_widenums)
  TFUNC(test_voidptrs)
  TFUNC(test_bytevecs)
  TFUNC(test_arrays)
  TFUNC(test_iters)
  TFUNC(test_utf8)
  TFUNC(test_symbols)
  TFUNC(test_envs)
  TFUNC(test_test2)
  )

TESTSUITE(suite1,
  TCASE(case1)
  )

int main ()
{
  RUNSUITE(suite1);
}

