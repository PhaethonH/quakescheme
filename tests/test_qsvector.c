#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test  [pointer] vector */

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


START_TEST(test_alloc1)
{
  init();

  qsptr_t v = qsvector_make(heap1, 10, QSNIL);
  ck_assert_int_ne(v, QSNIL);
  ck_assert_int_eq(qsvector_length(heap1, v), 10);
  ck_assert_int_eq(MGMT_GET_ALLOCSCALE(qsobj(heap1, v, NULL)->mgmt), 2);

  qsptr_t v2 = qsvector_make(heap1, 2, QSNIL);
  ck_assert_int_eq(MGMT_GET_ALLOCSCALE(qsobj(heap1, v2, NULL)->mgmt), 1);

  qsptr_t v3 = qsvector_make(heap1, 6, QSNIL);
  ck_assert_int_eq(MGMT_GET_ALLOCSCALE(qsobj(heap1, v3, NULL)->mgmt), 2);
}
END_TEST

START_TEST(test_sweep1)
{
  init();

  qsptr_t v2 = qsvector_make(heap1, 2, QSNIL);  // throwaway garbage
  qsptr_t v = qsvector_make(heap1, 10, QSNIL);
  qsptr_t v3 = qsvector_make(heap1, 2, QSNIL);  // throwaway garbage
  //qsvector_mark(heap1, v);
  qsobj_kmark(heap1, v);
  ck_assert(MGMT_IS_MARKED(qsobj(heap1,v,NULL)->mgmt));
  ck_assert(qsobj_marked_p(heap1,v));

  qsheap_sweep(heap1);
  ck_assert(qsvector_p(heap1, v));
  ck_assert(!qsvector_p(heap1, v2));
  ck_assert(!qsvector_p(heap1, v3));
  ck_assert_int_eq(COBJ26(v2), 19998);
  ck_assert_int_eq(heap1->end_freelist, 19998);

  qsheap_sweep(heap1);
  ck_assert(!qsvector_p(heap1, v));
  ck_assert_int_eq(heap1->end_freelist, 0);
  ck_assert_int_eq(qsfreelist_get_span(heap1,0), 20000);
}
END_TEST

START_TEST(test_inject1)
{
  init();

  qsptr_t cvec1[5] = {
      QSINT(1),
      QSINT(2),
      QSINT(3),
      QSINT(4),
      QSINT(5),
  };

  qsptr_t v1 = qsvector_inject(heap1, 5, cvec1);

  ck_assert(ISINT30(qsvector_ref(heap1, v1, 0)));
  ck_assert(ISINT30(qsvector_ref(heap1, v1, 1)));
  ck_assert(ISINT30(qsvector_ref(heap1, v1, 2)));
  ck_assert(ISINT30(qsvector_ref(heap1, v1, 3)));
  ck_assert(ISINT30(qsvector_ref(heap1, v1, 4)));

  ck_assert_int_eq(CINT30(qsvector_ref(heap1,v1,0)), 1);
  ck_assert_int_eq(CINT30(qsvector_ref(heap1,v1,1)), 2);
  ck_assert_int_eq(CINT30(qsvector_ref(heap1,v1,2)), 3);
  ck_assert_int_eq(CINT30(qsvector_ref(heap1,v1,3)), 4);
  ck_assert_int_eq(CINT30(qsvector_ref(heap1,v1,4)), 5);

  ck_assert(ISERROR16(qsvector_ref(heap1, v1, 5)));


  qsword veclen = 0;
  qsptr_t * xvec = 0;
  xvec = qsvector_cptr(heap1, v1, &veclen);
  ck_assert_int_eq(veclen, 5);
  ck_assert(xvec != NULL);
  ck_assert_int_eq(xvec[0], QSINT(1));
}
END_TEST


TESTCASE(vector1,
  TFUNC(test_alloc1)
  TFUNC(test_sweep1)
  TFUNC(test_inject1)
  )

TESTSUITE(suite1,
  TCASE(vector1)
  )

int main ()
{
  RUNSUITE(suite1);
}

