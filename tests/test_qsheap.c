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
  ck_assert_int_eq((qsheapcell_t*)b - heap1->space, 19999);
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
  ck_assert_int_eq((qsheapcell_t*)b - heap1->space, 19995);
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


int mark_cells (int ncells, qsheapaddr_t * cells, int * marking)
{
  /* set marks. */
  int i;
  for (i = 0; i < ncells; i++)
    {
      if (marking[i])
	{
	  qsheapcell_t * probe = qsheap_ref(heap1, cells[i]);
	  if (qsheapcell_is_used(probe))
	    qsheapcell_set_marked(probe, 1);
	}
    }
  return 0;
}

int check_used (int ncells, qsheapaddr_t * cells, int * marking)
{
  /* check mark-tagged cells are Used. */
  int i;
  for (i = 0; i < ncells; i++)
    {
      if (marking[i])
	{
	  qsheapcell_t * probe = qsheap_ref(heap1, cells[i]);
	  ck_assert(probe);
	  ck_assert(qsheapcell_is_used(probe));
	}
    }
  return 0;
}

int check_marks (int ncells, qsheapaddr_t * cells, int * marking)
{
  /* check markedness. */
  int i;
  for (i = 0; i < ncells; i++)
    {
      if (marking[i])
	{
	  qsheapcell_t * probe = qsheap_ref(heap1, cells[i]);
	  ck_assert(probe);
	  ck_assert(qsheapcell_is_marked(probe));
	}
    }
  return 0;
}

int check_unswept (int ncells, qsheapaddr_t * cells, int * marking)
{
  /* check mark-tagged cells are Used. */
  int i;
  for (i = 0; i < ncells; i++)
    {
      qsheapcell_t * probe = qsheap_ref(heap1, cells[i]);
      ck_assert(probe);
      if (marking[i])
	{
	  ck_assert(qsheapcell_is_used(probe));
	}
      else
	{
	  ck_assert(!qsheapcell_is_used(probe));
	}
    }
  return 0;
}

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
      qsheapcell_t * probe = qsheap_ref(heap1, held[i]);
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
  mark_cells(16, held, marking);
  check_marks(16, held, marking);
  check_used(16, held, marking);

  qsheap_sweep(heap1);

  check_unswept(16, held, marking);


  /* mark pattern: MMMMMMMMMMMMoooo */
  int marking2[16] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 };
  marking = marking2;
  mark_cells(16, held, marking);

  qsheap_sweep(heap1);

  check_unswept(16, held, marking);

  /* check coalescence. */
  ck_assert_int_eq(qsfreelist_get_span(heap1, 0), 19988);
  ck_assert_int_eq(qsfreelist_get_next(heap1, 0), QSFREE_SENTINEL);


  /* mark pattern: ooooMMMMMMMMoooo */
  int marking3[16] = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 };
  marking = marking3;
  mark_cells(16, held, marking);
  check_marks(16, held, marking);  /* check markedness */

  qsheap_sweep(heap1);

  check_unswept(16, held, marking);

  /* check coalescence. */
  ck_assert_int_eq(qsfreelist_get_span(heap1, 0), 19988);
  ck_assert_int_eq(qsfreelist_get_next(heap1, 0), 19996);


  /* mark pattern: ooooMMMooMMMoooo */
  int marking4[16] = { 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0 };
  marking = marking4;
  mark_cells(16, held, marking);
  check_marks(16, held, marking);  /* check markedness */

  qsheap_sweep(heap1);

  check_unswept(16, held, marking);

  /* check coalescence. */
  ck_assert_int_eq(qsfreelist_get_span(heap1, 0), 19988);
  ck_assert_int_eq(qsfreelist_get_next(heap1, 0), 19991);
  ck_assert_int_eq(qsfreelist_get_next(heap1, 19991), 19996);
  ck_assert_int_eq(heap1->end_freelist, 19996);


  /* mark pattern: ooooMMMooMMMoooo */
  int marking5[16] = { 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0 };
  marking = marking5;
  mark_cells(16, held, marking);
  check_marks(16, held, marking);  /* checked markedness */

  qsheap_sweep(heap1);

  check_unswept(16, held, marking);

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

