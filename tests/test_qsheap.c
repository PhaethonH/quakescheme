#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test: heap memory */

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


START_TEST(test_address_pedantry)
{
  init();

//  qsptr_t obj1 = qsheap_allocscale(heap1, 3);
  qsfreelist_t * a = qsfreelist_ref(heap1, 0);
  qsfreelist_t * b = NULL;
  qsheapaddr_t a0, a1;

  qsword oldspan = CINT30(a->span);
  ck_assert_int_eq(oldspan, 20000);
  qserror_t err = qsfreelist_split(heap1, 0, 1, &a0, &a1);
  a = qsfreelist_ref(heap1, a0);
  b = qsfreelist_ref(heap1, a1);
  ck_assert(a);
  ck_assert(b);
  ck_assert(b > a);
  ck_assert_int_eq(a1, 19999);
  ck_assert_int_eq((qsobj_t*)b - heap1->space, 19999);
  ck_assert(CINT30(b->span) == 1);
  ck_assert_int_lt(CINT30(a->span), oldspan);

  oldspan = CINT30(a->span);
  err = qsfreelist_split(heap1, 0, 4, &a0, &a1);
  a = qsfreelist_ref(heap1, a0);
  b = qsfreelist_ref(heap1, a1);
  ck_assert(a);
  ck_assert(b);
  ck_assert(b > a);
  ck_assert_int_eq(a1, 19995);
  ck_assert_int_eq((qsobj_t*)b - heap1->space, 19995);
  ck_assert(CINT30(b->span) == 4);
  ck_assert_int_lt(CINT30(a->span), oldspan);
}
END_TEST

START_TEST(test_alloc_pedantry)
{
  init();

  qsheapaddr_t held = 0;
  qserror_t err = 0;

  err = qsheap_allocscale(heap1, 4, &held);
  ck_assert_int_eq(held, 19984 /*SPACELEN-16*/);

  err = qsheap_allocscale(heap1, 2, &held);
  ck_assert_int_eq(held, 19980 /*SPACELEN-16-4*/);

  err = qsheap_allocscale(heap1, 30, &held);
  ck_assert_int_eq(err, QSERROR_NOMEM);
}
END_TEST

START_TEST(test_sweeping)
{
  init();

  qsheapaddr_t held[16] = { 0, };
  qserror_t err = 0;

  /* allocate a bunch of 1-cell objects. */
  int i;
  for (i = 0; i < 16; i++)
    {
      err = qsheap_allocscale(heap1, 0, &(held[i]));
      ck_assert_int_eq(err, QSERROR_OK);
      if (i > 0)
	{
	  ck_assert_int_lt(held[i], held[i-1]);
	}
    }

  for (i = 0; i < 16; i++)
    {
      qsobj_t * probe = qsheap_ref(heap1, held[i]);
      ck_assert(probe);
      ck_assert(MGMT_IS_USED(probe->mgmt));
      ck_assert_int_eq(MGMT_GET_ALLOCSCALE(probe->mgmt), 0);
    }

  qsfreelist_t * segment = qsfreelist_ref(heap1, heap1->end_freelist);
  ck_assert_int_eq(CINT30(segment->span), 19984);

  /* mark pattern: MMMMMMMMMMMMMMMM */
  int * marking;
  int marking1[16] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
  marking = marking1;
  for (i = 0; i < 16; i++)
    {
      if (marking[i])
	{
	  qsobj_t * probe = qsheap_ref(heap1, held[i]);
	  qsobj_set_marked(probe);
	}
    }

  /* check used=1 and marked=1 for each 'held' tagged. */
  for (i = 0; i < 16; i++)
    {
      qsobj_t * obj = qsheap_ref(heap1, held[i]);
      ck_assert(obj);
      ck_assert(MGMT_IS_USED(obj->mgmt));
      ck_assert(MGMT_IS_MARKED(obj->mgmt));
    }

  qsheap_sweep(heap1);

  /* used=1 and marked=0 for each 'held' tagged for marked. */
  for (i = 0; i < 16; i++)
    {
      qsobj_t * obj = qsheap_ref(heap1, held[i]);
      ck_assert(obj);
      if (marking[i]) ck_assert(MGMT_IS_USED(obj->mgmt));
      else ck_assert(! MGMT_IS_USED(obj->mgmt));
      ck_assert(! MGMT_IS_MARKED(obj->mgmt));
    }


  /* mark pattern: MMMMMMMMMMMMoooo */
  int marking2[16] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 };
  marking = marking2;
  for (i = 0; i < 16; i++)
    {
      if (marking[i])
	{
	  qsobj_t * probe = qsheap_ref(heap1, held[i]);
	  qsobj_set_marked(probe);
	}
    }

  qsheap_sweep(heap1);

  /* check used=1 for each 'held' tagged for marked; marked=0 for all. */
  for (i = 0; i < 16; i++)
    {
      qsobj_t * obj = qsheap_ref(heap1, held[i]);
      ck_assert(obj);
      if (marking[i]) ck_assert(MGMT_IS_USED(obj->mgmt));
      else ck_assert(! MGMT_IS_USED(obj->mgmt));
      ck_assert(! MGMT_IS_MARKED(obj->mgmt));
    }

  /* check coalescence. */
  ck_assert_int_eq(qsfreelist_get_span(heap1, 0), 19988);
  ck_assert_int_eq(qsfreelist_get_next(heap1, 0), QSFREE_SENTINEL);


  /* mark pattern: ooooMMMMMMMMoooo */
  int marking3[16] = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 };
  marking = marking3;
  for (i = 0; i < 16; i++)
    {
      if (marking[i])
	{
	  qsobj_t * probe = qsheap_ref(heap1, held[i]);
	  qsobj_set_marked(probe);
	  printf("-- mark %d\n", held[i]);
	}
    }
  /* checked markedness */
  for (i = 0; i < 16; i++)
    {
      if (marking[i])
	{
	  qsobj_t * obj = qsheap_ref(heap1, held[i]);
	  ck_assert(obj);
	  ck_assert(MGMT_IS_MARKED(obj->mgmt));
	}
    }

  qsheap_sweep(heap1);

  for (i = 0; i < 16; i++)
    {
      qsfreelist_crepr(heap1, held[i], buf, sizeof(buf));
      puts(buf);
    }
  qsfreelist_crepr(heap1, 0, buf, sizeof(buf)); puts(buf);

  /* check coalescence. */
  ck_assert_int_eq(qsfreelist_get_span(heap1, 0), 19988);
  ck_assert_int_eq(qsfreelist_get_next(heap1, 0), 19996);


  /* mark pattern: ooooMMMooMMMoooo */
  int marking4[16] = { 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0 };
  marking = marking4;
  for (i = 0; i < 16; i++)
    {
      if (marking[i])
	{
	  qsobj_t * probe = qsheap_ref(heap1, held[i]);
	  qsobj_set_marked(probe);
	  printf("-- mark %d\n", held[i]);
	}
    }
  /* checked markedness */
  for (i = 0; i < 16; i++)
    {
      if (marking[i])
	{
	  qsobj_t * obj = qsheap_ref(heap1, held[i]);
	  ck_assert(obj);
	  ck_assert(MGMT_IS_MARKED(obj->mgmt));
	}
    }

  qsheap_sweep(heap1);

  for (i = 0; i < 16; i++)
    {
      qsfreelist_crepr(heap1, held[i], buf, sizeof(buf));
      puts(buf);
    }
  qsfreelist_crepr(heap1, 0, buf, sizeof(buf)); puts(buf);

  /* check coalescence. */
  ck_assert_int_eq(qsfreelist_get_span(heap1, 0), 19988);
  ck_assert_int_eq(qsfreelist_get_next(heap1, 0), 19991);
  ck_assert_int_eq(qsfreelist_get_next(heap1, 19991), 19996);
  ck_assert_int_eq(heap1->end_freelist, 19996);


  /* mark pattern: ooooMMMooMMMoooo */
  int marking5[16] = { 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0 };
  marking = marking5;
  for (i = 0; i < 16; i++)
    {
      if (marking[i])
	{
	  qsobj_t * probe = qsheap_ref(heap1, held[i]);
	  qsobj_set_marked(probe);
	  printf("-- mark %d\n", held[i]);
	}
    }
  /* checked markedness */
  for (i = 0; i < 16; i++)
    {
      if (marking[i])
	{
	  qsobj_t * obj = qsheap_ref(heap1, held[i]);
	  ck_assert(obj);
	  ck_assert(MGMT_IS_MARKED(obj->mgmt));
	}
    }

  qsheap_sweep(heap1);

  for (i = 0; i < 16; i++)
    {
      qsfreelist_crepr(heap1, held[i], buf, sizeof(buf));
      puts(buf);
    }
  qsfreelist_crepr(heap1, 0, buf, sizeof(buf)); puts(buf);

  /* check coalescence. */
  ck_assert_int_eq(qsfreelist_get_span(heap1, 0), 19988);
  ck_assert_int_eq(qsfreelist_get_next(heap1, 19989), 19991);
  ck_assert_int_eq(qsfreelist_get_next(heap1, 19991), 19994);
  ck_assert_int_eq(qsfreelist_get_next(heap1, 19994), 19996);
  ck_assert_int_eq(heap1->end_freelist, 19996);
}
END_TEST



TESTCASE(qsheap1,
  TFUNC(test_address_pedantry)
  TFUNC(test_alloc_pedantry)
  TFUNC(test_sweeping)
  )

TESTSUITE(suite1,
  TCASE(qsheap1)
  )

int main ()
{
  RUNSUITE(suite1);
}

