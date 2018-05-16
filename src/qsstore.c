#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "qsstore.h"



#include <stdbool.h>

#if 0
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


#endif //0


qsfreelist_t * qsfreelist (qsstore_t * store, qsptr_t p)
{
  if (ISNIL(p)) return NULL;
  if (!ISOBJ26(p)) return NULL;
  qsstoreaddr_t addr = COBJ26(p);
  if ((addr < 0) || (addr > store->cap))
    return NULL;
  qsfreelist_t * retval = qsfreelist_ref(store, addr);
  return retval;
}

qsfreelist_t * qsfreelist_ref (qsstore_t * store, qsstoreaddr_t addr)
{
  if (addr == QSFREE_SENTINEL)
    return NULL;
  qsfreelist_t * probe = (qsfreelist_t*)qsstore_get(store, addr);
  if (!ISSYNC29(probe->mgmt))
    {
      return NULL;
    }
  if (MGMT_IS_USED(probe->mgmt)) return NULL;
  if (qsstorebay_is_used(store, addr)) return NULL;
  return probe;
}

qserror_t qsfreelist_reap (qsstore_t * store, qsstoreaddr_t addr, qsfreelist_t ** out_freelist)
{
  qsfreelist_t * probe = (qsfreelist_t*)qsstore_get(store, addr);
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
qserror_t qsfreelist_split (qsstore_t * store, qsstoreaddr_t addr, qsword len_second, qsstoreaddr_t * out_first, qsstoreaddr_t * out_second)
{
  qsfreelist_t * node = qsfreelist_ref(store, addr);
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
      qsstoreaddr_t second_addr = addr + resized_len;
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
qserror_t qsfreelist_fit_end (qsstore_t * store, qsstoreaddr_t addr, qsword ncells, qsstoreaddr_t * out_addr)
{
  /* Assume starting on highest fragment; work downwards. */
  qserror_t err = QSERROR_OK;
  int match = 0;

  qsstoreaddr_t segment_addr;
  qsfreelist_t * segment;
  segment_addr = addr;
  segment = qsfreelist_ref(store, segment_addr);
  if (!segment) return QSERROR_INVALID;

  while (!match && (segment != NULL))
    {
      segment = qsfreelist_ref(store, segment_addr);
      if (!segment) return QSERROR_INVALID;
      qsword segment_span = qsfreelist_get_span(store, segment_addr);
      if (segment_span < ncells)
	{
	  // too small.
	  // advance.
	  segment_addr = qsfreelist_get_prev(store, segment_addr);
	  segment = qsfreelist_ref(store, segment_addr);
	}
      else if (segment_span > ncells)
	{
	  // larger - split first.
	  if (out_addr)
	    {
	      qsstoreaddr_t a0, a1, next;
	      err = qsfreelist_split(store, segment_addr, ncells, &a0, &a1);
	      if (err != QSERROR_OK) return QSERROR_NOMEM;
	      // transfer 'next' field from second segment to first.
	      next = qsfreelist_get_next(store, a1);
	      qsfreelist_set_next(store, a0, next);
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
	      qsstoreaddr_t prev_addr = qsfreelist_get_prev(store, addr);
	      qsstoreaddr_t next_addr = qsfreelist_get_next(store, addr);
	      if (prev_addr != QSFREE_SENTINEL)
		qsfreelist_set_next(store, prev_addr, qsfreelist_get_next(store, segment_addr));
	      if (next_addr != QSFREE_SENTINEL)
		qsfreelist_set_prev(store, next_addr, qsfreelist_get_prev(store, segment_addr));
	      *out_addr = segment_addr;
	      match = 1;
	    }
	  // else nowhere to save; report as could fit, do not modify freelist.
	  return QSERROR_OK;
	}
    }

  return QSERROR_NOIMPL;
}

qsword qsfreelist_get_span (qsstore_t * store, qsstoreaddr_t cell_addr)
{
  qsfreelist_t * freelist = qsfreelist_ref(store, cell_addr);
  if (!freelist) return 0;
  return CINT30(freelist->span);
}

qsword qsfreelist_get_prev (qsstore_t * store, qsstoreaddr_t cell_addr)
{
  qsfreelist_t * freelist = qsfreelist_ref(store, cell_addr);
  if (!freelist) return 0;
  if (ISNIL(freelist->prev)) return QSFREE_SENTINEL;
  return CINT30(freelist->prev);
}

qsword qsfreelist_get_next (qsstore_t * store, qsstoreaddr_t cell_addr)
{
  qsfreelist_t * freelist = qsfreelist_ref(store, cell_addr);
  if (!freelist) return 0;
  if (ISNIL(freelist->next)) return QSFREE_SENTINEL;
  return CINT30(freelist->next);
}

qserror_t qsfreelist_set_span (qsstore_t * store, qsstoreaddr_t cell_addr, qsword val)
{
  qsfreelist_t * freelist = qsfreelist_ref(store, cell_addr);
  if (!freelist) return 0;
  freelist->span = QSINT(val);
}

qserror_t qsfreelist_set_prev (qsstore_t * store, qsstoreaddr_t cell_addr, qsword val)
{
  qsfreelist_t * freelist = qsfreelist_ref(store, cell_addr);
  if (!freelist) return 0;
  qsptr_t enc = QSNIL;
  if (val != QSFREE_SENTINEL) enc = QSINT(val);
  freelist->prev = enc;
}

qserror_t qsfreelist_set_next (qsstore_t * store, qsstoreaddr_t cell_addr, qsword val)
{
  qsfreelist_t * freelist = qsfreelist_ref(store, cell_addr);
  if (!freelist) return 0;
  qsptr_t enc = QSNIL;
  if (val != QSFREE_SENTINEL) enc = QSINT(val);
  freelist->next = enc;
}

int qsfreelist_crepr (qsstore_t * store, qsstoreaddr_t cell_addr, char * buf, int buflen)
{
  int n;
  qsfreelist_t * segment = qsfreelist_ref(store, cell_addr);
  if (!segment)
    {
      return snprintf(buf, buflen, "(!qsfreelist_t)(_0x%08x)", cell_addr);
    }
  int mgmt = segment ? segment->mgmt : 0;
  int span = qsfreelist_get_span(store, cell_addr);
  int prev = qsfreelist_get_prev(store, cell_addr);
  int next = qsfreelist_get_next(store, cell_addr);
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




#if 0
qsheap_t * qsheap_init (qsheap_t * heap, uint32_t ncells)
{
  /* Setting locale necessary for string encoding conversion. */
  setlocale(LC_ALL, "");

  heap->wlock = 0;
  heap->cap = ncells;
  heap->symstore = QSNIL;

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

qserror_t qsheap_alloc_nbays (qsheap_t * heap, qsword ncells, qsheapaddr_t * out_addr)
{
/* Take log2 of number of bays (2**n to accomodate bays)
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

qserror_t qsheap_alloc_ncells (qsheap_t * heap, qsword ncells, qsheapaddr_t * out_addr)
{
  return qsheap_alloc_nbays(heap, ncells, out_addr);
}

qserror_t qsheap_alloc_with_nptrs (qsheap_t * heap, qsword nptrs, qsheapaddr_t * out_addr)
{
  static const int nptr_per_bay = sizeof(qsbay0_t) / sizeof(qsptr_t);
  qsheapaddr_t addr = 0;
  qserror_t retval = QSERROR_OK;
  qsword nbays = 0;
  if (nptrs == 0)
    nbays = 1;
  else
    nbays = 1 + ((nptrs-1) / nptr_per_bay)+1;
  retval = qsheap_alloc_nbays(heap, nbays, &addr);
  if (out_addr)
    *out_addr = addr;
  return retval;
}

qserror_t qsheap_alloc_with_nbytes (qsheap_t * heap, qsword nbytes, qsheapaddr_t * out_addr)
{
  static const int nbyte_per_bay = sizeof(qsbay0_t);
  qsheapaddr_t addr = 0;
  qserror_t retval = QSERROR_OK;
  qsword nbays = 0;
  if (nbytes == 0)
    nbays = 1;
  else
    nbays = 1 + ((nbytes-1) / nbyte_per_bay)+1;
  retval = qsheap_alloc_nbays(heap, nbays, &addr);
  qsheap_set_octetate(heap, addr, 1);
  if (out_addr)
    *out_addr = addr;
  return retval;
}

int qsheap_is_valid (qsheap_t * heap, qsheapaddr_t cell_addr)
{
  if (cell_addr < 0) return 0;
  if (cell_addr >= heap->cap) return 0;
  return 1;
}

int qsheap_is_synced (qsheap_t * heap, qsheapaddr_t cell_addr)
{
  qsheapcell_t * heapcell = qsheap_ref(heap, cell_addr);
  if (!heapcell) return 0;
  return ISSYNC29(heapcell->mgmt);
}

int qsheap_get_allocscale (qsheap_t * heap, qsheapaddr_t cell_addr)
{
  qsheapcell_t * heapcell = qsheap_ref(heap, cell_addr);
  if (!heapcell) return 0;
  return qsheapcell_get_allocscale(heapcell);
}

int qsheap_is_used (qsheap_t * heap, qsheapaddr_t cell_addr)
{
  qsheapcell_t * heapcell = qsheap_ref(heap, cell_addr);
  if (!heapcell) return 0;
  return qsheapcell_is_used(heapcell);
}

qserror_t qsheap_set_used (qsheap_t * heap, qsheapaddr_t cell_addr, int val)
{
  qsheapcell_t * heapcell = qsheap_ref(heap, cell_addr);
  if (!heapcell) return QSERROR_INVALID;
  qsheapcell_set_used(heapcell, val);
  return QSERROR_OK;
}

int qsheap_is_marked (qsheap_t * heap, qsheapaddr_t cell_addr)
{
  qsheapcell_t * heapcell = qsheap_ref(heap, cell_addr);
  if (!heapcell) return 0;
  return qsheapcell_is_marked(heapcell);
}

qserror_t qsheap_set_marked (qsheap_t * heap, qsheapaddr_t cell_addr, int val)
{
  qsheapcell_t * heapcell = qsheap_ref(heap, cell_addr);
  if (!heapcell) return QSERROR_INVALID;
  qsheapcell_set_marked(heapcell, val);
  return QSERROR_OK;
}

int qsheap_is_octetate (qsheap_t * heap, qsheapaddr_t cell_addr)
{
  qsheapcell_t * heapcell = qsheap_ref(heap, cell_addr);
  if (!heapcell) return 0;
  return qsheapcell_is_octet(heapcell);
}

qserror_t qsheap_set_octetate (qsheap_t * heap, qsheapaddr_t cell_addr, int val)
{
  qsheapcell_t * heapcell = qsheap_ref(heap, cell_addr);
  if (!heapcell) return QSERROR_INVALID;
  MGMT_SET_OCTET(heapcell->mgmt);
  return QSERROR_OK;
}

int qsheap_get_parent (qsheap_t * heap, qsheapaddr_t cell_addr)
{
  qsheapcell_t * heapcell = qsheap_ref(heap, cell_addr);
  if (!heapcell) return 0;
  return qsheapcell_get_parent(heapcell);
}

qserror_t qsheap_set_parent (qsheap_t * heap, qsheapaddr_t cell_addr, int val)
{
  qsheapcell_t * heapcell = qsheap_ref(heap, cell_addr);
  if (!heapcell) return QSERROR_INVALID;
  qsheapcell_set_parent(heapcell, val);
  return QSERROR_OK;
}

int qsheap_get_score (qsheap_t * heap, qsheapaddr_t cell_addr)
{
  qsheapcell_t * heapcell = qsheap_ref(heap, cell_addr);
  if (!heapcell) return 0;
  return qsheapcell_get_score(heapcell);
}

qserror_t qsheap_set_score (qsheap_t * heap, qsheapaddr_t cell_addr, int val)
{
  qsheapcell_t * heapcell = qsheap_ref(heap, cell_addr);
  if (!heapcell) return QSERROR_INVALID;
  qsheapcell_set_score(heapcell, val);
  return QSERROR_OK;
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
#endif //0





/*********/
/* Store */
/*********/

qsstore_t * qsstore_init (qsstore_t * store, uint32_t nbays)
{
  /* Setting locale necessary for string encoding conversion. */
  setlocale(LC_ALL, "");

  store->wlock = 0;
  store->cap = nbays;
  store->symstore = QSNIL;

  uint32_t i;
  for (i = 0; i < nbays; i++)
    {
      store->space[i].mgmt = 0;
      store->space[i].fields[0] = 0;
      store->space[i].fields[1] = 0;
      store->space[i].fields[2] = 0;
    }

  // TODO: initial freelist.
  qsfreelist_t * freelist = (qsfreelist_t*)(store->space + 0);
  freelist->mgmt = TAG_SYNC29;
  freelist->span = QSINT(nbays);
  freelist->prev = QSNIL;
  freelist->next = QSNIL;
  store->end_freelist = 0;

  return store;
}

qsstore_t * qsstore_destroy (qsstore_t * store)
{
  return store;
}

qserror_t qsstore_allocscale (qsstore_t * store, qsword allocscale, qsstoreaddr_t * out_addr)
{
  /* TODO: seek lastest fitting freelist node. */
  if (!out_addr) return QSERROR_INVALID;
  qsword nbays = (1 << allocscale);
  qsstoreaddr_t addr = 0;
  //qserror_t err = qsfreelist_fit_end(store, store->end_freelist, nbays, &addr);
  qserror_t err = qsfreelist_fit_end(store, store->end_freelist, nbays, &addr);
  if (err == QSERROR_OK)
    {
      qsstorebay_clear(store, addr);
      qsstorebay_set_synced(store, addr, 1);
      qsstorebay_set_used(store, addr, 1);
      qsstorebay_set_marked(store, addr, 0);
      qsstorebay_set_allocscale(store, addr, allocscale);
      *out_addr = addr;
      return QSERROR_OK;
    }
  return QSERROR_NOMEM;
}

qserror_t qsstore_alloc_nbays (qsstore_t * store, qsword nbays, qsstoreaddr_t * out_addr)
{
/* Take log2 of number of bays (2**n to accomodate bays)
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
  if (nbays > 0) nbays--;
  while (nbays > 0)
    {
      nbits++;
      nbays >>= 1;
    }
  return qsstore_allocscale(store, nbits, out_addr);
}

qserror_t qsstore_alloc_with_nptrs (qsstore_t * store, qsword nptrs, qsstoreaddr_t * out_addr)
{
  static const int nptr_per_bay = sizeof(qsbay0_t) / sizeof(qsptr_t);
  qsstoreaddr_t addr = 0;
  qserror_t retval = QSERROR_OK;
  qsword nbays = 0;
  if (nptrs == 0)
    nbays = 1;
  else
    nbays = 1 + ((nptrs-1) / nptr_per_bay)+1;
  retval = qsstore_alloc_nbays(store, nbays, &addr);
  if (out_addr)
    *out_addr = addr;
  return retval;
}
qserror_t qsstore_alloc_with_nbytes (qsstore_t * store, qsword nbytes, qsstoreaddr_t * out_addr)
{
  static const int nbyte_per_bay = sizeof(qsbay0_t);
  qsstoreaddr_t addr = 0;
  qserror_t retval = QSERROR_OK;
  qsword nbays = 0;
  if (nbytes == 0)
    nbays = 1;
  else
    nbays = 1 + ((nbytes-1) / nbyte_per_bay)+1;
  retval = qsstore_alloc_nbays(store, nbays, &addr);
  qsstorebay_set_octetate(store, addr, 1);
  if (out_addr)
    *out_addr = addr;
  return retval;
}

bool qsstore_is_valid (qsstore_t * store, qsstoreaddr_t bay_addr)
{
  if (bay_addr < 0) return false;
  if (bay_addr >= store->cap) return false;
  return true;
}

qsbay0_t * qsstore_get (qsstore_t * store, qsstoreaddr_t bay_addr)
{
  if (!qsstore_is_valid(store, bay_addr)) return NULL;
  return store->space + bay_addr;
}

int qsstorebay_clear (qsstore_t * store, qsstoreaddr_t bay_addr)
{
  if (!qsstore_is_valid(store, bay_addr)) return 0;
  store->space[bay_addr].mgmt = 0;
  store->space[bay_addr].fields[0] = 0;
  store->space[bay_addr].fields[1] = 0;
  store->space[bay_addr].fields[2] = 0;
  return 0;
}

qsword qsstorebay_get_ptr (qsstore_t * store, qsstoreaddr_t bay_addr, qsword ofs)
{
  if (!qsstore_is_valid(store, bay_addr)) return 0;
  static const int nptr_per_bay = sizeof(qsbay0_t) / sizeof(qsptr_t);
  if (!qsstore_is_valid(store, bay_addr + (ofs / nptr_per_bay))) return 0;
  return *(((qsptr_t*)(store->space + bay_addr)) + ofs);
}

qsword qsstorebay_set_ptr (qsstore_t * store, qsstoreaddr_t bay_addr, qsword ofs, qsword ptrval)
{
  if (!qsstore_is_valid(store, bay_addr)) return 0;
  static const int nptr_per_bay = sizeof(qsbay0_t) / sizeof(qsptr_t);
  if (!qsstore_is_valid(store, bay_addr + (ofs / nptr_per_bay))) return 0;
  qsptr_t * ptrcell = (qsptr_t*)(store->space + bay_addr) + ofs;
  *ptrcell = ptrval;
  return 0;
}

int qsstorebay_get_oct (qsstore_t * store, qsstoreaddr_t bay_addr, qsword ofs)
{
  if (!qsstore_is_valid(store, bay_addr)) return 0;
  if (!qsstore_is_valid(store, bay_addr + (ofs / sizeof(qsbay0_t)))) return 0;
  return *(((uint8_t*)(store->space + bay_addr)) + ofs);
}

qsword qsstorebay_set_oct (qsstore_t * store, qsstoreaddr_t bay_addr, qsword ofs, qsword byteval)
{
  if (!qsstore_is_valid(store, bay_addr)) return 0;
  if (!qsstore_is_valid(store, bay_addr + (ofs / sizeof(qsbay0_t)))) return 0;
  uint8_t *bytecell = (uint8_t*)(store->space + bay_addr) + ofs;
  *bytecell = byteval;
  return 0;
}

static
qsword qsstorebay_get_mgmt (qsstore_t * store, qsstoreaddr_t bay_addr)
{
  return store->space[bay_addr].mgmt;
}

static
qsword qsstorebay_set_mgmt (qsstore_t * store, qsstoreaddr_t bay_addr, qsword val)
{
  return (store->space[bay_addr].mgmt = val);
}

bool qsstorebay_is_synced (qsstore_t * store, qsstoreaddr_t bay_addr)
{
  if (!qsstore_is_valid(store, bay_addr)) return false;
  return ISSYNC29(store->space[bay_addr].mgmt);
}

int qsstorebay_set_synced (qsstore_t * store, qsstoreaddr_t bay_addr, bool val)
{
  if (!qsstore_is_valid(store, bay_addr)) return false;
  qsword mgmt = store->space[bay_addr].mgmt;
  mgmt = mgmt & ~TAGMASK_SYNC29;
  if (val)
    mgmt |= TAG_SYNC29;
  store->space[bay_addr].mgmt = mgmt;
  return 0;
}

qsword qsstorebay_get_allocsize (qsstore_t * store, qsstoreaddr_t bay_addr)
{
  if (!qsstore_is_valid(store, bay_addr)) return 0;
  return (1 << MGMT_GET_ALLOCSCALE(store->space[bay_addr].mgmt));
}

int qsstorebay_get_allocscale (qsstore_t * store, qsstoreaddr_t bay_addr)
{
  if (!qsstore_is_valid(store, bay_addr)) return 0;
  return MGMT_GET_ALLOCSCALE(store->space[bay_addr].mgmt);
}

int qsstorebay_set_allocscale (qsstore_t * store, qsstoreaddr_t bay_addr, qsword val)
{
  if (!qsstore_is_valid(store, bay_addr)) return 0;
  return MGMT_SET_ALLOCSCALE(store->space[bay_addr].mgmt, val);
}

bool qsstorebay_is_used (qsstore_t * store, qsstoreaddr_t bay_addr)
{
  if (!qsstore_is_valid(store, bay_addr)) return 0;
  return MGMT_IS_USED(store->space[bay_addr].mgmt);
}

int qsstorebay_set_used (qsstore_t * store, qsstoreaddr_t bay_addr, bool val)
{
  if (!qsstore_is_valid(store, bay_addr)) return 0;
  if (val)
    MGMT_SET_USED(store->space[bay_addr].mgmt);
  else
    MGMT_CLR_USED(store->space[bay_addr].mgmt);
  return 0;
}

bool qsstorebay_is_marked (qsstore_t * store, qsstoreaddr_t bay_addr)
{
  if (!qsstore_is_valid(store, bay_addr)) return 0;
  return MGMT_IS_MARKED(store->space[bay_addr].mgmt);
}

int qsstorebay_set_marked (qsstore_t * store, qsstoreaddr_t bay_addr, bool val)
{
  if (!qsstore_is_valid(store, bay_addr)) return 0;
  if (val)
    MGMT_SET_MARKED(store->space[bay_addr].mgmt);
  else
    MGMT_CLR_MARKED(store->space[bay_addr].mgmt);
  return 0;
}

bool qsstorebay_is_octetate (qsstore_t * store, qsstoreaddr_t bay_addr)
{
  if (!qsstore_is_valid(store, bay_addr)) return 0;
  return MGMT_IS_OCTET(store->space[bay_addr].mgmt);
}

int qsstorebay_set_octetate (qsstore_t * store, qsstoreaddr_t bay_addr, bool val)
{
  if (!qsstore_is_valid(store, bay_addr)) return 0;
  if (val)
    MGMT_SET_OCTET(store->space[bay_addr].mgmt);
  else
    MGMT_CLR_OCTET(store->space[bay_addr].mgmt);
  return 0;
}

int qsstorebay_get_parent (qsstore_t * store, qsstoreaddr_t bay_addr)
{
  if (!qsstore_is_valid(store, bay_addr)) return 0;
  return MGMT_GET_PARENT(store->space[bay_addr].mgmt);
}

int qsstorebay_set_parent (qsstore_t * store, qsstoreaddr_t bay_addr, qsword val)
{
  if (!qsstore_is_valid(store, bay_addr)) return 0;
  MGMT_SET_PARENT(store->space[bay_addr].mgmt, val);
  return 0;
}

int qsstorebay_get_score (qsstore_t * store, qsstoreaddr_t bay_addr)
{
  if (!qsstore_is_valid(store, bay_addr)) return 0;
  return MGMT_GET_SCORE(store->space[bay_addr].mgmt);
}

int qsstorebay_set_score (qsstore_t * store, qsstoreaddr_t bay_addr, qsword val)
{
  if (!qsstore_is_valid(store, bay_addr)) return 0;
  MGMT_SET_SCORE(store->space[bay_addr].mgmt, val);
  return 0;
}


bool qsstorebay_is_uniptr (qsstore_t * store, qsstoreaddr_t bay_addr)
{
  if (!qsstore_is_valid(store, bay_addr)) return false;
  qsword mgmt = store->space[bay_addr].mgmt;
  if (! ISSYNC29(mgmt)) return false;
  if (MGMT_IS_OCTET(mgmt)) return false;
  if (MGMT_GET_ALLOCSCALE(mgmt) > 0) return false;
  return true;
}

bool qsstorebay_is_multiptr (qsstore_t * store, qsstoreaddr_t bay_addr)
{
  if (!qsstore_is_valid(store, bay_addr)) return false;
  qsword mgmt = store->space[bay_addr].mgmt;
  if (! ISSYNC29(mgmt)) return false;
  if (MGMT_IS_OCTET(mgmt)) return false;
  if (MGMT_GET_ALLOCSCALE(mgmt) == 0) return false;
  return true;
}

bool qsstorebay_is_unioct (qsstore_t * store, qsstoreaddr_t bay_addr)
{
  if (!qsstore_is_valid(store, bay_addr)) return false;
  qsword mgmt = store->space[bay_addr].mgmt;
  if (! ISSYNC29(mgmt)) return false;
  if (! MGMT_IS_OCTET(mgmt)) return false;
  if (MGMT_GET_ALLOCSCALE(mgmt) > 0) return false;
  return true;
}

bool qsstorebay_is_multioct (qsstore_t * store, qsstoreaddr_t bay_addr)
{
  if (!qsstore_is_valid(store, bay_addr)) return false;
  qsword mgmt = store->space[bay_addr].mgmt;
  if (! ISSYNC29(mgmt)) return false;
  if (! MGMT_IS_OCTET(mgmt)) return false;
  if (MGMT_GET_ALLOCSCALE(mgmt) == 0) return false;
  return true;
}


qserror_t qsstore_fetch_word (qsstore_t * store, qsstoreaddr_t word_addr, qsptr_t * out_word)
{
  qsptr_t * pword = NULL;
  qsword bay_addr = (word_addr >> 2);
  if (! qsstore_is_valid(store, bay_addr)) return QSERROR_INVALID;
  if (out_word)
    {
      qsword word_ofs = (word_addr & 0x3);
      qsword word_val = qsstorebay_get_ptr(store, bay_addr, word_ofs);
      *out_word = word_val;
    }
  return QSERROR_OK;
}

/*
obj_addr is turned into a free list segment.
if up_free is an adjacent free list segment, coalesce into a single large
segment at obj_addr,
otherwise free list segment at obj_addr links to up_free.
*/
qserror_t qsstore_reclaim (qsstore_t * store, qsstoreaddr_t obj_addr, qsstoreaddr_t down_free, qsstoreaddr_t * out_segment)
{
  qsfreelist_t * segment = (qsfreelist_t*)qsstore_get(store, obj_addr);
  qsword reclaimed_addr = obj_addr;
  qsword scaled_span = MGMT_GET_ALLOCSCALE(segment->mgmt);
  qsword span = 1 << scaled_span;
  segment->mgmt = TAG_SYNC29;
  segment->span = QSINT(span);
  segment->next = QSNIL;
  segment->prev = QSNIL;

  MGMT_SET_ALLOCSCALE(segment->mgmt, 0);
  qsstorebay_set_used(store, obj_addr, 0);
  qsstorebay_set_marked(store, obj_addr, 0);
  reclaimed_addr = obj_addr;

  qsword up_free = QSFREE_SENTINEL;

  /* possibly coalesce down. */
  if (down_free != QSFREE_SENTINEL)
    {
      qsword down_span = qsfreelist_get_span(store, down_free);
      up_free = qsfreelist_get_next(store, down_free);
      if ((down_free + down_span) == obj_addr)
	{
	  /* coalesce down. */
	  qsfreelist_set_span(store, down_free, span + down_span);
	  segment->mgmt = 0;
	  /* down's "next" unchanged, "prev" unchanged. */
	  span += down_span;
	  reclaimed_addr = down_free;
	}
      else
	{
	  /* link from down. */
	  qsword temp = qsfreelist_get_next(store, down_free);
	  qsfreelist_set_next(store, reclaimed_addr, temp);
	  qsfreelist_set_prev(store, reclaimed_addr, down_free);
	  qsfreelist_set_next(store, down_free, reclaimed_addr);
	}
    }
  /* check coalesce up. */
  if (up_free != QSFREE_SENTINEL)
    {
      if ((reclaimed_addr + span) == up_free)
	{
	  /* coalesce up. */
	  qsword up_span = qsfreelist_get_span(store, up_free);
	  qsword up_next = qsfreelist_get_next(store, up_free);
	  qsfreelist_set_span(store, reclaimed_addr, span + up_span);
	  qsfreelist_set_next(store, reclaimed_addr, up_next);
	  qsfreelist_t * up_segment = qsfreelist_ref(store, up_free);
	  up_segment->mgmt = 0;
	  span += up_span;
	}
      else
	{
#if 0 // upward link already established in down-coalescence.
	  /* link to up. */
	  qsword temp = qsfreelist_get_prev(store, up_free);
	  qsfreelist_set_prev(store, up_free, reclaimed_addr);
	  qsfreelist_set_next(store, reclaimed_addr, up_free);
	  qsfreelist_set_prev(store, reclaimed_addr, temp);
#endif //0
	}
    }
  if (out_segment)
    *out_segment = reclaimed_addr;

  return QSERROR_OK;
}


qserror_t qsstore_sweep (qsstore_t * store)
{
  qserror_t err = 0;
  qsstoreaddr_t prev_free, next_free;
  qsstoreaddr_t prev, curr;
  prev_free = QSFREE_SENTINEL;
  qsbay0_t * probe = NULL;
  /* Scan cells for unmarked objects. */
  curr = 0;
  while (curr < store->cap)
    {
      probe = qsstore_get(store, curr);
      if (ISSYNC29(probe->mgmt))
	{
	  // object or freelist.
	  if (qsstorebay_is_used(store, curr))
	    {
	      // object.
	      qsword scaled_span = qsstorebay_get_allocscale(store, curr);
	      qsword span = 1 << scaled_span;

	      if (!qsstorebay_is_marked(store, curr))
		{
		  err = qsstore_reclaim(store, curr, prev_free, &prev_free);
		  /* prev_free is updated. */
		  assert(err == QSERROR_OK);
		}
	      else
		{
		  /* clear marked. */
		  qsstorebay_set_marked(store, curr, 0);
		}

	      curr += span;
	    }
	  else
	    {
	      // freelist.
	      prev_free = curr;
	      curr += qsfreelist_get_span(store, curr);
	    }
	}
      else
	{
	  /* seek synchronization. */
	  curr++;
	}
    }
  store->end_freelist = prev_free;
  return QSERROR_NOIMPL;
}

