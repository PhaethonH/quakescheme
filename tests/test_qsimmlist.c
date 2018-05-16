#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsstore.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test  immlist */

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


START_TEST(test_immlist1)
{
  init();

  qsptr_t q0 = QSNIL;

  /* test injectl (construct from argument list): (1 2) */
  qsptr_t l0 = qsimmlist_injectl(heap1, QSINT(1), QSINT(2), QSEOL);
  int len0 = qsimmlist_length(heap1, l0);
  ck_assert_int_eq(len0, 2);

  /* test longer injectl: (1 2 3) */
  qsptr_t l1 = qsimmlist_injectl(heap1, QSINT(1), QSINT(2), QSINT(3), QSEOL);
  int len1 = qsimmlist_length(heap1, l1);
  ck_assert_int_eq(len1, 3);

  /* test inject (copy from image): (11 22 33 44) */
  qsptr_t img[] = { QSINT(11), QSINT(22), QSINT(33), QSINT(44), QSEOL };
  qsptr_t l2 = qsimmlist_inject(heap1, img, 5);
  int len2 = qsimmlist_length(heap1, l2);
  ck_assert_int_eq(len2, 4);
  q0 = qsimmlist_ref(heap1, l2, 2);
  ck_assert_int_eq(q0, QSINT(33));
  q0 = qsimmlist_ref(heap1, l2, 9);
  ck_assert_int_eq(q0, QSERROR_RANGE);

  char temp[100];

  /* test inject string immlist: (#\h #\i) */
  memset(temp, sizeof(temp), 0);
  qsptr_t img2[] = { QSCHAR('h'), QSCHAR('i'), QSEOL };
  qsptr_t l3 = qsimmlist_inject(heap1, img2, 3);
  qsstr_extract(heap1, l3, temp, sizeof(temp));
  ck_assert_str_eq(temp, "hi");

  /* test iterate immlist. */
  qsptr_t it2 = qsimmlist_iter(heap1, l2, 0);
  ck_assert(qsiter_p(heap1, it2));
  qsptr_t it = it2;
  q0 = qsiter_item(heap1, it);
  ck_assert_int_eq(q0, QSINT(11));
  it = qsiter_next(heap1, it);
  q0 = qsiter_item(heap1, it);
  ck_assert_int_eq(q0, QSINT(22));
  it = qsiter_next(heap1, it);
  q0 = qsiter_item(heap1, it);
  ck_assert_int_eq(q0, QSINT(33));
  it = qsiter_next(heap1, it);
  q0 = qsiter_item(heap1, it);
  ck_assert_int_eq(q0, QSINT(44));
  it = qsiter_next(heap1, it);
  q0 = qsiter_item(heap1, it);
  ck_assert(!ISITER28(q0));

  /* test iterate nested list: (1 (101 102) 2) */
  qsptr_t img3[] = { QSINT(1), QSBOL, QSINT(101), QSINT(102), QSEOL, QSINT(2), QSEOL };
  qsptr_t l4 = qsimmlist_inject(heap1, img3, 7);
  ck_assert_int_ne(QSNIL, l4);
  qsword raw_len = qsimmlist_len(heap1, l4);
  qsword adj_len = qsimmlist_length(heap1, l4);
  qsword iter_len = qslist_length(heap1, qsimmlist_iter(heap1, l4, 0));
  ck_assert_int_eq(raw_len, 6);
  ck_assert_int_eq(adj_len, 3);
  ck_assert_int_eq(iter_len, 3);

  qsptr_t it3 = qsimmlist_iter(heap1, l4, 0);
  q0 = qsiter_item(heap1, it3);
  ck_assert(ISINT30(q0));
  it3 = qsiter_next(heap1, it3);
  q0 = qsiter_item(heap1, it3);
  ck_assert(ISITER28(q0));
  it3 = qsiter_next(heap1, it3);
  q0 = qsiter_item(heap1, it3);
  ck_assert(ISINT30(q0));
  it3 = qsiter_next(heap1, it3);
  ck_assert(!ISITER28(it3));

  qsptr_crepr(heap1, l4, buf, sizeof(buf));
  ck_assert_str_eq(buf, "(1 (101 102) 2)");
}
END_TEST

START_TEST(test_test2)
{
  init();
}
END_TEST


TESTCASE(case1,
  TFUNC(test_immlist1)
  TFUNC(test_test2)
  )

TESTSUITE(suite1,
  TCASE(case1)
  )

int main ()
{
  RUNSUITE(suite1);
}

