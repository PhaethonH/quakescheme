#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test  skeleton */

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


int alloc_trees7 (qsptr_t cells[])
{
  cells[0] = qstree_make(heap1, QSNIL, QSINT(100), QSNIL);
  ck_assert_int_ne(cells[0], QSNIL);

  cells[1] = qstree_make(heap1, QSNIL, QSINT(200), QSNIL);
  ck_assert_int_ne(cells[1], QSNIL);

  cells[2] = qstree_make(heap1, QSNIL, QSINT(300), QSNIL);
  ck_assert_int_ne(cells[2], QSNIL);

  cells[3] = qstree_make(heap1, QSNIL, QSINT(400), QSNIL);
  ck_assert_int_ne(cells[3], QSNIL);


  cells[4] = qstree_make(heap1, cells[0], QSINT(10), cells[1]);
  ck_assert_int_ne(cells[4], QSNIL);

  cells[5] = qstree_make(heap1, cells[2], QSINT(20), cells[3]);
  ck_assert_int_ne(cells[5], QSNIL);


  cells[6] = qstree_make(heap1, cells[4], QSINT(1), cells[5]);
  ck_assert_int_ne(cells[6], QSNIL);

  return 0;
}

START_TEST(test_alloc1)
{
  init();

  qsptr_t cells[16];

  alloc_trees7(cells);

  int i;
  for (i = 0 ; i < 7; i++)
    {
      ck_assert(qstree(heap1, cells[i]));
    }
}
END_TEST

START_TEST(test_sweep1)
{
  init();

  qsptr_t cells[16];
  alloc_trees7(cells);

  cells[7] = qstree_make(heap1, QSNIL, QSINT(0), QSNIL);
  ck_assert_int_ne(cells[7], QSNIL);

  //qstree_mark(heap1, cells[6]);
  qsobj_kmark(heap1, cells[6]);

  int i;
  for (i = 0; i < 7; i++)
    {
      ck_assert(MGMT_IS_MARKED(qstree(heap1, cells[i])->mgmt));
    }

  qsheap_sweep(heap1);

  // make sure objects still persist.
  for (i = 0; i < 7; i++)
    {
      ck_assert(MGMT_IS_USED(qstree(heap1, cells[i])->mgmt));
    }
  // make sure this unmarked cell got swept.
  ck_assert(! qstree(heap1, cells[7]));
}
END_TEST


TESTCASE(tree1,
  TFUNC(test_alloc1)
  TFUNC(test_sweep1)
  )

TESTSUITE(suite1,
  TCASE(tree1)
  )

int main ()
{
  RUNSUITE(suite1);
}

