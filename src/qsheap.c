#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "qsheap.h"



#include <stdbool.h>

struct qsheapcell_report_s {
    qsheapaddr_t addr;
    bool used;
    bool marked;
    bool octetate;
    int allocscale;
    long allocsize;
};

struct qsheapcell_report_s qsobj_report (qsheap_t * heap, qsptr_t ptr)
{
  struct qsheapcell_report_s report;
  qsheapaddr_t addr = COBJ26(ptr);
  report.addr = addr;
  qsheapcell_t * heapcell = qsheap_ref(heap, addr);
  report.used = qsheapcell_is_used(heapcell);
  report.marked = qsheapcell_is_marked(heapcell);
  report.octetate = qsheapcell_is_octet(heapcell);
  report.allocscale = qsheapcell_get_allocscale(heapcell);
  report.allocsize = 1 << report.allocscale;
  return report;
}



qsheapcell_t * qsheapcell_init (qsheapcell_t * cell, int used, int marked, int allocscale)
{
  cell->mgmt = TAG_SYNC29;
  if (used) MGMT_SET_USED(cell->mgmt);
  if (marked) MGMT_SET_MARKED(cell->mgmt);
  MGMT_SET_ALLOCSCALE(cell->mgmt, allocscale);
  return cell;
}

int qsheapcell_is_synced (qsheapcell_t * heapcell)
{
  return ISSYNC29(heapcell->mgmt);
}

int qsheapcell_is_used (qsheapcell_t * heapcell)
{
  return !!(MGMT_IS_USED(heapcell->mgmt));
}

int qsheapcell_is_marked (qsheapcell_t * heapcell)
{
  return !!(MGMT_IS_MARKED(heapcell->mgmt));
}

int qsheapcell_is_octet (qsheapcell_t * heapcell)
{
  return !!(MGMT_IS_OCTET(heapcell->mgmt));
}

int qsheapcell_get_score (qsheapcell_t * heapcell)
{
  return MGMT_GET_SCORE(heapcell->mgmt);
}

int qsheapcell_get_parent (qsheapcell_t * heapcell)
{
  return MGMT_GET_PARENT(heapcell->mgmt);
}

int qsheapcell_get_allocscale (qsheapcell_t * heapcell)
{
  return MGMT_GET_ALLOCSCALE(heapcell->mgmt);
}

qsptr_t qsheapcell_get_field (qsheapcell_t * heapcell, int ofs)
{
  // TODO: bounds check.
  return heapcell->fields[ofs];
}

qsheapcell_t * qsheapcell_set_field (qsheapcell_t * heapcell, int ofs, qsptr_t val)
{
  // TODO: bounds check.
  heapcell->fields[ofs] = val;
  return heapcell;
}

qsheapcell_t * qsheapcell_set_used (qsheapcell_t * heapcell, int val)
{
  if (val) MGMT_SET_USED(heapcell->mgmt);
  else MGMT_CLR_USED(heapcell->mgmt);
  return heapcell;
}

qsheapcell_t * qsheapcell_set_marked (qsheapcell_t * heapcell, int val)
{
  if (val) MGMT_SET_MARKED(heapcell->mgmt);
  else MGMT_CLR_MARKED(heapcell->mgmt);
  return heapcell;
}

qsheapcell_t * qsheapcell_set_octet (qsheapcell_t * heapcell, int val)
{
  if (val) MGMT_SET_OCTET(heapcell->mgmt);
  else MGMT_CLR_OCTET(heapcell->mgmt);
  return heapcell;
}

qsheapcell_t * qsheapcell_set_score (qsheapcell_t * heapcell, int val)
{
  MGMT_SET_SCORE(heapcell->mgmt, val);
  return heapcell;
}

qsheapcell_t * qsheapcell_set_parent (qsheapcell_t * heapcell, int val)
{
  MGMT_SET_PARENT(heapcell->mgmt, val);
  return heapcell;
}

qsheapcell_t * qsheapcell_set_allocscale (qsheapcell_t * heapcell, int val)
{
  MGMT_SET_ALLOCSCALE(heapcell->mgmt, val);
  return heapcell;
}




qsfreelist_t * qsfreelist (qsheap_t * heap, qsptr_t p)
{
  if (ISNIL(p)) return NULL;
  if (!ISOBJ26(p)) return NULL;
  qsheapaddr_t addr = COBJ26(p);
  if ((addr < 0) || (addr > heap->cap))
    return NULL;
  qsfreelist_t * retval = qsfreelist_ref(heap, addr);
  return retval;
}

qsfreelist_t * qsfreelist_ref (qsheap_t * heap, qsheapaddr_t addr)
{
  if (addr == QSFREE_SENTINEL)
    return NULL;
  qsfreelist_t * probe = (qsfreelist_t*)qsheap_ref(heap, addr);
  if (!ISSYNC29(probe->mgmt))
    {
      return NULL;
    }
  if (MGMT_IS_USED(probe->mgmt)) return NULL;
  if (qsheapcell_is_used((qsheapcell_t*)probe)) return NULL;
  return probe;
}

qserror_t qsfreelist_reap (qsheap_t * heap, qsheapaddr_t addr, qsfreelist_t ** out_freelist)
{
  qsfreelist_t * probe = (qsfreelist_t*)qsheap_ref(heap, addr);
  if (!ISSYNC29(probe->mgmt))
    return QSERROR_INVALID;
  //int allocscale = qsobj_get_allocscale((qsobj_t*)probe);
  int allocscale = MGMT_GET_ALLOCSCALE(probe->mgmt);
  qsword ncells = 1 << allocscale;
  /*
  MGMT_CLR_USED(probe->mgmt);
  MGMT_CLR_MARKED(probe->mgmt);
  MGMT_CLR_GREY(probe->mgmt);
  MGMT_CLR_OCTET(probe->mgmt);
  MGMT_SET_SCORE(probe->mgmt, 0);
  MGMT_SET_ALLOCSCALE(probe->mgmt, 0);
  */
  probe->mgmt = TAG_SYNC29;
  probe->span = QSINT(ncells);
  probe->next = QSNIL;
  probe->prev = QSNIL;

  if (out_freelist)
    *out_freelist = probe;

  return QSERROR_OK;
}

/* split freelist in two, typically in preparation for allocation.
   'len' specifies the size of the second piece.
   Returns first segment as 'out_first' (NULL for perfect fit), returns the second segment as 'out_second' (to be written into).

   Returns OK if allocation succeeded, NOMEM if list cannot be split.
 */
qserror_t qsfreelist_split (qsheap_t * heap, qsheapaddr_t addr, qsword len_second, qsheapaddr_t * out_first, qsheapaddr_t * out_second)
{
  qsfreelist_t * node = qsfreelist_ref(heap, addr);
  qsword span = CINT30(node->span);
  if (span < len_second)
    {
      /* insufficient space. */
      if (out_first) *out_first = addr;
      if (out_second) *out_second = QSFREE_SENTINEL;
      return QSERROR_NOMEM;
    }
  else if (span == len_second)
    {
      /* exact fit. */
      if (out_first) *out_first = QSFREE_SENTINEL;
      if (out_second) *out_second = addr;
      return QSERROR_OK;
    }
  else
    {
      /* splitting. */
      qsword resized_len = span - len_second;
      qsheapaddr_t second_addr = addr + resized_len;
      qsfreelist_t * split = node + resized_len;
      node->span = QSINT(resized_len);
      split->span = QSINT(len_second);
      split->next = node->next;
      split->prev = QSINT(addr);
      split->mgmt = TAG_SYNC29; // still a freelist.
      node->next = QSINT(second_addr);
      if (out_first) *out_first = addr;
      if (out_second) *out_second = second_addr;
      return QSERROR_OK;
    }
}

/* find highest-address segment that can accommodate the requested number of
 cells. */
qserror_t qsfreelist_fit_end (qsheap_t * heap, qsheapaddr_t addr, qsword ncells, qsheapaddr_t * out_addr)
{
  /* Assume starting on highest fragment; work downwards. */
  qserror_t err = QSERROR_OK;
  int match = 0;

  qsheapaddr_t segment_addr;
  qsfreelist_t * segment;
  segment_addr = addr;
  segment = qsfreelist_ref(heap, segment_addr);
  if (!segment) return QSERROR_INVALID;

  while (!match && (segment != NULL))
    {
      segment = qsfreelist_ref(heap, segment_addr);
      if (!segment) return QSERROR_INVALID;
      qsword segment_span = qsfreelist_get_span(heap, segment_addr);
      if (segment_span < ncells)
	{
	  // too small.
	  // advance.
	  segment_addr = qsfreelist_get_prev(heap, segment_addr);
	  segment = qsfreelist_ref(heap, segment_addr);
	}
      else if (segment_span > ncells)
	{
	  // larger - split first.
	  if (out_addr)
	    {
	      qsheapaddr_t a0, a1, next;
	      err = qsfreelist_split(heap, segment_addr, ncells, &a0, &a1);
	      if (err != QSERROR_OK) return QSERROR_NOMEM;
	      // transfer 'next' field from second segment to first.
	      next = qsfreelist_get_next(heap, a1);
	      qsfreelist_set_next(heap, a0, next);
	      *out_addr = a1;
	      match = 1;
	    }
	  // else nowhere to save, do not modify freelist, but it could fit.
	  return QSERROR_OK;
	}
      else
	{
	  // exact match.
	  if (out_addr)
	    {
	      qsheapaddr_t prev_addr = qsfreelist_get_prev(heap, addr);
	      qsheapaddr_t next_addr = qsfreelist_get_next(heap, addr);
	      if (prev_addr != QSFREE_SENTINEL)
		qsfreelist_set_next(heap, prev_addr, qsfreelist_get_next(heap, segment_addr));
	      if (next_addr != QSFREE_SENTINEL)
		qsfreelist_set_prev(heap, next_addr, qsfreelist_get_prev(heap, segment_addr));
	      *out_addr = segment_addr;
	      match = 1;
	    }
	  // else nowhere to save; report as could fit, do not modify freelist.
	  return QSERROR_OK;
	}
    }

  return QSERROR_NOIMPL;
}

qsword qsfreelist_get_span (qsheap_t * heap, qsheapaddr_t cell_addr)
{
  qsfreelist_t * freelist = qsfreelist_ref(heap, cell_addr);
  if (!freelist) return 0;
  return CINT30(freelist->span);
}

qsword qsfreelist_get_prev (qsheap_t * heap, qsheapaddr_t cell_addr)
{
  qsfreelist_t * freelist = qsfreelist_ref(heap, cell_addr);
  if (!freelist) return 0;
  if (ISNIL(freelist->prev)) return QSFREE_SENTINEL;
  return CINT30(freelist->prev);
}

qsword qsfreelist_get_next (qsheap_t * heap, qsheapaddr_t cell_addr)
{
  qsfreelist_t * freelist = qsfreelist_ref(heap, cell_addr);
  if (!freelist) return 0;
  if (ISNIL(freelist->next)) return QSFREE_SENTINEL;
  return CINT30(freelist->next);
}

qserror_t qsfreelist_set_span (qsheap_t * heap, qsheapaddr_t cell_addr, qsword val)
{
  qsfreelist_t * freelist = qsfreelist_ref(heap, cell_addr);
  if (!freelist) return 0;
  freelist->span = QSINT(val);
}

qserror_t qsfreelist_set_prev (qsheap_t * heap, qsheapaddr_t cell_addr, qsword val)
{
  qsfreelist_t * freelist = qsfreelist_ref(heap, cell_addr);
  if (!freelist) return 0;
  qsptr_t enc = QSNIL;
  if (val != QSFREE_SENTINEL) enc = QSINT(val);
  freelist->prev = enc;
}

qserror_t qsfreelist_set_next (qsheap_t * heap, qsheapaddr_t cell_addr, qsword val)
{
  qsfreelist_t * freelist = qsfreelist_ref(heap, cell_addr);
  if (!freelist) return 0;
  qsptr_t enc = QSNIL;
  if (val != QSFREE_SENTINEL) enc = QSINT(val);
  freelist->next = enc;
}

int qsfreelist_crepr (qsheap_t * heap, qsheapaddr_t cell_addr, char * buf, int buflen)
{
  int n;
  qsfreelist_t * segment = qsfreelist_ref(heap, cell_addr);
  if (!segment)
    {
      return snprintf(buf, buflen, "(!qsfreelist_t)(_0x%08x)", cell_addr);
    }
  int mgmt = segment ? segment->mgmt : 0;
  int span = qsfreelist_get_span(heap, cell_addr);
  int prev = qsfreelist_get_prev(heap, cell_addr);
  int next = qsfreelist_get_next(heap, cell_addr);
  n = snprintf(buf, buflen, "(qsfreelist_t)(_0x%08x) = {\
 .mgmt=0x%08X,\
 .span=%d,\
 .prev=%d,\
 .next=%d }",
    cell_addr,
    mgmt,
    span,
    prev,
    next);
  return n;
}




qsheap_t * qsheap_init (qsheap_t * heap, uint32_t ncells)
{
  /* Setting locale necessary for string encoding conversion. */
  setlocale(LC_ALL, "");

  heap->wlock = 0;
  heap->cap = ncells;

  uint32_t i;
  for (i = 0; i < ncells; i++)
    {
      heap->space[i].mgmt = 0;
      heap->space[i].fields[0] = 0;
      heap->space[i].fields[1] = 0;
      heap->space[i].fields[2] = 0;
    }

  // TODO: initial freelist.
  qsfreelist_t * freelist = (qsfreelist_t*)(heap->space + 0);
  freelist->mgmt = TAG_SYNC29;
  freelist->span = QSINT(ncells);
  freelist->prev = QSNIL;
  freelist->next = QSNIL;
  heap->end_freelist = 0;

  return heap;
}

qsheap_t * qsheap_destroy (qsheap_t * heap)
{
  return heap;
}

qserror_t qsheap_allocscale (qsheap_t * heap, qsword allocscale, qsheapaddr_t * out_addr)
{
  /* TODO: seek lastest fitting freelist node. */
  if (!out_addr) return QSERROR_INVALID;
  qsword ncells = (1 << allocscale);
  qsheapaddr_t addr = 0;
  qserror_t err = qsfreelist_fit_end(heap, heap->end_freelist, ncells, &addr);
  if (err == QSERROR_OK)
    {
      qsheapcell_t * heapcell = qsheap_ref(heap, addr);
      qsheapcell_init(heapcell, 1, 0, allocscale);
      *out_addr = addr;
      return QSERROR_OK;
    }
  return QSERROR_NOMEM;
}

qserror_t qsheap_alloc_ncells (qsheap_t * heap, qsword ncells, qsheapaddr_t * out_addr)
{
/* Take log2 of number of cells (2*n to accomodate cells)
 0 => 0
 1 => 0
 2 => 1
 3 => 2
 4 => 2
 5 => 3
..8 => 3
 9 => 4
..16 => 4
*/
  int nbits = 0;
  if (ncells > 0) ncells--;
  while (ncells > 0)
    {
      nbits++;
      ncells >>= 1;
    }
  return qsheap_allocscale(heap, nbits, out_addr);
}

qserror_t qsheap_alloc_with_nbytes (qsheap_t * heap, qsword nbytes, qsheapaddr_t * out_addr)
{
  qserror_t retval = QSERROR_OK;
  qsword ncells = 0;
  if (nbytes == 0)
    ncells = 1;
  else
    ncells = 1 + ((nbytes-1) / sizeof(qsheapcell_t))+1;
  retval = qsheap_alloc_ncells(heap, ncells, out_addr);
  qsheapcell_t * heapcell = qsheap_ref(heap, *out_addr);
  MGMT_SET_OCTET(heapcell->mgmt);
  return retval;
}

qserror_t qsheap_set_used (qsheap_t * heap, qsheapaddr_t cell_addr, int val)
{
  qsheapcell_t * heapcell = qsheap_ref(heap, cell_addr);
  if (!heapcell) return QSERROR_INVALID;
  qsheapcell_set_used(heapcell, val);
  return QSERROR_OK;
}

qserror_t qsheap_set_marked (qsheap_t * heap, qsheapaddr_t cell_addr, int val)
{
  qsheapcell_t * heapcell = qsheap_ref(heap, cell_addr);
  if (!heapcell) return QSERROR_INVALID;
  qsheapcell_set_marked(heapcell, val);
  return QSERROR_OK;
}

int qsheap_is_sync (qsheap_t * heap, qsheapaddr_t cell_addr)
{
  qsheapcell_t * heapcell = qsheap_ref(heap, cell_addr);
  if (!heapcell) return 0;
  return ISSYNC29(heapcell->mgmt);
}

int qsheap_is_marked (qsheap_t * heap, qsheapaddr_t cell_addr)
{
  qsheapcell_t * heapcell = qsheap_ref(heap, cell_addr);
  if (!heapcell) return 0;
  return qsheapcell_is_marked(heapcell);
}

int qsheap_is_used (qsheap_t * heap, qsheapaddr_t cell_addr)
{
  qsheapcell_t * heapcell = qsheap_ref(heap, cell_addr);
  if (!heapcell) return 0;
  return qsheapcell_is_used(heapcell);
}

qsheapcell_t * qsheap_ref (qsheap_t * heap, qsheapaddr_t cell_addr)
{
  if ((cell_addr < 0) || (cell_addr >= heap->cap))
    return NULL;
  return (qsheapcell_t*)(heap->space + cell_addr);
}

qserror_t qsheap_word (qsheap_t * heap, qsheapaddr_t word_addr, qsptr_t * out_word)
{
  qsptr_t * pword = NULL;
  qsword cell_addr = (word_addr >> 2);
  qsheapcell_t * equiv_heapcell = qsheap_ref(heap, cell_addr);
  if (! equiv_heapcell)
    return QSERROR_INVALID;
  if (out_word)
    {
      qsword word_ofs = (word_addr & 0x3);
      pword = ((qsptr_t *)(equiv_heapcell)) + word_ofs;
      *out_word = *pword;
    }
  return QSERROR_OK;
}

/*
obj_addr is turned into a free list segment.
if up_free is an adjacent free list segment, coalesce into a single large
segment at obj_addr,
otherwise free list segment at obj_addr links to up_free.
*/
qserror_t qsheap_reclaim (qsheap_t * heap, qsheapaddr_t obj_addr, qsheapaddr_t down_free, qsheapaddr_t * out_segment)
{
  qsfreelist_t * segment = (qsfreelist_t*)qsheap_ref(heap, obj_addr);
  qsword reclaimed_addr = obj_addr;
  qsword scaled_span = MGMT_GET_ALLOCSCALE(segment->mgmt);
  qsword span = 1 << scaled_span;
  segment->mgmt = TAG_SYNC29;
  segment->span = QSINT(span);
  segment->next = QSNIL;
  segment->prev = QSNIL;

  MGMT_SET_ALLOCSCALE(segment->mgmt, 0);
  qsheap_set_used(heap, obj_addr, 0);
  qsheap_set_marked(heap, obj_addr, 0);
  reclaimed_addr = obj_addr;

  qsword up_free = QSFREE_SENTINEL;

  /* possibly coalesce down. */
  if (down_free != QSFREE_SENTINEL)
    {
      qsword down_span = qsfreelist_get_span(heap, down_free);
      up_free = qsfreelist_get_next(heap, down_free);
      if ((down_free + down_span) == obj_addr)
	{
	  /* coalesce down. */
	  qsfreelist_set_span(heap, down_free, span + down_span);
	  segment->mgmt = 0;
	  /* down's "next" unchanged, "prev" unchanged. */
	  span += down_span;
	  reclaimed_addr = down_free;
	}
      else
	{
	  /* link from down. */
	  qsword temp = qsfreelist_get_next(heap, down_free);
	  qsfreelist_set_next(heap, reclaimed_addr, temp);
	  qsfreelist_set_prev(heap, reclaimed_addr, down_free);
	  qsfreelist_set_next(heap, down_free, reclaimed_addr);
	}
    }
  /* check coalesce up. */
  if (up_free != QSFREE_SENTINEL)
    {
      if ((reclaimed_addr + span) == up_free)
	{
	  /* coalesce up. */
	  qsword up_span = qsfreelist_get_span(heap, up_free);
	  qsword up_next = qsfreelist_get_next(heap, up_free);
	  qsfreelist_set_span(heap, reclaimed_addr, span + up_span);
	  qsfreelist_set_next(heap, reclaimed_addr, up_next);
	  qsfreelist_t * up_segment = qsfreelist_ref(heap, up_free);
	  up_segment->mgmt = 0;
	  span += up_span;
	}
      else
	{
#if 0 // upward link already established in down-coalescence.
	  /* link to up. */
	  qsword temp = qsfreelist_get_prev(heap, up_free);
	  qsfreelist_set_prev(heap, up_free, reclaimed_addr);
	  qsfreelist_set_next(heap, reclaimed_addr, up_free);
	  qsfreelist_set_prev(heap, reclaimed_addr, temp);
#endif //0
	}
    }
  if (out_segment)
    *out_segment = reclaimed_addr;

  return QSERROR_OK;
}

qserror_t qsheap_sweep (qsheap_t * heap)
{
  qserror_t err = 0;
  qsheapaddr_t prev_free, next_free;
  qsheapaddr_t prev, curr;
  prev_free = QSFREE_SENTINEL;
  qsheapcell_t * probe = NULL;
  /* Scan cells for unmarked objects. */
  curr = 0;
  while (curr < heap->cap)
    {
      probe = qsheap_ref(heap, curr);
      if (ISSYNC29(probe->mgmt))
	{
	  // object or freelist.
	  if (qsheap_is_used(heap, curr))
	    {
	      // object.
	      qsheapcell_t * heapcell = qsheap_ref(heap, curr);
	      qsword scaled_span = MGMT_GET_ALLOCSCALE(heapcell->mgmt);
	      qsword span = 1 << scaled_span;

	      if (!qsheap_is_marked(heap, curr))
		{
		  err = qsheap_reclaim(heap, curr, prev_free, &prev_free);
		  /* prev_free is updated. */
		  assert(err == QSERROR_OK);
		}
	      else
		{
		  /* clear marked. */
		  qsheap_set_marked(heap, curr, 0);
		}

	      curr += span;
	    }
	  else
	    {
	      // freelist.
	      prev_free = curr;
	      curr += qsfreelist_get_span(heap, curr);
	    }
	}
      else
	{
	  /* seek synchronization. */
	  curr++;
	}
    }
  heap->end_freelist = prev_free;
  return QSERROR_NOIMPL;
}




#if  0
qsobj_t * qsobj_init (qsobj_t * obj, int is_octet)
{
  obj->_0 = QSNIL;
  obj->_1 = QSNIL;
  obj->_2 = QSNIL;
  return obj;
}

qsobj_t * qsobj_destroy (qsobj_t * obj)
{
  return obj;
}

qsptr_t qsobj_get_mgmt (qsobj_t * obj)
{
  return obj->mgmt;
}

qsobj_t * qsobj_set_marked (qsobj_t * obj)
{
  MGMT_SET_MARKED(obj->mgmt);
  return obj;
}

int qsobj_is_used (qsobj_t * obj)
{
  return !!(MGMT_IS_USED(obj->mgmt));
}

int qsobj_is_marked (qsobj_t * obj)
{
  return !!(MGMT_IS_MARKED(obj->mgmt));
}

int qsobj_is_octet (qsobj_t * obj)
{
  return !!(MGMT_IS_OCTET(obj->mgmt));
}

qsptr_t qsobj_get (qsobj_t * obj, int fld)
{
  qsptr_t retval = QSNIL;

  switch (fld)
    {
    case 0:
      retval = obj->_0;
      break;
    case 1:
      retval = obj->_1;
      break;
    case 2:
      retval = obj->_2;
      break;
    default:
      /* TODO: exception */
      break;
    }
  return retval;
}

void qsobj_set (qsobj_t * obj, int fld, qsptr_t val)
{
  switch (fld)
    {
    case 0:
      obj->_0 = val;
      break;
    case 1:
      obj->_1 = val;
      break;
    case 2:
      obj->_2 = val;
      break;
    default:
      /* TODO: exception */
      break;
    }
}

// increment reference count, for byte-objects.
void qsobj_up (qsobj_t * obj)
{
}

// decrement reference count, for byte-objects.
void qsobj_down (qsobj_t * obj)
{
}
#endif //0

