#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEBUG_QSSEGMENT

#include "check_qs.h"
#include "qsptr.h"
#include "qsstore.h"

/* Unit test  qsstore */

qsstore_t _store, *store = &_store;
char wmem[65536];


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

  store->wmem = NULL;
  free(store->wmem);
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

/* test allocation strategy, low-level calls. */
START_TEST(test_lowalloc)
{
  init();

  qssegment_t * segment = &(store->smem);

  ck_assert_int_eq(segment->freelist, 0);
  ck_assert_int_eq(((qsfreelist_t*)(segment->space))->next, QSFREE_SENTINEL);
  ck_assert_int_eq(((qsfreelist_t*)(segment->space))->prev, QSFREE_SENTINEL);

  /* allocate one boundary. */
  qsword allocsize = 1;
  qsaddr fit0 = _qssegment_fit(segment, allocsize);
  ck_assert_int_eq(fit0, 0);
  qsaddr region2 = _qssegment_split(segment, fit0, allocsize);
  ck_assert_int_ne(region2, 0);
  int res = _qssegment_unfree(segment, fit0);
  ck_assert_int_eq(res, 0);
  ck_assert_int_eq(segment->freelist, region2);

  /* allocate sixteen boundaries. */
  allocsize = 16;
  qsaddr fit1 = _qssegment_fit(segment, allocsize);
  ck_assert_int_gt(fit1, 1);
  region2 = _qssegment_split(segment, fit1, allocsize);
  ck_assert_int_gt(region2, 1);
  res = _qssegment_unfree(segment, fit1);
  ck_assert_int_eq(res, 0);
  ck_assert_int_eq(segment->freelist, region2);

  /* allocate too large. */
  allocsize = 99999;
  qsaddr fit99 = _qssegment_fit(segment, allocsize);
  ck_assert_int_eq(fit99, QSFREE_SENTINEL);
}
END_TEST

/* test allocation strategy, high-level calls. */
START_TEST(test_highalloc)
{
  init();

  size_t wmem_size = (1 << 10);
  qssegment_t * wmem = (qssegment_t*)calloc(wmem_size, 1);
  qssegment_init(wmem, 0x10000, wmem_size - 16);
  store->wmem = wmem;

  qsaddr addr = 0;
  qserr err = QSERR_OK;

  /* allocate one boundary (no-crossing). */
  err = qsstore_alloc_nbounds(store, 0, &addr);
  ck_assert_int_eq(err, QSERR_OK);
  ck_assert_int_eq(addr, 0x10000 + 0);

  /* allocate another boundary (no-crossing). */
  err = qsstore_alloc_nbounds(store, 0, &addr);
  ck_assert_int_eq(err, QSERR_OK);
  ck_assert_int_eq(addr, 0x10000 + 16);

  /* allocate sixteen boundaries (15 crossings). */
  err = qsstore_alloc_nbounds(store, 15, &addr);
  ck_assert_int_eq(err, QSERR_OK);
  ck_assert_int_eq(addr, 0x10000 + 32);

  /* allocate one more boundary (no-crossing). */
  err = qsstore_alloc_nbounds(store, 0, &addr);
  ck_assert_int_eq(err, QSERR_OK);
  ck_assert_int_eq(addr, 0x10000 + 32 + 16*16);


  /* allocate seven words. */
  err = qsstore_alloc_nwords(store, 7, &addr);
  ck_assert_int_eq(err, QSERR_OK);
  ck_assert_int_eq(addr, 0x10000 + 32 + 16*16 + 16);

  /* allocate one words (check alignment from previous). */
  err = qsstore_alloc_nwords(store, 1, &addr);
  ck_assert_int_eq(err, QSERR_OK);
  ck_assert_int_eq(addr, 0x10000 + 32 + 16*16 + 16 + 4*16);

  /* allocate 69 bytes. */
  err = qsstore_alloc_nbytes(store, 69, &addr);
  ck_assert_int_eq(err, QSERR_OK);
  ck_assert_int_eq(addr, 0x10000 + 32 + 16*16 + 16 + 4*16 + 2*16);

  /* gobble up 200 bytes. */
  err = qsstore_alloc_nbytes(store, 200, &addr);
  ck_assert_int_eq(err, QSERR_OK);
  ck_assert_int_eq(addr, 0x10000 + 32 + 16*16 + 16 + 4*16 + 2*16 + 8*16);


  /* check allocation in smem. */
  err = qsstore_alloc_nbytes(store, 700, &addr);
  ck_assert_int_eq(err, QSERR_OK);
  ck_assert_int_eq(addr, 0);
  err = qsstore_alloc_nbytes(store, 700, &addr);
  ck_assert_int_eq(err, QSERR_OK);
  ck_assert_int_eq(addr, (1 << 6) * 16);


  /* exhaustion. */
  err = qsstore_alloc_nbytes(store, 80000, &addr);
  ck_assert_int_eq(err, QSERR_NOMEM);


  store->wmem = NULL;
  free(store->wmem);
}
END_TEST


TESTCASE(case_store1,
  TFUNC(test_store1)
  TFUNC(test_store2)
  TFUNC(test_lowalloc)
  TFUNC(test_highalloc)
  )

TESTSUITE(suite_store1,
  TCASE(case_store1)
  )

int main ()
{
  RUNSUITE(suite_store1);
}

