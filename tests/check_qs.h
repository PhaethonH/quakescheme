#ifndef CHECK_QS_
#define CHECK_QS_

#include <check.h>
#include <stdio.h>
#include <stdlib.h>

/* Macro shortcuts specific to testing quakescheme */

// testlist is list of TESTFUNC without separators of any kind (they're inserted by the expansion) (also to fit macro syntax constraints)
/* e.g.
TESTCASE(greeting,
  TFUNC(test_hello)
  TFUNC(test_hi)
  TFUNC(test_hullo)
  )
*/
#define TESTCASE(name, testlist) TCase * tcase_##name() { \
  TCase * __tcase = tcase_create(#name); \
  testlist; \
  return __tcase; \
}
#define TFUNC(funcname) tcase_add_test(__tcase, funcname);


/* e.g.
TESTSUITE(helloworld,
  TCASE(greeting)
  TCASE(moregreets)
  )
*/
#define TESTSUITE(name, caselist) Suite * suite_##name() { \
  Suite * __ste = suite_create(#name); \
  caselist; \
  return __ste; \
}
#define TCASE(factory) suite_add_tcase(__ste, tcase_##factory());


/* e.g.
int main () { RUNSUITE(helloworld); }

RUNSUITE(helloworld,
  MORESUITE(foobar)
  MORESUITE(quux)
  )
*/
#define RUNSUITE(first_suite, suitelist...) \
  int failcount = 0; \
  Suite * s = suite_##first_suite(); \
  SRunner * r = srunner_create(s); \
  suitelist ; \
  srunner_run_all(r, CK_VERBOSE); \
  failcount += srunner_ntests_failed(r); \
  srunner_free(r); \
  return failcount==0 ? EXIT_SUCCESS : EXIT_FAILURE;
#define MORESUITE(factory) srunner_add_suite(r, suite_##factory());



/* Set up a test Scheme machine, containing:
  + pointer to heap instance named 'heap1'
  + pointer to machine instance named 'scheme1' (using heap1)
  + array of char named 'buf' for handling print representation.
  + test machine initialization function named 'SCRATCH_INIT'.
*/
#define SCRATCH_MACHINE(n) uint8_t _heap1[32 + n * sizeof(qsheaprec_t)]; \
 qsheap_t * heap1 = (qsheap_t*)&_heap1; \
 qs_t _scheme1, *scheme1 = &_scheme1; \
 char buf[1024]; \
 void SCRATCH_INIT() { qsheap_init(heap1, n); qs_init(scheme1, heap1); }



#define SCM_TYPEREPR_PTR_2_BUF(scm,typrepr,ptr,buf) qslist_to_cstr(scm, typrepr(scm, ptr), buf, sizeof(buf));
//#define SCMPTRREPR2BUF(scm,ptr,buf) SCM_TYPE_REPR_2_BUF(scm, qsptr, ptr, buf)
#define SCMPTRREPR2BUF(scm,ptr,buf) SCM_TYPEREPR_PTR_2_BUF(scm, qsptr_repr, ptr, buf)
#define PTRREPR2BUF(ptr,buf) SCMPTRREPR2BUF(scheme1, ptr, buf)
#define PTR_REPR(ptr) PTRREPR2BUF(ptr, buf)



#endif // CHECK_QS_
