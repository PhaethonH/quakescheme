#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsstore.h"
#include "qsobj.h"
#include "qsval.h"
#include "qsmach.h"

/* Unit test  machine */

qsmachine_t _machine, *machine=&_machine;

char buf[65536];


void init ()
{
  qsmachine_init(machine);
}


START_TEST(test_inject1)
{
  init();

  qsptr_t C = QSINT(5);
  qsptr_t E = QSNIL;
  qsptr_t K = QSNIL;

  int res = qsmachine_load(machine, C, E, K);
  ck_assert_int_eq(res, 0);
}
END_TEST

/* sample qsprim_f */
qsptr_t op_plus_one (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNIL;
  qsptr_t x = qspair_p(mach, args) ? qspair_car(mach, args) :
    qsiter_p(mach, args) ? qsiter_head(mach, args) : QSNIL;
  if (ISINT30(x))
    {
      retval = QSINT( CINT30(x) + 1 );
    }
  return retval;
}

START_TEST(test_atomeval1)
{
  init();

  qsptr_t C, E, K;
  qsptr_t p;
  int res;

  /* Evaluate integer. */
  p = qsmachine_eval_atomic(machine, QSINT(5));
  ck_assert_int_eq(p, QSINT(5));

  /* Evaluate variable. */
  qsptr_t y_x = qssymbol_intern_c(machine, "x");
  E = qsenv_make(machine, QSNIL);
  E = qsenv_insert(machine, E, qsname_sym(machine, y_x), QSINT(7));
  machine->E = E;
  p = qsmachine_eval_atomic(machine, y_x);
  ck_assert_int_eq(p, QSINT(7));

  /* Evaluate lambda. */
  qsptr_t lam = qslambda_make(machine, QSNIL, QSINT(9));
  p = qsmachine_eval_atomic(machine, lam);
  ck_assert(qsclosure_p(machine, p));

  /* Evaluate operation (primitive). */
  int primid = qsprimreg_register(machine, op_plus_one);
  qsptr_t op = qsprim_make(machine, primid);
  qsptr_t exp = qspair_make(machine, op, qspair_make(machine, QSINT(11), QSNIL));
  p = qsmachine_eval_atomic(machine, exp);
  ck_assert_int_eq(p, QSINT(12));
}
END_TEST

START_TEST(test_step1)
{
  /* Atomic Evaluation as step. */
  init();

  qsptr_t C, E, K;
  qsptr_t p;
  int res;

  /* Evaluate integer. */
  C = QSINT(5);
  E = QSNIL;
  K = QSNIL;
  res = qsmachine_load(machine, C, E, K);
  res = qsmachine_step(machine);
  ck_assert_int_eq(machine->A, QSINT(5));
  ck_assert(machine->halt);

  /* Evaluate variable. */
  qsptr_t y_x = qssymbol_intern_c(machine, "x");
  C = y_x;
  E = qsenv_make(machine, QSNIL);
  E = qsenv_insert(machine, E, qsname_sym(machine, y_x), QSINT(7));
  K = QSNIL;
  qsmachine_load(machine, C, E, K);
  qsmachine_step(machine);
  ck_assert_int_eq(machine->A, QSINT(7));
  ck_assert(machine->halt);

  /* Evaluate lambda. */
  qsptr_t lam = qslambda_make(machine, QSNIL, QSINT(9));
  C = lam;
  E = QSNIL;
  K = QSNIL;
  qsmachine_load(machine, C, E, K);
  qsmachine_step(machine);
  ck_assert(qsclosure_p(machine, machine->A));
  ck_assert(machine->halt);

  /* Evaluate operation (primitive). */
  int primid = qsprimreg_register(machine, op_plus_one);
  qsptr_t op = qsprim_make(machine, primid);
  C = qspair_make(machine, op, qspair_make(machine, QSINT(11), QSNIL));
  E = QSNIL;
  K = QSNIL;
  qsmachine_load(machine, C, E, K);
  qsmachine_step(machine);
  ck_assert_int_eq(machine->A, QSINT(12));
  ck_assert(machine->halt);
}
END_TEST

START_TEST(test_step2)
{
  /* Non-atomic steps. */
  init();

  qsptr_t C, E, K;
  qsptr_t p, exp;
  int res;

  qsptr_t y_if = qssymbol_intern_c(machine, "if");
  qsptr_t y_let = qssymbol_intern_c(machine, "let");
  qsptr_t y_letrec = qssymbol_intern_c(machine, "letrec");
  qsptr_t y_setq = qssymbol_intern_c(machine, "set!");
  qsptr_t y_callcc = qssymbol_intern_c(machine, "call/cc");
  qsptr_t y_x = qssymbol_intern_c(machine, "x");
  qsptr_t y_y = qssymbol_intern_c(machine, "y");
  qsptr_t y_z = qssymbol_intern_c(machine, "z");

  /* Conditional. */
  exp = qspair_make(machine, y_if,
        qspair_make(machine, QSTRUE,
        qspair_make(machine, QSINT(1),
        qspair_make(machine, QSINT(2), QSNIL))));
  C = exp;
  E = QSNIL;
  K = QSNIL;
  qsmachine_load(machine, C, E, K);
  qsmachine_step(machine);  /* pick branch. */
  qsptr_crepr(machine, machine->C, buf, sizeof(buf));
  ck_assert_str_eq(buf, "1");
  qsmachine_step(machine);  /* evaluate picked branch. */
  qsptr_crepr(machine, machine->A, buf, sizeof(buf));
  ck_assert_str_eq(buf, "1");
  ck_assert_int_eq(machine->A, QSINT(1));
  ck_assert(machine->halt);

  exp = qspair_make(machine, y_if,
        qspair_make(machine, QSFALSE,
        qspair_make(machine, QSINT(1),
        qspair_make(machine, QSINT(2), QSNIL))));
  C = exp;
  E = QSNIL;
  K = QSNIL;
  qsmachine_load(machine, C, E, K);
  qsmachine_step(machine);  /* pick branch. */
  qsmachine_step(machine);  /* evaluate picked branch. */
  ck_assert_int_eq(machine->A, QSINT(2));
  ck_assert(machine->halt);

  /* Let. */
  exp = qspair_make(machine, y_let,
	qspair_make(machine,
	  qspair_make(machine,
	    qspair_make(machine, y_x,
	    qspair_make(machine, QSINT(23), QSNIL)),
	    QSNIL),
	qspair_make(machine, y_x, QSNIL)));
  qsptr_crepr(machine, exp, buf, sizeof(buf));
  ck_assert_str_eq(buf, "(let ((x 23)) x)");
  C = exp;
  E = QSNIL;
  K = QSNIL;
  qsmachine_load(machine, C, E, K);
  qsmachine_step(machine);  /* letk created. */
  ck_assert_int_eq(machine->C, QSINT(23));
  qsmachine_step(machine);  /* evaluate exp. */
  qsmachine_step(machine);  /* evaluate body. */
  ck_assert(machine->halt);

  /* Mutation. */
  exp = qspair_make(machine, y_setq,
	qspair_make(machine, y_y,
	qspair_make(machine, QSINT(38), QSNIL)));
  C = exp;
  E = QSNIL;
  K = QSNIL;
  qsmachine_load(machine, C, E, K);
  qsmachine_step(machine);
  ck_assert(machine->halt);
  p = qsenv_lookup(machine, machine->E, y_y);
  qsptr_crepr(machine, p, buf, sizeof(buf));
  ck_assert_str_eq(buf, "38");
  ck_assert_int_eq(p, QSINT(38));

  /* Recursive Let. */
  qsptr_t lis;
  lis = qsarray_inject(machine,
          y_letrec,
	  QSBOL,
	    QSBOL, y_x, QSINT(121), QSEOL,
	    QSBOL, y_y, QSINT(122), QSEOL,
	    QSBOL, y_z, QSINT(123), QSEOL,
	  QSEOL,
	  y_y,
	QSEOL);
  exp = qsiter_begin(machine, lis);
  *buf = 0;
  qsptr_crepr(machine, exp, buf, sizeof(buf));
  ck_assert_str_eq(buf, "(letrec ((x 121) (y 122) (z 123)) y)");
  C = exp;
  E = QSNIL;
  K = QSNIL;
  qsmachine_load(machine, C, E, K);
  qsmachine_step(machine);  /* prep env and C <- body. */
  qsmachine_step(machine);  /* evaluate body. */
  ck_assert(machine->halt);
  qsptr_crepr(machine, machine->A, buf, sizeof(buf));
  ck_assert_str_eq(buf, "122");
  ck_assert_int_eq(machine->A, QSINT(122));

  qsptr_t param, body, lam, clo;

  /* First-class Continuation. */
  qsptr_t y_testcc = qssymbol_intern_c(machine, "testcc");
  param = qspair_make(machine, y_x, QSNIL);
  body = qspair_make(machine, y_x,
	 qspair_make(machine, QSINT(5), QSNIL));
  lam = qslambda_make(machine, param, body);
  clo = qsclosure_make(machine, lam, QSNIL);
  exp = qspair_make(machine, y_callcc,
	qspair_make(machine, y_testcc, QSNIL));
  C = exp;
  E = qsenv_make(machine, QSNIL);
  E = qsenv_insert(machine, E, y_testcc, clo);
  K = QSNIL;
  qsmachine_load(machine, C, E, K);
  qsmachine_step(machine);  /* evaluate to C <- (x 5) */
  qsptr_crepr(machine, machine->C, buf, sizeof(buf));
  ck_assert_str_eq(buf, "(x 5)");
  qsmachine_step(machine);  /* evaluates (#<kont> 5) */
  qsptr_crepr(machine, machine->A, buf, sizeof(buf));
  ck_assert_str_eq(buf, "5");
  ck_assert(machine->halt);

  /* Procedure call. */
  int primid = qsprimreg_register(machine, op_plus_one);
  qsptr_t o_plusone = qsprim_make(machine, primid);
  qsptr_t y_plusone = qssymbol_intern_c(machine, "+1");
  E = qsenv_make(machine, QSNIL);
  E = qsenv_insert(machine, E, y_plusone, o_plusone);
  exp = qspair_make(machine, y_plusone,
	qspair_make(machine, QSINT(2), QSNIL));
  C = exp;
  K = QSNIL;
  qsmachine_load(machine, C, E, K);
  qsmachine_step(machine);  /* eval to list of atomics */
  qsptr_crepr(machine, machine->C, buf, sizeof(buf));
  ck_assert_str_eq(buf, "(#<prim 0> 2)");
  qsmachine_step(machine);  /* eval operator. */
  qsptr_crepr(machine, machine->A, buf, sizeof(buf));
  ck_assert_str_eq(buf, "3");
  ck_assert_int_eq(machine->A, QSINT(3));

  param = qspair_make(machine, y_x, QSNIL);
  body = y_x;
  lam = qslambda_make(machine, param, body);
  clo = qsclosure_make(machine, lam, QSNIL);
  E = qsenv_make(machine, QSNIL);
  E = qsenv_insert(machine, E, y_z, clo);
  exp = qspair_make(machine, y_z,
	qspair_make(machine, QSINT(32), QSNIL));
  C = exp;
  K = QSNIL;
  qsmachine_load(machine, C, E, K);
  qsmachine_step(machine);  /* turn (z 32) into (#<closure> 32) */
  qsmachine_step(machine);  /* evaluate closure. */
  qsptr_crepr(machine, machine->A, buf, sizeof(buf));
  ck_assert_str_eq(buf, "32");
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

