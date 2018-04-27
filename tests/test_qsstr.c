#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test  string */

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


START_TEST(test_inject1)
{
  init();

  char * cs1 = "hello, world";
  wchar_t * ws1 = L"hello, world";
  wchar_t * ws2 = L"\u51b0"; // zh: bīng (en: "ice")

  qsptr_t s1 = qsstr_inject(heap1, cs1, strlen(cs1));
  qsptr_t s2 = qsstr_inject_wchar(heap1, ws1, wcslen(ws1));
  qsptr_t s3 = qsstr_inject_wchar(heap1, ws2, wcslen(ws2));

  ck_assert_int_eq(qsstr_length(heap1, s1), 12);
  ck_assert_int_eq(qsstr_length(heap1, s2), 12);
  ck_assert_int_eq(qsstr_length(heap1, s3), 1);

  unsigned char extract1[32] = { 0, };
  qsstr_extract(heap1, s3, extract1, sizeof(extract1));
  ck_assert_int_eq(strlen(extract1), 3);
  ck_assert_int_eq(extract1[0], 0xe5);
  ck_assert_int_eq(extract1[1], 0x86);
  ck_assert_int_eq(extract1[2], 0xb0);
  ck_assert_int_eq(extract1[3], 0);
}
END_TEST

START_TEST(test_cmp1)
{
  init();

  qsptr_t s1 = qsstr_inject(heap1, "foo", 0);
  qsptr_t s2 = qsstr_inject(heap1, "quux", 0);
  qsptr_t s3 = qsstr_inject(heap1, "foo", 0);
  qsptr_t s4 = qsstr_inject(heap1, "foot", 0);

  ck_assert_int_eq(qsstr_length(heap1, s1), 3);
  ck_assert_int_eq(qsstr_length(heap1, s2), 4);
  ck_assert_int_eq(qsstr_length(heap1, s3), 3);

  int cmp = qsstr_cmp(heap1, s1, s2);
  ck_assert_int_eq(cmp, CMP_LT);

  cmp = qsstr_cmp(heap1, s1, s1);
  ck_assert_int_eq(cmp, CMP_EQ);

  cmp = qsstr_cmp(heap1, s1, s3);
  ck_assert_int_eq(cmp, CMP_EQ);

  cmp = qsstr_cmp(heap1, s1, s4);
  ck_assert_int_eq(cmp, CMP_GT);
}
END_TEST


TESTCASE(str1,
  TFUNC(test_inject1)
  TFUNC(test_cmp1)
  )

TESTSUITE(suite1,
  TCASE(str1)
  )

int main ()
{
  RUNSUITE(suite1);
}

