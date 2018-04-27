#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test  continuation */

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


START_TEST(test_kont1)
{
  init();

  qsptr_t c0 = QSNIL;
  qsptr_t e0 = QSNIL;
  qsptr_t k0 = QSNIL;
  qsptr_t o0 = QSNIL;

  qsptr_t k1 = qskont_make(heap1, QSNIL,  k0, e0, c0, o0);
  ck_assert_int_ne(k1, QSNIL);
  ck_assert(qskont_p(heap1, k1));
  ck_assert_int_eq(k0, qskont_ref_kont(heap1, k1));
  ck_assert_int_eq(e0, qskont_ref_env(heap1, k1));
  ck_assert_int_eq(c0, qskont_ref_code(heap1, k1));
  ck_assert_int_eq(o0, qskont_ref_other(heap1, k1));
}
END_TEST

START_TEST(test_test2)
{
  init();
}
END_TEST


TESTCASE(case1,
  TFUNC(test_kont1)
  TFUNC(test_test2)
  )

TESTSUITE(suite1,
  TCASE(case1)
  )

int main ()
{
  RUNSUITE(suite1);
}

