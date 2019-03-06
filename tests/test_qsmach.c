#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsstore.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test  machine */

qsmachine_t _machine, *machine=&_machine;

char buf[65536];


void init ()
{
  qsmachine_init(machine);
}


START_TEST(test_inject1)
{
  init();

}
END_TEST

START_TEST(test_atomeval1)
{
  init();

}
END_TEST

START_TEST(test_step1)
{
  /* Atomic Evaluation as step. */
  init();

}
END_TEST

START_TEST(test_step2)
{
  init();

}
END_TEST


TESTCASE(mach1,
  TFUNC(test_inject1)
  )
TESTCASE(mach2,
  TFUNC(test_atomeval1)
  )
TESTCASE(mach3,
  TFUNC(test_step1)
  )
TESTCASE(mach4,
  TFUNC(test_step2)
  )


TESTSUITE(suite1,
  TCASE(mach1)
  TCASE(mach2)
  TCASE(mach3)
  TCASE(mach4)
  )

int main ()
{
  RUNSUITE(suite1);
}

