#include <math.h>

#include "qsobj.h"



/* Attempt to cast to object (else freelist or data) */
qsobj_t * qsobj (qsmem_t * mem, qsptr_t p, qsmemaddr_t * out_addr)
{
  if (!ISHEAP26(p)) return NULL;
  qsmemaddr_t addr = COBJ26(p);
  qsobj_t * retval = (qsobj_t*)qsheap_ref(mem, addr);
  if (!retval) return NULL;
  if (!qsobj_is_used(retval)) return NULL;
  qsptr_t mgmt = retval->mgmt;
  if (!ISSYNC29(mgmt)) return NULL;
  if (out_addr)
    *out_addr = addr;
  return retval;
}

/* Object as single-celled pointer contents. */
qsobj_t * qsobj_unicellular (qsmem_t * mem, qsptr_t p, qsmemaddr_t * out_addr)
{
  qsobj_t * obj = qsobj(mem, p, out_addr);
  if (!obj) return NULL;
  qsheapcell_t * heapcell = (qsheapcell_t*)obj;
  qsptr_t mgmt = heapcell->mgmt;
  if (MGMT_IS_OCTET(mgmt)) return NULL;
  if (MGMT_GET_ALLOCSCALE(mgmt) > 0) return NULL;
  return obj;
}
/* Object as multi-celled pointer contents. */
qsobj_t * qsobj_multicellular (qsmem_t * mem, qsptr_t p, qsmemaddr_t * out_addr)
{
  qsobj_t * obj = qsobj(mem, p, out_addr);
  if (!obj) return NULL;
  qsheapcell_t * heapcell = (qsheapcell_t*)obj;
  qsptr_t mgmt = heapcell->mgmt;
  if (MGMT_IS_OCTET(mgmt)) return NULL;
  if (MGMT_GET_ALLOCSCALE(mgmt) == 0) return NULL;
  return obj;
}
/* Object is single-celled octet contents (uni- octet -ate). */
qsobj_t * qsobj_unioctetate (qsmem_t * mem, qsptr_t p, qsmemaddr_t * out_addr)
{
  qsobj_t * obj = qsobj(mem, p, out_addr);
  if (!obj) return NULL;
  qsheapcell_t * heapcell = (qsheapcell_t*)obj;
  qsptr_t mgmt = heapcell->mgmt;
  if (! MGMT_IS_OCTET(mgmt)) return NULL;
  if (MGMT_GET_ALLOCSCALE(mgmt) > 0) return NULL;
  return obj;
}
/* Object is multi-celled octet contents (multi- octet -ate). */
qsobj_t * qsobj_multioctetate (qsmem_t * mem, qsptr_t p, qsmemaddr_t * out_addr)
{
  qsobj_t * obj = qsobj(mem, p, out_addr);
  if (!obj) return NULL;
  qsheapcell_t * heapcell = (qsheapcell_t*)obj;
  qsptr_t mgmt = heapcell->mgmt;
  if (! MGMT_IS_OCTET(mgmt)) return NULL;
  if (MGMT_GET_ALLOCSCALE(mgmt) == 0) return NULL;
  return obj;
}




qstree_t * qstree (qsmem_t * mem, qsptr_t t)
{
  qsobj_t * obj = qsobj_unicellular(mem, t, NULL);
  if (!obj) return NULL;
  return (qstree_t *)obj;
}

qsptr_t qstree_mark (qsmem_t * mem, qsptr_t t)
{
  qsptr_t currptr = t;
  qsptr_t backptr = QSNIL;
  qsptr_t tempptr = QSNIL;

  qstree_t * root = qstree(mem, t);
  while (! MGMT_IS_MARKED(root->mgmt))
    {
      /* main loop */
      qstree_t * tree = qstree(mem, currptr);
      if (tree)
	{
	  qsheapcell_t * cell = (qsheapcell_t*)tree;
	  if (! qsheapcell_is_marked(cell))
	    {
	      switch (qsheapcell_get_parent(cell))
		{
		case 0:
		  /* chase left. */
		  // store parent into "left" while chasing "left".
		  tempptr = cell->fields[0];
		  cell->fields[0] = backptr;
		  // chase "left".
		  backptr = currptr;
		  currptr = tempptr;
		  qsheapcell_set_parent(cell, 1);
		  break;
		case 1:
		  /* chase center. */
		  // swap "left=parent" with "center" while chasing "center".
		  tempptr = cell->fields[1];
		  cell->fields[1] = cell->fields[0];
		  cell->fields[0] = backptr;
		  // chase "center".
		  backptr = currptr;
		  currptr = tempptr;
		  qsheapcell_set_parent(cell, 2);
		  break;
		case 2:
		  /* chase right. */
		  // swap "center=parent" with "right" while chasing "right".
		  tempptr = cell->fields[2];
		  cell->fields[2] = cell->fields[1];
		  cell->fields[1] = backptr;
		  // chase "center".
		  backptr = currptr;
		  currptr = tempptr;
		  qsheapcell_set_parent(cell, 3);
		  break;
		case 3:
		  /* backtrack to parent. */
		  // restore "right=parent" and while chasing "parent".
		  tempptr = cell->fields[2];
		  cell->fields[2] = backptr;
		  // chase "right=parent".
		  backptr = currptr;
		  currptr = tempptr;
		  qsheapcell_set_parent(cell, 0);
		  // subtrees complete, set Marked.
		  qsheapcell_set_marked(cell, 1);
		  break;
		default:
		  break;
		}
	    }
	  else
	    {
	      tempptr = currptr;
	      currptr = backptr;
	      backptr = tempptr;
	    }
	}
      else
	{
	  // TODO: hand off marking to other types.
	  tempptr = currptr;
	  currptr = backptr;
	  backptr = tempptr;
	}

    }
  return t;
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
  qsptr_t retval = QSNIL;
  qsheapaddr_t addr;
  qserror_t err = qsheap_alloc_ncells(mem, 1, &addr);
  if (err != QSERROR_OK) return err;
  qstree_t * tree = (qstree_t*)qsheap_ref(mem, addr);
  if (tree)
    {
      tree->left = left;
      tree->data = data;
      tree->right = right;
      retval = QSOBJ(addr);
    }
  return retval;
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
  qsobj_t * obj = qsobj_multicellular(mem, v, NULL);
  if (!obj) return NULL;
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
  qsptr_t retval = QSNIL;
  qsheapaddr_t addr;
  qsword ncells = 1 + (k / 4)+1;
  qserror_t err = qsheap_alloc_ncells(mem, ncells, &addr);
  if (err != QSERROR_OK) return err;
  qsvector_t * vector = (qsvector_t*)qsheap_ref(mem, addr);
  if (vector)
    {
      vector->len = QSINT(k);
      vector->gc_backtrack = QSNIL;
      vector->gc_iter = QSNIL;
      qsword i;
      for (i = 0; i < k; i++)
	{
	  vector->_d[i] = fill;
	}
      retval = QSOBJ(addr);
    }
  return retval;
}

qserror_t qsvector_mark (qsmem_t * mem, qsptr_t v)
{
  qsword max = 0;
  qsvector_t * vec = qsvector(mem, v, &max);
  if (!vec) return QSERROR_INVALID;
  qsheapcell_t * cell = (qsheapcell_t*)vec;

  while (! MGMT_IS_MARKED(vec->mgmt))
    {
      /* main loop */
      if (qsheapcell_get_parent(cell) == 0)
	{
	  vec->gc_iter = QSINT(0);
	  qsheapcell_set_parent(cell, 1);
	}
      else if (qsheapcell_get_parent(cell) == 1)
	{
	  qsheapaddr_t idx = CINT30(qsheapcell_get_field(cell, 2));
	  while (idx < max)
	    {
	      qsptr_t elt = qsvector_ref(mem, v, idx);
	      /* TODO: recurse mark on elt. */
	      idx += 1;
	      qsheapcell_set_field(cell, 2, QSINT(idx));
	    }
	  qsheapcell_set_field(cell, 2, QSNIL);
	  qsheapcell_set_parent(cell, 0);
	  qsheapcell_set_marked(cell, 1);
	}
    }
  return QSERROR_OK;
}

int qsvector_crepr (qsmem_t * mem, qsptr_t v, char * buf, int buflen)
{
  return 0;
}








/**************/
/* Bytevector */
/**************/

qsbytevec_t * qsbytevec (qsmem_t * mem, qsptr_t bv, qsword * out_lim)
{
  qsobj_t * obj = qsobj_multioctetate(mem, bv, NULL);
  if (!obj) return NULL;
  if (! ISINT30(qsobj_get(obj, 0))) return NULL;
  if (out_lim)
    {
      *out_lim = CINT30(qsobj_get(obj, 0));
    }
  return (qsbytevec_t *)obj;
}

qserror_t qsbytevec_lock (qsmem_t * mem, qsptr_t bv)
{
  return QSERROR_NOIMPL;
}

qserror_t qsbytevec_unlock (qsmem_t * mem, qsptr_t bv)
{
  return QSERROR_NOIMPL;
}

qsword qsbytevec_refcount (qsmem_t * mem, qsptr_t bv)
{
  qsbytevec_t * bytevec = qsbytevec(mem, bv, NULL);
  if (!bytevec) return 0;
  return CINT30(bytevec->refcount);
}

// TODO: recount_increment and refcount_decrement

qsword qsbytevec_length (qsmem_t * mem, qsptr_t bv)
{
  qsbytevec_t * bytevec = qsbytevec(mem, bv, NULL);
  if (!bytevec) return 0;
  return CINT30(bytevec->len);
}

qsword qsbytevec_ref (qsmem_t * mem, qsptr_t bv, qsword ofs)
{
  qsbytevec_t * bytevec = qsbytevec(mem, bv, NULL);
  if (!bytevec) return 0;
  qsword max = qsbytevec_length(mem, bv);
  if ((ofs < 0) || (ofs >= max))
    return 0;
  return bytevec->_d[ofs];
}

qsptr_t qsbytevec_setq (qsmem_t * mem, qsptr_t bv, qsword ofs, qsword octet)
{
  qsbytevec_t * bytevec = qsbytevec(mem, bv, NULL);
  if (!bytevec) return 0;
  qsword max = qsbytevec_length(mem, bv);
  if ((ofs < 0) || (ofs >= max))
    return 0;
  bytevec->_d[ofs] = octet;
  return bv;
}

qsptr_t qsbytevec_make (qsmem_t * mem, qsword k, qsword fill)
{
  qsptr_t retval = QSNIL;
  qsheapaddr_t addr = 0;
  qserror_t err = qsheap_alloc_with_nbytes(mem, k, &addr);
  if (err != QSERROR_OK) return err;
  retval = QSOBJ(addr);
  qsbytevec_t * bytevec = qsbytevec(mem, retval, NULL);
  bytevec->len = QSINT(k);
  qsword i;
  for (i = 0; i < k; i++)
    {
      bytevec->_d[i] = (uint8_t)fill;
    }
  return retval;
}

int qsbytevec_crepr (qsmem_t * mem, qsptr_t bv, char * buf, int buflen)
{
  return 0;
}







/****************/
/* Wide numbers */
/****************/

qswidenum_t * qswidenum (qsmem_t * mem, qsptr_t n, qsnumtype_t * out_numtype)
{
  if (!ISHEAP26(n)) return NULL;
  qsobj_t * obj = qsobj_unioctetate(mem, n, NULL);
  qsptr_t discriminator = obj->_0;
  switch (discriminator)
    {
    case QSNUMTYPE_NAN:
    case QSNUMTYPE_LONG:
    case QSNUMTYPE_DOUBLE:
    case QSNUMTYPE_INT2:
    case QSNUMTYPE_FLOAT2:
    case QSNUMTYPE_FLOAT4:
    case QSNUMTYPE_FLOAT16CM:
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
  qswidenum_t * retval = NULL;
  qsheapaddr_t addr = 0;
  qserror_t err = QSERROR_OK;
  switch (variant)
    {
    case QSNUMTYPE_LONG:
    case QSNUMTYPE_DOUBLE:
    case QSNUMTYPE_INT2:
    case QSNUMTYPE_FLOAT2:
    case QSNUMTYPE_FLOAT4:
    case QSNUMTYPE_FLOAT16CM:
      err =  qsheap_alloc_with_nbytes(mem, 0, &addr);
      if (err != QSERROR_OK) return NULL;
      retval = (qswidenum_t*)(qsheap_ref(mem, addr));
      retval->variant = variant;
      break;
    default:
      retval->variant = QSNUMTYPE_NAN;
      break;
    }
  if (out_ptr)
    *out_ptr = QSOBJ(addr);
  return retval;
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
  if (err != QSERROR_OK)
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
      qserror_t err = qsheap_word(mem, ofs, &ref);
      if (err != QSERROR_OK)
	return QSNIL;
      if (ref == QSBOL)
	{
	  // nested list; nested iterator.
	  qsptr_t iter2 = QSITER( CITER28(it)+1 );
	  return iter2;
	}
      return ref;
    }
  // TODO: exception.
  return QSNIL;
}

qsptr_t qsiter_next (qsmem_t * mem, qsptr_t it)
{
  if (ISITER28(it))
    {
      qsptr_t peek = 0;
      qserror_t err = 0;
      int depth = 0;
      qsword startofs = qsiter_get(mem, it);
      err = qsheap_word(mem, startofs, &peek);
      if (err != QSERROR_OK) return QSNIL;
      if (peek == QSBOL) depth++; // goal: skip to end of nested list.

      qsword nextofs = startofs+1;
      err = qsheap_word(mem, nextofs, &peek);
      if (err != QSERROR_OK) return QSNIL;

      while (depth > 0)
	{
	  err = qsheap_word(mem, nextofs, &peek);
	  if (err != QSERROR_OK)
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
  int n = 0;
  int val = CINT30(i);
  n = snprintf(buf, buflen, "%d", val);
  return n;
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
  int n = 0;
  float val = 0;
  if (qsfloat(mem,f)) val = CFLOAT31(f);
  n = snprintf(buf, buflen, "%f", val);
  return n;
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
  int n = 0;
  int codepoint = 0;
  if (qschar(mem,c)) codepoint = CCHAR24(c);
  switch (codepoint)
    {
    case 7: /* bel */
      n += snprintf(buf+n, buflen-n, "#\\bel");
      break;
    case 8: /* bs */
      n += snprintf(buf+n, buflen-n, "#\\bs");
      break;
    case 9: /* tab */
      n += snprintf(buf+n, buflen-n, "#\\tab");
      break;
    case 10: /* lf */
      n += snprintf(buf+n, buflen-n, "#\\newline");
      break;
    case 13: /* cr */
      n += snprintf(buf+n, buflen-n, "#\\cr");
      break;
    case 27: /* esc */
      n += snprintf(buf+n, buflen-n, "#\\esc");
      break;
    case 32: /* space */
      n += snprintf(buf+n, buflen-n, "#\\space");
      break;
    default:
      if (codepoint <= 255)
	{
	  n += snprintf(buf+n, buflen-n, "#\\%c", codepoint);
	}
      else if (codepoint <= 0xffff)
	{
	  n += snprintf(buf+n, buflen-n, "#\\u%04x", codepoint);
	}
      else
	{
	  n += snprintf(buf+n, buflen-n, "#\\U%08x", codepoint);
	}
    }
  return n;
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




