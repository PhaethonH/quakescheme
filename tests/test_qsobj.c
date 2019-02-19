#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsobj.h"
#include "qsstore.h"

/* Unit test: Generic object access. */

qsstore_t _store, *store=&_store;

void init ()
{
  qsstore_init(store);
}


START_TEST(test_test1)
{
  init();

  ck_assert_int_eq(sizeof(qsobj_t), 4*sizeof(qsword));
  ck_assert_int_eq(sizeof(qstriplet_t), sizeof(qsobj_t));
  ck_assert_int_eq(sizeof(qspvec_t), sizeof(qsobj_t));
  ck_assert_int_eq(sizeof(qswideword_t), sizeof(qsobj_t));
  ck_assert_int_eq(sizeof(qsovec_t), sizeof(qsobj_t));

  qsobj_t obj1;
  qsobj_init(&obj1, 0, false);

  /* generic object manipulation. */
  ck_assert( ISSYNC29(obj1.mgmt) );
  ck_assert( qsobj_is_used(&obj1) );
  ck_assert( !qsobj_is_marked(&obj1) );
  ck_assert( !qsobj_is_grey(&obj1) );
  ck_assert( !qsobj_is_octetate(&obj1) );
  ck_assert_int_eq( qsobj_get_reversal(&obj1), 0 );
  ck_assert_int_eq( qsobj_get_score(&obj1), 0 );
  ck_assert_int_eq( qsobj_get_allocscale(&obj1), 0 );

  /* set flags. */
  qsobj_set_marked(&obj1, 1);
  qsobj_set_grey(&obj1, 1);
  qsobj_set_octetate(&obj1, 1);
  qsobj_set_reversal(&obj1, 3);
  qsobj_set_score(&obj1, 8);

  ck_assert( ISSYNC29(obj1.mgmt) );
  ck_assert( qsobj_is_used(&obj1) );
  ck_assert( qsobj_is_marked(&obj1) );
  ck_assert( qsobj_is_grey(&obj1) );
  ck_assert( qsobj_is_octetate(&obj1) );
  ck_assert_int_eq( qsobj_get_reversal(&obj1), 3 );
  ck_assert_int_eq( qsobj_get_score(&obj1), 8 );
  ck_assert_int_eq( qsobj_get_allocscale(&obj1), 0 );

  qsobj_set_allocscale(&obj1, 3);
  ck_assert_int_eq( qsobj_get_allocscale(&obj1), 3 );

  qsobj_set_used(&obj1, 0);
  ck_assert( !qsobj_is_used(&obj1) );
}
END_TEST

START_TEST(test_prototypes1)
{
  init();

  /* test manipulation object prototypes. */
  void * probe0 = NULL;
  qstriplet_t _obj1, *obj1=&_obj1;
  qsword array1[32] = { 0,0,0,0, QSINT(21), QSINT(23), QSINT(25), QSINT(27), };
  qspvec_t *obj2 = (qspvec_t*)(array1);
  qswideword_t _obj3, *obj3=&_obj3;
  qsbyte array2[256] = { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
      'H','e','l','l', 'o',',',' ','w',
      'o','r','l','d', 0 };
  qsovec_t * obj4 = (qsovec_t*)(array2);

  /* Prototype 1, single-boundary word-pointers. */
  probe0 = qstriplet_init(obj1, QSINT(10), QSINT(20), QSINT(30));

  ck_assert_ptr_eq(probe0, obj1);
  ck_assert_int_eq(qstriplet_ref_first(obj1), QSINT(10));
  ck_assert_int_eq(qstriplet_ref_second(obj1), QSINT(20));
  ck_assert_int_eq(qstriplet_ref_third(obj1), QSINT(30));

  qstriplet_setq_first(obj1, QSINT(100));
  qstriplet_setq_second(obj1, QSINT(200));
  qstriplet_setq_third(obj1, QSINT(300));

  ck_assert_int_eq(qstriplet_ref_first(obj1), QSINT(100));
  ck_assert_int_eq(qstriplet_ref_second(obj1), QSINT(200));
  ck_assert_int_eq(qstriplet_ref_third(obj1), QSINT(300));

  /* Prototye 2, cross-boundary word-pointers. */
  probe0 = qspvec_init(obj2, 4, QSINT(13));

  ck_assert_ptr_eq(probe0, obj2);
  ck_assert_int_eq(qspvec_ref_length(obj2), QSINT(13));
  ck_assert_int_eq(qsobj_get_allocscale((qsobj_t*)obj2), 4);
  ck_assert_int_eq(qspvec_ref(obj2, 0), QSINT(21));

  qspvec_setq_length(obj2, QSINT(7));
  ck_assert_int_eq(qspvec_ref_length(obj2), QSINT(7));
  ck_assert_int_eq(qspvec_ref(obj2, 17), QSNIL);

  qspvec_setq(obj2, 2, QSINT(2121));
  ck_assert_int_eq(qspvec_ref(obj2, 0), QSINT(21));
  ck_assert_int_eq(qspvec_ref(obj2, 1), QSINT(23));
  ck_assert_int_eq(qspvec_ref(obj2, 2), QSINT(2121));

  /* Prototype 3, single-boundary octet. */
  probe0 = qswideword_init(obj3, QSNUM_LONG, NULL);
  uint64_t inject3 = 7611924812;
  union qswidepayload_u payload3 = { 0, };
  uint64_t probe3 = 0;

  ck_assert_ptr_eq(probe0, obj3);
  ck_assert_int_eq(qswideword_ref_subtype(obj3), QSNUM_LONG);

  qswideword_put_payload(obj3, ((union qswidepayload_u*)&inject3));
  qswideword_fetch_payload(obj3, &payload3);
  probe3 = payload3.l;
  ck_assert(inject3 == probe3);

  /* Prototype 4, cross-boundary octet. */
  probe0 = qsovec_init(obj4, 4, QSINT(12));

  ck_assert_ptr_eq(probe0, obj4);
  ck_assert_int_eq(qsovec_ref_length(obj4), QSINT(12));
  ck_assert_int_eq(qsovec_ref(obj4, 0), 'H');
  ck_assert_int_eq(qsovec_ref(obj4, 1), 'e');
  ck_assert_int_eq(qsovec_ref(obj4, 2), 'l');
  ck_assert_int_eq(qsovec_ref(obj4, 3), 'l');
  ck_assert_int_eq(qsovec_ref(obj4, 4), 'o');

  qsovec_setq_length(obj4, QSINT(8));
  ck_assert_int_eq(qsovec_ref_length(obj4), QSINT(8));
}
END_TEST


TESTCASE(case1,
  TFUNC(test_test1)
  TFUNC(test_prototypes1)
  )

TESTSUITE(suite1,
  TCASE(case1)
  )

int main ()
{
  RUNSUITE(suite1);
}

