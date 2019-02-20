#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsstore.h"

/* Unit test  qsstore */

qsstore_t _store, *store = &_store;


void init ()
{
  store = qsstore_init(&_store);
}


/* test initializations. */
START_TEST(test_store1)
{
  init();

  /* initialize bare minimum store. */

  ck_assert_int_eq(store->smem.baseaddr, 0);
  ck_assert_int_eq(store->smem.cap, SMEM_SIZE);
  ck_assert_int_ge(store->smem.cap, 4096);
  ck_assert_int_eq(store->smem.space[0], 0);

  ck_assert_ptr_eq(store->wmem, NULL);
  ck_assert_ptr_eq(store->rmem, NULL);


  /* initialize with 4MB working memory. */
  store = qsstore_init(&_store);
  size_t wmem_size = 4 * (1 << 20);
  qssegment_t * wmem = (qssegment_t*)calloc(wmem_size, 1);
  qssegment_init(wmem, 0x10000, wmem_size - 16);
  store->wmem = wmem;

  ck_assert_int_eq(store->smem.baseaddr, 0);
  ck_assert_int_eq(store->smem.cap, SMEM_SIZE);
  ck_assert_int_ge(store->smem.cap, 4096);
  ck_assert_int_eq(store->smem.space[0], 0);

  ck_assert_int_eq(store->wmem->baseaddr, 0x00010000);
  ck_assert_int_eq(store->wmem->cap, wmem_size - 16);
  ck_assert_int_eq(store->wmem->space[0], 0);

  ck_assert_ptr_ne(store->wmem, NULL);
  ck_assert_ptr_eq(store->rmem, NULL);
}
END_TEST

/* test writing and reading store. */
START_TEST(test_store2)
{
  init();

  qserr err = QSERR_OK;
  qsptr val = QSINT(1024);
  err = qsstore_set_word(store, 0x0004, val);
  ck_assert_int_eq(err, QSERR_OK);
  qsword w = qsstore_get_word(store, 0x0004);
  ck_assert_int_eq(w, val);

  err = qsstore_set_word(store, 0x00010000, val);
  ck_assert_int_eq(err, QSERR_FAULT);
  w = qsstore_get_word(store, 0x00010000);
  ck_assert_int_eq(w, 0);
}
END_TEST


TESTCASE(case_store1,
  TFUNC(test_store1)
  TFUNC(test_store2)
  )

TESTSUITE(suite_store1,
  TCASE(case_store1)
  )

int main ()
{
  RUNSUITE(suite_store1);
}

