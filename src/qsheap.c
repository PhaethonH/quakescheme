#include <assert.h>
#include <stdlib.h>

#include "qsheap.h"


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
  if (qsobj_is_used((qsobj_t*)probe)) return NULL;
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
  MGMT_CLR_USED(probe->mgmt);
  MGMT_CLR_MARKED(probe->mgmt);
  MGMT_CLR_GREY(probe->mgmt);
  MGMT_CLR_RED(probe->mgmt);
  MGMT_CLR_OCTET(probe->mgmt);
  MGMT_SET_ALLOCSCALE(probe->mgmt, 0);
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
	      qsheapaddr_t a0, a1;
	      err = qsfreelist_split(heap, segment_addr, ncells, &a0, &a1);
	      if (err != QSERROR_OK) return QSERROR_NOMEM;
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
	      //prev->next = segment->next;
	      //next->prev = segment->prev;
	      qsfreelist_set_next(heap, prev_addr, qsfreelist_get_next(heap, segment_addr));
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
  if (ISINT30(val)) enc = QSINT(val);
  freelist->prev = enc;
}

qserror_t qsfreelist_set_next (qsheap_t * heap, qsheapaddr_t cell_addr, qsword val)
{
  qsfreelist_t * freelist = qsfreelist_ref(heap, cell_addr);
  if (!freelist) return 0;
  qsptr_t enc = QSNIL;
  if (ISINT30(val)) enc = QSINT(val);
  freelist->next = enc;
}




qsheap_t * qsheap_init (qsheap_t * heap, uint32_t ncells)
{
  heap->wlock = 0;
  heap->cap = ncells;

  uint32_t i;
  for (i = 0; i < ncells; i++)
    {
      heap->space[i].mgmt = 0;
      heap->space[i]._0 = 0;
      heap->space[i]._1 = 0;
      heap->space[i]._2 = 0;
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
      qsobj_t * obj = qsheap_ref(heap, addr);
      obj->mgmt = TAG_SYNC29;
      MGMT_SET_ALLOCSCALE(obj->mgmt, allocscale);
      MGMT_SET_USED(obj->mgmt);
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
  if (ncells > 1) ncells--;
  while (ncells > 0)
    {
      nbits++;
      ncells >>= 1;
    }
  return qsheap_allocscale(heap, nbits, out_addr);
}

qserror_t qsheap_set_used (qsheap_t * heap, qsheapaddr_t cell_addr, int val)
{
  qsobj_t * obj = qsheap_ref(heap, cell_addr);
  if (!obj) return QSERROR_INVALID;
  if (val)
    MGMT_SET_USED(obj->mgmt);
  else
    MGMT_CLR_USED(obj->mgmt);
  return QSERROR_OK;
}

qserror_t qsheap_set_marked (qsheap_t * heap, qsheapaddr_t cell_addr, int val)
{
  qsobj_t * obj = qsheap_ref(heap, cell_addr);
  if (!obj) return QSERROR_INVALID;
  if (val)
    MGMT_SET_MARKED(obj->mgmt);
  else
    MGMT_CLR_MARKED(obj->mgmt);
  return QSERROR_OK;
}

int qsheap_is_marked (qsheap_t * heap, qsheapaddr_t cell_addr)
{
  qsobj_t * obj = qsheap_ref(heap, cell_addr);
  if (!obj) return 0;
  return !!MGMT_IS_MARKED(obj->mgmt);
}

int qsheap_is_used (qsheap_t * heap, qsheapaddr_t cell_addr)
{
  qsobj_t * obj = qsheap_ref(heap, cell_addr);
  if (!obj) return 0;
  return !!MGMT_IS_USED(obj->mgmt);
}

qsobj_t * qsheap_ref (qsheap_t * heap, qsheapaddr_t cell_addr)
{
  if ((cell_addr < 0) || (cell_addr >= heap->cap))
    return NULL;
  return heap->space + cell_addr;
}

qserror_t qsheap_word (qsheap_t * heap, qsheapaddr_t word_addr, qsptr_t * out_word)
{
  qsptr_t * pword = NULL;
  qsword cell_addr = (word_addr >> 2);
  qsobj_t * equiv_obj = qsheap_ref(heap, cell_addr);
  if (! equiv_obj)
    return QSERROR_INVALID;
  if (out_word)
    {
      qsword word_ofs = (word_addr & 0x3);
      pword = ((qsptr_t *)(equiv_obj)) + word_ofs;
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
qserror_t qsheap_reclaim (qsheap_t * heap, qsheapaddr_t obj_addr, qsheapaddr_t up_free, qsheapaddr_t * out_segment)
{
  qsfreelist_t * segment = (qsfreelist_t*)qsheap_ref(heap, obj_addr);
  qsword scaled_span = MGMT_GET_ALLOCSCALE(segment->mgmt);
  qsword span = 1 << scaled_span;
  segment->mgmt = TAG_SYNC29;
  segment->span = QSINT(span);
  segment->next = QSNIL;
  segment->prev = QSNIL;

  MGMT_SET_ALLOCSCALE(segment->mgmt, 0);
  qsheap_set_used(heap, obj_addr, 0);
  qsheap_set_marked(heap, obj_addr, 0);

  if ((up_free != QSFREE_SENTINEL) && (up_free > obj_addr))
    {
      qsword up_span = qsfreelist_get_span(heap, up_free);
      if ((obj_addr + span) == up_free)
	{
	  /* coalesce up. */
	  span += up_span;
	  segment->span = QSINT(span);
	  (qsfreelist_ref(heap, up_free))->mgmt = 0;
	}
      else
	{
	  /* insert, link. */
	  qsfreelist_t * up_segment = qsfreelist_ref(heap, up_free);
	  segment->next = QSINT(up_free);
	  segment->prev = up_segment->prev;
	  up_segment->prev = QSINT(obj_addr);
	}
    }

  /* possibly coalesce down. */
  qsheapaddr_t down_free = QSFREE_SENTINEL;
  if (up_free == QSFREE_SENTINEL)
    down_free = heap->end_freelist;
  else
    down_free = qsfreelist_get_prev(heap, obj_addr);
  if (down_free == QSFREE_SENTINEL)
    {
      // cannot go down.
      if (out_segment) *out_segment = obj_addr;
      return QSERROR_OK;
    }

  // check adjacency of down.
  qsword down_span = qsfreelist_get_span(heap, down_free);
  if ((down_free + down_span) == obj_addr) {
      /* coalesce down. */
      span += down_span;
      qsfreelist_set_span(heap, down_free, span);
      if (out_segment) *out_segment = down_free;
      return QSERROR_OK;
    }

  return QSERROR_OK;
}

qserror_t qsheap_sweep (qsheap_t * heap)
{
  qserror_t err = 0;
  qsheapaddr_t prev_free, next_free;
  qsheapaddr_t prev, curr;
  prev_free = next_free = heap->end_freelist;
  prev = curr = 0;
  qsobj_t * probe = NULL;
  /* Scan cells for unmarked objects. */
  while (curr < heap->cap)
    {
      probe = qsheap_ref(heap, curr);
      if (ISSYNC29(probe->mgmt))
	{
	  // object or freelist.
	  if (qsheap_is_used(heap, curr))
	    {
	      // object.
	      qsobj_t * obj = qsheap_ref(heap, curr);
	      qsword scaled_span = MGMT_GET_ALLOCSCALE(obj->mgmt);
	      qsword span = 1 << scaled_span;

	      if (!qsheap_is_marked(heap, curr))
		{
		  err = qsheap_reclaim(heap, curr, next_free, &next_free);
		  assert(err == QSERROR_OK);
		  span = qsfreelist_get_span(heap, curr);
		  next_free = curr;
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
	      curr += qsfreelist_get_span(heap, curr);
	    }
	}
      else
	{
	  /* seek synchronization. */
	  curr++;
	}
    }
  heap->end_freelist = next_free;
  return QSERROR_NOIMPL;
}




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

