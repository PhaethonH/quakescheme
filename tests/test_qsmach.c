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

  qsptr C = QSINT(5);
  qsptr E = QSNIL;
  qsptr K = QSNIL;

  int res = qsmachine_load(machine, C, E, K);
  ck_assert_int_eq(res, 0);
}
END_TEST

/* sample qsprim_f */
qsptr op_plus_one (qsmachine_t * mach, qsptr args)
{
  qsptr retval = QSNIL;
  qsptr x = qspair_p(mach, args) ? qspair_car(mach, args) :
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

  qsptr C, E, K;
  qsptr p;
  int res;

  /* Evaluate integer. */
  p = qsmachine_eval_atomic(machine, QSINT(5));
  ck_assert_int_eq(p, QSINT(5));

  /* Evaluate variable. */
  qsptr y_x = qssymbol_intern_c(machine, "x");
  E = qsenv_make(machine, QSNIL);
  E = qsenv_insert(machine, E, qsname_sym(machine, y_x), QSINT(7));
  machine->E = E;
  p = qsmachine_eval_atomic(machine, y_x);
  ck_assert_int_eq(p, QSINT(7));

  /* Evaluate lambda. */
  qsptr lam = qslambda_make(machine, QSNIL, QSINT(9));
  p = qsmachine_eval_atomic(machine, lam);
  ck_assert(qsclosure_p(machine, p));

  /* Evaluate operation (primitive). */
  int primid = qsprimreg_register(machine, op_plus_one);
  qsptr op = qsprim_make(machine, primid);
  qsptr exp = qspair_make(machine, op, qspair_make(machine, QSINT(11), QSNIL));
  p = qsmachine_eval_atomic(machine, exp);
  ck_assert_int_eq(p, QSINT(12));
}
END_TEST

START_TEST(test_step1)
{
  /* Atomic Evaluation as step. */
  init();

  qsptr C, E, K;
  qsptr p;
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
  qsptr y_x = qssymbol_intern_c(machine, "x");
  C = y_x;
  E = qsenv_make(machine, QSNIL);
  E = qsenv_insert(machine, E, qsname_sym(machine, y_x), QSINT(7));
  K = QSNIL;
  qsmachine_load(machine, C, E, K);
  qsmachine_step(machine);
  ck_assert_int_eq(machine->A, QSINT(7));
  ck_assert(machine->halt);

  /* Evaluate lambda. */
  qsptr lam = qslambda_make(machine, QSNIL, QSINT(9));
  C = lam;
  E = QSNIL;
  K = QSNIL;
  qsmachine_load(machine, C, E, K);
  qsmachine_step(machine);
  ck_assert(qsclosure_p(machine, machine->A));
  ck_assert(machine->halt);

  /* Evaluate operation (primitive). */
  int primid = qsprimreg_register(machine, op_plus_one);
  qsptr op = qsprim_make(machine, primid);
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

  qsptr C, E, K;
  qsptr p, exp;
  int res;

  qsptr y_if = qssymbol_intern_c(machine, "if");
  qsptr y_let = qssymbol_intern_c(machine, "let");
  qsptr y_letrec = qssymbol_intern_c(machine, "letrec");
  qsptr y_setq = qssymbol_intern_c(machine, "set!");
  qsptr y_callcc = qssymbol_intern_c(machine, "call/cc");
  qsptr y_x = qssymbol_intern_c(machine, "x");
  qsptr y_y = qssymbol_intern_c(machine, "y");
  qsptr y_z = qssymbol_intern_c(machine, "z");

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
  qsptr lis;
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

  /* First-class Continuation. */

  /* Procedure call. */
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

