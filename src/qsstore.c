#include "qsstore.h"

/* operations for the Scheme Store. */

/* memory segment. */

qsmem_t * qsmem_init (qsmem_t * mem, qsword baseaddr, qsword cap)
{
  memset(mem, 0, sizeof(qsmem_t));
  mem->baseaddr = baseaddr;
  if (cap == 0)
    {
      mem->cap = SMEM_SIZE;
    }
  else
    {
      mem->cap = cap;
      /* write zeroes in units of qsword. */
      for (int i = 0; i < (mem->cap / sizeof(qsword)); mem++)
	{
	  ((qsword*)(mem->space))[i] = 0;
	}
    }
  return mem;
}

qsmem_t * qsmem_destroy (qsmem_t * mem)
{
  return mem;
}




qsstore_t * qsstore_init (qsstore_t * store)
{
  memset(store, 0, sizeof(qsstore_t));
  qsmem_init(&(store->smem), 0, SMEM_SIZE);
  return store;
}

qsstore_t * qsstore_destroy (qsstore_t * store)
{
  return store;
}


qsmem_t * _qsstore_get_mem (qsstore_t * store, qsword addr)
{
  if ((store->smem.baseaddr <= addr)
      && (addr < store->smem.baseaddr + store->smem.cap))
    return &(store->smem);
  if ((store->wmem->baseaddr <= addr)
      && (addr < store->wmem->baseaddr + store->wmem->cap))
    return store->wmem;
  return NULL;
}

const qsmem_t * _qsstore_get_mem_const (const qsstore_t * store, qsword addr)
{
  if ((store->smem.baseaddr <= addr)
      && (addr < store->smem.baseaddr + store->smem.cap))
    return &(store->smem);
  if ((store->wmem->baseaddr <= addr)
      && (addr < store->wmem->baseaddr + store->wmem->cap))
    return store->wmem;
  if ((store->rmem->baseaddr <= addr)
      && (addr < store->rmem->baseaddr + store->rmem->cap))
    return store->rmem;
  return NULL;
}


#define qsmem_byte_at(store,addr)  ((qsbyte*)(mem->space + addr))
#define qsmem_word_at(store,addr)  ((qsword*)(mem->space + addr))


qsbyte qsstore_get_byte (const qsstore_t * store, qsword addr)
{
  const qsmem_t * mem = _qsstore_get_mem_const(store, addr);
  if (! mem)
    return 0;
  return *(qsmem_byte_at(mem, addr));
}

qsword qsstore_get_word (const qsstore_t * store, qsword addr)
{
  addr &= ~(0x3);  /* floor to word boundary. */
  const qsmem_t * mem = _qsstore_get_mem_const(store, addr);
  if (! mem)
    return 0;
  return *(qsmem_word_at(mem,addr));
}

qsword qsstore_fetch_words (const qsstore_t * store, qsword addr, qsword * dest, qsword word_count)
{
  return 0;
}

const qsword * qsstore_word_at_const (const qsstore_t * store, qsword addr)
{
  addr &= ~(0x3);  /* floor to word boundary. */
  const qsmem_t * mem = _qsstore_get_mem_const(store, addr);
  if (mem != NULL)
    {
      return qsmem_word_at(store, addr);
    }
}


void qsstore_set_byte (qsstore_t * store, qsword addr, qsbyte val)
{
  qsmem_t * mem = _qsstore_get_mem(store, addr);
  if (mem != NULL)
    {
      *(qsmem_byte_at(store, addr)) = val;
    }
  return;
}

void qsstore_set_word (qsstore_t * store, qsword addr, qsword val)
{
  addr &= ~(0x3);  /* floor to word boundary. */
  qsmem_t * mem = _qsstore_get_mem(store, addr);
  if (mem != NULL)
    {
      *(qsmem_word_at(store, addr)) = val;
    }
  return;
}

qsword qsstore_put_words (qsstore_t * store, qsword addr, qsword * src, qsword count)
{
  return 0;
}

qsword * qsstore_word_at (qsstore_t * store, qsword addr)
{
  addr &= ~(0x3);  /* floor to word boundary. */
  qsmem_t * mem = _qsstore_get_mem(store, addr);
  if (mem != NULL)
    {
      return qsmem_word_at(store, addr);
    }
}

