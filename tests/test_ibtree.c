#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test  indexed-binary tree */

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


START_TEST(test_build1)
{
  init();

  qsptr_t cells[32];
  qsptr_t root = qsibtree_make(heap1);

  root = qsibtree_setq(heap1, root, 0, QSINT(100));

  ck_assert(!ISNIL(root));
  ck_assert(!ISNIL(qsibtree_ref_idx0(heap1, root)));
  ck_assert(ISNIL(qsibtree_ref_ones(heap1, root)));
  ck_assert_int_eq(qsibtree_ref(heap1, root, 0), QSINT(100));

  root = qsibtree_setq(heap1, root, 1, QSINT(101));
  ck_assert(!ISNIL(root));
  ck_assert(!ISNIL(qsibtree_ref_idx0(heap1, root)));
  ck_assert(!ISNIL(qsibtree_ref_ones(heap1, root)));
  ck_assert_int_eq(qsibtree_ref(heap1, root, 1), QSINT(101));
  ck_assert_int_eq(qsibtree_ref(heap1, root, 2), QSNIL);

  root = qsibtree_setq(heap1, root, 10, QSINT(110));
  ck_assert_int_eq(qsibtree_ref(heap1, root, 10), QSINT(110));

  root = qsibtree_setq(heap1, root, 6, QSINT(106));
  ck_assert_int_eq(qsibtree_ref(heap1, root, 1), QSINT(101));
  ck_assert_int_eq(qsibtree_ref(heap1, root, 6), QSINT(106));
  ck_assert_int_eq(qsibtree_ref(heap1, root, 10), QSINT(110));
}
END_TEST

START_TEST(test_test2)
{
  init();
}
END_TEST


TESTCASE(ibtree1,
  TFUNC(test_build1)
  TFUNC(test_test2)
  )

TESTSUITE(suite1,
  TCASE(ibtree1)
  )

int main ()
{
  RUNSUITE(suite1);
}

