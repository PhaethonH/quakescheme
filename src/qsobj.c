#include <math.h>

#include "qsobj.h"


/* printf-able, tag pointer with type. */
const char * __p (qsptr_t p)
{
  static char buf[32] = { 0, };
  if (ISFLOAT31(p))
    {
      snprintf(buf, sizeof(buf), "#f:%08x", p);
    }
  else if (ISINT30(p))
    {
      snprintf(buf, sizeof(buf), "#i:%08x", p);
    }
  else if (ISSYNC29(p))
    {
      snprintf(buf, sizeof(buf), "#s:%08x", p);
    }
  else if (ISITER28(p))
    {
      snprintf(buf, sizeof(buf), "#ι:%08x", p);
    }
  else if (ISOBJ26(p))
    {
      snprintf(buf, sizeof(buf), "#o:%08x", p);
    }
  else if (ISCHAR24(p))
    {
      snprintf(buf, sizeof(buf), "#c:%08x", p);
    }
  else if (ISERROR16(p))
    {
      snprintf(buf, sizeof(buf), "#E:%08x", p);
    }
  else if (ISCONST16(p))
    {
      snprintf(buf, sizeof(buf), "#k:%08x", p);
    }
  else
    {
      snprintf(buf, sizeof(buf), "#?:%08x", p);
    }
  return buf;
}




/* Attempt to cast to object (else freelist or data) */
qsobj_t * qsobj (qsmem_t * mem, qsptr_t p, qsmemaddr_t * out_addr)
{
  if (!ISHEAP26(p)) return NULL;
  qsmemaddr_t addr = COBJ26(p);
  qsobj_t * retval = (qsobj_t*)qsheap_ref(mem, addr);
  if (!retval) return NULL;
  qsptr_t mgmt = retval->mgmt;
  if (!ISSYNC29(mgmt)) return NULL;
  if (!MGMT_IS_USED(mgmt)) return NULL;
  if (out_addr)
    *out_addr = addr;
  return retval;
}

int qsobj_is_used (qsmem_t * mem, qsptr_t p)
{
  qsobj_t * obj = qsobj(mem, p, NULL);
  if (!obj) return 0;
  return !!MGMT_IS_USED(obj->mgmt);
}

int qsobj_is_marked (qsmem_t * mem, qsptr_t p)
{
  qsobj_t * obj = qsobj(mem, p, NULL);
  if (!obj) return 0;
  return !!MGMT_IS_MARKED(obj->mgmt);
}

int qsobj_is_red (qsmem_t * mem, qsptr_t p)
{
  qsobj_t * obj = qsobj(mem, p, NULL);
  if (!obj) return 0;
  return !!MGMT_IS_RED(obj->mgmt);
}

qsptr_t qsobj_get (qsmem_t * mem, qsptr_t p, qsword field_idx)
{
  qsobj_t * obj = qsobj(mem, p, NULL);
  if (!obj) return QSNIL;
  switch (field_idx)
    {
    case 0: return obj->_0; break;
    case 1: return obj->_1; break;
    case 2: return obj->_2; break;
    default: return QSNIL; break;
    }
  return QSNIL;
}

qsptr_t qsobj_set_marked (qsmem_t * mem, qsptr_t p, qsword val)
{
  qsobj_t * obj = qsobj(mem, p, NULL);
  if (!obj) return 0;
  if (val)
    MGMT_SET_MARKED(obj->mgmt);
  else
    MGMT_CLR_MARKED(obj->mgmt);
  return p;
}

qsptr_t qsobj_set_red (qsmem_t * mem, qsptr_t p, qsword val)
{
  qsobj_t * obj = qsobj(mem, p, NULL);
  if (!obj) return 0;
  if (val)
    MGMT_SET_RED(obj->mgmt);
  else 
    MGMT_CLR_RED(obj->mgmt);
  return p;
}

qsptr_t qsobj_set_parent (qsmem_t * mem, qsptr_t p, qsword val)
{
  qsobj_t * obj = qsobj(mem, p, NULL);
  if (!obj) return 0;
  MGMT_SET_PARENT(obj->mgmt, val);
  return p;
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

/*
   Conglomerated allocation logic:

   [in]k - number of cells (non-octetate) or number of bytes (octetate).
   [in]octetate - contents are bytes, else contents are pointers.
   [out]out_addr - linear address in heap space.

Returns: pointer value of newly allocation object, or error code (QSERROR_*).
*/
qsptr_t qsobj_make (qsmem_t * mem, qsword k, int octetate, qsmemaddr_t * out_addr)
{
  qsptr_t retval = QSERROR_NOMEM;
  qsmemaddr_t addr = 0;

  if (octetate)
    {
      retval = qsheap_alloc_with_nbytes(mem, (k>0)?k:0, &addr);
    }
  else
    {
      retval = qsheap_alloc_ncells(mem, (k>1)?k:1, &addr);
    }

  if (retval == QSERROR_OK)
    {
      retval = QSOBJ(addr);
      if (out_addr)
	*out_addr = addr;
    }
  // else retval already holds error code.
  return retval;
}

qserror_t qsobj_kmark (qsmem_t * mem, qsptr_t p)
{
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  if (!obj) return QSERROR_OK;
  qsptr_t currptr = p;
  qsptr_t backptr = QSNIL;
  qsptr_t next_visit = QSNIL;

  backptr = QSERROR_INVALID;
  while (currptr != QSERROR_INVALID)
    {
      if (qstree(mem, currptr))
	{
	  qstree_kmark(mem, currptr, backptr, &next_visit);
	  backptr = currptr;
	}
      else if (qsvector(mem, currptr, NULL))
	{
	  qsvector_kmark(mem, currptr, backptr, &next_visit);
	  backptr = currptr;
	}
      else
	{
	  next_visit = backptr;
	  backptr = currptr;
	}
      currptr = next_visit;
    }
  return QSERROR_OK;
}




qstree_t * qstree (qsmem_t * mem, qsptr_t t)
{
  qsobj_t * obj = qsobj_unicellular(mem, t, NULL);
  if (!obj) return NULL;
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
  qsptr_t retval = QSNIL;
  qsmemaddr_t addr = 0;
  if (!ISOBJ26((retval = qsobj_make(mem, 1, 0, &addr)))) return retval;

  qstree_t * tree = (qstree_t*)qsobj(mem, retval, NULL);
  tree->left = left;
  tree->data = data;
  tree->right = right;
  return retval;
}

qserror_t qstree_kmark (qsmem_t * mem, qsptr_t p, qsptr_t backptr, qsptr_t * nextptr)
{
  *nextptr = QSNIL;
  qsptr_t currptr = p;
  qsptr_t tempptr = QSNIL;
  qstree_t * tree = qstree(mem, p);  // try to get as tree.
  qsheapcell_t * heapcell = (qsheapcell_t*)tree;  // manipulate heap mgmt.
  if (!heapcell || qsheapcell_is_marked(heapcell))
    {
      /* invalid tree or already marked; nothing to recurse. */
      *nextptr = backptr;
      return QSERROR_OK;
    }
  int parent = 0;

  parent = qsheapcell_get_parent(heapcell);
  switch (parent)
    {
    case 0:
      /* chase left. */
      // store parent into "left" while chasing "left".
      tempptr = heapcell->fields[0];
      heapcell->fields[0] = backptr;
      // chase "left".
      currptr = tempptr;
      qsheapcell_set_parent(heapcell, 1);
      break;
    case 1:
      /* chase center. */
      // swap "left=parent" with "center" while chasing "center".
      tempptr = heapcell->fields[1];
      heapcell->fields[1] = heapcell->fields[0];
      heapcell->fields[0] = backptr;
      // chase "center".
      currptr = tempptr;
      qsheapcell_set_parent(heapcell, 2);
      break;
    case 2:
      /* chase right. */
      // swap "center=parent" with "right" while chasing "right".
      tempptr = heapcell->fields[2];
      heapcell->fields[2] = heapcell->fields[1];
      heapcell->fields[1] = backptr;
      // chase "center".
      currptr = tempptr;
      qsheapcell_set_parent(heapcell, 3);
      break;
    case 3:
      /* backtrack to parent. */
      // restore "right=parent" and while chasing "parent".
      tempptr = heapcell->fields[2];
      heapcell->fields[2] = backptr;
      // chase "right=parent".
      currptr = tempptr;
      qsheapcell_set_parent(heapcell, 0);
      // children are all chased, set Marked.
      qsheapcell_set_marked(heapcell, 1);
      break;
    default:
      break;
    }

  *nextptr = currptr;

  return QSERROR_OK;
}

int qstree_crepr (qsmem_t * mem, qsptr_t t, char * buf, int buflen)
{
  int n = 0;
  return n;
}




/* Pair is a degenerate tree where left==QSNIL */
qspair_t * qspair (qsmem_t * mem, qsptr_t p)
{
  qstree_t * tree = qstree(mem, p);
  if (!tree) return NULL;
  if (tree->left != QSNIL) return NULL;
  return (qspair_t *)tree;
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
  return qstree_make(mem, QSNIL, a, b);
}

int qspair_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  return n;
}




/* Construct list from QSBOL terminated arguments. */
qsptr_t qslist_make (qsmem_t * mem, ...)
{
}

qsword qslist_length (qsmem_t * mem, qsptr_t p)
{
  qsword retval = 0;
  qsptr_t curr = p;
  while (curr != QSNIL)
    {
      retval++;
      qsptr_t next = qspair_ref_d(mem, curr);
      if (qspair(mem, next))
	{
	  curr = next;
	}
      else
	{
	  curr = QSNIL;
	}
    }
  return retval;
}

qsptr_t qslist_tail (qsmem_t * mem, qsptr_t p, qsword nth)
{
  qsptr_t curr = p;
  qsword ofs = 0;
  while ((ofs < nth) && (curr != QSNIL))
    {
      ofs++;
      qsptr_t next = qspair_ref_d(mem, curr);
      if (qspair(mem, next))
	{
	  curr = next;
	}
      else
	{
	  curr = QSNIL;
	}
    }
  if (curr)
    {
      return curr;
    }
  return QSNIL;
}

qsptr_t qslist_ref (qsmem_t * mem, qsptr_t p, qsword k)
{
  qsptr_t curr = qslist_tail(mem, p, k);
  if (curr != QSNIL)
    {
      return qspair_ref_a(mem, p);
    }
  return QSERROR_INVALID;
}




qsvector_t * qsvector (qsmem_t * mem, qsptr_t v, qsword * out_lim)
{
  qsobj_t * obj = qsobj_multicellular(mem, v, NULL);
  if (!obj) return NULL;
  if (! ISINT30(qsobj_get(mem, v, 0))) return NULL;
  if (out_lim)
    {
      *out_lim = CINT30(qsobj_get(mem, v, 0));
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
  qsword lim = 0;
  qsvector_t * vec = qsvector(mem, v, &lim);
  if (!vec) return QSERROR_RANGE;
  if ((ofs < 0) || (ofs >= lim))
    {
      // TODO: exception.
      return QSERROR_RANGE;
    }
  return vec->_d[ofs];
}

qsptr_t qsvector_setq (qsmem_t * mem, qsptr_t v, qsword ofs, qsptr_t val)
{
  qsword lim = 0;
  qsvector_t * vec = qsvector(mem, v, &lim);
  if (!vec) return QSNIL;
  if ((ofs < 0) || (ofs >= lim))
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
  qsmemaddr_t addr = 0;
  qsword ncells = 1 + (k / 4)+1;  // always terminate with QSEOL.
  if (!ISOBJ26((retval = qsobj_make(mem, ncells, 0, &addr)))) return retval;

  qsvector_t * vector = (qsvector_t*)qsheap_ref(mem, addr);
  vector->len = QSINT(k);
  vector->gc_backtrack = QSNIL;
  vector->gc_iter = QSNIL;
  qsword i;
  for (i = 0; i < k; i++)
    {
      vector->_d[i] = fill;
    }
  retval = QSOBJ(addr);
  vector->_d[k] = QSEOL; // terminate with QSEOL for qsiter implementation.
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
	  qsmemaddr_t idx = CINT30(qsheapcell_get_field(cell, 2));
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

qserror_t qsvector_kmark (qsmem_t * mem, qsptr_t p, qsptr_t backptr, qsptr_t * nextptr)
{
  qsword lim = 0;
  qsvector_t * vec = qsvector(mem, p, &lim);  // try to get as vector.
  qsheapcell_t * heapcell = (qsheapcell_t*)vec; // heap manipulation.
  if (!heapcell || qsheapcell_is_marked(heapcell))
    {
      *nextptr = backptr;
      return 0;
    }

  int parent = 0;
  qsptr_t elt = QSNIL;

  parent = qsheapcell_get_parent(heapcell);
  if (parent == 0)
    {
      qsheapcell_set_parent(heapcell, 1);
      vec->gc_backtrack = backptr;
      vec->gc_iter = QSINT(0);
    }
  else if (parent == 1)
    {
      qsword idx = CINT30(vec->gc_iter);
      if (idx >= lim)
	{
	  qsheapcell_set_marked(heapcell, 1);
	  qsheapcell_set_parent(heapcell, 0);
	  backptr = vec->gc_backtrack;
	  vec->gc_backtrack = QSNIL;
	  vec->gc_iter = QSNIL;
	  *nextptr = backptr;
	  return 0;
	}
      else
	{
	  elt = qsvector_ref(mem, p, idx);
	  vec->gc_iter = QSINT(idx+1);
	}
    }
  else
    {
      return QSERROR_INVALID;
    }

  *nextptr = elt;
  return QSERROR_OK;
}

int qsvector_crepr (qsmem_t * mem, qsptr_t v, char * buf, int buflen)
{
  int n = 0;
  return n;
}

/* Copy a qsvector from C memory into Scheme memory. */
qsptr_t qsvector_inject (qsmem_t * mem, qsword nelts, qsptr_t * carray)
{
  qsptr_t v;
  v = qsvector_make(mem, nelts, QSNIL);
  if (! ISOBJ26(v)) return 0;
  qsword i;
  for (i = 0; i < nelts; i++)
    {
      qsvector_setq(mem, v, i, carray[i]);
    }
  return v;
}








/**************/
/* Bytevector */
/**************/

qsbytevec_t * qsbytevec (qsmem_t * mem, qsptr_t bv, qsword * out_lim)
{
  qsobj_t * obj = qsobj_multioctetate(mem, bv, NULL);
  if (!obj) return NULL;
  if (! ISINT30(qsobj_get(mem, bv, 0))) return NULL;
  if (out_lim)
    {
      *out_lim = CINT30(qsobj_get(mem, bv, 0));
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
  qsmemaddr_t addr = 0;
  if (!ISOBJ26((retval = qsobj_make(mem, k, 1, &addr)))) return retval;

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
  int n = 0;
  return n;
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
  qsptr_t p = QSNIL;
  qsmemaddr_t addr = 0;
  qserror_t err = QSERROR_OK;
  switch (variant)
    {
    case QSNUMTYPE_LONG:
    case QSNUMTYPE_DOUBLE:
    case QSNUMTYPE_INT2:
    case QSNUMTYPE_FLOAT2:
    case QSNUMTYPE_FLOAT4:
    case QSNUMTYPE_FLOAT16CM:
      if (!ISOBJ26((p = qsobj_make(mem, 0, 1, &addr)))) return NULL;
      retval = (qswidenum_t*)qsobj(mem, p, NULL);
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
  int n = 0;
  return n;
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
  int n = 0;
  return n;
}




int qswidenum_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  qsnumtype_t variant = 0;
  qswidenum_t * wn = qswidenum(mem, n, &variant);
  switch (variant)
    {
    case QSNUMTYPE_LONG:
      n += qslong_crepr(mem, p, buf+n, buflen-n);
      break;
    case QSNUMTYPE_DOUBLE:
      n += qsdouble_crepr(mem, p, buf+n, buflen-n);
      break;
    default:
      n += snprintf(buf+n, buflen-n, "#<WIDENUM:%d:%08x>", variant, p);
      break;
    }
  return n;
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

int qsiter_on_pair (qsmem_t * mem, qsptr_t it, qsptr_t * out_pairptr)
{
  if (ISITER28(it))
    {
      qsword ofs = qsiter_get(mem, it);
      qsmemaddr_t addr = (ofs >> 2);
      qsptr_t pairptr = QSOBJ(addr);
      qspair_t * pair = qspair(mem, pairptr);
      if (pair)
	{
	  if (out_pairptr)
	    *out_pairptr = pairptr;
	  return 1;
	}
    }
  return 0;
}

qsptr_t qsiter_item (qsmem_t * mem, qsptr_t it)
{
  qsptr_t pairptr = QSNIL;
  if (qsiter_on_pair(mem, it, &pairptr))
    {
      /* iter is pointing to pair cell. */
      return qspair_ref_a(mem, pairptr);
    }
  else if (ISITER28(it))
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
  qsptr_t pairptr = QSNIL;
  if (qsiter_on_pair(mem, it, &pairptr))
    {
      /* iter is pointing to pair cell. */
      qsptr_t next = qspair_ref_d(mem, pairptr);
      if (ISNIL(next))
	return QSNIL;
      qsmemaddr_t next_addr = COBJ26(next);
      qsmemaddr_t iter_addr = next_addr << 2;
      return qsiter_make(mem, iter_addr);
    }
  else if (ISITER28(it))
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

qsptr_t qsiter_crepr (qsmem_t * mem, qsptr_t it, char * buf, int buflen)
{
  int n = 0;
  return n;
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
  int n = 0;
  switch (c)
    {
    case QSERROR_OK:
      n += snprintf(buf+n, buflen-n, "#<ERROR:%s>", "OK");
      break;
    case QSERROR_INVALID:
      n += snprintf(buf+n, buflen-n, "#<ERROR:%s>", "INVALID");
      break;
    case QSERROR_NOMEM:
      n += snprintf(buf+n, buflen-n, "#<ERROR:%s>", "NOMEM");
      break;
    case QSERROR_NOIMPL:
      n += snprintf(buf+n, buflen-n, "#<ERROR:%s>", "NOIMPL");
      break;
    case QSERROR_RANGE:
      n += snprintf(buf+n, buflen-n, "#<ERROR:%s>", "RANGE");
      break;
    case QSERROR_TYPE:
      n += snprintf(buf+n, buflen-n, "#<ERROR:%s>", "TYPE");
      break;
    default:
      n += snprintf(buf+n, buflen-n, "#<ERROR:%08x>", c);
      break;
    }
  return n;
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
  int n = 0;
  switch (c)
    {
    case QSNIL:
      n += snprintf(buf+n, buflen-n, "'()");
      break;
    case QSTRUE:
      n += snprintf(buf+n, buflen-n, "#t");
      break;
    case QSBOL:
      n += snprintf(buf+n, buflen-n, "#bol");
      break;
    case QSEOL:
      n += snprintf(buf+n, buflen-n, "#eol");
      break;
    case QSBLACKHOLE:
      break;
    default:
      n += snprintf(buf+n, buflen-n, "#<const:%08x>", c);
      break;
    }
  return n;
}




/*******************************/
/* Variable-substrate objects. */
/*******************************/

/* List:
   1. linked pair cells
   2. vector with BOL termination
 */
// expanded qsiter_*() to accommodate this object.

/* String:
   1. pair cells of char24
   2. vector of char24 with BOL termination
   3. bytevector (UTF-8)
 */





int qsobj_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  if (qspair(mem, p))
    {
      n += qspair_crepr(mem, p, buf+n, buflen-n);
    }
  else if (qstree(mem, p))
    {
      n += qstree_crepr(mem, p, buf+n, buflen-n);
    }
  else if (qsvector(mem, p, NULL))
    {
      n += qsvector_crepr(mem, p, buf+n, buflen-n);
    }
  else if (qswidenum(mem, p, NULL))
    {
      n += qswidenum_crepr(mem, p, buf+n, buflen-n);
    }
  else if (qsbytevec(mem, p, NULL))
    {
      n += qsbytevec_crepr(mem, p, buf+n, buflen-n);
    }
  return n;
}

int qsunknown_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  n += snprintf(buf+n, buflen-n, "#<UNKNWON:%08x>", p);
  return n;
}

int qsptr_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  if (ISFLOAT31(p))
    {
      n += qsfloat_crepr(mem, p, buf+n, buflen-n);
    }
  else if (ISINT30(p))
    {
      n += qsint_crepr(mem, p, buf+n, buflen-n);
    }
  else if (ISITER28(p))
    {
      n += qsiter_crepr(mem, p, buf+n, buflen-n);
    }
  else if (ISOBJ26(p))
    {
      n += qsobj_crepr(mem, p, buf+n, buflen-n);
    }
  else if (ISCHAR24(p))
    {
      n += qschar_crepr(mem, p, buf+n, buflen-n);
    }
  else if (ISERROR16(p))
    {
      n += qserr_crepr(mem, p, buf+n, buflen-n);
    }
  else if (ISCONST16(p))
    {
      n += qsconst_crepr(mem, p, buf+n, buflen-n);
    }
  else
    {
      n += qsunknown_crepr(mem, p, buf+n, buflen-n);
    }
  return n;
}
