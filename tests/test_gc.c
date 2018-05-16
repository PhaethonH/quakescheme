#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test  garbage collection */

#define SPACELEN 20000

uint8_t _heap1[sizeof(qsstore_t) + SPACELEN*sizeof(qsobj_t)];
qsstore_t *heap1 = (qsstore_t*)&_heap1;

qs_t _scheme1, *scheme1 = &_scheme1;

char buf[131072];


void init ()
{
  qsstore_init(heap1, SPACELEN);
  qs_init(scheme1, heap1);

  //heap_dump(heap1, 0);
}


START_TEST(test_mark1)
{
  init();

  // mix of tree and vector to test yielding marking.
  qsptr_t cells[16];

  cells[0] = qspair_make(heap1, QSINT(1), QSINT(11));
  cells[1] = qspair_make(heap1, QSINT(2), QSINT(22));
  cells[2] = qspair_make(heap1, QSINT(3), QSINT(33));
  cells[3] = qspair_make(heap1, QSINT(4), QSINT(44));
  cells[4] = qspair_make(heap1, QSINT(555), cells[3]);
  cells[5] = qsvector_make(heap1, 8, QSNIL);
  qsptr_t v = cells[5];
  qsvector_setq(heap1, v, 0, cells[0]);
  qsvector_setq(heap1, v, 1, cells[1]);
  qsvector_setq(heap1, v, 2, cells[2]);
  qsvector_setq(heap1, v, 3, cells[3]);
  qsvector_setq(heap1, v, 4, cells[4]);

  ck_assert_int_eq(qsvector_ref(heap1, v, 0), cells[0]);

  ck_assert(qspair_p(heap1, cells[0]));
  ck_assert(qspair_p(heap1, cells[1]));
  ck_assert(qspair_p(heap1, cells[2]));
  ck_assert(qspair_p(heap1, cells[3]));
  ck_assert(qspair_p(heap1, cells[4]));
  ck_assert(qsvector_p(heap1, v));
  ck_assert_int_eq(COBJ26(cells[0]), 19999);
  ck_assert_int_eq(COBJ26(cells[1]), 19998);
  ck_assert_int_eq(COBJ26(cells[2]), 19997);
  ck_assert_int_eq(COBJ26(cells[3]), 19996);
  ck_assert_int_eq(COBJ26(cells[4]), 19995);
  ck_assert_int_eq(COBJ26(v), 19991);

  qsobj_kmark(heap1, v);

  ck_assert(qsobj_marked_p(heap1, cells[0]));
  ck_assert(qsobj_marked_p(heap1, cells[1]));
  ck_assert(qsobj_marked_p(heap1, cells[2]));
  ck_assert(qsobj_marked_p(heap1, cells[3]));
  ck_assert(qsobj_marked_p(heap1, cells[4]));
  ck_assert(qsobj_marked_p(heap1, v));
}
END_TEST

START_TEST(test_sweep1)
{
  init();
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

