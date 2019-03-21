#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsstore.h"
#include "qsval.h"
#include "qsmach.h"

/* Unit test  garbage collection */

qsmachine_t _machine, *machine=&_machine;

char buf[65536];


void init ()
{
  qsmachine_init(machine);
}


bool is_used (qsptr p)
{
  if (! ISOBJ26(p)) return false;
  qsword addr = COBJ26(p) << 4;
  const qsword * refmgmt = qsstore_word_at_const(&(machine->S), addr);
  qsword mgmt = *refmgmt;
  if (! ISSYNC29(mgmt)) return false;
  return MGMT_IS_USED(mgmt) ? true : false;
}

bool is_marked (qsptr p)
{
  if (! ISOBJ26(p)) return false;
  qsword addr = COBJ26(p) << 4;
  const qsword * refmgmt = qsstore_word_at_const(&(machine->S), addr);
  qsword mgmt = *refmgmt;
  if (! ISSYNC29(mgmt)) return false;
  if (! MGMT_IS_USED(mgmt)) return false;
  return MGMT_IS_MARK(mgmt) ? true : false;
}

int gcmark (qsptr p)
{
  if (! COBJ26(p)) return false;
  qsword addr = COBJ26(p) << 4;
  qsstore_trace(&(machine->S), addr, 1);
  return 0;
}

int gcsweep ()
{
  qsstore_sweep(&(machine->S));
  return 0;
}

START_TEST(test_mark1)
{
  init();

  /* test marking one of many pairs. */
  qsptr cells[16];

  cells[7] = qspair_make(machine, QSINT(17), QSNIL);
  cells[6] = qspair_make(machine, QSINT(16), QSNIL);
  cells[5] = qspair_make(machine, QSINT(15), QSNIL);
  cells[4] = qspair_make(machine, QSINT(14), QSNIL);
  cells[3] = qspair_make(machine, QSINT(13), QSNIL);
  cells[2] = qspair_make(machine, QSINT(12), QSNIL);
  cells[1] = qspair_make(machine, QSINT(11), QSNIL);
  cells[0] = qspair_make(machine, QSINT(10), QSNIL);

  int i;

  for (i = 0; i < 8; i++)
    {
      ck_assert(qspair_p(machine, cells[i]));
      ck_assert(! is_marked(cells[i]));
    }

  gcmark(cells[3]);
  ck_assert(is_marked(cells[3]));

  ck_assert(!is_marked(cells[1]));


  /* test chain-marking of a list. */
  init ();

  cells[7] = qspair_make(machine, QSINT(17), QSNIL);
  cells[6] = qspair_make(machine, QSINT(16), cells[7]);
  cells[5] = qspair_make(machine, QSINT(15), cells[6]);
  cells[4] = qspair_make(machine, QSINT(14), cells[5]);
  cells[3] = qspair_make(machine, QSINT(13), cells[4]);
  cells[2] = qspair_make(machine, QSINT(12), cells[3]);
  cells[1] = qspair_make(machine, QSINT(11), cells[2]);
  cells[0] = qspair_make(machine, QSINT(10), cells[1]);

  qsword addr;
  qsword mgmt;
  gcmark(cells[3]);
  ck_assert(is_marked(cells[3]));
  ck_assert(is_marked(cells[4]));
  ck_assert(is_marked(cells[5]));
  ck_assert(is_marked(cells[6]));
  ck_assert(is_marked(cells[7]));
  ck_assert(! is_marked(cells[8]));

  ck_assert(! is_marked(cells[0]));
  ck_assert(! is_marked(cells[1]));
  ck_assert(! is_marked(cells[2]));

  /* test chain-marking pairs from vector. */
  init ();
  cells[7] = qspair_make(machine, QSINT(17), QSNIL);
  cells[6] = qspair_make(machine, QSINT(16), QSNIL);
  cells[5] = qspair_make(machine, QSINT(15), QSNIL);
  cells[4] = qspair_make(machine, QSINT(14), QSNIL);
  cells[3] = qspair_make(machine, QSINT(13), QSNIL);
  cells[2] = qspair_make(machine, QSINT(12), QSNIL);
  cells[1] = qspair_make(machine, QSINT(11), QSNIL);
  cells[0] = qspair_make(machine, QSINT(10), QSNIL);
  qsptr v0 = qsvector_make(machine, 4, QSNIL);
  qsvector_setq(machine, v0, 0, cells[1]);
  qsvector_setq(machine, v0, 1, cells[3]);
  qsvector_setq(machine, v0, 2, cells[5]);
  qsvector_setq(machine, v0, 3, cells[7]);

  ck_assert(! is_marked(cells[0]));
  ck_assert(! is_marked(cells[1]));
  ck_assert(! is_marked(cells[2]));
  ck_assert(! is_marked(cells[3]));
  ck_assert(! is_marked(cells[4]));
  ck_assert(! is_marked(cells[5]));
  ck_assert(! is_marked(cells[6]));
  ck_assert(! is_marked(cells[7]));
  gcmark(v0);
  ck_assert(! is_marked(cells[0]));
  ck_assert(is_marked(cells[1]));
  ck_assert(! is_marked(cells[2]));
  ck_assert(is_marked(cells[3]));
  ck_assert(! is_marked(cells[4]));
  ck_assert(is_marked(cells[5]));
  ck_assert(! is_marked(cells[6]));
  ck_assert(is_marked(cells[7]));
}
END_TEST

START_TEST(test_sweep1)
{
  init();

  /* test sweeping disparate pairs. */
  qsptr cells[16];

  cells[7] = qspair_make(machine, QSINT(17), QSNIL);
  cells[6] = qspair_make(machine, QSINT(16), QSNIL);
  cells[5] = qspair_make(machine, QSINT(15), QSNIL);
  cells[4] = qspair_make(machine, QSINT(14), QSNIL);
  cells[3] = qspair_make(machine, QSINT(13), QSNIL);
  cells[2] = qspair_make(machine, QSINT(12), QSNIL);
  cells[1] = qspair_make(machine, QSINT(11), QSNIL);
  cells[0] = qspair_make(machine, QSINT(10), QSNIL);

  ck_assert_int_eq(qspair_ref_head(machine, cells[3]), QSINT(13));

  gcmark(cells[3]);
  gcmark(cells[5]);

  gcsweep();

  /*  check freeing */
  ck_assert(! is_used(cells[0]));
  ck_assert(! is_used(cells[1]));
  ck_assert(! is_used(cells[2]));
  ck_assert(is_used(cells[3]));
  ck_assert(! is_used(cells[4]));
  ck_assert(is_used(cells[5]));
  ck_assert(! is_used(cells[6]));
  ck_assert(! is_used(cells[7]));
  ck_assert(! is_used(cells[8]));

  /*  check content validity. */
  ck_assert_int_eq(qspair_ref_head(machine, cells[3]), QSINT(13));
  ck_assert_int_eq(qspair_ref_tail(machine, cells[3]), QSNIL);
  ck_assert_int_eq(qspair_ref_head(machine, cells[5]), QSINT(15));
  ck_assert_int_eq(qspair_ref_tail(machine, cells[5]), QSNIL);


  /* test sweeping list. */
  init ();

  cells[7] = qspair_make(machine, QSINT(17), QSNIL);
  cells[6] = qspair_make(machine, QSINT(16), cells[7]);
  cells[5] = qspair_make(machine, QSINT(15), cells[6]);
  cells[4] = qspair_make(machine, QSINT(14), cells[5]);
  cells[3] = qspair_make(machine, QSINT(13), cells[4]);
  cells[2] = qspair_make(machine, QSINT(12), cells[3]);
  cells[1] = qspair_make(machine, QSINT(11), cells[2]);
  cells[0] = qspair_make(machine, QSINT(10), cells[1]);

  ck_assert_int_eq( qspair_ref_tail(machine,cells[3]), cells[4]);

  qsword addr;
  qsword mgmt;
  gcmark(cells[3]);
  gcsweep();

  ck_assert(is_used(cells[3]));
  ck_assert(is_used(cells[4]));
  ck_assert(is_used(cells[5]));
  ck_assert(is_used(cells[6]));
  ck_assert(is_used(cells[7]));
  ck_assert(! is_used(cells[8]));

  ck_assert(! is_used(cells[0]));
  ck_assert(! is_used(cells[1]));
  ck_assert(! is_used(cells[2]));

  /* check data integrity. */
  ck_assert_int_eq( qspair_ref_head(machine,cells[3]), QSINT(13) );
  ck_assert_int_eq( qspair_ref_tail(machine,cells[3]), cells[4]);
  ck_assert_int_eq( qspair_ref_head(machine,cells[4]), QSINT(14) );
  ck_assert_int_eq( qspair_ref_tail(machine,cells[4]), cells[5]);
  ck_assert_int_eq( qspair_ref_head(machine,cells[5]), QSINT(15) );
  ck_assert_int_eq( qspair_ref_tail(machine,cells[5]), cells[6]);
  ck_assert_int_eq( qspair_ref_head(machine,cells[6]), QSINT(16) );
  ck_assert_int_eq( qspair_ref_tail(machine,cells[6]), cells[7]);
  ck_assert_int_eq( qspair_ref_head(machine,cells[7]), QSINT(17) );
  ck_assert_int_eq( qspair_ref_tail(machine,cells[7]), QSNIL);
}
END_TEST


TESTCASE(gc1,
  TFUNC(test_mark1)
  TFUNC(test_sweep1)
  )

TESTSUITE(suite1,
  TCASE(gc1)
  )

int main ()
{
  RUNSUITE(suite1);
}


