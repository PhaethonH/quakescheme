#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsstore.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test  skeleton */

qsstore_t _store, *store=&_store;


void init ()
{
  qsstore_init(store);
}


START_TEST(test_test1)
{
  init();

}
END_TEST

START_TEST(test_test2)
{
  init();
}
END_TEST


TESTCASE(case1,
  TFUNC(test_test1)
  TFUNC(test_test2)
  )

TESTSUITE(suite1,
  TCASE(case1)
  )

int main ()
{
  RUNSUITE(suite1);
}

