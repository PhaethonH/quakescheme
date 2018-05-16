#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test  iterator */

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


START_TEST(test_iter_pairs)
{
  init();

  qsptr_t cells[16];

  cells[0] = qspair_make(heap1, QSINT(60), QSNIL);
  ck_assert_int_ne(qspair_ref_a(heap1, cells[0]), QSNIL);
  ck_assert_int_eq(qspair_ref_a(heap1, cells[0]), QSINT(60));
  cells[1] = qspair_make(heap1, QSINT(50), cells[0]);
  cells[2] = qspair_make(heap1, QSINT(40), cells[1]);
  cells[3] = qspair_make(heap1, QSINT(30), cells[2]);
  cells[4] = qspair_make(heap1, QSINT(20), cells[3]);
  cells[5] = qspair_make(heap1, QSINT(10), cells[4]);
  cells[6] = qspair_make(heap1, QSINT(0), cells[5]);

  ck_assert_int_eq(qslist_length(heap1, cells[6]), 7);

  qsptr_t root = cells[6];
  qsstoreaddr_t word_addr = COBJ26(root) << 2;
  qsptr_t it1 = qsiter_make(heap1, word_addr);
  int i;
  for (i = 0; i < 7; i++)
    {
      ck_assert_int_ne(it1, QSNIL);

      qsptr_t elt = qsiter_item(heap1, it1);
      ck_assert_int_ne(elt, QSNIL);

      it1 = qsiter_next(heap1, it1);
    }

  ck_assert_int_eq(it1, QSNIL);
}
END_TEST

START_TEST(test_iter_vector)
{
  init();

  qsptr_t v;
  v = qsvector_make(heap1, 15, QSINT(15));
  ck_assert_int_eq(qsvector_length(heap1,v), 15);
  ck_assert_int_eq(qsvector_ref(heap1,v,0), QSINT(15));

  qsword word_addr = (COBJ26(v) << 2) + 4;
  qsptr_t it1 = qsiter_make(heap1, word_addr);

  int i;
  for (i = 0; i < 15; i++)
    {
      ck_assert_int_ne(it1, QSNIL);

      qsptr_t elt = qsiter_item(heap1, it1);
      ck_assert(ISINT30(elt));

      it1 = qsiter_next(heap1, it1);
    }

  ck_assert_int_eq(it1, QSNIL);
}
END_TEST


TESTCASE(iter1,
  TFUNC(test_iter_pairs)
  TFUNC(test_iter_vector)
  )

TESTSUITE(suite1,
  TCASE(iter1)
  )

int main ()
{
  RUNSUITE(suite1);
}

