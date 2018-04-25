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

  // create symbol table.
  qsptr_t y1 = qssymbol_make(heap1, strs[0]);
  ck_assert_int_ne(y1, QSNIL);
  ck_assert(qssymbol(heap1, y1));

  // intern symbol "foobar".
  qsptr_t ystore = qssymstore_make(heap1);
  qssymstore_intern(heap1, ystore, y1);

  // look up that symbol.
  qsptr_t q0 = qssymstore_assoc(heap1, ystore, strs[0]);
  ck_assert_int_ne(QSNIL, q0);

  // insert rest of strings as symbols.
  int i;
  for (i = 1; i < 4; i++)
    {
      qsptr_t y = qssymbol_make(heap1, strs[i]);
      ck_assert_int_ne(y1, QSNIL);
      qssymstore_intern(heap1, ystore, y);
    }

  // look up symbol "quux".
  qsptr_t q2 = qssymstore_assoc(heap1, ystore, strs[2]);
  ck_assert_int_ne(QSNIL, q2);
  qsptr_t s = qstree_ref_data(heap1, q2);
  ck_assert_int_eq(s, strs[2]);
}
END_TEST

START_TEST(test_test2)
{
  init();
}
END_TEST


TESTCASE(symbols1,
  TFUNC(test_build1)
  TFUNC(test_test2)
  )

TESTSUITE(suite1,
  TCASE(symbols1)
  )

int main ()
{
  RUNSUITE(suite1);
}

