#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test  symbols */

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


START_TEST(test_build1)
{
  init();

  qsptr_t strs[32];
  strs[0] = qsstr_inject(heap1, "foobar", 0);
  strs[1] = qsstr_inject(heap1, "baz", 0);
  strs[2] = qsstr_inject(heap1, "quux", 0);
  strs[3] = qsstr_inject(heap1, "quuuux", 0);
  strs[4] = qsstr_inject(heap1, "quuuux", 0);

  // create symbol table.
  qsptr_t y1 = qssymbol_make(heap1, strs[0]);
  ck_assert_int_ne(y1, QSNIL);
  ck_assert(qssymbol_p(heap1, y1));

  // intern symbol "foobar".
  qsptr_t ystore = qssymstore_make(heap1);
  qssymstore_intern(heap1, ystore, y1);

  // look up that symbol.
  qsptr_t q0 = qssymstore_assoc(heap1, ystore, strs[0]);
  ck_assert(qssymbol_p(heap1, q0));
  ck_assert_int_ne(QSNIL, q0);
  ck_assert_int_eq(q0, y1);

  // insert rest of strings as symbols.
  int i;
  for (i = 1; i < 5; i++)
    {
      qsptr_t y = qssymbol_make(heap1, strs[i]);
      ck_assert_int_ne(y1, QSNIL);
      qssymstore_intern(heap1, ystore, y);
      qsptr_t q = qssymstore_assoc(heap1, ystore, strs[i]);
      ck_assert(qssymbol_p(heap1, q));
    }

  // look up symbol "quux".
  qsptr_t q2 = qssymstore_assoc(heap1, ystore, strs[2]);
  ck_assert(qssymbol_p(heap1, q2));
  ck_assert_int_ne(QSNIL, q2);
  qsptr_t s = qstree_ref_data(heap1, q2);
  ck_assert_int_eq(s, strs[2]);

  // look up symbol "baz".
  qsptr_t q1 = qssymstore_assoc(heap1, ystore, strs[1]);
  ck_assert(qssymbol_p(heap1, q1));
  ck_assert_int_ne(QSNIL, q1);
  s = qstree_ref_data(heap1, q1);
  ck_assert_int_eq(s, strs[1]);
}
END_TEST

START_TEST(test_order1)
{
  init();

  qsptr_t strs[32];
  strs[10] = qsstr_inject(heap1, "alpha", 0);
  strs[11] = qsstr_inject(heap1, "bravo", 0);
  strs[8] = qsstr_inject(heap1, "charlie", 0);
  strs[9] = qsstr_inject(heap1, "delta", 0);
  strs[6] = qsstr_inject(heap1, "echo", 0);
  strs[7] = qsstr_inject(heap1, "foxtrot", 0);
  strs[4] = qsstr_inject(heap1, "golf", 0);
  strs[5] = qsstr_inject(heap1, "hotel", 0);
  strs[2] = qsstr_inject(heap1, "india", 0);
  strs[3] = qsstr_inject(heap1, "juliet", 0);
  strs[0] = qsstr_inject(heap1, "kilo", 0);
  strs[1] = qsstr_inject(heap1, "lima", 0);

  qsptr_t ystore = qssymstore_make(heap1);

  int i;
  for (i = 0; i < 12; i++)
    {
      qsptr_t y = qssymbol_make(heap1, strs[i]);
      ck_assert_int_ne(y, QSNIL);
      qssymstore_intern(heap1, ystore, y);
      qsptr_t q = qssymstore_assoc(heap1, ystore, strs[i]);
      ck_assert(qssymbol_p(heap1, q));
    }
  qsrbtree_crepr(heap1, qssymstore_ref_tree(heap1, ystore), buf, sizeof(buf));
  ck_assert_str_eq(buf, "(rbtree alpha bravo charlie delta echo foxtrot golf hotel india juliet kilo lima)");
}
END_TEST


TESTCASE(symbols1,
  TFUNC(test_build1)
  TFUNC(test_order1)
  )

TESTSUITE(suite1,
  TCASE(symbols1)
  )

int main ()
{
  RUNSUITE(suite1);
}

