#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test  wide number */

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


START_TEST(test_alloc1)
{
  init();

  qsptr_t l1 = qslong_make(heap1, 42);
  ck_assert_int_eq(qslong_get(heap1, l1), 42);
  ck_assert_int_eq(qsfreelist_get_span(heap1, 0), 19999);
}
END_TEST

START_TEST(test_sweep1)
{
  init();

  qsptr_t l1 = qslong_make(heap1, 42);
  ck_assert(qslong_p(heap1, l1));
  ck_assert_int_eq(qsfreelist_get_span(heap1, 0), 19999);
  qsstore_sweep(heap1);
  ck_assert_int_eq(qsfreelist_get_span(heap1, 0), 20000);
}
END_TEST


TESTCASE(widenum1,
  TFUNC(test_alloc1)
  TFUNC(test_sweep1)
  )

TESTSUITE(suite1,
  TCASE(widenum1)
  )

int main ()
{
  RUNSUITE(suite1);
}

