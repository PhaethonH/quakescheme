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
int _qssegment_unfree (qssegment_t * segment, qsaddr local_addr)
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
qsaddr _qssegment_split (qssegment_t * segment, qsaddr local_addr, qsword nth_boundary)
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

  qsaddr retval = currnode->next;
  return retval;
}

/* find suitable freelist node that can fit the requested number of bounds.
   Returns address of the candidate note (unmapped address).

   Expected to be subjected to qssegment_split() immediately after.
 */
qsaddr _qssegment_fit (qssegment_t * segment, qsword spanbounds)
{
  qsfreelist_t * scannode = NULL;
  qsaddr scan = segment->freelist;
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


qssegment_t * _qsstore_get_mem (qsstore_t * store, qsaddr addr)
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

const qssegment_t * _qsstore_get_mem_const (const qsstore_t * store, qsaddr addr)
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


qsbyte qsstore_get_byte (const qsstore_t * store, qsaddr addr)
{
  const qssegment_t * segment = _qsstore_get_mem_const(store, addr);
  if (! segment)
    return 0;
  return *(qssegment_byte_at(segment, addr));
}

qsword qsstore_get_word (const qsstore_t * store, qsaddr addr)
{
  addr &= ~(0x3);  /* floor to word boundary. */
  const qssegment_t * segment = _qsstore_get_mem_const(store, addr);
  if (! segment)
    return 0;
  return *(qssegment_word_at(segment,addr));
}

qsword qsstore_fetch_words (const qsstore_t * store, qsaddr addr, qsword * dest, qsword word_count)
{
  /* TODO */
  return 0;
}

const qsword * qsstore_word_at_const (const qsstore_t * store, qsaddr addr)
{
  addr &= ~(0x3);  /* floor to word boundary. */
  const qssegment_t * segment = _qsstore_get_mem_const(store, addr);
  if (segment != NULL)
    {
      return qssegment_word_at(store, addr);
    }
  return NULL;
}


qserr qsstore_attach_wmem (qsstore_t * store, qssegment_t * wmem, qsaddr baseaddr)
{
  store->wmem = wmem;
  return QSERR_OK;
}

qserr qsstore_attach_rmem (qsstore_t * store, const qssegment_t * rmem, qsaddr baseaddr)
{
  store->rmem = rmem;
  return QSERR_OK;
}


qserr qsstore_set_byte (qsstore_t * store, qsaddr addr, qsbyte val)
{
  qssegment_t * segment = _qsstore_get_mem(store, addr);
  if (segment != NULL)
    {
      *(qssegment_byte_at(store, addr)) = val;
      return QSERR_OK;
    }
  return QSERR_FAULT;
}

qserr qsstore_set_word (qsstore_t * store, qsaddr addr, qsword val)
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

qsword qsstore_put_words (qsstore_t * store, qsaddr addr, qsword * src, qsword count)
{
  /* TODO */
  return 0;
}

qsword * qsstore_word_at (qsstore_t * store, qsaddr addr)
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
qserr qsstore_alloc (qsstore_t * store, qsword allocscale, qsaddr * out_addr)
{
  qsword allocsize = 1 << allocscale;
  qssegment_t * segment = NULL;
  qsaddr fit = QSFREE_SENTINEL;

  /* Try wmem, fall back to smem. */
  segment = store->wmem;
  fit = segment ? _qssegment_fit(segment, allocsize) : QSFREE_SENTINEL;
  if (fit == QSFREE_SENTINEL)
    segment = &(store->smem);
  fit = segment ? _qssegment_fit(segment, allocsize) : QSFREE_SENTINEL;
  if (fit == QSFREE_SENTINEL)
    return QSERR_NOMEM;

  /* split up freelist and claim memory. */
  qsaddr other = _qssegment_split(segment, fit, allocsize);
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

qserr qsstore_alloc_nbounds (qsstore_t * store, qsword nbounds, qsaddr * out_addr)
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

qserr qsstore_alloc_nwords (qsstore_t * store, qsword nwords, qsaddr * out_addr)
{
  int words_per_bounds = sizeof(qsobj_t) / sizeof(qsword);
  int nbounds = (nwords > 0) ? ((nwords - 1) / words_per_bounds) + 1 : 0;
  nbounds += 1;  /* header */
  return qsstore_alloc_nbounds(store, nbounds, out_addr);
}

qserr qsstore_alloc_nbytes (qsstore_t * store, qsword nbytes, qsaddr * out_addr)
{
  int bytes_per_bounds = sizeof(qsobj_t) / sizeof(qsbyte);
  int nbounds = (nbytes > 0) ? ((nbytes - 1) / bytes_per_bounds) + 1 : 0;
  nbounds += 1;  /* header */
  return qsstore_alloc_nbounds(store, nbounds, out_addr);
}



/* Garbage collection. */

qserr qsstore_trace (qsstore_t * store, qsaddr root, int mark)
{
  if (root == QSFREE_SENTINEL)
    return QSERR_OK;

  qsobj_t * obj = NULL;
  qsaddr curr = root;
  qsaddr prev = QSFREE_SENTINEL;

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
		  qsptr temp;
		  qsptr parent = prev;
#define LD(ptr)  ((COBJ26(ptr)) << 4)
#define ST(addr) ((QSOBJ(addr)) >> 4)
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
		      prev = curr;		  /* true child. */
		      curr = parent;		  /* true parent. */
		      MGMT_SET_MARK(obj->mgmt);
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
		  qsword max = 1 << qsobj_get_allocscale(obj);
		  qsword ofs;
		  qsaddr eltaddr;
		  qsptr elt;
		  /* trace word-vector. */
		  if (obj->fields[2] == QSNIL)
		    {
		      /* start iteration. */
		      obj->fields[2] = prev;
		      ofs = 0;
		    }
		  else
		    {
		      ofs = CINT30(obj->fields[3]);
		    }
		  eltaddr = (curr + sizeof(qsobj_t)) + ofs * sizeof(qsptr);
		  elt = (qsptr)(qsstore_get_word(store, eltaddr));
		  if (ISOBJ26(elt))
		    {
		      prev = curr;
		      curr = LD(elt);
		    }
		  ++ofs;
		  if (ofs < max)
		    {
		      obj->fields[3] = QSINT(ofs);
		    }
		  else
		    {
		      /* done. */
		      obj->fields[2] = QSNIL;
		      obj->fields[3] = QSNIL;
		      MGMT_SET_MARK(obj->mgmt);
		    }
		}
	    }
	}
    }
  return QSERR_OK;
}


