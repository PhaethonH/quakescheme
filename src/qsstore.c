#include "qsstore.h"

/* operations for the Scheme Store. */

/* memory segment. */

qssegment_t * qssegment_init (qssegment_t * segment, qsword baseaddr, qsword cap)
{
  segment->baseaddr = baseaddr;
  if (cap == 0)
    {
      segment->cap = SMEM_SIZE;
    }
  else
    {
      segment->cap = cap;
    }
  qssegment_clear(segment);
  return segment;
}

qssegment_t * qssegment_destroy (qssegment_t * segment)
{
  return segment;
}

qssegment_t * qssegment_clear (qssegment_t * segment)
{
  /* write zeroes in units of qsword. */
  for (int i = 0; i < (segment->cap / sizeof(qsword)); i++)
    {
      ((qsword*)(segment->space))[i] = 0;
    }

  segment->freelist = 0;
  qsfreelist_t * freelist = (qsfreelist_t*)(segment->space + segment->freelist);
  freelist->prev = QSFREE_SENTINEL;
  freelist->next = QSFREE_SENTINEL;
  freelist->length = segment->cap;

  return segment;
}

/* Convert freelist node into an allocated object. */
int _qssegment_unfree (qssegment_t * segment, qsaddr_t local_addr)
{
  /* prev's next points to my next, and next's prev points to my prev. */
  qsfreelist_t *prevnode = NULL, *currnode = NULL, *nextnode = NULL;
  currnode = (qsfreelist_t*)(segment->space + local_addr);
  if (currnode->mgmt & MGMT_MASK_USED)  /* validity check. */
    return -1;

  /* Establish previous and next nodes. */
  if (currnode->prev != QSFREE_SENTINEL)
    prevnode = (qsfreelist_t*)(segment->space + currnode->prev);
  if (currnode->next != QSFREE_SENTINEL)
    nextnode = (qsfreelist_t*)(segment->space + currnode->next);

  if (prevnode)
    prevnode->next = currnode->next; /* skip unfreed node. */
  else
    segment->freelist = currnode->next; /* update start of freelist. */

  if (nextnode)
    nextnode->prev = currnode->prev;  /* skip unfreed node. */

  /* Mark not freelist. */
  currnode->mgmt |= MGMT_MASK_USED;
  return 0;
}

/* Split freelist node into two nodes, at the specified boundary.
   Returns second node address (unmapped address).

   One of these segments expected to be subjected to qssegment_unfree().
 */
qsaddr_t _qssegment_split (qssegment_t * segment, qsaddr_t local_addr, qsword nth_boundary)
{
  qsfreelist_t * nextnode = NULL;
  qsfreelist_t * currnode = (qsfreelist_t*)(segment->space + local_addr);
  /* TODO: validity check? */

  qsword spanbytes = (nth_boundary * sizeof(qsobj_t));
  /* TODO: bounds check? */
  if (spanbytes > currnode->length)
    return QSFREE_SENTINEL;

  nextnode = currnode + nth_boundary;
  nextnode->next = currnode->next;
  nextnode->prev = local_addr;
  nextnode->length = currnode->length - spanbytes;
  currnode->length = spanbytes;
  currnode->next = local_addr + spanbytes;

  qsaddr_t retval = currnode->next;
  return retval;
}

/* find suitable freelist node that can fit the requested number of bounds.
   Returns address of the candidate note (unmapped address).

   Expected to be subjected to qssegment_split() immediately after.
 */
qsaddr_t _qssegment_fit (qssegment_t * segment, qsword spanbounds)
{
  qsfreelist_t * scannode = NULL;
  qsaddr_t scan = segment->freelist;
  while (scan != QSFREE_SENTINEL)
    {
      /* TODO: check bounds of 'scan'? */
      scannode = (qsfreelist_t*)(segment->space + scan);
      qsword nbounds = scannode->length / sizeof(qsobj_t);
      if (spanbounds <= nbounds)
	return scan;
      scan = scannode->next;
    }
  return QSFREE_SENTINEL;
}

int _qssegment_stat (qssegment_t * segment, qsaddr_t addr, qsword ** out_mgmtptr, qsword * out_width)
{
  qsword width = sizeof(qsobj_t);
  qsword * refmgmt = (qsword*)(segment->space + addr);
  if (MGMT_IS_USED(*refmgmt))
    {
      width = (1 << MGMT_GET_ALLOC(*refmgmt)) * sizeof(qsobj_t);
    }
  else
    {
      qsfreelist_t * freelist = (qsfreelist_t*)(segment->space + addr);
      width = freelist->length;
    }
  if (out_width) *out_width =  width;
  if (out_mgmtptr) *out_mgmtptr = refmgmt;
  return 0;
}

/* Visit memory boundary for sweeping.
Given: address of most recent freelist,
       address of current visit
Returns: address of most-recent freelist.
       (typ. returns prev for merged freelist, curr for disjointed freelist)
*/
typedef qsaddr_t (*gcvisitor)(qssegment_t *, qsaddr_t, qsaddr_t);

qsaddr_t _qssegment_visit_free (qssegment_t * segment, qsaddr_t prevfree, qsaddr_t curr)
{
  qsword width, *refmgmt;
  _qssegment_stat(segment, curr, &refmgmt, &width);
  qsfreelist_t * prevfreelist = NULL;
  qsfreelist_t * currfreelist = NULL;
  if (MGMT_IS_USED(*refmgmt))
    {
      if (MGMT_IS_MARK(*refmgmt))
	{
	  /* no freeing; pass over. */
	  return prevfree;
	}
      else
	{
	  /* collect, convert to free list, coalesce freelist. */
	  qsword allocscale = MGMT_GET_ALLOC(*refmgmt);
	  width = (1 << allocscale) * sizeof(qsobj_t);
	  if (allocscale > 0)
	    {
	      /* vector: if octetate, check for refcount. */
	      if (MGMT_IS_OCT(*refmgmt))
		{
		  /* octet-vector, check refcount */
		  qsobj_t * obj = (qsobj_t*)(segment->space + curr);
		  qsptr_t refcount = obj->fields[2];
		  if (ISINT30(refcount) && (CINT30(refcount) > 0))
		    {
		      /* unmarked, but with references - do not sweep. */
		      return prevfree;
		    }
		}
	      /* else pointer-vector. */
	    }
	  MGMT_CLR_USED(*refmgmt);
	  currfreelist = (qsfreelist_t*)(segment->space + curr);
	  currfreelist->length = width;
	  /* initialize with sane values, link into freelist later. */
	  currfreelist->prev = QSFREE_SENTINEL;
	  currfreelist->next = QSFREE_SENTINEL;
	}
    }

  if (! MGMT_IS_USED(*refmgmt)) /* may have been just freed. */
    {
      /* freelist, check for coalesce. */
      if (prevfree != QSFREE_SENTINEL)
	{
	  /* previous exists - link freelist. */
	  qsword prevwidth, *prevmgmt;
	  _qssegment_stat(segment, prevfree, &prevmgmt, &prevwidth);
	  prevfreelist = (qsfreelist_t*)(segment->space + prevfree);
	  currfreelist = (qsfreelist_t*)(segment->space + curr);
	  if (prevfree+prevwidth == curr)
	    {
	      /* coalesce into previous freelist. */
	      prevfreelist->length += currfreelist->length;
	      currfreelist->mgmt = 0;
	      currfreelist->length = 0;
	      currfreelist->prev = 0;
	      currfreelist->next = 0;
	      /* preserve next? */
	      /* repeated coalescence on next cycle. */
	      return prevfree;  /* returned coalesced freelist. */
	    }
	  else
	    {
	      /* link into freelist. */
	      currfreelist->next = prevfreelist->next;
	      prevfreelist->next = curr;
	      currfreelist->prev = prevfree;
	    }
	}
      else
	{
	  /* no previous - start of freelist. */
	  /* handle assignment outside of visitor. */
	}
    }

  return curr;
}


int qssegment_sweep (qssegment_t * segment)
{
  qsaddr_t skip;
  qsaddr_t prev, curr;
  skip = 0;
  prev = QSFREE_SENTINEL;
  bool startfree = true;
  curr = 0;
  gcvisitor visitor = _qssegment_visit_free;
  while ((curr != QSFREE_SENTINEL) && (curr < segment->cap))
    {
      qsword *refmgmt;
      _qssegment_stat(segment, curr, &refmgmt, &skip);
      prev = visitor(segment, prev, curr);
      if (startfree && (prev != QSFREE_SENTINEL))
	{
	  segment->freelist = curr;
	  startfree = false;
	}
      curr += skip;
    }
  return 0;
}



qsstore_t * qsstore_init (qsstore_t * store)
{
  memset(store, 0, sizeof(qsstore_t));
  qssegment_init(&(store->smem), 0, SMEM_SIZE);
  return store;
}

qsstore_t * qsstore_destroy (qsstore_t * store)
{
  return store;
}


qssegment_t * _qsstore_get_mem (qsstore_t * store, qsaddr_t addr)
{
  if ((store->smem.baseaddr <= addr)
      && (addr < store->smem.baseaddr + store->smem.cap))
    return &(store->smem);
  if (store->wmem
      && (store->wmem->baseaddr <= addr)
      && (addr < store->wmem->baseaddr + store->wmem->cap))
    return store->wmem;
  return NULL;
}

const qssegment_t * _qsstore_get_mem_const (const qsstore_t * store, qsaddr_t addr)
{
  if ((store->smem.baseaddr <= addr)
      && (addr < store->smem.baseaddr + store->smem.cap))
    return &(store->smem);
  if (store->wmem
      && (store->wmem->baseaddr <= addr)
      && (addr < store->wmem->baseaddr + store->wmem->cap))
    return store->wmem;
  if (store->rmem
      && (store->rmem->baseaddr <= addr)
      && (addr < store->rmem->baseaddr + store->rmem->cap))
    return store->rmem;
  return NULL;
}


#define qssegment_byte_at(store,addr)  ((qsbyte*)(segment->space + addr))
#define qssegment_word_at(store,addr)  ((qsword*)(segment->space + addr))


qsbyte qsstore_get_byte (const qsstore_t * store, qsaddr_t addr)
{
  const qssegment_t * segment = _qsstore_get_mem_const(store, addr);
  if (! segment)
    return 0;
  return *(qssegment_byte_at(segment, addr));
}

qsword qsstore_get_word (const qsstore_t * store, qsaddr_t addr)
{
  addr &= ~(0x3);  /* floor to word boundary. */
  const qssegment_t * segment = _qsstore_get_mem_const(store, addr);
  if (! segment)
    return 0;
  return *(qssegment_word_at(segment,addr));
}

qsword qsstore_fetch_words (const qsstore_t * store, qsaddr_t addr, qsword * dest, qsword word_count)
{
  /* TODO */
  return 0;
}

const qsword * qsstore_word_at_const (const qsstore_t * store, qsaddr_t addr)
{
  addr &= ~(0x3);  /* floor to word boundary. */
  const qssegment_t * segment = _qsstore_get_mem_const(store, addr);
  if (segment != NULL)
    {
      return qssegment_word_at(store, addr);
    }
  return NULL;
}


qserr_t qsstore_attach_wmem (qsstore_t * store, qssegment_t * wmem, qsaddr_t baseaddr)
{
  store->wmem = wmem;
  return QSERR_OK;
}

qserr_t qsstore_attach_rmem (qsstore_t * store, const qssegment_t * rmem, qsaddr_t baseaddr)
{
  store->rmem = rmem;
  return QSERR_OK;
}


qserr_t qsstore_set_byte (qsstore_t * store, qsaddr_t addr, qsbyte val)
{
  qssegment_t * segment = _qsstore_get_mem(store, addr);
  if (segment != NULL)
    {
      *(qssegment_byte_at(store, addr)) = val;
      return QSERR_OK;
    }
  return QSERR_FAULT;
}

qserr_t qsstore_set_word (qsstore_t * store, qsaddr_t addr, qsword val)
{
  addr &= ~(0x3);  /* floor to word boundary. */
  qssegment_t * segment = _qsstore_get_mem(store, addr);
  if (segment != NULL)
    {
      *(qssegment_word_at(store, addr)) = val;
      return QSERR_OK;
    }
  return QSERR_FAULT;
}

qsword qsstore_put_words (qsstore_t * store, qsaddr_t addr, qsword * src, qsword count)
{
  /* TODO */
  return 0;
}

qsword * qsstore_word_at (qsstore_t * store, qsaddr_t addr)
{
  addr &= ~(0x3);  /* floor to word boundary. */
  qssegment_t * segment = _qsstore_get_mem(store, addr);
  if (segment != NULL)
    {
      return qssegment_word_at(store, addr);
    }
  return NULL;
}


/* Memory allocation. */
/* Preferentially allocate in working memory, fallback to start memory. */

/* Allocate by power-of-two boundaries. */
/*
  0 = single-boundary (16o) region
  1 = 2-boundary (32o) region
  2 = 4-boundary (64o) region
  3 = 8-boudnary (128o) region
   etc.
*/
qserr_t qsstore_alloc (qsstore_t * store, qsword allocscale, qsaddr_t * out_addr)
{
  qsword allocsize = 1 << allocscale;
  qssegment_t * segment = NULL;
  qsaddr_t fit = QSFREE_SENTINEL;

  /* Try wmem, fall back to smem. */
  segment = store->wmem;
  fit = segment ? _qssegment_fit(segment, allocsize) : QSFREE_SENTINEL;
  if (fit == QSFREE_SENTINEL)
    segment = &(store->smem);
  fit = segment ? _qssegment_fit(segment, allocsize) : QSFREE_SENTINEL;
  if (fit == QSFREE_SENTINEL)
    return QSERR_NOMEM;

  /* split up freelist and claim memory. */
  qsaddr_t other = _qssegment_split(segment, fit, allocsize);
  if (other == QSFREE_SENTINEL)
    return QSERR_NOMEM;
  int res = _qssegment_unfree(segment, fit);
  if (res != 0)
    return QSERR_NOMEM;

  /* update mgmt word. */
  qsobj_t * obj = (qsobj_t*)(segment->space + fit);
  obj->mgmt = TAG_SYNC29;
  MGMT_SET_ALLOC(obj->mgmt, allocscale);

  /* TODO: disallow loss of handle to allocated memory? */
  if (out_addr)
    *out_addr = fit + segment->baseaddr;

  return QSERR_OK;
}

qserr_t qsstore_alloc_nbounds (qsstore_t * store, qsword nbounds, qsaddr_t * out_addr)
{
  int nbits = 0;
  if (nbounds > 0) nbounds--;
  while (nbounds > 0)
    {
      nbits++;
      nbounds >>= 1;
    }
  return qsstore_alloc(store, nbits, out_addr);
}

qserr_t qsstore_alloc_nwords (qsstore_t * store, qsword nwords, qsaddr_t * out_addr)
{
  int words_per_bounds = sizeof(qsobj_t) / sizeof(qsword);
  int nbounds = (nwords > 0) ? ((nwords - 1) / words_per_bounds) + 1 : 0;
  nbounds += 1;  /* header */
  return qsstore_alloc_nbounds(store, nbounds, out_addr);
}

qserr_t qsstore_alloc_nbytes (qsstore_t * store, qsword nbytes, qsaddr_t * out_addr)
{
  int bytes_per_bounds = sizeof(qsobj_t) / sizeof(qsbyte);
  int nbounds = (nbytes > 0) ? ((nbytes - 1) / bytes_per_bounds) + 1 : 0;
  nbounds += 1;  /* header */
  return qsstore_alloc_nbounds(store, nbounds, out_addr);
}



/* Garbage collection. */

qserr_t qsstore_trace (qsstore_t * store, qsaddr_t root, int mark)
{
  if (root == QSFREE_SENTINEL)
    return QSERR_OK;

  qsobj_t * obj = NULL;
  qsaddr_t curr = root;
  qsaddr_t prev = QSFREE_SENTINEL;

  while (curr != QSFREE_SENTINEL)
    {
      if (_qsstore_get_mem_const(store, curr) == store->rmem)
	{
	  /* immutable memory, no garbage possible; backtrack. */
	  curr = QSFREE_SENTINEL;
	  obj = NULL;
	}
      else
	{
	  obj = (qsobj_t*)(qsstore_word_at(store, curr));
	}

      if (obj)
	{
	  if (MGMT_IS_MARK(obj->mgmt))
	    {
	      /* already marked, start backtracking. */
	      curr = QSFREE_SENTINEL;
	      obj = NULL;
	    }
	  else if (! MGMT_IS_USED(obj->mgmt))
	    {
	      /* free list, backtrack. */
	      curr = QSFREE_SENTINEL;
	      obj = NULL;
	    }
	  else if (MGMT_GET_ALLOC(obj->mgmt) == 0)
	    {
	      /* single-bounded. */
	      if (MGMT_IS_OCT(obj->mgmt))
		{
		  /* trace wide-word. */
		  curr = QSFREE_SENTINEL;
		  obj = NULL;
		}
	      else
		{
		  /* trace triplet. */
		  int reverse = MGMT_GET_REVERS(obj->mgmt);
		  qsptr_t parent = prev;
#define LD(ptr)  ((COBJ26(ptr)) << 4)
#define ST(addr) ((QSOBJ(addr >> 4)))
		  /* normalize parent. */
		  switch (reverse)
		    {
		    case 0:
		      parent = prev;
		      break;
		    case 1:
		      parent = LD(obj->fields[0]);
		      obj->fields[0] = ST(prev);
		      break;
		    case 2:
		      parent = LD(obj->fields[1]);
		      obj->fields[1] = ST(prev);
		      break;
		    case 3:
		    default:
		      parent = LD(obj->fields[2]);
		      obj->fields[2] = ST(prev);
		      break;
		    }

		  /* determine next descent. */
		  switch (reverse)
		    {
		    case 0: /* left ... */
		      if (ISOBJ26(obj->fields[0]))
			{
			  MGMT_SET_REVERS(obj->mgmt, 1);
			  prev = curr;			/* parent of descent. */
			  curr = LD(obj->fields[0]);    /* next descent. */
			  obj->fields[0] = ST(parent);  /* reversal. */
			  break;
			}
		      /* fallthrough. */
		    case 1: /* center ... */
		      if (ISOBJ26(obj->fields[1]))
			{
			  MGMT_SET_REVERS(obj->mgmt, 2);
			  prev = curr;		        /* parent of descent. */
			  curr = LD(obj->fields[1]);    /* next descent. */
			  obj->fields[1] = ST(parent);  /* reversal. */
			  break;
			}
		      /* fallthrough. */
		    case 2: /* right ... */
		      if (ISOBJ26(obj->fields[2]))
			{
			  MGMT_SET_REVERS(obj->mgmt, 3);
			  prev = curr;			/* parent of descent. */
			  curr = LD(obj->fields[2]);    /* next descent. */
			  obj->fields[2] = ST(parent);	/* reversal. */
			  break;
			}
		      /* fallthrough. */
		    case 3: /* done. */
		      MGMT_SET_REVERS(obj->mgmt, 0);
		      MGMT_SET_MARK(obj->mgmt);
		      prev = curr;		  /* true child. */
		      curr = parent;		  /* true parent. */
		      break;
		    }
		}
	    }
	  else if (MGMT_GET_ALLOC(obj->mgmt) > 0)
	    {
	      /* cross-boundary. */
	      if (MGMT_IS_OCT(obj->mgmt))
		{
		  /* trace byte-vector. */
		  MGMT_SET_MARK(obj->mgmt);
		}
	      else
		{
		  const qsword ptrs_per_bound = sizeof(qsobj_t) / sizeof(qsptr_t);
		  qsword max = (1 << qsobj_get_allocscale(obj)) * ptrs_per_bound;
		  qsword ofs;
		  qsaddr_t eltaddr;
		  qsptr_t elt;
		  /* trace word-vector. */
		  qsptr_t gcback = obj->fields[2];
		  if (ISNIL(gcback))
		    {
		      /* start iteration. */
		      obj->fields[2] = prev;
		      ofs = 0;
		    }
		  else
		    {
		      /* reversal pointer established, continue iteration. */
		      ofs = CINT30(obj->fields[3]);
		      prev = gcback;
		    }
		  if (ofs < max)
		    {
		      /* in range, recurse. */
		      eltaddr = (curr + sizeof(qsobj_t)) + ofs * sizeof(qsptr_t);
		      elt = (qsptr_t)(qsstore_get_word(store, eltaddr));
		      if (ISOBJ26(elt))
			{
			  prev = curr;
			  curr = LD(elt);
			}
		      ++ofs;
		      obj->fields[3] = QSINT(ofs);
		    }
		  else
		    {
		      /* Out of bounds.  Terminate.  Backtrack. */
		      obj->fields[2] = QSNIL;
		      obj->fields[3] = QSNIL;
		      MGMT_SET_MARK(obj->mgmt);
		      curr = gcback;
		    }
		}
	    }
	}
    }
  return QSERR_OK;
}

qserr_t qsstore_sweep (qsstore_t * store)
{
  qssegment_sweep(&(store->smem));
  if (store->wmem)
    qssegment_sweep(store->wmem);
  return QSERR_OK;
}


