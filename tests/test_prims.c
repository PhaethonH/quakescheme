#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsstore.h"
#include "qsobj.h"
#include "qsmach.h"
#include "qsprimlib.h"
#include "qssexpr.h"

/* Unit test  primitives */

#define SPACELEN 20000

uint8_t _heap1[sizeof(qsstore_t) + SPACELEN*sizeof(qsobj_t)];
qsstore_t *heap1 = (qsstore_t*)&_heap1;

qs_t _scheme1, *scheme1 = &_scheme1;

char buf[131072];


void init ()
{
  puts("init on");
  qsstore_init(heap1, SPACELEN);
  qs_init(scheme1, heap1);
  puts("init done");

  //heap_dump(heap1, 0);
}


START_TEST(test_prims1)
{
  init();

  ck_assert_int_gt(scheme1->prims.len, 0);
}
END_TEST

int lim_run (int max)
{
  int i = 0;
  while (!scheme1->halt && (i < max))
    {
      qs_step(scheme1);
      i++;
    }
  printf("ran %d\n", i);
  return i;
}

START_TEST(test_double1)
{
  init();

  const char * sxstr = NULL;
  qsptr_t sx = QSNIL;

  sxstr = "(&:nd 1.2)";
  sx = qssexpr_parse_cstr(heap1, 1, sxstr, NULL);
  qs_inject_exp(scheme1, sx);
  lim_run(1000);
  ck_assert(qsdouble_p(heap1, scheme1->A));
  qsptr_crepr(heap1, scheme1->A, buf, sizeof(buf));
  ck_assert_str_eq(buf, "1.2");
}
END_TEST


TESTCASE(prim1,
  TFUNC(test_prims1)
  )

TESTCASE(double1,
  TFUNC(test_double1)
)

TESTSUITE(suite1,
  TCASE(double1)
  TCASE(prim1)
  )

int main ()
{
  RUNSUITE(suite1);
}

