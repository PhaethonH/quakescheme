#include <math.h>

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




qstree_t * qstree (qsmem_t * mem, qsptr_t t)
{
  qsobj_t * obj = qsobj(mem, t, NULL);
  if (!qsobj_is_used(obj)) return NULL;
  if (qsobj_is_octet(obj)) return NULL;
  return (qstree_t *)obj;
}

qsptr_t qstree_get_left (qsmem_t * mem, qsptr_t t)
{
  qstree_t * tree = qstree(mem, t);
  if (!tree)
    {
      // TODO: exception.
      return QSNIL;
    }
  return tree->left;
}

qsptr_t qstree_get_data (qsmem_t * mem, qsptr_t t)
{
  qstree_t * tree = qstree(mem, t);
  if (!tree)
    {
      // TODO: exception.
      return QSNIL;
    }
  return tree->data;
}

qsptr_t qstree_get_right (qsmem_t * mem, qsptr_t t)
{
  qstree_t * tree = qstree(mem, t);
  if (!tree)
    {
      // TODO: exception.
      return QSNIL;
    }
  return tree->right;
}

qsptr_t qstree_setq_left (qsmem_t * mem, qsptr_t t, qsptr_t val)
{
  qstree_t * tree = qstree(mem, t);
  if (!tree)
    {
      // TODO: exception.
      return QSNIL;
    }
  tree->left = val;
  return t;
}

qsptr_t qstree_setq_data (qsmem_t * mem, qsptr_t t, qsptr_t val)
{
  qstree_t * tree = qstree(mem, t);
  if (!tree)
    {
      // TODO: exception.
      return QSNIL;
    }
  tree->data = val;
  return t;
}

qsptr_t qstree_setq_right (qsmem_t * mem, qsptr_t t, qsptr_t val)
{
  qstree_t * tree = qstree(mem, t);
  if (!tree)
    {
      // TODO: exception.
      return QSNIL;
    }
  tree->right = val;
  return t;
}

qsptr_t qstree_make (qsmem_t * mem, qsptr_t left, qsptr_t data, qsptr_t right)
{
  return QSNIL;
}

int qstree_crepr (qsmem_t * mem, qsptr_t t, char * buf, int buflen)
{
  return 0;
}




qspair_t * qspair (qsmem_t * mem, qsptr_t p)
{
  qsobj_t * obj = qsobj(mem, p, NULL);
  if (!qsobj_is_used(obj)) return NULL;
  if (qsobj_is_octet(obj)) return NULL;
  if (qsobj_get(obj, 0) != QSNIL) return NULL;
  return (qspair_t *)obj;
}

qsptr_t qspair_ref_a (qsmem_t * mem, qsptr_t p)
{
  qspair_t * pair = qspair(mem, p);
  if (!pair)
    {
      /* TODO: exception. */
      return QSNIL;
    }
  return pair->a;
}

qsptr_t qspair_ref_d (qsmem_t * mem, qsptr_t p)
{
  qspair_t * pair = qspair(mem, p);
  if (!pair)
    {
      /* TODO: exception. */
      return QSNIL;
    }
  return pair->d;
}

qsptr_t qspair_setq_a (qsmem_t * mem, qsptr_t p, qsptr_t val)
{
  qspair_t * pair = qspair(mem, p);
  if (!pair)
    {
      /* TODO: exception. */
      return QSNIL;
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
      return QSNIL;
    }
  pair->d = val;
  return p;
}

qserror_t qspair_alloc (qsmem_t * mem, qsptr_t * out_ptr, qsmemaddr_t * out_addr)
{
  return QSERROR_NOIMPL;
}

qsptr_t qspair_make (qsmem_t * mem, qsptr_t a, qsptr_t b)
{
  qsptr_t retval;
  qserror_t err = qspair_alloc(mem, &retval, NULL);
  if (QSERROR_OK == err)
    {
      return retval;
    }
  return QSNIL;
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
  if (!vec) return QSNIL;
  if ((lim < 0) || (lim >= vec->len))
    {
      // TODO: exception.
      return QSNIL;
    }
  return vec->_d[ofs];
}

qsptr_t qsvector_setq (qsmem_t * mem, qsptr_t v, qsword ofs, qsptr_t val)
{
  qsword lim;
  qsvector_t * vec = qsvector(mem, v, &lim);
  if (!vec) return QSNIL;
  if ((lim < 0) || (lim >= vec->len))
    {
      // TODO: exception.
      return QSNIL;
    }
  vec->_d[ofs] = val;
  return v;
}

qserror_t qsvector_alloc (qsmem_t * mem, qsptr_t * out_ptr, qsmemaddr_t * out_addr, qsword cap)
{
  return QSERROR_NOIMPL;
}

qsptr_t qsvector_make (qsmem_t * mem, qsword k, qsptr_t fill)
{
  return QSNIL;
}

int qsvector_crepr (qsmem_t * mem, qsptr_t v, char * buf, int buflen)
{
  return 0;
}








/****************/
/* Wide numbers */
/****************/

qswidenum_t * qswidenum (qsmem_t * mem, qsptr_t n, qsnumtype_t * out_numtype)
{
  if (!ISHEAP26(n)) return NULL;
  qsobj_t * obj = qsobj(mem, n, NULL);
  if (!qsobj_is_used(obj)) return NULL;
  if (!qsobj_is_octet(obj)) return NULL;
  qsptr_t discriminator = obj->_0;
  switch (discriminator)
    {
    case QSNUMTYPE_NAN:
    case QSNUMTYPE_LONG:
    case QSNUMTYPE_DOUBLE:
    case QSNUMTYPE_INT2:
    case QSNUMTYPE_FLOAT2:
      if (out_numtype)
	*out_numtype = discriminator;
      return (qswidenum_t*)obj;
      break;
    default:
      return NULL;
      break;
    }
}

qsnumtype_t qswidenum_variant (qsmem_t * mem, qsptr_t n)
{
  qsnumtype_t variant = QSNUMTYPE_NAN;
  qswidenum_t * wn = qswidenum(mem, n, &variant);
  if (!wn) return QSNUMTYPE_NAN;
  return variant;
}

qswidenum_t * qswidenum_premake (qsmem_t * mem, qsnumtype_t variant, qsptr_t * out_ptr)
{
  switch (variant)
    {
    case QSNUMTYPE_LONG:
      break;
    case QSNUMTYPE_DOUBLE:
      break;
    case QSNUMTYPE_INT2:
      break;
    case QSNUMTYPE_FLOAT2:
      break;
    default:
      break;
    }
  return NULL;
}




qswidenum_t * qslong (qsmem_t * mem, qsptr_t l)
{
  qsnumtype_t variant = QSNUMTYPE_NAN;
  qswidenum_t * wn = qswidenum(mem, l, &variant);
  if (!wn) return NULL;
  if (variant != QSNUMTYPE_LONG) return NULL;
  return wn;
}

qserror_t qslong_fetch (qsmem_t * mem, qsptr_t l, long * out_long)
{
  qswidenum_t * wn = qslong(mem, l);
  if (!wn) return QSERROR_INVALID;
  if (out_long)
    *out_long = wn->payload.l;
  return QSERROR_OK;
}

long qslong_get (qsmem_t * mem, qsptr_t l)
{
  long retval = 0;
  qserror_t err = qslong_fetch(mem, l, &retval);
  if (err)
    return 0;
  return retval;
}

qsptr_t qslong_make (qsmem_t * mem, long val)
{
  qswidenum_t * wn = NULL;
  qsptr_t retval = QSNIL;
  wn = qswidenum_premake(mem, QSNUMTYPE_LONG, &retval);
  if (wn)
    {
      wn->payload.l = val;
      return retval;
    }
  return QSERROR_NOMEM;
}

qsptr_t qslong_make2 (qsmem_t * mem, int32_t high, uint32_t low)
{
  return QSERROR_NOIMPL;
}

int qslong_crepr (qsmem_t * mem, qsptr_t l, char * buf, int buflen)
{
  return 0;
}




qswidenum_t * qsdouble (qsmem_t * mem, qsptr_t d)
{
  qsnumtype_t variant = QSNUMTYPE_NAN;
  qswidenum_t * wn = qswidenum(mem, d, &variant);
  if (!wn) return NULL;
  if (variant != QSNUMTYPE_LONG) return NULL;
  return wn;
}

qserror_t qsdouble_fetch (qsmem_t * mem, qsptr_t d, double * out_double)
{
  qswidenum_t * wn = qsdouble(mem, d);
  if (!wn) return QSERROR_INVALID;
  if (out_double)
    *out_double = wn->payload.d;
  return QSERROR_OK;
}

double qsdouble_get (qsmem_t * mem, qsptr_t d)
{
  double retval = 0;
  qserror_t err = qsdouble_fetch(mem, d, &retval);
  if (err)
    return 0;
  return retval;
}

qsptr_t qsdouble_make (qsmem_t * mem, double val)
{
  qswidenum_t * wn = NULL;
  qsptr_t retval = QSNIL;
  wn = qswidenum_premake(mem, QSNUMTYPE_LONG, &retval);
  if (wn)
    {
      wn->payload.d = val;
      return retval;
    }
  return QSERROR_NOMEM;
}

int qsdouble_crepr (qsmem_t * mem, qsptr_t d, char * buf, int buflen)
{
  return 0;
}








/**********************/
/* Word-sized objects */
/**********************/


/* Iterator pointer, for iterating immlist and vectors-as-list. */
qsptr_t qsiter (qsmem_t * mem, qsptr_t it)
{
  if (ISITER28(it)) return it;
  return QSNIL;
}

qsptr_t qsiter_make (qsmem_t * meme, qsmemaddr_t addr)
{
  // TODO: bounds check.
  return QSITER(addr);
}

qsword qsiter_get (qsmem_t * mem, qsptr_t it)
{
  if (ISITER28(it))
    {
      return CITER28(it);
    }
  // TODO: exception.
  return 0;
}

qsptr_t qsiter_item (qsmem_t * mem, qsptr_t it)
{
  if (ISITER28(it))
    {
      qsword ofs = qsiter_get(mem, it);
      qsptr_t ref = 0;
      if (0 == qsheap_word(mem, ofs, &ref))
	{
	  return ref;
	}
    }
  // TODO: exception.
  return QSNIL;
}

qsptr_t qsiter_next (qsmem_t * mem, qsptr_t it)
{
  if (ISITER28(it))
    {
      qsword nextofs = qsiter_get(mem, it)+1;
      qsptr_t peek = 0;
      qserror_t err = 0;
      int depth = 1;
      while (depth > 0)
	{
	  err = qsheap_word(mem, nextofs, &peek);
	  if (err)
	    {
	      // invalid address, treat as end of iterator.
	      depth = 0;
	      peek = QSEOL;
	      break;
	    }
	  else if (peek == QSEOL)
	    {
	      depth--;
	      // might be end of nested list, keep moving.
	    }
	  else if (peek == QSBOL)
	    {
	      depth++;
	      // beginning of list, seek end of nested list.
	    }
	  else if (ISSYNC29(peek))
	    {
	      // impinging on next object.
	      depth = 0;
	      peek = QSEOL;
	      break;
	    }
	  nextofs++;
	}
      if (peek != QSEOL)  // did not end on end-of-list marker.
	return qsiter_make(mem, nextofs);
    }
  // invalid iterator, or iterator ended on end-of-list.
  return QSNIL;
}




qsptr_t qsint (qsmem_t * mem, qsptr_t q)
{
  if (ISINT30(q)) return q;
  else if (ISFLOAT31(q))
    {
      int casted_int = (int)CFLOAT31(q);
      return QSINT(casted_int);
    }
  else
    {
      // TODO: wide numbers.
      // TODO: exception.
      return QSNIL;
    }
  return QSNIL;
}

int32_t qsint_get (qsmem_t * mem, qsptr_t q)
{
  if (ISINT30(q)) return CINT30(q);
  return 0;
}

qsptr_t qsint_make (qsmem_t * mem, int32_t val)
{
  if ((val < -0x400000000) || (val > 0x3fffffff))
    {
      return QSERROR_RANGE;
    }
  return QSINT(val);
}

int qsint_crepr (qsmem_t * mem, qsptr_t i, char * buf, int buflen)
{
  return 0;
}




qsptr_t qsfloat (qsmem_t * mem, qsptr_t q)
{
  if (ISFLOAT31(q)) return q;
  if (ISINT30(q))
    {
      float casted_float = (float)CINT30(q);
      return QSFLOAT(casted_float);
    }
  else
    {
      // TODO: wide numbers.
      // TODO: exception.
      return QSNIL;
    }
  return QSNIL;
}

float qsfloat_get (qsmem_t * mem, qsptr_t q)
{
  if (ISFLOAT31(q)) return CFLOAT31(q);
  return NAN;
}

qsptr_t qsfloat_make (qsmem_t * mem, float val)
{
  return QSFLOAT(val);
}

int qsfloat_crepr (qsmem_t * mem, qsptr_t f, char * buf, int buflen)
{
  return 0;
}




qsptr_t qschar (qsmem_t * mem, qsptr_t q)
{
  if (ISCHAR24(q)) return q;
  return QSNIL;
}

int qschar_get (qsmem_t * mem, qsptr_t q)
{
  if (ISCHAR24(q)) return CCHAR24(q);
  return 0;
}

qsptr_t qschar_make (qsmem_t * mem, int val)
{
  if ((val < 0) || (val >= (1 << 24)))
    {
      return QSERROR_RANGE;
    }
  return QSCHAR(val);
}

int qschar_crepr (qsmem_t * mem, qsptr_t c, char * buf, int buflen)
{
  return 0;
}




qsptr_t qserr (qsmem_t * mem, qsptr_t e)
{
  if (ISERROR16(e)) return e;
  return QSNIL;
}

int qserr_get (qsmem_t * mem, qsptr_t e)
{
  if (ISERROR16(e)) return CERROR16(e);
  return 0;
}

qsptr_t qserr_make (qsmem_t * mem, int errcode)
{
  return QSERROR(errcode);
}

int qserr_crepr (qsmem_t * mem, qsptr_t c, char * buf, int buflen)
{
  return 0;
}




qsptr_t qsconst (qsmem_t * mem, qsptr_t n)
{
  if (ISCONST16(n)) return n;
  return QSNIL;
}

int qsconst_get (qsmem_t * mem, qsptr_t n)
{
  if (ISCONST16(n)) return CCONST16(n);
  return 0;
}

qsptr_t qsconst_make (qsmem_t * mem, int constcode)
{
  return QSCONST(constcode);
}

int qsconst_crepr (qsmem_t * mem, qsptr_t c, char * buf, int buflen)
{
  return 0;
}




