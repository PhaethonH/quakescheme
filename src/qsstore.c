#include "qsstore.h"

/* operations for the Scheme Store. */

/* memory segment. */

qsmem_t * qsmem_init (qsmem_t * mem)
{
  return mem;
}

qsmem_t * qsmem_destroy (qsmem_t * mem)
{
  return mem;
}




qsstore_t * qsstore_init (qsstore_t * store)
{
  return store;
}

qsstore_t * qsstore_destroy (qsstore_t * store)
{
  return store;
}

qsbyte qsstore_get_byte (const qsstore_t * store, qsword addr)
{
  return 0;
}

qsword qsstore_get_word (const qsstore_t * store, qsword addr)
{
  return 0;
}

qsword qsstore_fetch_words (const qsstore_t * store, qsword addr, qsword * dest, qsword count)
{
  return 0;
}


void qsstore_set_byte (qsstore_t * store, qsword addr, qsbyte val)
{
  return;
}

void qsstore_set_word (qsstore_t * store, qsword addr, qsword val)
{
  return;
}

qsword qsstore_put_words (qsstore_t * store, qsword addr, qsword * src, qsword count)
{
  return 0;
}

