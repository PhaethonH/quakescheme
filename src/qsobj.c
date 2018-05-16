#include <assert.h>
#include <stdarg.h>

#include <errno.h>
#include <math.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>

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




#define qsbay_is_valid(addr) qsstore_is_valid(mem, addr)
#define qsbay_get_allocscale(addr) qsstorebay_get_allocscale(mem, addr)
#define qsbay_is_synced(addr) qsstorebay_is_synced(mem, addr)
#define qsbay_is_used(addr) qsstorebay_is_used(mem, addr)
#define qsbay_set_used(addr,val) qsstorebay_set_used(mem, addr, val)
#define qsbay_is_marked(addr) qsstorebay_is_marked(mem, addr)
#define qsbay_set_marked(addr,val) qsstorebay_set_marked(mem, addr, val)
#define qsbay_is_octetate(addr) qsstorebay_is_octetate(mem, addr)
#define qsbay_set_octetate(addr,val) qsstorebay_set_octetate(mem, addr, val)
#define qsbay_get_parent(addr) qsstorebay_get_parent(mem, addr)
#define qsbay_set_parent(addr,val) qsstorebay_set_parent(mem, addr, val)
#define qsbay_get_score(addr) qsstorebay_get_score(mem, addr)
#define qsbay_set_score(addr,val) qsstorebay_set_score(mem, addr, val)



/* Attempt to cast to object (else freelist or data) */
qsobj_t * qsobj_at (qsmem_t * mem, qsmemaddr_t addr)
{
  if (! qsbay_is_valid(addr)) return NULL;
  qsobj_t * obj = (qsobj_t*)qsstore_get(mem, addr);
  if (! qsbay_is_synced(addr)) return NULL;
  if (! qsbay_is_used(addr)) return NULL;
  return obj;
}

qsobj_t * qsobj (qsmem_t * mem, qsptr_t p, qsmemaddr_t * out_addr)
{
  if (!ISHEAP26(p)) return NULL;
  qsmemaddr_t addr = COBJ26(p);
  qsobj_t * obj = qsobj_at(mem, addr);
  if (!obj) return NULL;
  if (out_addr)
    *out_addr = addr;
  return obj;
}

int qsobj_used_p (qsmem_t * mem, qsptr_t p)
{
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  if (!obj) return 0;
  return qsbay_is_used(addr);
}

int qsobj_marked_p (qsmem_t * mem, qsptr_t p)
{
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  if (!obj) return 0;
  return qsbay_is_marked(addr);
}

int qsobj_octetate_p (qsmem_t * mem, qsptr_t p)
{
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  if (!obj) return 0;
  return qsbay_is_octetate(addr);
}

int qsobj_ref_parent (qsmem_t * mem, qsptr_t p)
{
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  if (!obj) return 0;
  return qsbay_get_parent(addr);
}

int qsobj_ref_score (qsmem_t * mem, qsptr_t p)
{
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  if (!obj) return 0;
  return qsbay_get_score(addr);
}

qsword qsobj_ref_nbays (qsmem_t * mem, qsptr_t p)
{
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  if (!obj) return 0;
  int allocscale = qsbay_get_allocscale(addr);
  return (1 << allocscale);
}

qsword qsobj_ref_allocsize (qsmem_t * mem, qsptr_t p)
{
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  if (!obj) return 0;
  int allocscale = qsbay_get_allocscale(addr);
  return (1 << allocscale);
}

qsptr_t qsobj_setq_marked (qsmem_t * mem, qsptr_t p, qsword val)
{
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  if (!obj) return QSERROR_INVALID;
  qsbay_set_marked(addr, val);
  return p;
}

qsptr_t qsobj_setq_score (qsmem_t * mem, qsptr_t p, qsword val)
{
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  if (!obj) return QSERROR_INVALID;
  qsbay_set_score(addr, val);
  return p;
}

qsptr_t qsobj_setq_parent (qsmem_t * mem, qsptr_t p, qsword val)
{
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  if (!obj) return 0;
  qsbay_set_parent(addr, val);
  return p;
}

/* Object as single-bay pointer contents. */
qsobj_t * qsobj_unibayptr (qsmem_t * mem, qsptr_t p, qsmemaddr_t * out_addr)
{
  qsobj_t * obj = qsobj(mem, p, out_addr);
  if (!obj) return NULL;
  if (qsobj_octetate_p(mem, p)) return NULL;
  if (qsobj_ref_nbays(mem, p) != 1) return NULL;
  return obj;
}

/* Object as multi-bay pointer contents. */
qsobj_t * qsobj_multibayptr (qsmem_t * mem, qsptr_t p, qsmemaddr_t * out_addr)
{
  qsobj_t * obj = qsobj(mem, p, out_addr);
  if (!obj) return NULL;
  if (qsobj_octetate_p(mem, p)) return NULL;
  if (qsobj_ref_nbays(mem, p) <= 1) return NULL;
  return obj;
}

/* Object is single-bay octet contents. */
qsobj_t * qsobj_unibayoct (qsmem_t * mem, qsptr_t p, qsmemaddr_t * out_addr)
{
  qsobj_t * obj = qsobj(mem, p, out_addr);
  if (!obj) return NULL;
  if (! qsobj_octetate_p(mem, p)) return NULL;
  if (qsobj_ref_nbays(mem, p) > 1) return NULL;
  return obj;
}

/* Object is multi-bay octet contents. */
qsobj_t * qsobj_multibayoct (qsmem_t * mem, qsptr_t p, qsmemaddr_t * out_addr)
{
  qsobj_t * obj = qsobj(mem, p, out_addr);
  if (!obj) return NULL;
  if (! qsobj_octetate_p(mem, p)) return NULL;
  if (qsobj_ref_nbays(mem, p) <= 1) return NULL;
  return obj;
}


/* Helper: prepare for access or mutation of a ptr field.
   Returns pointer to qsptr_t ready for read and write.
   NULL if inaccessible (typically out-of-bounds).
 */
qsptr_t * qsobj_prepare_ptr (qsmem_t * mem, qsptr_t p, qsword field_idx)
{
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  if (!obj) return NULL;

  if (field_idx == 0) return &(obj->mgmt); /* shortcut around other checks. */

  int octetate = qsbay_is_octetate(addr);
  int multi = (qsbay_get_allocscale(addr) > 0);
  qsword allocscale = qsbay_get_allocscale(addr);
  static const int nptr_scale = 4;  /* ptr-per-bay */
  size_t hardlimit = (1 << allocscale) * nptr_scale;
  if (octetate)
    {
      if (!multi)
	{
	  /* unibay_oct: 0,1 */
	  hardlimit = 2;
	}
      else
	{
	  /* multibay_oct: 0..3 */
	  hardlimit = 4;
	}
    }
  if (field_idx >= hardlimit)
    return NULL;
  /* Warning: pointer arithmetic */
  return ((qsptr_t*)(obj)) + field_idx;
}

/* Get start of arbitrary data elements (address of d[0]).
   Writes size of memory region (in bytes) to '*len', if not NULL.
   Returns NULL if no such field available.

   Primarily for accessing as widenum.
   Also helps with accessing members of vector and bytevector.

   Warning: lots of pointer arithmetic.
*/
void * qsobj_ref_data (qsmem_t * mem, qsptr_t p, size_t * len)
{
  void * retval = NULL;
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  if (!obj) return NULL;

  int octetate = qsbay_is_octetate(addr);
  int multi = (qsbay_get_allocscale(addr) > 0);
  static const int noct_scale = 16;
  qsword allocscale = qsbay_get_allocscale(addr);
  size_t all_bytes = (1 << allocscale) * noct_scale;
  if (octetate)
    {
      if (multi)
	{
	  all_bytes -= 16;
	  retval = (void*)(obj+1);
	}
      else
	{
	  all_bytes -= 8;
	  retval = (void*)&(obj->_1);
	}
    }
  else
    {
      if (multi)
	{
	  all_bytes -= 16;
	  retval = (void*)(obj+1);
	}
      else
	{
	  all_bytes = 0;
	  retval = NULL;
	}
    }
  if (len)
    *len = all_bytes;
  return retval;
}

/* Helper: prepare for access or mutation of an octet (byte).
   Returns pointer to uint8_t ready for read and write.
   NULL if inaccessible (typically out-of-bounds).
 */
uint8_t * qsobj_prepare_octet (qsmem_t * mem, qsptr_t p, qsword field_idx)
{
  uint8_t * octet_base = NULL;
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  if (!obj) return NULL;

  int octetate = qsbay_is_octetate(addr);
  if (!octetate) return NULL;

  size_t hardlimit = 0;
  octet_base = (uint8_t*)qsobj_ref_data(mem, p, &hardlimit);
  if (!octet_base) return NULL;
  if ((field_idx < 0) || (field_idx >= hardlimit)) return NULL;
  /* Warning: pointer arithmetic */
  return octet_base + field_idx;
}

qsptr_t qsobj_ref_ptr (qsmem_t * mem, qsptr_t p, qsword field_idx)
{
  qsptr_t * ptr = qsobj_prepare_ptr(mem, p, field_idx);
  if (!ptr) return QSERROR_INVALID;
  return *ptr;
}

int qsobj_ref_octet (qsmem_t * mem, qsptr_t p, qsword field_idx)
{
  uint8_t * octet = qsobj_prepare_octet(mem, p, field_idx);
  if (!octet) return -1;
  return *octet;
}

qserror_t qsobj_setq_ptr (qsmem_t * mem, qsptr_t p, qsword field_idx, qsptr_t val)
{
  qsptr_t * ptr = qsobj_prepare_ptr(mem, p, field_idx);
  if (!ptr) return QSERROR_INVALID;
  *ptr = val;
  return p;
}

qserror_t qsobj_setq_octet (qsmem_t * mem, qsptr_t p, qsword field_idx, int val)
{
  uint8_t * octet = qsobj_prepare_octet(mem, p, field_idx);
  if (!octet) return QSERROR_INVALID;
  *octet = val;
  return p;
}


/*
   Conglomerated allocation logic:

   [in]k - number of bays (non-octetate) or number of bytes (octetate).
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
      retval = qsstore_alloc_with_nbytes(mem, (k>0)?k:0, &addr);
    }
  else
    {
      retval = qsstore_alloc_nbays(mem, (k>1)?k:1, &addr);
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


qserror_t qsobj_kmark_unibayptr (qsmem_t * mem, qsptr_t p, qsptr_t backptr, qsptr_t * nextptr)
{
  *nextptr = QSNIL;
  qsptr_t currptr = p;
  qsptr_t tempptr = QSNIL;
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  if (!obj || qsobj_marked_p(mem, p))
    {
      /* invalid object or already marked; nothing to recurse. */
      *nextptr = backptr;
      return QSERROR_OK;
    }

  int parent = 0;
  parent = qsobj_ref_parent(mem, p);
  qsptr_t parentptr = QSNIL;
  switch (parent)
    {
    case 0:
      /* chase left. */
      // store parent into "left" while chasing "left".
      tempptr = qsobj_ref_ptr(mem, p, 1);
      qsobj_setq_ptr(mem, p, 1, backptr);
      // chase "left".
      currptr = tempptr;
      qsobj_setq_parent(mem, p, 1);
      break;
    case 1:
      /* chase center. */
      // swap "left=parent" with "center" while chasing "center".
      tempptr = qsobj_ref_ptr(mem, p, 2);
      parentptr = qsobj_ref_ptr(mem, p, 1);
      qsobj_setq_ptr(mem, p, 2, parentptr);
      qsobj_setq_ptr(mem, p, 1, backptr);
      // chase "center".
      currptr = tempptr;
      qsobj_setq_parent(mem, p, 2);
      break;
    case 2:
      /* chase right. */
      // swap "center=parent" with "right" while chasing "right".
      tempptr = qsobj_ref_ptr(mem, p, 3);
      parentptr = qsobj_ref_ptr(mem, p, 2);
      qsobj_setq_ptr(mem, p, 3, parentptr);
      qsobj_setq_ptr(mem, p, 2, backptr);
      // chase "center".
      currptr = tempptr;
      qsobj_setq_parent(mem, p, 3);
      break;
    case 3:
      /* backtrack to parent. */
      // restore "right=parent" and while chasing "parent".
      tempptr = qsobj_ref_ptr(mem, p, 3);
      qsobj_setq_ptr(mem, p, 3, backptr);
      // chase "right=parent".
      currptr = tempptr;
      qsobj_setq_parent(mem, p, 0);
      // children are all chased, set Marked.
      qsobj_setq_marked(mem, p, 1);
      break;
    default:
      break;
    }

  *nextptr = currptr;

  return QSERROR_OK;
}

qserror_t qsobj_kmark_multibayptr (qsmem_t * mem, qsptr_t p, qsptr_t backptr, qsptr_t * nextptr)
{
  static const int hdr_adjust = 1 * (sizeof(qsbay0_t) / sizeof(qsptr_t));
  static const int nptr_per_bay = sizeof(qsbay0_t) / sizeof(qsptr_t);
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  qsword lim = (qsobj_ref_nbays(mem, p) * nptr_per_bay) - hdr_adjust;

  if (!obj || qsobj_marked_p(mem, p))
    {
      *nextptr = backptr;
      return QSERROR_OK;
    }

  int parent = 0;
  qsptr_t elt = QSNIL;

  parent = qsobj_ref_parent(mem, p);
  if (parent == 0)
    {
      /* start of marking. */
      qsobj_setq_parent(mem, p, 1);
      /* initialize gc fields. */
      qsobj_setq_ptr(mem, p, 2, backptr);  /* gc_backgrack = backptr */
      qsobj_setq_ptr(mem, p, 3, QSINT(0)); /* gc_iter = 0 */
    }
  else if (parent == 1)
    {
      /* mid-marking */
      qsptr_t gc_iter = qsobj_ref_ptr(mem, p, 3);
      qsword idx = CINT30(gc_iter);
      if (idx >= lim)
	{
	  /* mark next. */
	  qsobj_setq_marked(mem, p, 1);
	  qsobj_setq_parent(mem, p, 0);
	  backptr = qsobj_ref_ptr(mem, p, 2);
	  qsobj_setq_ptr(mem, p, 2, QSNIL);  /* gc_backtrack = nil */
	  qsobj_setq_ptr(mem, p, 3, QSNIL);  /* gc_iter = nil */
	  *nextptr = backptr;
	  return QSERROR_OK;
	}
      else
	{
	  /* end of marking. */
	  elt = qsobj_ref_ptr(mem, p, idx + hdr_adjust); /* elt = _d[idx] */
	  qsobj_setq_ptr(mem, p, 3, QSINT(idx+1));  /* gc_iter++ */
	}
    }
  else
    {
      return QSERROR_INVALID;
    }

  *nextptr = elt;
  return QSERROR_OK;
}

/* TODO:
   qsobj_kmark_unibayoct
   qsobj_kmark_multibayoct
*/

qserror_t qsobj_kmark (qsmem_t * mem, qsptr_t p)
{
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  if (!obj) return QSERROR_OK;

  qsptr_t currptr = p;
  qsptr_t backptr = QSNIL;
  qsptr_t next_visit = QSNIL;
  int octetate = 0;
  int multi = 0;

  backptr = QSERROR_INVALID;
  while (currptr != QSERROR_INVALID)
    {
      qserror_t res = QSERROR_OK;
      octetate = qsobj_octetate_p(mem, currptr);
      multi = (qsobj_ref_nbays(mem, currptr) > 1);
      if (octetate)
	{
	  if (multi)
	    {
	      /* e.g. qsbytevector */
	      qsobj_setq_marked(mem, currptr, 1);
	    }
	  else
	    {
	      /* e.g. qswidenum */
	      qsobj_setq_marked(mem, currptr, 1);
	    }
	}
      else
	{
	  if (multi)
	    {
	      /* e.g. qsvector */
	      res = qsobj_kmark_multibayptr(mem, currptr, backptr, &next_visit);
	      backptr = currptr;
	    }
	  else
	    {
	      /* e.g. qstree */
	      res = qsobj_kmark_unibayptr(mem, currptr, backptr, &next_visit);
	      backptr = currptr;
	    }
	}
      /* TODO: check res */
      currptr = next_visit;
    }
  return QSERROR_OK;
}

int qsobj_p (qsmem_t * mem, qsptr_t p)
{
  if (! ISOBJ26(p)) return 0;
  qsmemaddr_t addr = 0;
  addr = COBJ26(p);
  if (! qsbay_is_synced(addr)) return 0;
  if (! qsbay_is_used(addr)) return 0;
  return 1;
}

const char * qsobj_typeof (qsmem_t * mem, qsptr_t p)
{
  if (qsfloat_p(mem, p)) return "float31";
  if (qsint_p(mem, p)) return "int30";
  if (qschar_p(mem, p)) return "char24";
  if (qserr_p(mem, p)) return "error16";
  if (qsconst_p(mem, p)) return "const16";
  if (qsobj_p(mem, p))
    {
      if (qsrbtree_p(mem, p)) return "Rbtree";
      if (qsimmlist_p(mem, p)) return "Immlist";
      if (qsvector_p(mem, p)) return "Vector";
      if (qsbytevec_p(mem, p)) return "Bytevec";
      if (qsutf8_p(mem, p)) return "Utf8";
      if (qssymbol_p(mem, p)) return "Symbol";
      if (qssymstore_p(mem, p)) return "Symstore";
      if (qsenv_p(mem, p)) return "Env";
      if (qslambda_p(mem, p)) return "Lambda";
      if (qsclosure_p(mem, p)) return "Closure";
      if (qskont_p(mem, p)) return "Continuation";
      if (qswidenum_p(mem, p))
        {
          if (qslong_p(mem, p)) return "Long";
          if (qsdouble_p(mem, p)) return "Double";
          return "Widenum(...)";
        }
      if (qspair_p(mem, p)) return "Pair";
      if (qstree_p(mem, p)) return "Tree";
    }
  if (qsiter_p(mem, p))
    {
      return "Iter";
    }
  return "UNK";
}






/* Define is-a predicate function. */
#define PREDICATE(OBJTYPE) bool OBJTYPE##_p (qsmem_t * mem, qsptr_t p)

/* Expects primary locator (qsptr_t) to be named 'p' */
/* Filter macros -- run subsequent if filter yields false. */
#define FILTER_ISA(pred)    if (! pred(mem,p))
#define FILTER_IS_OBJ       if (! qsobj_p(mem, p))
#define FILTER_IS_POINTER   if (qsobj_octetate_p(mem, p))
#define FILTER_IS_OCTETATE  if (! qsobj_octetate_p(mem, p))
#define FILTER_IS_UNIBAY    if (qsobj_ref_allocsize(mem, p) > 1)
#define FILTER_IS_MULTIBAY  if (qsobj_ref_allocsize(mem, p) <= 1)
#define FILTER_E_IS(match)  if (qsobj_ref_ptr(mem, p, 1) != match)
#define FILTER_A_IS(match)  if (qsobj_ref_ptr(mem, p, 2) != match)
#define FILTER_D_IS(match)  if (qsobj_ref_ptr(mem, p, 3) != match)
#define FILTER_E_ISA(pred)  if (!pred(mem, qsobj_ref_ptr(mem,p,1)))
#define FILTER_A_ISA(pred)  if (!pred(mem, qsobj_ref_ptr(mem,p,2)))
#define FILTER_D_ISA(pred)  if (!pred(mem, qsobj_ref_ptr(mem,p,3)))
#define ACCESS_PTR(nth)         qsobj_ref_ptr(mem, p, nth)
#define ACCESS_OCTET(nth)       qsobj_ref_octet(mem, p, nth)
#define MUTATE_PTR(nth, val)    qsobj_setq_ptr(mem, p, nth, val)
#define MUTATE_OCTET(nth, val)  qsobj_setq_octet(mem, p, nth, val)

/* No-change (identity) codec */
#define QSID(_) _

/* Define an accessor and mutator (Read-Write) for an object field. */
#define FIELD_RW(access_type, OBJTYPE, fldname, fldnum, encode, decode) \
access_type OBJTYPE##_ref_##fldname (qsmem_t * mem, qsptr_t p) \
{ \
  FILTER_ISA(OBJTYPE##_p)  return QSERROR_INVALID; \
  return decode(ACCESS_PTR(fldnum)); \
} \
qsptr_t OBJTYPE##_setq_##fldname (qsmem_t * mem, qsptr_t p, access_type val) \
{ \
  FILTER_ISA(OBJTYPE##_p)  return QSERROR_INVALID; \
  MUTATE_PTR(fldnum, encode(val)); \
  return p; \
}
/* Define accessor only (Read-Only) for an object field. */
#define FIELD_RO(access_type, OBJTYPE, fldname, fldnum, decode) \
access_type OBJTYPE##_ref_##fldname (qsmem_t * mem, qsptr_t p) \
{ \
  FILTER_ISA(OBJTYPE##_p)  return QSERROR_INVALID; \
  return decode(ACCESS_PTR(fldnum)); \
}

/* Newly created object is named 'p', also creates 'addr' if address is needed. */
#define OBJ_MAKE_BAYS(nbays, octetate) \
  qsptr_t p = QSNIL; \
  qsmemaddr_t addr = 0; \
  if (!ISOBJ26((p = qsobj_make(mem, nbays, octetate, &addr)))) return p
#define RETURN_OBJ return p

#define RETURN_TRUE return 1
#define RETURN_FALSE return 0

/* Define comparator. */
#define CMP_FUNC(OBJTYPE) \
cmp_t OBJTYPE##_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b) \
/*  simplistic is-same-object equality test. */
#define NAIVE_COMPARE_ADDR \
  return ((a == b) ? CMP_EQ : CMP_NE)
#define CMP_FUNC_NAIVE(OBJTYPE)  \
CMP_FUNC(OBJTYPE) { NAIVE_COMPARE_ADDR; }




/* Tree - generic tree node. */
PREDICATE(qstree)
{
  FILTER_ISA(qsobj_p)  return 0;
  FILTER_IS_UNIBAY     return 0;
  FILTER_IS_POINTER    return 0;
  return 1;
}

FIELD_RW(qsptr_t, qstree, left, 1, QSID,QSID)
FIELD_RW(qsptr_t, qstree, data, 2, QSID,QSID)
FIELD_RW(qsptr_t, qstree, right, 3, QSID,QSID)


qsptr_t qstree_make (qsmem_t * mem, qsptr_t left, qsptr_t data, qsptr_t right)
{
  OBJ_MAKE_BAYS(1, 0);

  MUTATE_PTR(1, left);
  MUTATE_PTR(2, data);
  MUTATE_PTR(3, right);

  RETURN_OBJ;
}

qserror_t qstree_kmark (qsmem_t * mem, qsptr_t p, qsptr_t backptr, qsptr_t * nextptr)
{
  if (!qstree_p(mem, p))
    {
      /* not a tree; nothing to recurse. */
      *nextptr = backptr;
      return QSERROR_OK;
    }
  return qsobj_kmark_unibayptr(mem, p, backptr, nextptr);
}

int qstree_crepr (qsmem_t * mem, qsptr_t t, char * buf, int buflen)
{
  int n = 0;
  return n;
}




/* Helper functions for R-B tree. */

/* Rotate the (sub)tree left, returns new root. */
qsptr_t qsrbnode_rotate_left (qsmem_t * mem, qsptr_t pivot)
{
  if (ISNIL(pivot)) return pivot;  // no tree.
  qsptr_t rchild = qstree_ref_right(mem, pivot);
  if (ISNIL(rchild)) return pivot;  // no child to bring up.
  qsptr_t temp = qstree_ref_left(mem, rchild);
  qstree_setq_left(mem, rchild, pivot);
  qstree_setq_right(mem, pivot, temp);
  return rchild;
}

/* Rotate the (sub)tree right, returns new root. */
qsptr_t qsrbnode_rotate_right (qsmem_t * mem, qsptr_t pivot)
{
  if (ISNIL(pivot)) return pivot;  // no tree.
  qsptr_t lchild = qstree_ref_left(mem, pivot);
  if (ISNIL(lchild)) return pivot;  // no child to bring up.
  qsptr_t temp = qstree_ref_right(mem, lchild);
  qstree_setq_right(mem, lchild, pivot);
  qstree_setq_left(mem, pivot, temp);
  return lchild;
}

int qsrbnode_red_p (qsmem_t * mem, qsptr_t node)
{
  if (ISNIL(node)) return 0;
  return (1 == qsobj_ref_score(mem, node));
}

int qsrbnode_black_p (qsmem_t * mem, qsptr_t node)
{
  if (ISNIL(node)) return 1;
  return !qsrbnode_red_p(mem, node);
}

qsptr_t qsrbnode_paint (qsmem_t * mem, qsptr_t node, int val)
{
  return qsobj_setq_score(mem, node, val ? 1 : 0);
}

qsptr_t qsrbnode_paint_red (qsmem_t * mem, qsptr_t node)
{
  return qsrbnode_paint(mem, node, 1);
}

qsptr_t qsrbnode_paint_black (qsmem_t * mem, qsptr_t node)
{
  return qsrbnode_paint(mem, node, 0);
}




PREDICATE(qsrbtree)
{
  FILTER_ISA(qsobj_p)   return 0;
  FILTER_IS_POINTER     return 0;
  if (qsobj_ref_allocsize(mem, p) != 2)   return 0;
  FILTER_E_IS(QST_RBTREE) return 0;
  return 1;
}

FIELD_RW(qsptr_t, qsrbtree, top, 4, QSID,QSID)
FIELD_RW(qsptr_t, qsrbtree, cmp, 5, QSID,QSID)
FIELD_RW(qsptr_t, qsrbtree, up, 6, QSID,QSID)
FIELD_RW(qsptr_t, qsrbtree, down, 7, QSID,QSID)

/* Lock tree for update, indicates tree may be in inconsistent state. */
qsptr_t qsrbtree_lock (qsmem_t * mem, qsptr_t root)
{
  return QSTRUE;
}

/* Unlock tree after updating. */
qsptr_t qsrbtree_unlock (qsmem_t * mem, qsptr_t root)
{
  return QSTRUE;
}

qsptr_t qsrbtree_make (qsmem_t * mem, qsptr_t top_node, qsptr_t cmp)
{
  OBJ_MAKE_BAYS(2, 0);

  MUTATE_PTR(1, QST_RBTREE);    /* .variant = QST_RBTREE */
  MUTATE_PTR(4, top_node);    /* .top = top_node */
  MUTATE_PTR(5, cmp);         /* .cmp = cmp */
  MUTATE_PTR(6, QSNIL);       /* .up = nil */
  MUTATE_PTR(7, QSNIL);       /* .down = nil */

  return p;
}

/* Split a tree to set up reverse pointers.
   Updates internal "splits" state to track the current split point.
   Returns the rbtree instance, or error if splitting failed.
 */
qsptr_t qsrbtree_split (qsmem_t * mem, int leftward, qsptr_t p)
{
  FILTER_ISA(qsrbtree_p)  return QSERROR_INVALID;
  qsptr_t next = QSNIL;
  qsptr_t parent = QSNIL;
  qsptr_t currptr = qsrbtree_ref_down(mem, p);
  if (leftward)
    {
      next = qstree_ref_left(mem, currptr);
      qsptr_t parent = qsrbtree_ref_up(mem, p);
      /* indicate .left is actually parent. */
      currptr = qstree_setq_left(mem, currptr, parent);
      currptr = qsobj_setq_parent(mem, currptr, 1); 
    }
  else
    {
      next = qstree_ref_right(mem, currptr);
      qsptr_t parent = qsrbtree_ref_up(mem, p);
      /* indicate .right is actually parent. */
      currptr = qstree_setq_right(mem, currptr, parent);
      currptr = qsobj_setq_parent(mem, currptr, 2); 
    }
  qsrbtree_setq_up(mem, p, currptr); // parent of subtree.
  qsrbtree_setq_down(mem, p, next); // subtree
  return p;
}

qsptr_t qsrbtree_split_left (qsmem_t * mem, qsptr_t rootptr)
{
  return qsrbtree_split(mem, 1, rootptr);
}

qsptr_t qsrbtree_split_right (qsmem_t * mem, qsptr_t rootptr)
{
  return qsrbtree_split(mem, 0, rootptr);
}

/* Mend a tree to restore from reverse pointers.
   Attaches 'currptr' back into 'root' based on internal "splits" state.
   Returns pointer to newly mended parent node (parent of subtree).
 */
qsptr_t qsrbtree_mend (qsmem_t * mem, qsptr_t rootptr)
{
  qsptr_t up = QSNIL;
  assert(rootptr);

  qsptr_t parent = qsrbtree_ref_up(mem, rootptr);
  if (ISNIL(parent))
    {
      /* Done. */
      rootptr = qsrbtree_setq_down(mem, rootptr, QSNIL);
      return rootptr;
    }
  qsptr_t currptr = qsrbtree_ref_down(mem, rootptr);

  int ancestry = qsobj_ref_parent(mem, parent);
  if (ancestry == 1)
    {
      /* restore left. */
      up = qstree_ref_left(mem, parent);
      currptr = qstree_setq_left(mem, parent, currptr);
    }
  else if (ancestry == 2)
    {
      /* restore right. */
      up = qstree_ref_right(mem, parent);
      currptr = qstree_setq_right(mem, parent, currptr);
    }
  else
    {
      /* inconsistent state. */
      abort();
    }
  currptr = qsobj_setq_parent(mem, parent, 0);  /* clear reversal. */

  /* move up. */
  rootptr = qsrbtree_setq_up(mem, rootptr, up);
  rootptr = qsrbtree_setq_down(mem, rootptr, currptr);

  return rootptr;
}

qsptr_t qsrbtree_ref_uncle (qsmem_t * mem, qsptr_t rootptr)
{
  qsptr_t parent = qsrbtree_ref_up(mem, rootptr);
  if (ISNIL(parent)) return QSNIL;
  int ancestry = qsobj_ref_parent(mem, parent);
  qsptr_t grandparent = QSNIL;
  if (ancestry == 1)
    {
      /* (grand)parent is stored in "left". */
      grandparent = qstree_ref_left(mem, parent);
    }
  else if (ancestry == 2)
    {
      /* (grand)parent is stored in "right". */
      grandparent = qstree_ref_right(mem, parent);
    }
  else
    {
      /* inconsistent state. */
      abort();
    }

  if (ISNIL(grandparent)) return QSNIL;

  ancestry = qsobj_ref_parent(mem, grandparent);
  if (ancestry == 1)
    {
      /* greatgrandparent in "left", sibling is right. */
      return qstree_ref_right(mem, grandparent);
    }
  else if (ancestry == 2)
    {
      /* greatgrandparent in "right", sibling is left. */
      return qstree_ref_left(mem, grandparent);
    }
  else
    {
      /* inconsistent state. */
      abort();
    }

  return QSNIL;
}

qsptr_t qsrbtree_ref_grandparent (qsmem_t * mem, qsptr_t rootptr)
{
  qsptr_t currptr = qsrbtree_ref_down(mem, rootptr);
  qsptr_t parent = qsrbtree_ref_up(mem, rootptr);
  if (ISNIL(currptr)) return QSNIL;
  if (ISNIL(parent)) return QSNIL;
  int ancestry = qsobj_ref_parent(mem, parent);
  if (ancestry == 1)
    {
      /* (grand)parent is stored in "left". */
      return qstree_ref_left(mem, parent);
    }
  else if (ancestry == 2)
    {
      /* (grand)parent is stored in "right". */
      return qstree_ref_right(mem, parent);
    }
  else
    {
      /* inconsistent state. */
      abort();
    }
  return QSNIL;
}


/* Insert association pair, rebalancing as needed.
   Merging whole trees not supported.
 */
qsptr_t qsrbtree_insert (qsmem_t * mem, qsptr_t root, qsptr_t apair)
{
  /* the "New" tree node; starts off black. */
  qsptr_t N = qstree_make(mem, QSNIL, apair, QSNIL);

  if (ISNIL(root))
    {
      /* create tree. */
      root = qsrbtree_make(mem, N, QSNIL);
      return root;
    }
  if (ISNIL(qsrbtree_ref_top(mem, root)))
    {
      /* empty tree. */
      root = qsrbtree_setq_top(mem, root, N);
      return root;
    }

  qsrbtree_lock(mem, root);
  root = qsrbtree_setq_up(mem, root, QSNIL);
  root = qsrbtree_setq_down(mem, root, qsrbtree_ref_top(mem, root));

  qsptr_t d = QSNIL;
  //qsptr_t newkey = qspair_ref_a(mem, apair);
  qsptr_t newkey = qstree_ref_data(mem, apair);
  cmp_t libra = 0;
  qsword lim = 258;  /* avoid infinite loop; max used depth around  31. */

  /* traverse to bottom of tree. */
  while (!ISNIL(qsrbtree_ref_down(mem, root)) && (--lim > 1))
    {
      qsptr_t currnode = qsrbtree_ref_down(mem, root);
      d = qstree_ref_data(mem, currnode);
      // TODO: custom comparator
      if (qstree_p(mem, d)) // also covers pair (apair).
	{
	  //qsptr_t currkey = qspair_ref_a(mem, d);
	  qsptr_t currkey = qstree_ref_data(mem, d);
          /* TODO: qsobj_cmp */
	  libra = qsobj_cmp(mem, newkey, currkey);
	}
      else
	{
	  /* No compare; bias to right (as would a list). */
	  libra = CMP_GT;
	}
      switch (libra)
        {
        case CMP_EQ: /* match */
          lim = 1;
          break;
        case CMP_LT:
          /* go left. */
	  root = qsrbtree_split_left(mem, root);
          break;
        case CMP_NE:  /* not-equal or no-compare, go right. */
        case CMP_GT:
	  /* go right. */
	  root = qsrbtree_split_right(mem, root);
          break;
        }
    }

  /* insertion. */
  /* mend() naturally stitches new node into correct place. */
  root = qsrbtree_setq_down(mem, root, N);

  /* rebalancing. */
  int case3recursion = 0;  /* Case 3 requires recursion of rebalancing. */

  do
    {
      qsptr_t P = qsrbtree_ref_up(mem, root);
      if (!qsrbnode_red_p(mem, P))
	{
	  /* case 2: parent is black, N (should be) red. */
	  qsrbnode_paint_red(mem, N);
	  goto unwind;
	}
      /* Parent is red. */
      N = qsrbnode_paint_red(mem, N);
      qsptr_t U = qsrbtree_ref_uncle(mem, root);
      qsptr_t G = qsrbtree_ref_grandparent(mem, root);
      if (qsrbnode_red_p(mem, U))
	{
	  /* case 3: red P, red U
	     => blacken P, blacken U, redden G, recurse on G.
	     */
	  P = qsrbnode_paint_black(mem, P);
	  U = qsrbnode_paint_black(mem, U);
	  G = qsrbnode_paint_red(mem, G);

	  /* recurse on G. */
	  root = qsrbtree_mend(mem, root);  /* curr = P */
	  assert(!ISNIL(root));
	  root = qsrbtree_mend(mem, root);  /* curr = G */
	  assert(!ISNIL(root));

	  P = qsrbtree_ref_up(mem, root);
	  if (ISNIL(P) || (N == qsrbtree_ref_top(mem, root)))
	    {
	      /* ended up at root: make it black, done. */
	      qsptr_t rootnode = qsrbtree_ref_top(mem, root);
	      rootnode = qsrbnode_paint_black(mem, rootnode);
	      root = qsrbtree_setq_top(mem, root, rootnode);
	      goto unwind;
	    }

	  case3recursion = 1;
	  N = G;
	  continue;  /* repeat loop: recurse on G (N' = G). */
	}

      /* case 4 or 5: red P, U black.
	 => find out if inner-rotation or outer-rotation.
	 */
      /* Path Grandparent-to-Parent; left=-1, right=1 */
      int gp = qsobj_ref_parent(mem, G) == 1 ? -1 : 1;
      /* Path Parent-to-Node; left=-1, right=1 */
      int pn = qsobj_ref_parent(mem, P) == 1 ? -1 : 1;

      /* Move up to grandparent to track rotates on G. */
      root = qsrbtree_mend(mem, root);
      assert(!ISNIL(root));
      root = qsrbtree_mend(mem, root);
      assert(!ISNIL(root));

      assert(qsobj_ref_parent(mem, P) == 0);
      assert(qsobj_ref_parent(mem, U) == 0);
      assert(qsobj_ref_parent(mem, G) == 0);
      assert(qsobj_ref_parent(mem, N) == 0);

      if (gp != pn)
	{
	  /* case 4: G.left,P.right || G.right,P.left  (inner rotation)
	     => Convert to case 5.
	     */
	  if (pn == -1)
	    {
	      /* G.right, P.left  becomes  G.right, P.right (case 5) */
	      P = qsrbnode_rotate_right(mem, P);
	      G = qstree_setq_right(mem, G, P);
	      pn = 1;
	    }
	  else
	    {
	      /* G.left, P.right  becomes  G.left, P.left (case 5) */
	      P = qsrbnode_rotate_left(mem, P);
	      G = qstree_setq_left(mem, G, P);
	      pn = -1;
	    }
	}

      if (gp == pn)
	{
	  /* case 5: G.left,P.left || G.right,P.right  (outer rotation) */
	  qsptr_t S = QSNIL;  /* sibling to N after rotation. */
	  if (pn == -1)
	    {
	      G = qsrbnode_rotate_right(mem, G);
	      S = qstree_ref_right(mem, G);
	    }
	  else
	    {
	      G = qsrbnode_rotate_left(mem, G);
	      S = qstree_ref_left(mem, G);
	    }
	  /* paint S red, paint G' black, N is red. */
	  S = qsrbnode_paint_red(mem, S);
	  G = qsrbnode_paint_black(mem, G);
	  /* rearrange splits. */
	  root = qsrbtree_setq_down(mem, root, G);

	  goto unwind;
	}
      else
	{
	  /* inconsistent state. */
	  abort();
	}

    }
  while (case3recursion);

unwind:
  /* recursive mend */
  while (!ISNIL(qsrbtree_ref_up(mem, root)))
    {
      root = qsrbtree_mend(mem, root);
      assert(!ISNIL(root));
    }

  /* Top node might have changed. */
  qsptr_t final_mend = qsrbtree_ref_down(mem, root);
  if (final_mend != qsrbtree_ref_top(mem, root))
    {
      root = qsrbtree_setq_top(mem, root, final_mend);
    }
  assert(ISNIL(qsrbtree_ref_up(mem, root)));
  root = qsrbtree_setq_down(mem, root, QSNIL);
  qsrbtree_unlock(mem, root);

  return root;
}

/* Return best tree node fulfilling criterion 'key'.
   nil if not found.
 */
qsptr_t qstree_find (qsmem_t * mem, qsptr_t t, qsptr_t key, qsptr_t * nearest)
{
  qsptr_t probe = t;
  qsptr_t prev = QSNIL;
  qsptr_t d;
  cmp_t libra = 0;
  qsptr_t probekey = QSNIL;
  /* TODO: resolve having mostly-duplicated code with qsrbtree_insert. */
  while (!ISNIL(probe))
    {
      prev = probe;
      d = qstree_ref_data(mem, probe);
      probekey = QSNIL;
      libra = CMP_NE;
      // TODO: custom comparator
      if (qstree_p(mem, d))
	{
	  probekey = qstree_ref_data(mem, d);
          /* TODO: qsobj_cmp */
	  libra = qsobj_cmp(mem, key, probekey);
	}
      else
	{
	  /* uncomparable; just go right. */
	  libra = CMP_GT;
	}
      switch (libra)
        {
        case CMP_EQ: /* match */
          return probe;
          break;
        case CMP_LT:
	  /* left */
          probe = qstree_ref_left(mem, probe);
          break;
        case CMP_NE:  /* not-equal or no-compare, go right. */
        case CMP_GT:
	  /* right */
          probe = qstree_ref_right(mem, probe);
          break;
        }
    }
  if (nearest)
    *nearest = prev;  // best match (ready for insert).
  return QSNIL;
}

/* Returns association-pair matching criteria 'key'.
   nil if not found.
   */
qsptr_t qstree_assoc (qsmem_t * mem, qsptr_t t, qsptr_t key)
{
  qsptr_t probe = qstree_find(mem, t, key, NULL);
  if (ISNIL(probe)) return QSNIL;
  qsptr_t d = qstree_ref_data(mem, probe);
  return d;
}

qsptr_t qsrbtree_find (qsmem_t * mem, qsptr_t root, qsptr_t key, qsptr_t * nearest)
{
  qsptr_t raw_tree = qsrbtree_ref_top(mem, root);
  return qstree_find(mem, raw_tree, key, nearest);
}

qsptr_t qsrbtree_assoc (qsmem_t * mem, qsptr_t root, qsptr_t key)
{
  qsptr_t probe = qsrbtree_find(mem, root, key, NULL);
  if (ISNIL(probe)) return QSNIL;
  qsptr_t d = qstree_ref_data(mem, probe);
  return d;
}

int qsrbtree_node_crepr (qsmem_t * mem, qsptr_t node, char * buf, int buflen)
{
  int n = 0;
  qsptr_t left = qstree_ref_left(mem, node);
  qsptr_t data = qstree_ref_data(mem, node);
  qsptr_t right = qstree_ref_right(mem, node);
  if (!ISNIL(left)) {
    n += qsrbtree_node_crepr(mem, left, buf+n, buflen-n);
    n += snprintf(buf+n, buflen-n, " ");
  }
  if (qspair_p(mem, data))
    {
      qsptr_t a = qspair_ref_a(mem, data);
      qsptr_t d = qspair_ref_d(mem, data);
      n += qsptr_crepr(mem, a, buf+n, buflen-n);
      n += snprintf(buf+n, buflen-n, ":");
      n += qsptr_crepr(mem, d, buf+n, buflen-n);
    }
  else
    {
      n += qsptr_crepr(mem, data, buf+n, buflen-n);
    }

  if (!ISNIL(right)) {
    n += snprintf(buf+n, buflen-n, " ");
    n += qsrbtree_node_crepr(mem, right, buf+n, buflen-n);
  }
  return n;
}

int qsrbtree_crepr (qsmem_t * mem, qsptr_t rbtree, char * buf, int buflen)
{
  int n = 0;

  qsptr_t top = qsrbtree_ref_top(mem, rbtree);
  qsptr_t probe = top;
  n += snprintf(buf+n, buflen-n, "(rbtree ");
  n += qsrbtree_node_crepr(mem, probe, buf+n, buflen-n);
  n += snprintf(buf+n, buflen-n, ")");

  return n;
}




PREDICATE(qsibtree)
{
  FILTER_ISA(qstree_p)   return 0;
  return 1;
}

FIELD_RW(qsptr_t, qsibtree, filled, 1, QSID,QSID)
FIELD_RW(qsptr_t, qsibtree, idx0, 2, QSID,QSID)
FIELD_RW(qsptr_t, qsibtree, ones, 3, QSID,QSID)

qsptr_t qsibtree_make (qsmem_t * mem)
{
  OBJ_MAKE_BAYS(1, 0);

  MUTATE_PTR(1, QSNIL);
  MUTATE_PTR(2, QSNIL);
  MUTATE_PTR(3, QSNIL);

  RETURN_OBJ;
}

qsptr_t qsibnode_find (qsmem_t * mem, qsptr_t t, qsword path)
{
  qsptr_t curr = t;
  qsword remainder = path;
  while ((remainder > 1) && !ISNIL(curr))
    {
      if (remainder & 0x01)
	{
	  /* go right. */
	  curr = qstree_ref_right(mem, curr);
	}
      else
	{
	  /* go left. */
	  curr = qstree_ref_left(mem, curr);
	}
      remainder >>= 1;
    }
  return curr;
}

qsptr_t qsibnode_pave (qsmem_t * mem, qsptr_t t, qsword path)
{
  qsptr_t curr = t;
  qsword remainder = path;
  while (remainder > 1)
    {
      if (remainder & 0x01)
	{
	  /* rightwards. */
	  if (!ISNIL(qstree_ref_right(mem, curr)))
	    {
	      /* go right. */
	      curr = qstree_ref_right(mem, curr);
	    }
	  else
	    {
	      /* make right. */
	      qsptr_t next = qstree_make(mem, QSNIL, QSNIL, QSNIL);
	      qstree_setq_right(mem, curr, next);
	      curr = next;
	    }
	}
      else
	{
	  /* leftwards. */
	  if (!ISNIL(qstree_ref_left(mem, curr)))
	    {
	      /* go left. */
	      curr = qstree_ref_left(mem, curr);
	    }
	  else
	    {
	      /* make left. */
	      qsptr_t next = qstree_make(mem, QSNIL, QSNIL, QSNIL);
	      qstree_setq_left(mem, curr, next);
	      curr = next;
	    }
	}
      remainder >>= 1;
    }
  return curr;
}

qsword qsibtree_length (qsmem_t * mem, qsptr_t t)
{
  return qsibtree_ref_filled(mem, t);
}

qsptr_t qsibtree_ref (qsmem_t * mem, qsptr_t t, qsword path)
{
  if (path == 0)
    {
      return qsibtree_ref_idx0(mem, t);
    }
  qsptr_t ones = qsibtree_ref_ones(mem, t);
  qsptr_t target = qsibnode_find(mem, ones, path);
  if (ISNIL(target)) return QSNIL;
  return qstree_ref_data(mem, target);
}

qsptr_t qsibtree_setq (qsmem_t * mem, qsptr_t t, qsword path, qsptr_t entry)
{
  if (path == 0)
    {
      qsibtree_setq_idx0(mem, t, entry);
    }
  else if (path == 1)
    {
      qsptr_t ones = qsibtree_ref_ones(mem, t);
      if (ISNIL(ones))
	{
	  ones = qstree_make(mem, QSNIL, entry, QSNIL);
	  qsibtree_setq_ones(mem, t, ones);
	}
      else
	{
	  qstree_setq_data(mem, ones, entry);
	}
    }
  else
    {
      qsptr_t target = qsibnode_pave(mem, qsibtree_ref_ones(mem, t), path);
      qstree_setq_data(mem, target, entry);
    }

  if (path > CINT30(qsibtree_ref_filled(mem, t)))
    {
      qsibtree_setq_filled(mem, t, path);
    }

  return t;
}

cmp_t qsobj_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b)
{
  qsptr_t cast_b = b;
  if (qsstr_p(mem, a))
    {
      if (! qsstr_p(mem, b)) return CMP_NE;
      return qsstr_cmp(mem, a, b);
    }
  else if (qssymbol_p(mem, a))
    {
      if (! qssymbol_p(mem, b)) return CMP_NE;
      return qssymbol_cmp(mem, a, b);
    }
  else
    {
      // TODO: lots.
    }
  return CMP_NE;
}




/* Pair is a degenerate tree where left==QSNIL */
PREDICATE(qspair)
{
  FILTER_ISA(qstree_p)  return 0;
  FILTER_E_IS(QSNIL)    return 0;
  return 1;
}

FIELD_RW(qsptr_t, qspair, a, 2, QSID,QSID)
FIELD_RW(qsptr_t, qspair, d, 3, QSID,QSID)

qsptr_t qspair_make (qsmem_t * mem, qsptr_t a, qsptr_t d)
{
  OBJ_MAKE_BAYS(1, 0);

  MUTATE_PTR(1, QSNIL);
  MUTATE_PTR(2, a);
  MUTATE_PTR(3, d);

  RETURN_OBJ;
}

qsptr_t qspair_iter (qsmem_t * mem, qsptr_t p)
{
  qsmemaddr_t addr = (COBJ26(p) << 2);
  return QSITER(addr);
}

int qspair_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  n += snprintf(buf+n, buflen-n, "( ");
  n += qsptr_crepr(mem, qspair_ref_a(mem,p), buf+n, buflen-n);
  n += snprintf(buf+n, buflen-n, " . ");
  n += qsptr_crepr(mem, qspair_ref_a(mem,p), buf+n, buflen-n);
  n += snprintf(buf+n, buflen-n, ")");
  return n;
}




/* List semantics: pair, vector, immlist, iter.
   See also qsiter_*
 */
qsptr_t qslist (qsmem_t * mem, qsptr_t p)
{
  if (qsiter_p(mem, p)) return p;
  if (qspair_p(mem, p)) return qspair_iter(mem, p);
  if (qsimmlist_p(mem, p)) return qsimmlist_iter(mem, p, 0);
  if (qsvector_p(mem, p)) return qsvector_iter(mem, p, 0);
  return QSNIL;
}

bool qslist_p (qsmem_t * mem, qsptr_t p)
{
  if (qsiter_p(mem, p)) return 1;
  if (qspair_p(mem, p)) return 1;
  if (qsimmlist_p(mem, p)) return 1;
  return 0;
}

qsword qslist_length (qsmem_t * mem, qsptr_t p)
{
  qsptr_t it = qslist(mem, p);
  if (ISNIL(it)) return 0;

  qsword len = 1;
  /* Count how many times next() can be called until iterator terminates. */
  /* TODO: bounds for circular list. */
  while (qsiter_p(mem, it))
    {
      it = qsiter_next(mem, it);
      len++;
    }
  return len-1;
}

qsptr_t qslist_tail (qsmem_t * mem, qsptr_t p, qsword nth)
{
  qsptr_t it = qslist(mem, p);
  if (ISNIL(it)) return QSNIL;

  while (nth > 0)
    {
      it = qsiter_next(mem, it);
      nth--;
    }
  return it;
}

qsptr_t qslist_ref (qsmem_t * mem, qsptr_t p, qsword k)
{
  qsptr_t it = qslist_tail(mem, p, k);
  if (ISNIL(it)) return QSNIL;
  if (ISERROR16(it)) return it;
  return qsiter_item(mem, it);
}

int qslist_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen)
{
  int n = 0;

  n += snprintf(buf+n, buflen-n, "(");
  qsptr_t it = qslist(mem, p);
  qsptr_t it0 = it;
  while (ISITER28(it) && (n < buflen))
    {
      if (it != it0)
        n += snprintf(buf+n, buflen-n, " ");
      qsptr_t elt = qsiter_item(mem, it);
      if (ISERROR16(elt))
	break;
      if (ISITER28(elt))
	{
	  n += qslist_crepr(mem, elt, buf+n, buflen-n);
	}
      else
	{
	  n += qsptr_crepr(mem, elt, buf+n, buflen-n);
	}
      it = qsiter_next(mem, it);
      if (ISERROR16(it))
	break;
    }
  n += snprintf(buf+n, buflen-n, ")");

  return n;
}





PREDICATE(qsvector)
{
  FILTER_ISA(qsobj_p)   return 0;
  FILTER_IS_MULTIBAY	return 0;
  FILTER_IS_POINTER     return 0;
  FILTER_E_ISA(qsint_p)	return 0;
  return 1;
}

FIELD_RW(qsptr_t, qsvector, len, 1, QSINT,CINT30)

qsword qsvector_length (qsmem_t * mem, qsptr_t p)
{
  return qsvector_ref_len(mem, p);
}

qsptr_t qsvector_ref (qsmem_t * mem, qsptr_t p, qsword ofs)
{
  static const int hdr_adjust = 1 * (sizeof(qsbay0_t) / sizeof(qsptr_t));
  FILTER_ISA(qsvector_p)    return 0;
  qsword lim = qsvector_length(mem, p);
  if ((ofs < 0) || (ofs >= lim))
    {
      // TODO: exception.
      return QSERROR_RANGE;
    }
  return qsobj_ref_ptr(mem, p, ofs + hdr_adjust);  /* ._d[ofs] */
}

qsptr_t qsvector_setq (qsmem_t * mem, qsptr_t p, qsword ofs, qsptr_t val)
{
  static const int hdr_adjust = 1 * (sizeof(qsbay0_t) / sizeof(qsptr_t));
  FILTER_ISA(qsvector_p)    return 0;
  qsword lim = qsvector_length(mem, p);
  if ((ofs < 0) || (ofs >= lim))
    {
      // TODO: exception.
      return QSNIL;
    }
  qsobj_setq_ptr(mem, p, ofs + hdr_adjust, val);  /* ._d[ofs] = val */
  return p;
}

qsptr_t qsvector_make (qsmem_t * mem, qsword k, qsptr_t fill)
{
  static const int hdr_adjust = 1 * (sizeof(qsbay0_t) / sizeof(qsptr_t));
  static const int nptr_per_bay = sizeof(qsbay0_t) / sizeof(qsptr_t);
  qsptr_t retval = QSNIL;
  qsmemaddr_t addr = 0;
  qsword nbays = 1 + (k / nptr_per_bay)+1;  // always terminate with QSEOL.
  if (!ISOBJ26((retval = qsobj_make(mem, nbays, 0, &addr)))) return retval;

  qsobj_setq_ptr(mem, retval, 1, QSINT(k)); /* len = QSINT(k) */
  qsobj_setq_ptr(mem, retval, 2, QSNIL);    /* gc_backtrack = QSNIL */
  qsobj_setq_ptr(mem, retval, 3, QSNIL);    /* gc_iter = QSNIL */
  qsword i;
  qsptr_t * raw_data = (qsptr_t*)qsobj_ref_data(mem, retval, NULL);
  for (i = 0; i < k; i++)
    {
      raw_data[i] = fill;  /* ._d[i] = fill */
    }
  retval = QSOBJ(addr);
  /* Terminate with QSEOL for sake of qsiter. */
  qsobj_setq_ptr(mem, retval, k + hdr_adjust, QSEOL);
  return retval;
}

qserror_t qsvector_kmark (qsmem_t * mem, qsptr_t p, qsptr_t backptr, qsptr_t * nextptr)
{
  if (! qsvector_p(mem, p))
    {
      *nextptr = backptr;
      return QSERROR_OK;
    }
  return qsobj_kmark_multibayptr(mem, p, backptr, nextptr);
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

qsptr_t qsvector_iter (qsmem_t * mem, qsptr_t p, qsword ofs)
{
  static const int hdr_adjust = 1 * (sizeof(qsbay0_t) / sizeof(qsptr_t));
  qsmemaddr_t addr = (COBJ26(p) << 2) + hdr_adjust + ofs;
  return QSITER(addr);
}

qsptr_t * qsvector_cptr (qsmem_t * mem, qsptr_t p, qsword * out_len)
{
  FILTER_ISA(qsvector_p)    return NULL;
  static const int bytes_per_ptr = sizeof(qsptr_t);
  if (out_len)
      *out_len = qsvector_length(mem, p);
  return (qsptr_t*)qsobj_ref_data(mem, p, NULL);  /* ._d */
}




PREDICATE(qsimmlist)
{
  FILTER_ISA(qsobj_p)   return 0;
  FILTER_IS_MULTIBAY	return 0;
  FILTER_IS_POINTER     return 0;
  FILTER_E_IS(QSNIL)    return 0;
  return 1;
}

/* length, counting raw encoding elements. */
qsword qsimmlist_len (qsmem_t * mem, qsptr_t p)
{
  FILTER_ISA(qsimmlist_p)   return 0;

  qsptr_t * _d = (qsptr_t*)qsobj_ref_data(mem, p, NULL);
  qsword depth = 1;
  qsword len = 0;
  while (depth > 0)
    {
      qsptr_t elt = _d[len];
      if (elt == QSBOL)
	depth++;
      else if (elt == QSEOL)
	depth--;
      else if (ISSYNC29(elt))
	depth = 0;
      len++;
    }
  return len-1;
}

/* length, accouting for nested lists. */
qsword qsimmlist_length (qsmem_t * mem, qsptr_t p)
{
  FILTER_ISA(qsimmlist_p)   return 0;

  qsptr_t * elts = (qsptr_t*)qsobj_ref_data(mem, p, NULL);
  qsword depth = 1;
  qsword len = 0;
  while (depth > 0)
    {
      if (*elts == QSBOL)
	depth++;
      else if (*elts == QSEOL)
	depth--;
      else if (ISSYNC29(*elts))
	depth = 0;
      if (depth == 1)
	len++;
      elts++;
    }
  return len;
}

/* elements in list, including encoding details.
   see also qsimmlist_ref().
 */
qsptr_t qsimmlist_at (qsmem_t * mem, qsptr_t p, qsword nth)
{
  static const int hdr_adjust = 1 * (sizeof(qsbay0_t) / sizeof(qsptr_t));
  static const int nptr_per_bay = sizeof(qsbay0_t) / sizeof(qsptr_t);
  FILTER_ISA(qsimmlist_p)   return QSERROR_INVALID;

  qsword depth = 1;
  qsword idx = 0;
  qsptr_t * raw_data = (qsptr_t*)qsobj_ref_data(mem, p, NULL);
  while (depth > 0)
    {
      qsptr_t elt = raw_data[idx];
      if (idx >= nth)
	{
	  return elt;
	}
      if (elt == QSBOL)
	depth++;
      else if (elt == QSEOL)
	depth--;
      else if (ISSYNC29(elt))
	depth = 0;
      idx++;
    }
  return QSERROR_RANGE;
}

/* nth element in list, adjusting for nested listed.
   Nexted list yields an iterator on the nested list.
   */
qsptr_t qsimmlist_ref (qsmem_t * mem, qsptr_t p, qsword nth)
{
  static const int hdr_adjust = 1 * (sizeof(qsbay0_t) / sizeof(qsptr_t));
  qsptr_t * raw_data = (qsptr_t*)qsobj_ref_data(mem, p, NULL);
  qsword depth = 1;
  qsword idx = 0;
  while (depth > 0)
    {
      qsptr_t elt = raw_data[idx];
      if (idx == nth)
	{
	  if (elt == QSBOL)
	    {
	      /* iterator starting at word after QSBOL. */
	      return qsimmlist_iter(mem, p, idx+1);
	    }
	  return elt;
	}
      if (elt == QSBOL)
	depth++;
      else if (elt == QSEOL)
	depth--;
      else if (ISSYNC29(elt))
	depth = 0;
      if (depth == 1)
	idx++;
    }
  return QSERROR_RANGE;
}

qsptr_t qsimmlist_make (qsmem_t * mem, qsword k, qsword fill)
{
  static const int hdr_adjust = 1 * (sizeof(qsbay0_t) / sizeof(qsptr_t));
  static const int nptr_per_bay = sizeof(qsbay0_t) / sizeof(qsptr_t);
  qsptr_t retval = QSNIL;
  qsmemaddr_t addr = 0;
  qsword nbays = 1 + (k / nptr_per_bay)+1;  /* always terminate with QSEOL. */
  if (!ISOBJ26((retval = qsobj_make(mem, nbays, 0, &addr)))) return retval;

  qsobj_setq_ptr(mem, retval, 1, QSNIL);    /* len = nil */
  /* use allocsize for absolute maximum allowed. */
  qsobj_setq_ptr(mem, retval, 2, QSNIL);    /* gc_backtrack = QSNIL */
  qsobj_setq_ptr(mem, retval, 3, QSNIL);    /* gc_iter = QSNIL */
  qsptr_t * raw_data = (qsptr_t*)qsobj_ref_data(mem, retval, NULL);
  qsword i;
  for (i = 0; i < k; i++)
    {
      raw_data[i] = fill;  /* ._d[i] = fill */
    }
  retval = QSOBJ(addr);
  /* Terminate with QSEOL for sake of qsiter. */
  qsobj_setq_ptr(mem, retval, k + hdr_adjust, QSEOL);
  return retval;
}

qsptr_t qsimmlist_inject (qsmem_t * mem, qsptr_t * cmem, qsword nptrs)
{
  static const int hdr_adjust = 1 * (sizeof(qsbay0_t) / sizeof(qsptr_t));
  qsptr_t retval = qsimmlist_make(mem, nptrs, QSNIL);
  qsptr_t * _d = qsobj_ref_data(mem, retval, NULL);
  qsword i;
  for (i = 0; i < nptrs; i++)
    {
      _d[i] = cmem[i];
    }
  _d[i] = QSEOL;
  return retval;
}

qsptr_t qsimmlist_injectv (qsmem_t * mem, va_list vp)
{
  int depth = 1;
  qsword nptrs = 0;
  qsptr_t elt = QSNIL;

  /* Count len. */
  va_list vp0;
  va_copy(vp0, vp);
  while (depth > 0)
    {
      elt = va_arg(vp0, qsptr_t);
      if (elt == QSBOL)
	depth++;
      else if (elt == QSEOL)
	depth--;
      nptrs++;
    }
  nptrs++;
  va_end(vp0);

  /* Prepare memory. */
  qsptr_t retval = qsimmlist_make(mem, nptrs, QSNIL);

  /* Populate list. */
  qsword i = 0;
  qsptr_t * raw_data = (qsptr_t*)qsobj_ref_data(mem, retval, NULL);
  for (i = 0; i < nptrs; i++)
    {
      elt = va_arg(vp, qsptr_t);
      raw_data[i] = elt;
    }
  raw_data[i] = QSEOL;

  return retval;
}

qsptr_t qsimmlist_injectl (qsmem_t * mem, ...)
{
  va_list ap;
  va_start(ap, mem);
  qsptr_t retval = qsimmlist_injectv(mem, ap);
  va_end(ap);
  return retval;
}

/* Return iterator (to first element). */
qsptr_t qsimmlist_iter (qsmem_t * mem, qsptr_t p, qsword ofs)
{
  static const int hdr_adjust = 1 * (sizeof(qsbay0_t) / sizeof(qsptr_t));
  qsmemaddr_t iteraddr = 0;
  iteraddr = (COBJ26(p) << 2) + hdr_adjust + ofs;
  return QSITER(iteraddr);
}








/**************/
/* Bytevector */
/**************/

PREDICATE(qsbytevec)
{
  FILTER_ISA(qsobj_p)   return 0;
  FILTER_IS_MULTIBAY    return 0;
  FILTER_IS_OCTETATE    return 0;
  FILTER_E_ISA(qsint_p) return 0;
  return 1;
}

qserror_t qsbytevec_lock (qsmem_t * mem, qsptr_t bv)
{
  return QSERROR_NOIMPL;
}

qserror_t qsbytevec_unlock (qsmem_t * mem, qsptr_t bv)
{
  return QSERROR_NOIMPL;
}

FIELD_RO(qsword, qsbytevec, len, 1, CINT30)
FIELD_RO(qsword, qsbytevec, refcount, 2, CINT30)

qsword qsbytevec_length (qsmem_t * mem, qsptr_t p)
{
  return qsbytevec_ref_len(mem, p);
}

int qsbytevec_ref (qsmem_t * mem, qsptr_t p, qsword ofs)
{
  FILTER_ISA(qsbytevec_p)   return -1;
  qsword max = qsbytevec_length(mem, p);
  if ((ofs < 0) || (ofs >= max))
    return -1;
  int octet = qsobj_ref_octet(mem, p, ofs);
  return octet;
}

qsptr_t qsbytevec_setq (qsmem_t * mem, qsptr_t p, qsword ofs, qsword octet)
{
  FILTER_ISA(qsbytevec_p)    return QSERROR_INVALID;
  qsword max = qsbytevec_length(mem, p);
  if ((ofs < 0) || (ofs >= max))
    return 0;
  qsobj_setq_octet(mem, p, ofs, octet);  /* ._d[ofs] = octet */
  return p;
}

qsptr_t qsbytevec_make (qsmem_t * mem, qsword k, qsword fill)
{
  /* TODO: change name of macro; slightly misleading. */
  OBJ_MAKE_BAYS(k, 1);

  MUTATE_PTR(1, QSINT(k));
  uint8_t * raw_data = qsobj_ref_data(mem, p, NULL);
  uint8_t raw_fill = (uint8_t)fill;
  qsword i;
  for (i = 0; i < k; i++)
    {
      raw_data[i] = raw_fill;  /* ._d[i] = raw_fill */
    }
  RETURN_OBJ;
}

int qsbytevec_crepr (qsmem_t * mem, qsptr_t bv, char * buf, int buflen)
{
  int n = 0;
  return n;
}

uint8_t * qsbytevec_cptr (qsmem_t * mem, qsword p, qsword * out_len)
{
  FILTER_ISA(qsbytevec_p)   return NULL;
  if (out_len)
    *out_len = qsbytevec_length(mem, p);
  return (uint8_t*)qsobj_ref_data(mem, p, NULL);  /* ._d */
}

qsptr_t qsbytevec_inject (qsmem_t * mem, qsword nbytes, uint8_t * carray)
{
  qsptr_t retval;
  retval = qsbytevec_make(mem, nbytes, 0);
  if (! ISOBJ26(retval)) return retval;
  uint8_t * target = qsbytevec_cptr(mem, retval, NULL);
  qsword i = 0;
  for (i = 0; i < nbytes; i++)
    {
      target[i] = carray[i];
    }
  return retval;
}








/****************/
/* Wide numbers */
/****************/

#define OBJ_MAKE_WIDENUM(variant) \
  void * payload = NULL; \
  qsptr_t p = QSNIL; \
  p = qswidenum_make(mem, variant, &payload);
#define WIDENUM_SET_PAYLOAD(native_type) \
  if (payload)  *((native_type*)payload) = val
#define WIDENUM_GET_PAYLOAD(native_type, discriminator) \
  qsnumtype_t variant = QSNUMTYPE_NAN; \
  void * payload = qswidenum_ref_payload(mem, p, &variant); \
  if (variant != discriminator)    return QSERROR_INVALID; \
  if (out_val) { *out_val = *((native_type*)payload); }


PREDICATE(qswidenum)
{
  FILTER_ISA(qsobj_p)   return 0;
  FILTER_IS_OCTETATE    return 0;
  FILTER_IS_UNIBAY      return 0;
  qsptr_t discriminator = qsobj_ref_ptr(mem, p, 1);
  switch (discriminator)
    {
    case QSNUMTYPE_NAN:
    case QSNUMTYPE_LONG:
    case QSNUMTYPE_DOUBLE:
    case QSNUMTYPE_INT2:
    case QSNUMTYPE_FLOAT2:
    case QSNUMTYPE_FLOAT4:
    case QSNUMTYPE_FLOAT16CM:
      return 1;
    default:
      return 0;
    }
}

qsnumtype_t qswidenum_ref_variant (qsmem_t * mem, qsptr_t p)
{
  FILTER_ISA(qswidenum_p)   return QSNUMTYPE_NAN;
  qsptr_t variant = ACCESS_PTR(1);
  return variant;
}

void * qswidenum_ref_payload (qsmem_t * mem, qsptr_t p, qsnumtype_t * numtype)
{
  FILTER_ISA(qswidenum_p)   return NULL;
  if (numtype)
    *numtype = ACCESS_PTR(1);
  return qsobj_ref_data(mem, p, NULL);
}

/*
   Returns object ready to take wide number.

   Writes out pointer to payload region into '*payload' (to be cast to native pointer type).
*/
qsptr_t qswidenum_make (qsmem_t * mem, qsnumtype_t variant, void ** payload)
{
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
      if (!ISOBJ26((p = qsobj_make(mem, 0, 1, &addr))))
       	return QSERROR_NOMEM;
      qsobj_setq_ptr(mem, p, 1, variant);
      if (payload)
	*payload = (void*)qsobj_ref_data(mem, p, NULL);
      break;
    default:
      qsobj_setq_ptr(mem, p, 1, QSNUMTYPE_NAN);
      break;
    }
  return p;
}




PREDICATE(qslong)
{
  FILTER_ISA(qswidenum_p)     return 0;
  FILTER_E_IS(QSNUMTYPE_LONG) return 0;
  return 1;
}

qserror_t qslong_fetch (qsmem_t * mem, qsptr_t p, long * out_val)
{
  FILTER_ISA(qslong_p)    return QSERROR_INVALID;
  WIDENUM_GET_PAYLOAD(long, QSNUMTYPE_LONG);
  return QSERROR_OK;
}

long qslong_get (qsmem_t * mem, qsptr_t p)
{
  FILTER_ISA(qslong_p)    return 0;

  long retval = 0;
  qserror_t err = qslong_fetch(mem, p, &retval);
  if (err != QSERROR_OK)  return 0;
  return retval;
}

qsptr_t qslong_make (qsmem_t * mem, long val)
{
  OBJ_MAKE_WIDENUM(QSNUMTYPE_LONG);
  WIDENUM_SET_PAYLOAD(long);
  return p;
}

qsptr_t qslong_make2 (qsmem_t * mem, int32_t high, uint32_t low)
{
  return QSERROR_NOIMPL;
}

int qslong_crepr (qsmem_t * mem, qsptr_t l, char * buf, int buflen)
{
  int n = 0;
  n += snprintf(buf+n, buflen-n, "%ld", qslong_get(mem, l));
  return n;
}




PREDICATE(qsdouble)
{
  FILTER_ISA(qswidenum_p)         return 0;
  FILTER_E_IS(QSNUMTYPE_DOUBLE)   return 0;
  return 1;
}

qserror_t qsdouble_fetch (qsmem_t * mem, qsptr_t p, double * out_val)
{
  FILTER_ISA(qswidenum_p)   return QSERROR_INVALID;
  WIDENUM_GET_PAYLOAD(double, QSNUMTYPE_DOUBLE);
  return QSERROR_OK;
}

double qsdouble_get (qsmem_t * mem, qsptr_t p)
{
  FILTER_ISA(qsdouble_p)  return QSERROR_INVALID;
  double retval = 0;
  qserror_t err = qsdouble_fetch(mem, p, &retval);
  if (err)
    return 0;
  return retval;
}

qsptr_t qsdouble_make (qsmem_t * mem, double val)
{
  OBJ_MAKE_WIDENUM(QSNUMTYPE_DOUBLE);
  WIDENUM_SET_PAYLOAD(double);
  return p;
}

int qsdouble_crepr (qsmem_t * mem, qsptr_t d, char * buf, int buflen)
{
  int n = 0;
  n += snprintf(buf+n, buflen-n, "%g", qsdouble_get(mem, d));
  return n;
}




int qswidenum_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  qsnumtype_t variant = qswidenum_ref_variant(mem, p);
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

PREDICATE(qsiter)
{
  if (ISITER28(p)) return 1;
  return 0;
}

qsptr_t qsiter_make (qsmem_t * mem, qsmemaddr_t addr)
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
      if (qspair_p(mem, pairptr))
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
      /* iter is pointing to pair bay. */
      return qspair_ref_a(mem, pairptr);
    }
  else if (ISITER28(it))
    {
      qsword ofs = qsiter_get(mem, it);
      qsptr_t ref = QSERROR_INVALID;
      qserror_t res = qsstore_fetch_word(mem, ofs, &ref);
      if (res != QSERROR_OK)
	return QSNIL;
      if (ref == QSBOL)
	{
	  // nested list; nested iterator.
	  qsptr_t iter2 = QSITER( CITER28(it)+1 );
	  return iter2;
	}
      else if (ref == QSEOL)
	{
	  // failure in qsiter_next() for returning pointer to QSEOL.
	  return QSERROR_RANGE;
	}
      return ref;
    }
  // TODO: exception.
  return QSERROR_INVALID;
}

qsptr_t qsiter_next (qsmem_t * mem, qsptr_t it)
{
  qsptr_t pairptr = QSNIL;
  if (qsiter_on_pair(mem, it, &pairptr))
    {
      /* iter is pointing to pair bay. */
      qsptr_t next = qspair_ref_d(mem, pairptr);
      if (! ISOBJ26(next))
	return QSNIL;
      qsmemaddr_t next_addr = COBJ26(next);
      qsmemaddr_t iter_addr = next_addr << 2;
      return qsiter_make(mem, iter_addr);
    }
  else if (ISITER28(it))
    {
      qsptr_t peek = QSNIL;
      int depth = 0;
      qsword startofs = qsiter_get(mem, it);
      qserror_t res = qsstore_fetch_word(mem, startofs, &peek);
      if (res != QSERROR_OK)
	return QSNIL;
      if (peek == QSBOL) depth++; // goal: skip to end of nested list.

      qsword nextofs = startofs+1;
      res = qsstore_fetch_word(mem, nextofs, &peek);
      if (res != QSERROR_OK)
	return QSNIL;

      while (depth > 0)
	{
	  if (res != QSERROR_OK)
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
	  res = qsstore_fetch_word(mem, nextofs, &peek);
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
  qsptr_t pair = QSNIL;
  qsptr_t elt;
  if (qsiter_on_pair(mem, it, &pair))
    {
      elt = qspair_ref_a(mem, pair);
      if (! ISERROR16(elt))
	n += qsptr_crepr(mem, elt, buf+n, buflen-n);
    }
  else
    {
      elt = qsiter_item(mem, it);
      if (elt == QSBOL)
	{
	  n += snprintf(buf+n, buflen-n, "(");
	  n += qslist_crepr(mem, qsiter_next(mem,it), buf+n, buflen-n);
	  n += snprintf(buf+n, buflen-n, ")");
	}
      else if (! ISERROR16(elt))
        n += qsptr_crepr(mem, elt, buf+n, buflen-n);
    }
  return n;
}




PREDICATE(qsint)
{
  return ISINT30(p);
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

PREDICATE(qsfloat)
{
  return ISFLOAT31(p);
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

bool qschar_p (qsmem_t * mem, qsptr_t p)
{
  return ISCHAR24(p);
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

CMP_FUNC(qschar)
{
  if (!qschar_p(mem, a) &&  !qschar_p(mem, b))
    {
      return CMP_NE;
    }
  if (!qschar_p(mem, a))
    return CMP_LT;
  if (!qschar_p(mem,b))
    return CMP_GT;
  int cp_a = CCHAR24(a);
  int cp_b = CCHAR24(b);
  if (cp_a < cp_b) return CMP_LT;
  if (cp_a > cp_b) return CMP_GT;
  return CMP_EQ;
}




qsptr_t qserr (qsmem_t * mem, qsptr_t e)
{
  if (ISERROR16(e)) return e;
  return QSNIL;
}

bool qserr_p (qsmem_t * me, qsptr_t p)
{
  return ISERROR16(p);
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

PREDICATE(qsconst)
{
  return ISCONST16(p);
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
      n += snprintf(buf+n, buflen-n, "()");
      break;
    case QSTRUE:
      n += snprintf(buf+n, buflen-n, "#t");
      break;
    case QSFALSE:
      n += snprintf(buf+n, buflen-n, "#f");
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

/* String:
   1. conschar24: pair objects of char24
   2. vecchar24: vector of char24 with BOL termination
   3. qsutf8: bytevector (UTF-8)
 */


/*
   qsutf8 = using qsbytevector space as C string.
   Canonical string format (due to interchange with string.h functions).
*/

static
qsword _qsutf8_hardlimit (qsmem_t * mem, qsptr_t s)
{
  static const int nbyte_per_bay = sizeof(qsbay0_t);
  qsword nbays = qsobj_ref_nbays(mem, s);
  qsword retval = nbays * nbyte_per_bay;
  return retval;
}

PREDICATE(qsutf8)
{
  FILTER_ISA(qsobj_p)   return 0;
  FILTER_IS_OCTETATE    return 0;
  FILTER_IS_MULTIBAY    return 0;
  FILTER_E_IS(QSNIL)    return 0;
  return 1;
}

qsword qsutf8_length (qsmem_t * mem, qsptr_t p)
{
  FILTER_ISA(qsobj_p)  return 0;
  const char * raw_cstr = (const char *)qsobj_ref_data(mem, p, NULL);
  qsword retval = strlen(raw_cstr);
  return retval;
}

int qsutf8_ref (qsmem_t * mem, qsptr_t p, qsword k)
{
  FILTER_ISA(qsobj_p)   return -1;

  qsword max = _qsutf8_hardlimit(mem, p);
  if ((k < 0) || (k >= max))
    {
      return 0;
    }
  return qsobj_ref_octet(mem, p, k);  /* ._d[k] */
}

qsptr_t qsutf8_setq (qsmem_t * mem, qsptr_t p, qsword k, qsword val)
{
  FILTER_ISA(qsobj_p)   return QSERROR_INVALID;

  qsword max = _qsutf8_hardlimit(mem, p);
  if ((k < 0) || (k >= max))
    {
      return QSERROR_RANGE;
    }
  uint8_t raw_val = val & 0xff;
  qsobj_setq_octet(mem, p, k, raw_val);  /* ._d[k] = raw_val */
  return p;
}

qsptr_t qsutf8_make (qsmem_t * mem, qsword slen)
{
  qsword k = slen+1;
  OBJ_MAKE_BAYS(k, 1);

  MUTATE_PTR(1, QSNIL);   /* .variant = nil */
  qsword i;
  uint8_t * raw_cstr = qsobj_ref_data(mem, p, NULL);
  uint8_t raw_fill = 0;
  for (i = 0; i < k; i++)
    {
      raw_cstr[i] = raw_fill;  /* ._d[i] = '\0' */
    }
  RETURN_OBJ;
}

int qsutf8_crepr (qsmem_t * mem, qsptr_t s, char * buf, int buflen)
{
  int n = 0;
  const char * raw_str = (const char *)qsobj_ref_data(mem, s, NULL);
  n += snprintf(buf+n, buflen-n, "%s", raw_str);
  return n;
}

const uint8_t * qsutf8_cptr (qsmem_t * mem, qsptr_t p)
{
  FILTER_ISA(qsutf8_p)    return NULL;
  return qsobj_ref_data(mem, p, NULL);  /* ._d */
}

/* Create qsutf8 object using contents of C string. */
qsptr_t qsutf8_inject (qsmem_t * mem, const char * cstr, qsword slen)
{
  qsptr_t retval;
  qsword k = slen;
  if (k < 1)
    {
      k = strlen(cstr) + 1;
    }
  retval = qsutf8_make(mem, k);
  if (! ISOBJ26(retval)) return retval;
  /*
  uint8_t * target = (uint8_t*)qsutf8_cptr(mem, retval);
  */
  uint8_t * target = (uint8_t*)qsobj_ref_data(mem, retval, NULL);
  qsword i = 0;
  for (i = 0; i < k; i++)
    {
      target[i] = cstr[i];
    }
  target[i] = 0;
  i++;
  return retval;
}

/* Copy out C string from qsutf8 object. */
int qsutf8_extract (qsmem_t * mem, qsptr_t s, char * cstr, qsword slen)
{
  qsword lim = _qsutf8_hardlimit(mem, s);
  const char * _d = qsutf8_cptr(mem, s);
  qsword i;
  for (i = 0; (i < lim) && (i+1 < slen) && (_d[i] != 0); i++)
    {
      cstr[i] = _d[i];
    }
  cstr[i] = 0;
  i++;
  return i;
}




PREDICATE(qsstr)
{
  if (qsutf8_p(mem, p))  return 1;
  else if (qspair_p(mem, p))
    {
      if (ISCHAR24(qspair_ref_a(mem, p))) return 1;
    }
  else if (qsvector_p(mem, p))
    {
      if (ISCHAR24(qsvector_ref(mem, p, 0))) return 1;
    }
  else if (qsbytevec_p(mem, p))
    {
      return 1;
    }
  return 0;
}

static qsword qsstr_length_cons (qsmem_t * mem, qsptr_t s)
{
  qsword n = 0;
  qsptr_t curr = s;
  while (qspair_p(mem,curr))
    {
      n++;
      curr = qspair_ref_d(mem,curr);
    }
  return n;
}

static qsword qsstr_length_vec (qsmem_t * mem, qsptr_t s)
{
  return qsvector_length(mem, s);
}

static qsword qsstr_length_bytevec (qsmem_t * mem, qsptr_t s)
{
  return qsbytevec_length(mem, s);
}

qsword qsstr_length (qsmem_t * mem, qsptr_t s)
{
  qsword lim = 0;
  if (qsutf8_p(mem,s))
    {
      return qsutf8_length(mem, s);
    }
  else if (qspair_p(mem,s))
    {
      lim = qsstr_length_cons(mem,s);
      return qsstr_length_cons(mem,s);
    }
  else if (qsvector_p(mem, s))
    {
      lim = qsvector_length(mem, s);
    }
  else if (qsbytevec_p(mem, s))
    {
      lim = qsbytevec_length(mem, s);
    }
  return lim;
}

static qsptr_t qsstr_ref_cons (qsmem_t * mem, qsptr_t s, qsword nth)
{
  qsword ofs = 0;
  qsptr_t curr = s;
  while ((ofs < nth) && qspair_p(mem,curr))
    {
      ofs++;
    }
  if (!qspair_p(mem,curr))
    {
      return QSERROR_RANGE;
    }
  qsptr_t elt = qspair_ref_a(mem, curr);
  if (ISCHAR24(elt)) return elt;
  else return QSCHAR(0);
}

static qsptr_t qsstr_ref_vec (qsmem_t * mem, qsptr_t p, qsword nth, qsword hard_limit)
{
  qsword lim = qsvector_length(mem, p);
  if ((nth >= hard_limit) || (nth < 0) || (nth >= lim))
    return QSERROR_RANGE;
  qsptr_t elt = qsvector_ref(mem, p, nth);
  if (ISCHAR24(elt)) return elt;
  else return QSCHAR(0);
}

static qsptr_t qsstr_ref_immlist (qsmem_t * mem, qsptr_t p, qsword nth)
{
  qsword depth = 1;
  size_t hard_limit = 0;
  qsptr_t * raw_data = (qsptr_t*)qsobj_ref_data(mem, p, &hard_limit);
  qsptr_t elt = QSNIL;
  while ((depth > 1) && (nth > 0))
    {
      elt = raw_data[nth];
      if (!ISCHAR24(elt))
	return QSCHAR(0);
    }
  if (ISCHAR24(elt)) return elt;
  else return QSCHAR(0);
}

static qsptr_t qsstr_ref_bytevec (qsmem_t * mem, qsptr_t p, qsword nth, qsword hard_limit)
{
  qsword lim = qsbytevec_length(mem, p);
  if ((nth >= hard_limit) || (nth < 0) || (nth >= lim))
    return QSERROR_RANGE;
  qsword ch = qsbytevec_ref(mem, p, nth);
  return QSCHAR(ch);
}

qsptr_t qsstr_ref (qsmem_t * mem, qsptr_t s, qsword nth)
{
  qsword lim = 0;
  if (qsutf8_p(mem, s))
    {
      return qsutf8_ref(mem, s, nth);
    }
  else if (qspair_p(mem, s))
    {
      return qsstr_ref_cons(mem, s, nth);
    }
  else if (qsvector_p(mem, s))
    {
      lim = qsvector_length(mem, s);
      return qsstr_ref_vec(mem, s, nth, lim);
    }
  else if (qsimmlist_p(mem, s))
    {
      return qsstr_ref_immlist(mem, s, nth);
    }
  else if (qsbytevec_p(mem, s))
    {
      lim = qsbytevec_length(mem, s);
      return qsstr_ref_bytevec(mem, s, nth, lim);
    }
  return QSCHAR(0);
}

qsptr_t qsstr_setq (qsmem_t * mem, qsptr_t s, qsword nth, qsword codepoint)
{
  qsword lim = 0;
  if (qsutf8_p(mem, s))
    {
      return qsutf8_setq(mem, s, nth, codepoint);
    }
  else if (qsvector_p(mem, s))
    {
      lim = qsvector_length(mem, s);
      if (nth >= lim) return QSERROR_RANGE;
      qsvector_setq(mem, s, nth, QSCHAR(codepoint));
      return s;
    }
  else if (qsbytevec_p(mem, s))
    {
      lim = qsbytevec_length(mem, s);
      int filtered = codepoint & 0xff;
      if (nth >= lim) return QSERROR_RANGE;
      qsbytevec_setq(mem, s, nth, filtered);
      return s;
    }
  /* cannot be modified in place. */
  return QSERROR_INVALID;
}

qsptr_t qsstr_make (qsmem_t * mem, qsword k, qsword codepoint_fill)
{
  qsptr_t retval = QSNIL;
  /* default is qsutf8 */
  retval = qsutf8_make(mem, k);
  return retval;
}

/* inject C string as utf-8 bytevector */
qsptr_t qsstr_inject (qsmem_t * mem, const char * cstr, qsword slen)
{
  return qsutf8_inject(mem, cstr, slen);
}

/* inject C wide-char string as utf-32 vector */
qsptr_t qsstr_inject_wchar (qsmem_t * mem, const wchar_t * ws, qsword wslen)
{
  qsptr_t retval = QSNIL;
  retval = qsvector_make(mem, wslen, QSCHAR(0));
  if (! ISOBJ26(retval)) return retval;
  qsword i = 0;
  for (i = 0; i < wslen; i++)
    {
      qsvector_setq(mem, retval, i, QSCHAR(ws[i]));
    }
  return retval;
}

/* copy out as C string; list/vector of char to char, or copy bytevector. */
qsword qsstr_extract (qsmem_t * mem, qsptr_t s, char * cstr, qsword slen)
{
  // TODO: support iter28
  qsword retval = 0;
  qsword lim = 0;
  qsword idx = 0;
  if (qsutf8_p(mem, s))
    {
      return qsutf8_extract(mem, s, cstr, slen);
    }
  else if (qspair_p(mem, s))
    {
      qsptr_t curr = s;
      mbstate_t ps = { 0, };
      while (qspair_p(mem, curr) && (idx+1 < slen-MB_CUR_MAX))
	{
	  qsptr_t elt = qspair_ref_a(mem, curr);
	  wchar_t ch = ISCHAR24(elt) ? CCHAR24(elt) : 0;
	  int span = wcrtomb(cstr + idx, ch, &ps);
	  if (span > 0)
	    {
	      idx += span;
	    }
	  else
	    {
	      abort();
	    }

	  curr = qspair_ref_d(mem, curr);
	}
      cstr[idx] = 0;
      retval = idx;
    }
  else if (qsvector_p(mem, s))
    {
      qsword i;
      mbstate_t ps = { 0, };
      lim = qsvector_length(mem, s);
      for (i = 0; (i+1 < slen) && (i < lim); i++)
	{
	  qsptr_t elt = qsvector_ref(mem, s, i);
	  wchar_t ch = ISCHAR24(elt) ? CCHAR24(elt) : 0;
	  int span = wcrtomb(cstr + idx, ch, &ps);
	  if (span > 0)
	    {
	      idx += span;
	    }
	  else
	    {
	      abort();
	    }
	}
      cstr[idx] = 0;
      retval = idx;
    }
  else if (qsimmlist_p(mem, s))
    {
      qsptr_t elt = QSNIL;
      size_t hardlimit = 0;
      mbstate_t ps = { 0, };
      qsptr_t * elts = qsobj_ref_data(mem, s, &hardlimit);
      while (ISCHAR24(*elts))
	{
	  wchar_t ch = CCHAR24(*elts);
	  int span = wcrtomb(cstr + idx, ch, &ps);
	  if (span > 0)
	    {
	      idx += span;
	    }
	  else
	    {
	      abort();
	    }
	  elts++;
	}
      cstr[idx] = 0;
      retval = idx;
    }
  else if (qsbytevec_p(mem, s))
    {
      lim = qsbytevec_length(mem, s);
      const char * bvstr = qsbytevec_cptr(mem, s, &lim);
      qsword i;
      for (i = 0; (i < lim) && (i+1 < slen); i++)
	{
	  cstr[i] = bvstr[i];
	}
      cstr[i] = 0;
      retval = i;
    }
  return retval;
}

/* copy out as C wide-string; list/vector of char to wchar, or bytevector to wchar */
qsword qsstr_extract_wchar (qsmem_t * mem, qsptr_t s, wchar_t * ws, qsword wlen)
{
  qsword retval = 0;
  qsword lim = 0;
  qsword idx = 0;
  if (qspair_p(mem, s))
    {
      qsptr_t curr = s;
      while (qspair_p(mem, curr) && (idx+1 < wlen))
	{
	  qsptr_t elt = qspair_ref_a(mem, curr);
	  if (! ISCHAR24(elt)) elt = QSCHAR(0);
	  ws[idx] = CCHAR24(elt);
	  idx++;
	  curr = qspair_ref_d(mem, curr);
	}
      ws[idx] = 0;
      retval = idx;
    }
  else if (qsvector_p(mem, s))
    {
      qsword i;
      lim = qsvector_length(mem, s);
      for (i = 0; (i+1 < wlen) && (i < lim); i++)
	{
	  qsptr_t elt = qsvector_ref(mem, s, i);
	  if (! ISCHAR24(elt)) elt = QSCHAR(0);
	  ws[idx] = qsvector_ref(mem, s, idx);
	  idx++;
	}
      ws[idx] = 0;
      retval = idx;
    }
  else if (qsimmlist_p(mem, s))
    {
      qsptr_t elt = QSNIL;
      size_t hardlimit = 0;
      qsptr_t * elts = qsobj_ref_data(mem, s, &hardlimit);
      hardlimit /= (sizeof(qsbay0_t));
      while (ISCHAR24(*elts))
	{
	  ws[idx] = CCHAR24(*elts);
	  idx++;
	  elts++;
	}
      ws[idx] = 0;
      retval = idx;
    }
  else if (qsbytevec_p(mem, s))
    {
      lim = qsbytevec_length(mem, s);
      const char * cstr = qsbytevec_cptr(mem, s, &lim);
      retval = mbstowcs(ws, cstr, wlen);
    }
  return retval;
}

cmp_t qsstr_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b)
{
  cmp_t retval = 0;
  if (a == b) return CMP_EQ;
  qsword k = 0;
  qsword lim_a = qsstr_length(mem, a);
  qsword lim_b = qsstr_length(mem, b);
  qsword lim = (lim_a < lim_b) ? lim_a : lim_b;
  for (k = 0; k < lim; k++)
    {
      int ch_a = k < lim_a ? qsstr_ref(mem, a, k) : 0;
      int ch_b = k < lim_b ? qsstr_ref(mem, b, k) : 0;
      if (ch_a < ch_b) return CMP_LT;
      if (ch_a > ch_b) return CMP_GT;
      if (ch_a <= 0) return CMP_LT;
      if (ch_b <= 0) return CMP_GT;
      // else ch_a == ch_b  =>  continue
    }
  if (lim_a < lim_b) return CMP_GT;
  else if (lim_a > lim_b) return CMP_LT;
  return CMP_EQ;
}

int qsstr_crepr (qsmem_t * mem, qsptr_t s, char * buf, int buflen)
{
  int n = 0;
  if (qsutf8_p(mem, s))
    {
      n += snprintf(buf+n, buflen-n, "\"");
      n += qsutf8_crepr(mem, s, buf+n, buflen-n);
      n += snprintf(buf+n, buflen-n, "\"");
    }
  else if (qspair_p(mem, s))
    {
    }
  else if (qsvector_p(mem, s))
    {
    }
  else if (qsimmlist_p(mem, s))
    {
    }
  return n;
}




/* Symbols. */
/* qssym - interned symbols. */
qsptr_t qssym (qsmem_t * mem, qsptr_t y)
{
  if (ISSYM26(y)) return y;
  else return QSNIL;
}

bool qssym_p (qsmem_t * mem, qsptr_t p)
{
  return ISSYM26(p);
}

qsword qssym_get (qsmem_t * mem, qsptr_t y)
{
  if (ISNIL(qssym(mem, y))) return 0;
  return CSYM26(y);
}

qsptr_t qssym_make (qsmem_t * mem, qsptr_t symbol_id)
{
  return QSSYM(symbol_id);
}

int qssym_crepr (qsmem_t * mem, qsptr_t y, char * buf, int buflen)
{
  int n = 0;
  return n;
}



/* qssymbol - tie name and id. */
qssymbol_t * qssymbol (qsmem_t * mem, qsptr_t yy)
{
  qsmemaddr_t out_addr = 0;
  qsobj_t * obj = qsobj(mem, yy, &out_addr);
  if (!obj) return NULL;
  qssymbol_t * symbol = (qssymbol_t*)obj;

  qsptr_t indicator = qsobj_ref_ptr(mem, yy, 1);  /* .indicator */
  if (indicator != QST_SYMBOL) return NULL;

  /* right field should be a symbol id that points back to object itself. */
  if (CINT30(qstree_ref_right(mem, yy)) != COBJ26(yy)) return NULL;
  return symbol;
}

PREDICATE(qssymbol)
{
  FILTER_ISA(qstree_p)    return 0;
  FILTER_E_IS(QST_SYMBOL)   return 0;
  return 1;
}

FIELD_RO(qsptr_t, qssymbol, name, 2, QSID)
FIELD_RO(qsptr_t, qssymbol, id, 3, QSID)

qsptr_t qssymbol_make (qsmem_t * mem, qsptr_t name)
{
  // TODO: copy name?
#if 0
  qsptr_t retval = qstree_make(mem, QSNIL, name, QSNIL);
  if (!ISOBJ26(retval)) return retval;
//  /* left field points back to object. */
//  qstree_setq_left(mem, retval, retval);
  /* right field ends up pointing to self. */
  qsmemaddr_t addr = COBJ26(retval);
  qsptr_t y = qssym_make(mem, addr);
  qstree_setq_right(mem, retval, y);
#else
  int always_intern = 0;
  if (always_intern)
    {
      qsptr_t found = qssymstore_assoc(mem, mem->symstore, name);
      if (qssymbol_p(mem, found))
	return found;
    }

  qsptr_t retval = qstree_make(mem, QST_SYMBOL, name, QSNIL);
  if (!ISOBJ26(retval)) return retval;
  /* right field value to address value. */
  qstree_setq_right(mem, retval, QSINT(COBJ26(retval)));

  if (always_intern)
    {
      // (?) always intern
      if (ISNIL(mem->symstore))
	{
	  mem->symstore = qssymstore_make(mem);
	}
      qssymstore_intern(mem, mem->symstore, retval);
    }
#endif //0

  return retval;
}

CMP_FUNC(qssymbol)
{
  if (! qssymbol_p(mem, a)) return CMP_NE;
  if (! qssymbol_p(mem, b)) return CMP_NE;
  if (a == b) return CMP_EQ;

  /*
     Symbols spelled the same would have same symbol addresses.
     At this point, comparison would be for hashing/keying purposes,
     so compare hashable values.

     For now, compare addresses.
   */
  if (COBJ26(a) < COBJ26(b))
    return CMP_LT;
  else
    return CMP_GT;
}

int qssymbol_crepr (qsmem_t * mem, qsptr_t y, char * buf, int buflen)
{
  int n = 0;
  qsptr_t s = qssymbol_ref_name(mem, y);
  n += qsstr_extract(mem, s, buf+n, buflen-n) - 1;
  return n;
}



/* symstore - composed of two structures:
   1. symtable - indexed by integer, resolves symbol_id to symbol_name.
   2. symtree - indexed by name, resolves symbol_name to symbol_id.
*/

PREDICATE(qssymstore)
{
  FILTER_ISA(qstree_p)      return 0;
  FILTER_E_IS(QST_SYMSTORE) return 0;
  return 1;
}

FIELD_RW(qsptr_t, qssymstore, table, 2, QSID,QSID)
FIELD_RW(qsptr_t, qssymstore, tree, 3, QSID,QSID)

qsptr_t qssymstore_make (qsmem_t * mem)
{
  OBJ_MAKE_BAYS(1, 0);

  qsptr_t tag = QST_SYMSTORE;
  qsptr_t table = qsibtree_make(mem);
  qsptr_t tree = qsrbtree_make(mem, QSNIL, QSNIL);

  MUTATE_PTR(1, tag);     /* .tag = tag */
  MUTATE_PTR(2, table);   /* .table = table */
  MUTATE_PTR(3, tree);    /* .tree = tree */

  if (ISNIL(mem->symstore))
    {
      /* system symstore auto-binds to first created symstore. */
      mem->symstore = p;
    }

  return p;
}

/* symbol objects are not interned by default. */
qsptr_t qssymstore_intern (qsmem_t * mem, qsptr_t o, qsptr_t symbol_object)
{
  /* insert linearly into table. */
#if 0
  qsptr_t ibtree = qssymstore_ref_table(mem, o);
  qsword symid = qsibtree_ref_filled(mem, ibtree);
  ibtree = qsibtree_setq(mem, ibtree, symid, symbol_object);
  qssymstore_setq_table(mem, o, ibtree);
#endif //0
  /* insert lookup into tree. */
  qsptr_t rbtree = qssymstore_ref_tree(mem, o);
  rbtree = qsrbtree_insert(mem, rbtree, symbol_object);
  qssymstore_setq_tree(mem, o, rbtree);
  
  return o;
}

/* Obtain symbol object by id:int. */
qsptr_t qssymstore_ref (qsmem_t * mem, qsptr_t o, qsword sym_id)
{
  qsptr_t ibtree = qssymstore_ref_table(mem, o);
  qsptr_t retval = qsibtree_ref(mem, ibtree, sym_id);
  return retval;
}

/* Obtain symbol object by name:str. */
qsptr_t qssymstore_assoc (qsmem_t * mem, qsptr_t o, qsptr_t key)
{
  qsptr_t rbtree = qssymstore_ref_tree(mem, o);
  qsptr_t retval = qsrbtree_assoc(mem, rbtree, key);
  return retval;
}

int qssymstore_crepr (qsmem_t * mem, qsptr_t o)
{
  int n = 0;
  return n;
}




/************************************/
/* Execution model data structures. */
/************************************/

/* Environment. */

PREDICATE(qsenv)
{
  FILTER_ISA(qstree_p)  return 0;
  FILTER_E_IS(QST_ENV)  return 0;
  return 1;
}

FIELD_RW(qsptr_t, qsenv, dict, 2, QSID,QSID)
FIELD_RW(qsptr_t, qsenv, next, 3, QSID,QSID)

qsptr_t qsenv_assoc (qsmem_t * mem, qsptr_t p, qsptr_t key)
{
  FILTER_ISA(qsenv_p)   return QSNIL;
  qsptr_t dict = qsenv_ref_dict(mem, p);
  if (ISNIL(dict))      return QSNIL;
  qsptr_t apair = qsrbtree_assoc(mem, dict, key);
  return apair;
}

qsword qsenv_sublen (qsmem_t * mem, qsptr_t treenode)
{
  int n = 1;
  qsptr_t left = qstree_ref_left(mem, treenode);
  qsptr_t right = qstree_ref_right(mem, treenode);
  if (ISOBJ26(left))
    {
      n += qsenv_sublen(mem, left);
    }
  if (ISOBJ26(right))
    {
      n += qsenv_sublen(mem, right);
    }
  return n;
}

qsword qsenv_length (qsmem_t * mem, qsptr_t p)
{
  FILTER_ISA(qsenv_p)	return 0;
  qsptr_t dict = qsenv_ref_dict(mem, p);
  if (ISNIL(dict))	return 0;
  qsptr_t tree = qsrbtree_ref_top(mem, dict);
  int n = qsenv_sublen(mem, tree);
  qsptr_t next = qsenv_ref_next(mem, p);
  if (!ISNIL(next))
    {
      //n += qsenv_length(mem, next);
    }
  return n;
}

qsptr_t qsenv_ref (qsmem_t * mem, qsptr_t p, qsptr_t key)
{
  qsptr_t frame = p;
  while (qsenv_p(mem, frame))
    {
      qsptr_t apair = qsenv_assoc(mem, frame, key);
      if (qstree_p(mem, apair))
        {
          return qspair_ref_d(mem, apair);
        }
      /* Search next frame. */
      frame = qsenv_ref_next(mem, frame);
    }
  return QSERROR_INVALID;
}

qsptr_t qsenv_setq (qsmem_t * mem, qsptr_t p, qsptr_t key, qsptr_t val)
{
  /*
  FILTER_ISA(qsenv_p)   return p;
  */
  if (! qsenv_p(mem, p))
    {
      p = qsenv_make(mem, QSNIL);
    }

  qsptr_t apair = qsenv_assoc(mem, p, key);
  if (ISNIL(apair))
    {
      apair = qspair_make(mem, key, val);
      qsptr_t dict = qsenv_ref_dict(mem, p);
      dict = qsrbtree_insert(mem, dict, apair);
    }
  else
    {
      qspair_setq_d(mem, apair, val);
    }
  return p;
}

qsptr_t qsenv_make (qsmem_t * mem, qsptr_t next)
{
  OBJ_MAKE_BAYS(1, 0);

  qsptr_t dict = qsrbtree_make(mem, QSNIL, QSNIL);

  MUTATE_PTR(1, QST_ENV);
  MUTATE_PTR(2, dict);
  MUTATE_PTR(3, next);

  RETURN_OBJ;
}

CMP_FUNC_NAIVE(qsenv);

int qsenv_helper_crepr (qsmem_t * mem, qsptr_t x, char * buf, int buflen)
{
  int n = 0;
  qsptr_t left = qstree_ref_left(mem, x);
  qsptr_t data = qstree_ref_data(mem, x);
  qsptr_t right = qstree_ref_right(mem, x);
  if (!ISNIL(left)) {
    n += qsenv_helper_crepr(mem, left, buf+n, buflen-n);
    n += snprintf(buf+n, buflen-n, " ");
  }
  if (qspair_p(mem, data))
    {
      qsptr_t a = qspair_ref_a(mem, data);
      qsptr_t d = qspair_ref_d(mem, data);
//      n += snprintf(buf+n, buflen-n, " ");
      n += qsptr_crepr(mem, a, buf+n, buflen-n);
      n += snprintf(buf+n, buflen-n, ":");
      n += qsptr_crepr(mem, d, buf+n, buflen-n);
//      n += snprintf(buf+n, buflen-n, " ");
    }
  else
    {
      n += qsptr_crepr(mem, data, buf+n, buflen-n);
    }

  if (!ISNIL(right)) {
    n += snprintf(buf+n, buflen-n, " ");
    n += qsenv_helper_crepr(mem, right, buf+n, buflen-n);
  }
  return n;
}

int qsenv_crepr (qsmem_t * mem, qsptr_t e, char * buf, int buflen)
{
  int n = 0;

  qsptr_t rbtree = qsenv_ref_dict(mem, e);
  qsptr_t top = qsrbtree_ref_top(mem, rbtree);
  qsptr_t probe = top;
  n += snprintf(buf+n, buflen-n, "(ENV ");
//  while (!ISNIL(probe))
    {
      n += qsenv_helper_crepr(mem, probe, buf+n, buflen-n);
      //n += qsrbtree_node_crepr(mem, probe, buf+n, buflen-n);
    }
  n += snprintf(buf+n, buflen-n, ")");

  return n;
}



PREDICATE(qslambda)
{
  FILTER_ISA(qstree_p)    return 0;
  FILTER_E_IS(QST_LAMBDA) return 0;
  return 1;
}

FIELD_RW(qsptr_t, qslambda, param, 2, QSID,QSID)
FIELD_RW(qsptr_t, qslambda, body, 3, QSID,QSID)

qsptr_t qslambda_make (qsmem_t * mem, qsptr_t param, qsptr_t body)
{
  OBJ_MAKE_BAYS(1, 0);

  MUTATE_PTR(1, QST_LAMBDA);
  MUTATE_PTR(2, param);
  MUTATE_PTR(3, body);

  RETURN_OBJ;
}

CMP_FUNC_NAIVE(qslambda);

int qslambda_crepr (qsmem_t * mem, qsptr_t lam, char * buf, int buflen)
{
  int n = 0;
  return n;
}




PREDICATE(qsclosure)
{
  FILTER_ISA(qstree_p)      return 0;
  FILTER_E_IS(QST_CLOSURE)  return 0;
  return 1;
}

FIELD_RW(qsptr_t, qsclosure, env, 2, QSID,QSID)
FIELD_RW(qsptr_t, qsclosure, lambda, 3, QSID,QSID)

qsptr_t qsclosure_make (qsmem_t * mem, qsptr_t env, qsptr_t lambda)
{
  OBJ_MAKE_BAYS(1, 0);

  MUTATE_PTR(1, QST_CLOSURE);
  MUTATE_PTR(2, env);
  MUTATE_PTR(3, lambda);

  RETURN_OBJ;
}

CMP_FUNC_NAIVE(qsclosure);

int qsclosure_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  return n;
}




PREDICATE(qskont)
{
  FILTER_ISA(qsobj_p)     return 0;
  FILTER_IS_POINTER       return 0;
  if (qsobj_ref_allocsize(mem,p) != 2) return 0;

  qsptr_t variant = qsobj_ref_ptr(mem, p, 1);
  switch (variant)
    {
    case QST_KONT:
    case QSKONT_HALT:
    case QSKONT_RETURN:
    case QSKONT_BRANCH:
    case QSKONT_LETK:
      return 1;
      break;
    default:
      return 0;
      break;
    }
}

FIELD_RW(qsptr_t, qskont, env, 4, QSID,QSID)
FIELD_RW(qsptr_t, qskont, kont, 5, QSID,QSID)
FIELD_RW(qsptr_t, qskont, code, 6, QSID,QSID)
FIELD_RW(qsptr_t, qskont, other, 7, QSID,QSID)

qsptr_t qskont_make (qsmem_t * mem, qsptr_t variant, qsptr_t kont, qsptr_t env, qsptr_t code, qsptr_t other)
{
  OBJ_MAKE_BAYS(2, 0);

  MUTATE_PTR(1, QST_KONT);
  MUTATE_PTR(4, env);
  MUTATE_PTR(5, kont);
  MUTATE_PTR(6, code);
  MUTATE_PTR(7, other);

  RETURN_OBJ;
}

CMP_FUNC_NAIVE(qskont);

int qskont_crepr (qsmem_t * mem, qsptr_t kont, char * buf, int buflen)
{
  int n = 0;
  return n;
}



/* Built-in ports. */
PREDICATE(qsSTDIO)
{
  switch (p)
    {
    case QSIO_IN:
    case QSIO_OUT:
    case QSIO_ERR:
      return 1;
    default:
      return 0;
      break;
    }
}

qsptr_t qsSTDIO_make (qsmem_t * mem, qsptr_t stdioe)
{
  qsptr_t retval = QSERROR_INVALID;
  switch (stdioe)
    {
    case QSIO_IN:
    case QSIO_OUT:
    case QSIO_ERR:
      retval = stdioe;
      break;
    default:
      retval = QSERROR_INVALID;
      break;
    }
  return retval;
}

static
FILE * CFILE (qsptr_t stdioe)
{
  switch (stdioe)
    {
    case QSIO_IN: return stdin; break;
    case QSIO_OUT: return stdout; break;
    case QSIO_ERR: return stderr; break;
    default: abort(); break;
    }
}

qsptr_t qsSTDIO_read_u8 (qsmem_t * mem, qsptr_t p)
{
  qsptr_t retval = QSNIL;
  int nextch = 0;
  switch (p)
    {
    case QSIO_IN:
      nextch = fgetc(CFILE(p));
      if (nextch < 0)
	retval = QSEOF;
      else
	retval = QSINT(nextch);
      break;
    case QSIO_OUT:
    case QSIO_ERR:
    default:
      retval = QSERROR_INVALID;
      break;
    }
  return retval;
}

qsptr_t qsSTDIO_write_u8 (qsmem_t * mem, qsptr_t p, int octet)
{
  switch (p)
    {
    case QSIO_OUT:
    case QSIO_ERR:
      if (octet >= 0)
	fputc(octet, CFILE(p));
      break;
    case QSIO_IN:
    default:
      return QSERROR_INVALID;
      break;
    }
  return p;
}




/* Create atom object based on string representation of object. */
qsptr_t qsatom_parse_cstr (qsmem_t * mem, const char * repr, int reprmax)
{
  char * endptr = NULL;

  /* Cheating by using libc functions. */

  if (!repr) return QSNIL;
  if (!*repr) return QSNIL;
  if (reprmax == 0)
    {
      reprmax = strlen(repr);
    }

  /* try integer. */
  long ival = strtol(repr, &endptr, 10);
  if (endptr && (*endptr == 0))
    { /* all of it was integer. */
       if ((MIN_INT30 <= ival) && (ival <= MAX_INT30))
	{
	  /* fits in one-word. */
	  return QSINT(ival);
	}
      else
	{
	  /* create object to store as long. */
	  return qslong_make(mem, ival);
	}
    }
  /* try float */
  float fval = strtof(repr, &endptr);
  if ((errno == 0) && endptr && (! *endptr))
    {
      return QSFLOAT(fval);
    }
  /* try double */
  double dval = strtod(repr, &endptr);
  if ((errno == 0) && endptr && !*endptr)
    {
      return qsdouble_make(mem, dval);
    }
  /* try string */
  int slen = strlen(repr);
  if ((*repr == '"') && (repr[slen-1] == '"'))
    {
      /* string-like. */
      /* TODO: interpret escape sequences. */
      return qsstr_inject(mem, repr+1, slen-2);
    }
  /* try quoted */
  qsptr_t symname = QSNIL;
  /* try special */
  if (*repr == '#')
    {
      if ((0 == strcmp(repr+1, "t")) || (0 == strcmp(repr+1, "true")))
	{
	  return QSTRUE;
	}
      if ((0 == strcmp(repr+1, "f")) || (0 == strcmp(repr+1, "false")))
	{
	  return QSFALSE;
	}
      if (*(repr+1) == ':')
	{ /* keyword. */
	  symname = qsstr_inject(mem, repr, 0);
	}
    }
  /* TODO: fails if multiple quote-class characters in a row. */
  if (*repr == '\'')
    {
      symname = qsstr_inject(mem, repr+1, 0);
    }
  if (*repr == '`')
    {
      symname = qsstr_inject(mem, repr+1, 0);
    }
  if (*repr == ',')
    {
      if (repr[1] == '@')
	{
	  symname = qsstr_inject(mem, repr+2, 0);
	}
      else
	{
	  symname = qsstr_inject(mem, repr+1, 0);
	}
    }
  /* try symbol. */
  /* if (qssymbol_is_valid_name(repr)) */
  if (ISNIL(symname))
    {
      symname = qsstr_inject(mem, repr, 0);
    }

  if (!ISNIL(symname))
    {
      if (ISNIL(mem->symstore))
	{
	  mem->symstore = qssymstore_make(mem);
	}
      qsptr_t found = qssymstore_assoc(mem, mem->symstore, symname);
      if (qssymbol_p(mem, found))
	{
	  return found;
	}
      found = qssymbol_make(mem, symname);
      mem->symstore = qssymstore_intern(mem, mem->symstore, found);
	{
	  qsptr_t chk = qssymstore_assoc(mem, mem->symstore, symname);
	}
      return found;
    }

  /* give up */
  return QSBLACKHOLE;
}



int qsobj_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen)
{
  int n = 0;

  if (qspair_p(mem, p))
    {
      //n += qspair_crepr(mem, p, buf+n, buflen-n);
      n += qslist_crepr(mem, p, buf+n, buflen-n);
    }
  else if (qsvector_p(mem, p))
    {
      n += qsvector_crepr(mem, p, buf+n, buflen-n);
    }
  else if (qswidenum_p(mem, p))
    {
      n += qswidenum_crepr(mem, p, buf+n, buflen-n);
    }
  else if (qsbytevec_p(mem, p))
    {
      n += qsbytevec_crepr(mem, p, buf+n, buflen-n);
    }
  else if (qsimmlist_p(mem, p))
    {
      n += qslist_crepr(mem, p, buf+n, buflen-n);
    }
  else if (qssymbol_p(mem, p))
    {
      n += qssymbol_crepr(mem, p, buf+n, buflen-n);
    }
  else if (qsstr_p(mem, p))
    {
      n += qsstr_crepr(mem, p, buf+n, buflen-n);
    }
  else if (qstree_p(mem, p))
    {
      n += qstree_crepr(mem, p, buf+n, buflen-n);
    }
  else
    {
      n += snprintf(buf+n, buflen-n, "#<OBJ@%08xp>", COBJ26(p));
    }
  return n;
}

int qsunknown_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  n += snprintf(buf+n, buflen-n, "#<UNKNOWN:%08x>", p);
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
      //n += qsiter_crepr(mem, p, buf+n, buflen-n);
      n += qslist_crepr(mem, p, buf+n, buflen-n);
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
