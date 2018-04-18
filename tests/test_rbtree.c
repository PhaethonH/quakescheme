#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test  red-black tree */

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


START_TEST(test_rotleft1)
{
  init();

  qsptr_t cells[8];

  cells[0] = qstree_make(heap1, QSNIL, QSINT(13), QSNIL);
  cells[1] = qstree_make(heap1, QSNIL, QSINT(28), QSNIL);
  cells[2] = qstree_make(heap1, cells[0], QSINT(86), cells[1]);
/*
starts:
     2
   0   1

ends:
     1
  2
 0

*/

  qsptr_t t = qsrbtree_rotate_left(heap1, cells[2]);
  ck_assert_int_eq(t, cells[1]);
  ck_assert_int_eq(qstree_ref_left(heap1, t), cells[2]);
  ck_assert_int_eq(qstree_ref_left(heap1, cells[2]), cells[0]);
}
END_TEST

START_TEST(test_rotright2)
{
  init();

  qsptr_t cells[8];

  cells[0] = qstree_make(heap1, QSNIL, QSINT(13), QSNIL);
  cells[1] = qstree_make(heap1, QSNIL, QSINT(28), QSNIL);
  cells[2] = qstree_make(heap1, cells[0], QSINT(86), cells[1]);
/*
starts:
     2
   0   1

ends:
     0
        2
         1

*/

  qsptr_t t = qsrbtree_rotate_right(heap1, cells[2]);
  ck_assert_int_eq(t, cells[0]);
  ck_assert_int_eq(qstree_ref_right(heap1, t), cells[2]);
  ck_assert_int_eq(qstree_ref_right(heap1, cells[2]), cells[1]);
}
END_TEST

START_TEST(test_assoc1)
{
  init();

  qsptr_t keys[8];
  keys[0] = qsstr_inject(heap1, "alpha", 0);
  keys[1] = qsstr_inject(heap1, "bravo", 0);
  keys[2] = qsstr_inject(heap1, "charlie", 0);

  qsptr_t apairs[8];
  apairs[0] = qspair_make(heap1, keys[0], QSINT(1));
  apairs[1] = qspair_make(heap1, keys[1], QSINT(2));
  apairs[2] = qspair_make(heap1, keys[2], QSINT(3));

  qsptr_t cells[8];
  cells[0] = qstree_make(heap1, QSNIL, apairs[0], QSNIL);
  cells[1] = qstree_make(heap1, QSNIL, apairs[2], QSNIL);
  cells[2] = qstree_make(heap1, cells[0], apairs[1], cells[1]);

  qsptr_t treeroot = cells[2];
/*
dict:
     bravo
  alpha  charlie
*/

  qsptr_t x = qstree_assoc(heap1, treeroot, keys[0]);
  ck_assert(!ISNIL(x));
  ck_assert_int_eq(x, apairs[0]);

  x = qstree_assoc(heap1, treeroot, keys[2]);
  ck_assert(!ISNIL(x));
  ck_assert_int_eq(x, apairs[2]);

  x = qstree_assoc(heap1, treeroot, QSNIL);
  ck_assert(ISNIL(x));

  qsptr_t almost = QSNIL;
  x = qstree_find(heap1, treeroot, QSNIL, &almost);
  ck_assert(ISNIL(x));
  ck_assert(!ISNIL(almost));
}
END_TEST

START_TEST(test_split1)
{
  init();

  qsptr_t keys[8];
  keys[0] = qsstr_inject(heap1, "alpha", 0);
  keys[1] = qsstr_inject(heap1, "bravo", 0);
  keys[2] = qsstr_inject(heap1, "charlie", 0);

  qsptr_t apairs[8];
  apairs[0] = qspair_make(heap1, keys[0], QSINT(1));
  apairs[1] = qspair_make(heap1, keys[1], QSINT(2));
  apairs[2] = qspair_make(heap1, keys[2], QSINT(3));

  qsptr_t cells[8];
  cells[0] = qstree_make(heap1, QSNIL, apairs[0], QSNIL);
  cells[1] = qstree_make(heap1, QSNIL, apairs[2], QSNIL);
  cells[2] = qstree_make(heap1, cells[0], apairs[1], cells[1]);

  qsptr_t rbtree = qsrbtree_make(heap1, cells[2]);

  rbtree = qsrbtree_split_left(heap1, rbtree);
  ck_assert_int_ne(QSNIL, rbtree);
  ck_assert_int_eq(QSNIL, qstree_ref_left(heap1, cells[2]));
}
END_TEST

START_TEST(test_build1)
{
  init();

  qsptr_t keys[8];
  keys[0] = qsstr_inject(heap1, "alpha", 0);
  keys[1] = qsstr_inject(heap1, "bravo", 0);
  keys[2] = qsstr_inject(heap1, "charlie", 0);

  qsptr_t apairs[8];
  apairs[0] = qspair_make(heap1, keys[0], QSINT(1));
  apairs[1] = qspair_make(heap1, keys[1], QSINT(2));
  apairs[2] = qspair_make(heap1, keys[2], QSINT(3));

//  qsptr_t treenode = qstree_make(heap1, QSNIL, apairs[0], QSNIL);

  qsptr_t treeroot = QSNIL;
  // insert "alpha" into (empty) tree.
  treeroot = qsrbtree_insert(heap1, treeroot, apairs[0]);
  ck_assert(!ISNIL(treeroot));
  ck_assert(!ISNIL(qstree_ref_data(heap1, treeroot)));
  // raw tree
  qsptr_t t0 = qstree_ref_data(heap1, treeroot);
  ck_assert_int_ne(QSNIL, t0);
  // data node (apair)
  qsptr_t d = qstree_ref_data(heap1, t0);
  ck_assert_int_eq(d, apairs[0]);
  // "alpha" should resolve.
  qsptr_t x = QSNIL;
  x = qsrbtree_assoc(heap1, treeroot, keys[0]);
  ck_assert_int_ne(QSNIL, x);

  // insert "bravo" into tree.
  treeroot = qsrbtree_insert(heap1, treeroot, apairs[1]);
  ck_assert(!ISNIL(treeroot));
  ck_assert(!ISNIL(qstree_ref_data(heap1, treeroot)));
  ck_assert_int_eq(qsobj_ref_parent(heap1, treeroot), 0);
  ck_assert(!ISNIL(qstree_ref_right(heap1, t0)));

  // "alpha" and "bravo" should still resolve.
  t0 = qstree_ref_data(heap1, treeroot);
  qsptr_t q0 = qstree_assoc(heap1, t0, keys[0]);
  ck_assert_int_ne(QSNIL, q0);
  qsptr_t q1 = qstree_assoc(heap1, t0, keys[1]);
  ck_assert_int_ne(QSNIL, q1);
}
END_TEST


TESTCASE(rbtree1,
  TFUNC(test_rotleft1)
  TFUNC(test_rotright2)
  TFUNC(test_assoc1)
  //TFUNC(test_build1)
  TFUNC(test_split1)
  )

TESTSUITE(suite1,
  TCASE(rbtree1)
  )

int main ()
{
  RUNSUITE(suite1);
}

