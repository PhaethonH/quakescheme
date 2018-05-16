#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test: pointer encoding and decoding */

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


START_TEST(test_int)
{
  int res = 0;

  init();

  qsptr_t int1 = QSINT(17);
  res = qsint_crepr(heap1, int1, buf, sizeof(buf));
  ck_assert(ISINT30(int1));
  ck_assert_int_eq(CINT30(int1), 17);
  ck_assert_str_eq(buf, "17");

  qsptr_t int2 = QSINT(42);
  ck_assert(ISINT30(int2));
  res = qsint_crepr(heap1, int2, buf, sizeof(buf));
  ck_assert_int_eq(CINT30(int2), 42);
  ck_assert_str_eq(buf, "42");

  qsptr_t int3 = QSINT(119);
  ck_assert(ISINT30(int3));
  res = qsint_crepr(heap1, int3, buf, sizeof(buf));
  ck_assert_int_eq(CINT30(int3), 119);
  ck_assert_str_eq(buf, "119");

  qsptr_t int4 = QSINT(-1023);
  ck_assert(ISINT30(int4));
  res = qsint_crepr(heap1, int4, buf, sizeof(buf));
  ck_assert_int_eq(CINT30(int4), -1023);
  ck_assert_str_eq(buf, "-1023");
}
END_TEST


START_TEST(test_float)
{
  init();

  int res = 0;

  qsptr_t flo1 = QSFLOAT(1);
  res = qsfloat_crepr(heap1, flo1, buf, sizeof(buf));
  ck_assert(ISFLOAT31(flo1));
  ck_assert_str_eq(buf, "1.000000");

  qsptr_t flo2 = QSFLOAT(2);
  res = qsfloat_crepr(heap1, flo2, buf, sizeof(buf));
  ck_assert(ISFLOAT31(flo2));
  ck_assert_str_eq(buf, "2.000000");

  qsptr_t flo3 = QSFLOAT(3);
  res = qsfloat_crepr(heap1, flo3, buf, sizeof(buf));
  ck_assert(ISFLOAT31(flo3));
  ck_assert_str_eq(buf, "3.000000");

  qsptr_t flo4 = QSFLOAT(3.141592);
  res = qsfloat_crepr(heap1, flo4, buf, sizeof(buf));
  ck_assert(ISFLOAT31(flo4));
  ck_assert_str_eq(buf, "3.141592");
}
END_TEST


START_TEST(test_char)
{
  init();

  int res = 0;

  qsptr_t chr1 = QSCHAR('a');
  res = qschar_crepr(heap1, chr1, buf, sizeof(buf));
  ck_assert(ISCHAR24(chr1));
  ck_assert_int_eq(CCHAR24(chr1), 'a');
  ck_assert_str_eq(buf, "#\\a");

  qsptr_t chr2 = QSCHAR('A');
  res = qschar_crepr(heap1, chr2, buf, sizeof(buf));
  ck_assert(ISCHAR24(chr2));
  ck_assert_int_eq(CCHAR24(chr2), 'A');
  ck_assert_str_eq(buf, "#\\A");

  qsptr_t chr3 = QSCHAR(' ');
  res = qschar_crepr(heap1, chr3, buf, sizeof(buf));
  ck_assert(ISCHAR24(chr3));
  ck_assert_int_eq(CCHAR24(chr3), ' ');
  ck_assert_str_eq(buf, "#\\space");
}
END_TEST


START_TEST(test_const)
{
  init();

  int res = 0;

  ck_assert(ISCONST16(QSNIL));
  ck_assert_int_eq(CCONST16(QSNIL), 0);

  ck_assert(ISCONST16(QSTRUE));
  ck_assert_int_eq(CCONST16(QSTRUE), 1);
}
END_TEST


START_TEST(test_error)
{
  init();

  int res = 0;

  ck_assert(ISERROR16(QSERROR_OK));
  ck_assert_int_eq(CERROR16(QSERROR_OK), 0);

  ck_assert(ISERROR16(QSERROR_NOMEM));
  ck_assert_int_eq(CERROR16(QSERROR_NOMEM), 2);

  ck_assert(ISERROR16(QSERROR_TYPE));
  ck_assert_int_eq(CERROR16(QSERROR_TYPE), 5);
}
END_TEST


START_TEST(test_iter)
{
  init();

  int res = 0;

  qsptr_t anchor = QSOBJ(8);
  ck_assert(ISOBJ26(anchor));
  qsbay0_t * heapcell = qsstore_get(heap1, 8);
  qsobj_t * obj = (qsobj_t*)heapcell;
  ck_assert(heapcell - heap1->space == 8);
  obj->mgmt |= (1 << 31);  /* used */
  obj->mgmt |= TAG_SYNC29; /* typing */
  obj->mgmt |= (2 << 4);   /* 16 cells */
  obj->_0 = QSNIL;
  obj->_1 = QSNIL;
  obj->_2 = QSNIL;
  qsptr_t * _d = (qsptr_t*)(obj+1);
  // (10 20 30 (11 22))
  _d[0] = QSINT(10);
  _d[1] = QSINT(20);
  _d[2] = QSINT(30);
   _d[3] = QSBOL;
   _d[4] = QSINT(11);
    _d[5] = QSBOL;
    _d[6] = QSINT(111);
    _d[7] = QSINT(222);
    _d[8] = QSINT(333);
    _d[9] = QSINT(444);
    _d[10] = QSEOL;
   _d[11] = QSINT(22);
   _d[12] = QSEOL;
  _d[13] = QSEOL;

  qsword word_addr = (8 * 4) + 0  +  4;
  qsptr_t iter1 = QSITER(word_addr);
  ck_assert(ISITER28(iter1));
  ck_assert_int_eq(CITER28(iter1), word_addr);

  qsptr_t ref1 = 0;
  qsstore_fetch_word(heap1, word_addr, &ref1);
  ck_assert(ISINT30(ref1));

  qsptr_t itern = iter1;
  ck_assert(ISITER28(itern));
  ref1 = qsiter_item(heap1, itern);
  ck_assert_int_eq(ref1, QSINT(10));

  itern = qsiter_next(heap1, itern);
  ck_assert(ISITER28(itern));
  ref1 = qsiter_item(heap1, itern);
  ck_assert(ISINT30(ref1));
  ck_assert_int_eq(CINT30(ref1), 20);

  itern = qsiter_next(heap1, itern);
  ref1 = qsiter_item(heap1, itern);
  ck_assert_int_eq(CINT30(ref1), 30);

  itern = qsiter_next(heap1, itern);
  ref1 = qsiter_item(heap1, itern);
  ck_assert(ISITER28(ref1));

  itern = qsiter_next(heap1, itern);
  ck_assert(ISNIL(itern));
}
END_TEST



TESTCASE(nums1,
  TFUNC(test_int)
  TFUNC(test_float)
  TFUNC(test_char)
  )

TESTCASE(sys1,
  TFUNC(test_const)
  TFUNC(test_error)
  TFUNC(test_iter)
  )

TESTSUITE(ptr1,
  TCASE(nums1)
  TCASE(sys1)
  )

int main ()
{
  RUNSUITE(ptr1);
}

