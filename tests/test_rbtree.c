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

  qsptr_t t = qsrbnode_rotate_left(heap1, cells[2]);
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

  qsptr_t t = qsrbnode_rotate_right(heap1, cells[2]);
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
  keys[3] = qsstr_inject(heap1, "delta", 0);

  qsptr_t apairs[8];
  apairs[0] = qspair_make(heap1, keys[0], QSINT(1));
  apairs[1] = qspair_make(heap1, keys[1], QSINT(2));
  apairs[2] = qspair_make(heap1, keys[2], QSINT(3));
  apairs[3] = qspair_make(heap1, keys[3], QSINT(4));

  qsptr_t cells[8];
  cells[0] = qstree_make(heap1, QSNIL, apairs[0], QSNIL);
  cells[1] = qstree_make(heap1, QSNIL, apairs[2], QSNIL);
  cells[2] = qstree_make(heap1, cells[0], apairs[1], cells[1]);

  cells[3] = qstree_make(heap1, QSNIL, apairs[3], QSNIL);
  cells[1] = qstree_setq_right(heap1, cells[1], cells[3]);

  //qsptr_t rbtree = qsrbtree_make(heap1, cells[2]);
  qsptr_t rbtree = qsrbtree_make(heap1, cells[2], QSNIL);
  ck_assert(qsrbtree(heap1, rbtree));

  qsrbtree_setq_up(heap1, rbtree, QSNIL);
  qsrbtree_setq_down(heap1, rbtree, qsrbtree_ref_top(heap1, rbtree));

  rbtree = qsrbtree_split_left(heap1, rbtree);
  ck_assert_int_ne(QSNIL, rbtree);
  ck_assert_int_eq(QSNIL, qstree_ref_left(heap1, qsrbtree_ref_top(heap1, rbtree)));

  rbtree = qsrbtree_mend(heap1, rbtree);
  ck_assert_int_eq(cells[0], qstree_ref_left(heap1, qsrbtree_ref_top(heap1, rbtree)));
  ck_assert_int_eq(cells[1], qstree_ref_right(heap1, qsrbtree_ref_top(heap1, rbtree)));
  ck_assert_int_eq(cells[3], qstree_ref_right(heap1, qstree_ref_right(heap1, qsrbtree_ref_top(heap1, rbtree))));

/*
   B
 A   C
       D
*/
  qsrbtree_setq_up(heap1, rbtree, QSNIL);
  qsrbtree_setq_down(heap1, rbtree, qsrbtree_ref_top(heap1, rbtree));
  rbtree = qsrbtree_split_right(heap1, rbtree);
  rbtree = qsrbtree_split_right(heap1, rbtree);
  ck_assert_int_eq(apairs[1], qstree_ref_data(heap1, qsrbtree_ref_top(heap1, rbtree)));
  ck_assert_int_eq(apairs[2], qstree_ref_data(heap1, qsrbtree_ref_up(heap1, rbtree)));
  ck_assert_int_eq(apairs[3], qstree_ref_data(heap1, qsrbtree_ref_down(heap1, rbtree)));
  qsptr_t P = qsrbtree_ref_up(heap1, rbtree);
  qsptr_t U = qsrbtree_ref_uncle(heap1, rbtree);
  qsptr_t G = qsrbtree_ref_grandparent(heap1, rbtree);
  rbtree = qsrbtree_mend(heap1, rbtree);
  rbtree = qsrbtree_mend(heap1, rbtree);
}
END_TEST

START_TEST(test_build1)
{
  init();

  qsptr_t keys[8];
  keys[0] = qsstr_inject(heap1, "alpha", 0);
  keys[1] = qsstr_inject(heap1, "bravo", 0);
  keys[2] = qsstr_inject(heap1, "charlie", 0);
  keys[3] = qsstr_inject(heap1, "delta", 0);
  keys[4] = qsstr_inject(heap1, "echo", 0);

  qsptr_t apairs[8];
  apairs[0] = qspair_make(heap1, keys[0], QSINT(1));
  apairs[1] = qspair_make(heap1, keys[1], QSINT(2));
  apairs[2] = qspair_make(heap1, keys[2], QSINT(3));
  apairs[3] = qspair_make(heap1, keys[3], QSINT(4));
  apairs[4] = qspair_make(heap1, keys[4], QSINT(5));

//  qsptr_t treenode = qstree_make(heap1, QSNIL, apairs[0], QSNIL);

  qsptr_t treeroot = QSNIL;
  // insert "alpha" into (empty) tree.
  treeroot = qsrbtree_insert(heap1, treeroot, apairs[0]);
  ck_assert(!ISNIL(treeroot));
  ck_assert(qsrbtree(heap1, treeroot));
  ck_assert(!ISNIL(qsrbtree_ref_top(heap1, treeroot)));
  // raw tree
  qsptr_t t0 = qsrbtree_ref_top(heap1, treeroot);
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
  t0 = qsrbtree_ref_top(heap1, treeroot);
  ck_assert(!ISNIL(t0));
  ck_assert_int_eq(qsobj_ref_parent(heap1, t0), 0);
  ck_assert(ISNIL(qstree_ref_left(heap1, t0)));
  ck_assert(!ISNIL(qstree_ref_right(heap1, t0)));
  ck_assert_int_eq(qstree_ref_data(heap1, qstree_ref_right(heap1, t0)), apairs[1]);

  // "alpha" and "bravo" should still resolve.
  qsptr_t q0 = qsrbtree_assoc(heap1, treeroot, keys[0]);
  ck_assert_int_ne(QSNIL, q0);
  qsptr_t q1 = qsrbtree_assoc(heap1, treeroot, keys[1]);
  ck_assert_int_ne(QSNIL, q1);


  // insert "charlie" into tree.
/*
   B
  / \
 A   C
*/
  treeroot = qsrbtree_insert(heap1, treeroot, apairs[2]);
  ck_assert(!ISNIL(treeroot));
  t0 = qsrbtree_ref_top(heap1, treeroot);
  ck_assert_int_ne(t0, QSNIL);
  ck_assert_int_eq(qstree_ref_data(heap1, t0), apairs[1]);
  ck_assert_int_eq(qstree_ref_data(heap1, qstree_ref_left(heap1, t0)), apairs[0]);
  ck_assert_int_eq(qstree_ref_data(heap1, qstree_ref_right(heap1, t0)), apairs[2]);

  // insert "delta" into tree.
/*
   B
  / \
 A   C
      \
       D
*/
  treeroot = qsrbtree_insert(heap1, treeroot, apairs[3]);
  ck_assert(!ISNIL(treeroot));
  t0 = qsrbtree_ref_top(heap1, treeroot);
  ck_assert_int_eq(qstree_ref_data(heap1, qstree_ref_right(heap1, qstree_ref_right(heap1, t0))), apairs[3]);

  // "delta" resolves.
  qsptr_t q3 = qsrbtree_assoc(heap1, treeroot, keys[3]);
  ck_assert_int_ne(QSNIL, q3);


/*
   B
  / \
 A   D
    / \
    C E
*/
  // insert "echo" into tree.
  treeroot = qsrbtree_insert(heap1, treeroot, apairs[4]);
  ck_assert(!ISNIL(treeroot));
  t0 = qsrbtree_ref_top(heap1, treeroot);
  ck_assert(!ISNIL(t0));
  ck_assert_int_eq(qstree_ref_data(heap1, qstree_ref_right(heap1, qstree_ref_right(heap1, t0))), apairs[4]);
}
END_TEST

START_TEST(test_mass1)
{
  init();

  qsptr_t keys[32];
  keys[0] = qsstr_inject(heap1, "alpha", 0);
  keys[1] = qsstr_inject(heap1, "bravo", 0);
  keys[2] = qsstr_inject(heap1, "charlie", 0);
  keys[3] = qsstr_inject(heap1, "delta", 0);
  keys[4] = qsstr_inject(heap1, "echo", 0);
  keys[5] = qsstr_inject(heap1, "foxtrot", 0);
  keys[6] = qsstr_inject(heap1, "golf", 0);
  keys[7] = qsstr_inject(heap1, "hotel", 0);
  keys[8] = qsstr_inject(heap1, "india", 0);
  keys[9] = qsstr_inject(heap1, "juliet", 0);
  keys[10] = qsstr_inject(heap1, "kilo", 0);
  keys[11] = qsstr_inject(heap1, "lima", 0);
  keys[12] = qsstr_inject(heap1, "mike", 0);
  keys[13] = qsstr_inject(heap1, "november", 0);
  keys[14] = qsstr_inject(heap1, "october", 0);
  keys[15] = qsstr_inject(heap1, "papa", 0);
  keys[16] = qsstr_inject(heap1, "quebec", 0);
  keys[17] = qsstr_inject(heap1, "romeo", 0);
  keys[18] = qsstr_inject(heap1, "sierra", 0);
  keys[19] = qsstr_inject(heap1, "tango", 0);
  keys[20] = qsstr_inject(heap1, "uniform", 0);
  keys[21] = qsstr_inject(heap1, "victor", 0);
  keys[22] = qsstr_inject(heap1, "whiskey", 0);
  keys[23] = qsstr_inject(heap1, "xray", 0);

  qsptr_t apairs[32];
  int i;
  for (i = 0; i < 24; i++)
    {
      apairs[i] = qspair_make(heap1, keys[i], QSINT(i+1));
    }

  qsptr_t treeroot = QSNIL;
  for (i = 0; i < 24; i++)
    {
      treeroot = qsrbtree_insert(heap1, treeroot, apairs[i]);
    }

  // "alpha"
  qsptr_t q0 = qsrbtree_assoc(heap1, treeroot, keys[0]);
  ck_assert_int_ne(QSNIL, q0);
  // "bravo"
  qsptr_t q1 = qsrbtree_assoc(heap1, treeroot, keys[1]);
  ck_assert_int_ne(QSNIL, q1);
  // "uniform"
  qsptr_t q20 = qsrbtree_assoc(heap1, treeroot, keys[20]);
  ck_assert_int_ne(QSNIL, q20);
}
END_TEST

TESTCASE(rbtree1,
  TFUNC(test_rotleft1)
  TFUNC(test_rotright2)
  TFUNC(test_assoc1)
  TFUNC(test_split1)
  TFUNC(test_build1)
  TFUNC(test_mass1)
  )

TESTSUITE(suite1,
  TCASE(rbtree1)
  )

int main ()
{
  RUNSUITE(suite1);
}

