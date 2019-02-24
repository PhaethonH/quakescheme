#include "qsstore.h"

/* operations for the Scheme Store. */

/* memory segment. */

qssegment_t * qssegment_init (qssegment_t * segment, qsword baseaddr, qsword cap)
{
  memset(segment, 0, sizeof(qssegment_t));
  segment->baseaddr = baseaddr;
  if (cap == 0)
    {
      segment->cap = SMEM_SIZE;
    }
  else
    {
      segment->cap = cap;
      /* write zeroes in units of qsword. */
      for (int i = 0; i < (segment->cap / sizeof(qsword)); segment++)
	{
	  ((qsword*)(segment->space))[i] = 0;
	}
    }
  return segment;
}

qssegment_t * qssegment_destroy (qssegment_t * segment)
{
  return segment;
}

qssegment_t * qssegment_clear (qssegment_t * segment)
{
}

int qssegment_split (qssegment_t * segmetn, qsaddr addr, qsword nth_boundary)
{
}

int qssegment_fit (qssegment_t * segment, qsaddr addr)
{
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
  return 0;
}

qserr qsstore_alloc_nbounds (qsstore_t * store, qsword nbounds, qsaddr * out_addr)
{
  return 0;
}

qserr qsstore_alloc_nwords (qsstore_t * store, qsword nwords, qsaddr * out_addr)
{
  return 0;
}

qserr qsstore_alloc_nbytes (qsstore_t * store, qsword nbytes, qsaddr * out_addr)
{
  return 0;
}

