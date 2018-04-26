#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test  closure, lambda, env */

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


START_TEST(test_env1)
{
  init();

  qsptr_t e0 = qsenv_make(heap1, QSNIL);
  ck_assert_int_ne(e0, QSNIL);

  qsptr_t strs[32] = { QSNIL, };
  strs[0] = qsstr_inject(heap1, "foo", 0);
  strs[1] = qsstr_inject(heap1, "bar", 0);
  strs[2] = qsstr_inject(heap1, "baz", 0);
  strs[3] = qsstr_inject(heap1, "lorem_ipsum", 0);

  qsptr_t syms[32] = { QSNIL, };
  int i;
  for (i = 0; i < 4; i++)
    {
      qsptr_t y = qssymbol_make(heap1, strs[i]);
      ck_assert_int_ne(y, QSNIL);
      syms[i] = y;
    }

  qsenv_setq(heap1, e0, syms[0], QSINT(10));

  qsptr_t q0 = qsenv_ref(heap1, e0, syms[0]);
  ck_assert_int_eq(q0, QSINT(10));
  q0 = qsenv_ref(heap1, e0, syms[1]);
  ck_assert_int_eq(q0, QSERROR_INVALID);


  qsptr_t e1 = qsenv_make(heap1, QSNIL);

}
END_TEST

START_TEST(test_test2)
{
  init();
}
END_TEST


TESTCASE(closure1,
  TFUNC(test_env1)
  TFUNC(test_test2)
  )

TESTSUITE(suite1,
  TCASE(closure1)
  )

int main ()
{
  RUNSUITE(suite1);
}

