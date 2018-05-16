#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsstore.h"
#include "qsobj.h"
#include "qsmach.h"
#include "qsprimlib.h"

/* Unit test  primitives */

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


START_TEST(test_prims1)
{
  init();

  ck_assert_int_gt(scheme1->prims.len, 0);
}
END_TEST

START_TEST(test_test2)
{
  init();
}
END_TEST


TESTCASE(case1,
  TFUNC(test_prims1)
  TFUNC(test_test2)
  )

TESTSUITE(suite1,
  TCASE(case1)
  )

int main ()
{
  RUNSUITE(suite1);
}

