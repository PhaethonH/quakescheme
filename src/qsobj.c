#include "qsobj.h"



qsobj_t * qsobj (qsmem_t * mem, qsptr_t p, qsmemaddr_t * out_addr)
{
  if (!ISHEAP26(p)) return NULL;
  qsmemaddr_t addr = COBJ26(p);
  qsobj_t * retval = (qsobj_t*)qsheap_ref(mem, addr);
  if (out_addr)
    *out_addr = addr;
  return retval;
}




qspair_t * qspair (qsmem_t * mem, qsptr_t p)
{
  qsobj_t * obj = qsobj(mem, p, NULL);
  if (!qsobj_is_used(obj)) return NULL;
  if (qsobj_is_octet(obj)) return NULL;
  if (qsobj_get(obj, 0) != QSNULL) return NULL;
  return (qspair_t *)obj;
}

qsptr_t qspair_ref_a (qsmem_t * mem, qsptr_t p)
{
  qspair_t * pair = qspair(mem, p);
  if (!pair)
    {
      /* TODO: exception. */
      return QSNULL;
    }
  return pair->a;
}

qsptr_t qspair_ref_d (qsmem_t * mem, qsptr_t p)
{
  qspair_t * pair = qspair(mem, p);
  if (!pair)
    {
      /* TODO: exception. */
      return QSNULL;
    }
  return pair->d;
}

qsptr_t qspair_setq_a (qsmem_t * mem, qsptr_t p, qsptr_t val)
{
  qspair_t * pair = qspair(mem, p);
  if (!pair)
    {
      /* TODO: exception. */
      return QSNULL;
    }
  pair->a = val;
  return p;
}

qsptr_t qspair_setq_d (qsmem_t * mem, qsptr_t p, qsptr_t val)
{
  qspair_t * pair = qspair(mem, p);
  if (!pair)
    {
      /* TODO: exception. */
      return QSNULL;
    }
  pair->d = val;
  return p;
}

int qspair_alloc (qsmem_t * mem, qsptr_t * out_ptr, qsmemaddr_t * out_addr)
{
  return 0;
}

qsptr_t qspair_make (qsmem_t * mem, qsptr_t a, qsptr_t b)
{
  qsptr_t retval;
  int res = qspair_alloc(mem, &retval, NULL);
  if (res == 1)
    {
      return retval;
    }
  return QSNULL;
}

int qspair_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen)
{
  return 0;
}




qsvector_t * qsvector (qsmem_t * mem, qsptr_t v, qsword * out_lim)
{
  qsobj_t * obj = qsobj(mem, v, NULL);
  if (qsobj_is_octet(obj)) return NULL;
  if (! ISINT30(qsobj_get(obj, 0))) return NULL;
  if (out_lim)
    {
      *out_lim = CINT30(qsobj_get(obj, 0));
    }
  return (qsvector_t*)obj;
}

qsword qsvector_length (qsmem_t * mem, qsptr_t v)
{
  qsword len = 0;
  qsvector_t * vec = qsvector(mem, v, &len);
  if (!vec) return 0;  // TODO: exception
  return len;
}

qsptr_t qsvector_ref (qsmem_t * mem, qsptr_t v, qsword ofs)
{
  qsword lim;
  qsvector_t * vec = qsvector(mem, v, &lim);
  if (!vec) return QSNULL;
  if ((lim < 0) || (lim >= vec->len))
    {
      // TODO: exception.
      return QSNULL;
    }
  return vec->_d[ofs];
}

qsptr_t qsvector_setq (qsmem_t * mem, qsptr_t v, qsword ofs, qsptr_t val)
{
  qsword lim;
  qsvector_t * vec = qsvector(mem, v, &lim);
  if (!vec) return QSNULL;
  if ((lim < 0) || (lim >= vec->len))
    {
      // TODO: exception.
      return QSNULL;
    }
  vec->_d[ofs] = val;
  return v;
}

int qsvector_alloc (qsmem_t * mem, qsptr_t * out_ptr, qsmemaddr_t * out_addr, qsword cap)
{
  return 0;
}

qsptr_t qsvector_make (qsmem_t * mem, qsword k, qsptr_t fill)
{
  return QSNULL;
}

int qsvector_crepr (qsmem_t * mem, qsptr_t v, char * buf, int buflen)
{
  return 0;
}



