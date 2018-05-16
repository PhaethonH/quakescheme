#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test  bytevector */

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

  qsptr_t bv = qsbytevec_make(heap1, 9, 0);
  ck_assert(qsbytevec_p(heap1, bv));
  ck_assert_int_eq(qsbytevec_length(heap1, bv), 9);
  ck_assert_int_eq(qsfreelist_get_span(heap1, 0), 19998);

  qsbytevec_setq(heap1, bv, 0, 'h');
  qsword ch = qsbytevec_ref(heap1, bv, 0);
  ck_assert_int_eq(ch, 'h');

  qsbytevec_setq(heap1, bv, 1, 'e');
  qsbytevec_setq(heap1, bv, 2, 'l');
  qsbytevec_setq(heap1, bv, 3, 'l');
  qsbytevec_setq(heap1, bv, 4, 'o');
  qsbytevec_setq(heap1, bv, 5, 0);
  ck_assert_str_eq(qsbytevec_cptr(heap1, bv, NULL), "hello");

  int a = MGMT_GET_ALLOCSCALE(qsobj(heap1, bv, NULL)->mgmt);
  ck_assert_int_eq(a, 1);
}
END_TEST

START_TEST(test_test2)
{
  init();
}
END_TEST


TESTCASE(bytevec1,
  TFUNC(test_alloc1)
  TFUNC(test_test2)
  )

TESTSUITE(suite1,
  TCASE(bytevec1)
  )

int main ()
{
  RUNSUITE(suite1);
}

