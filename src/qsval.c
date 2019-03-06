#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "qsval.h"
#include "qsobj.h"


int qs_vsnprintf (char * buf, int buflen, char * fmt, va_list vp)
{
  int written = vsnprintf(buf, buflen, fmt, vp);
  if (written >= buflen)
    {
      /* not actually written; write shorter */
      written = vsnprintf(buf, written-1, fmt, vp);
    }
  return written;
}

int qs_snprintf (char * buf, int buflen, char * fmt, ...)
{
  va_list vp;
  int retval = 0;
  va_start(vp, fmt);
  retval = qs_vsnprintf(buf, buflen, fmt, vp);
  va_end(vp);
  return retval;
}


qsptr qsptr_make (qsmachine_t * mach, qsaddr mapped_addr)
{
  qsptr retval = QSOBJ(mapped_addr >> 4);
  return retval;
}


/* Directly encoded values. */

qsptr qsbool_make (qsmachine_t * mach, int val)
{
  return val ? QSTRUE : QSFALSE;
}

bool qsbool_p (const qsmachine_t * mach, qsptr p)
{
  return (p == QSTRUE) || (p == QSFALSE);
}

int qsbool_crepr (const qsmachine_t * mach, qsptr p, char *buf, int buflen)
{
  int n = 0;

  if (p == QSFALSE)
    {
      n += qs_snprintf(buf+n, buflen-n, "%s", "#f");
    }
  else if (p == QSTRUE)
    {
      n += qs_snprintf(buf+n, buflen-n, "%s", "#t");
    }
  return n;
}


qsptr qsfloat_make (qsmachine_t * mach, float val)
{
  return QSFLOAT(val);
}

bool qsfloat_p (const qsmachine_t * mach, qsptr p)
{
  return ISFLOAT31(p);
}

float qsfloat_get (const qsmachine_t * mach, qsptr p)
{
  float fval = CFLOAT31(p);
  return fval;
}

int qsfloat_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  float fval = CFLOAT31(p);

  n += qs_snprintf(buf+n, buflen-n, "%g", fval);
  return n;
}


qsptr qsint_make (qsmachine_t * mach, int32_t val)
{
  return QSINT(val);
}

bool qsint_p (const qsmachine_t * mach, qsptr p)
{
  return ISINT30(p);
}

int32_t qsint_get (const qsmachine_t * mach, qsptr p)
{
  int ival = CINT30(p);
  return ival;
}

int qsint_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  int ival = CINT30(p);

  n += qs_snprintf(buf+n, buflen-n, "%d", ival);
  return n;
}


qsptr qschar_make (qsmachine_t * mach, int val)
{
  return QSCHAR(val);
}

bool qschar_p (const qsmachine_t * mach, qsptr p)
{
  return ISCHAR24(p);
}

int qschar_get (const qsmachine_t * mach, qsptr p)
{
  int chval = CCHAR24(p);
  return chval;
}

int qschar_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  int chval = CCHAR24(p);
  const char * special = NULL;

  switch (chval)
    {
      case ' ': special = "space"; break;
      case '\0': special = "null"; break;
      case '\a': special = "alarm"; break;
      case '\b': special = "backspace"; break;
      case '\x27': special = "escape"; break;
      case '\xff': special = "delete"; break;
      case '\t': special = "tab"; break;
      case '\n': special = "newline"; break;
      case '\r': special = "return"; break;
      default: break;
    }
  if (special != NULL)
    {
      n += qs_snprintf(buf+n, buflen-n, "#\\%s", special);
    }
  else
    {
      n += qs_snprintf(buf+n, buflen-n, "#\\%c", chval);
    }
  return n;
}


qsptr qsconst_make (qsmachine_t * mach, int const_id)
{
  return QSCONST(const_id);
}

bool qsconst_p (const qsmachine_t * mach, qsptr p)
{
  return ISCONST20(p);
}

int qsconst_id (const qsmachine_t * mach, qsptr p)
{
  return CCONST20(p);
}

int qsconst_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  int constval = CCONST20(p);

  switch (p)
    {
    case QSNIL:
      n += qs_snprintf(buf+n, buflen-n, "%s", "()");
      break;
    case QSTRUE:
      n += qs_snprintf(buf+n, buflen-n, "%s", "#t");
      break;
    case QSFALSE:
      n += qs_snprintf(buf+n, buflen-n, "%s", "#f");
      break;
    case QSINF:
      n += qs_snprintf(buf+n, buflen-n, "%s", "inf");
      break;
    case QSNINF:
      n += qs_snprintf(buf+n, buflen-n, "%s", "-inf");
      break;
    case QSNAN:
      n += qs_snprintf(buf+n, buflen-n, "%s", "nan");
      break;
    case QSNNAN:
      n += qs_snprintf(buf+n, buflen-n, "%s", "-nan");
      break;
    default:
      n += qs_snprintf(buf+n, buflen-n, "#<const %d>", constval);
      break;
    }

  return n;
}


qsptr qsfd_make (qsmachine_t * mach, int val)
{
  /* TODO: check value too large. */
  return QSFD(val);
}

bool qsfd_p (const qsmachine_t * mach, qsptr p)
{
  return ISFD20(p);
}

int qsfd_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  int fdval = CFD20(p);

  n += qs_snprintf(buf+n, buflen-n, "#<fd %d>", fdval);

  return n;
}


qsptr qsprim_make (qsmachine_t * mach, qsword primid)
{
  return QSPRIM(primid);
}

bool qsprim_p (const qsmachine_t * mach, qsptr p)
{
  return ISPRIM20(p);
}

int qsprim_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  int primid = CPRIM20(p);

  n += qs_snprintf(buf+n, buflen-n, "#<prim %d>", primid);

  return n;
}


qsptr qserr_make (qsmachine_t * mach, qsword errid)
{
  return QSERR(errid);
}

qsword qserr_p (const qsmachine_t * mach, qsptr p)
{
  return ISERR20(p);
}

qsword qserr_id (const qsmachine_t * mach, qsptr p)
{
  return CERR20(p);
}

int qserr_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  qsword err_id = CERR20(p);

  n += qs_snprintf(buf+n, buflen-n, "#<err %d>", err_id);
  return n;
}


qsptr qsobj_make (qsmachine_t * mach, qsword obj_id)
{
  return QSOBJ(obj_id);
}

qsword qsobj_p (qsmachine_t * mach, qsptr p)
{
  return ISOBJ26(p);
}

qsword qsobj_id (qsmachine_t * mach, qsptr p)
{
  return COBJ26(p);
}

const qsobj_t * qsobj_const (const qsmachine_t * mach, qsptr p)
{
  qsword obj_id = COBJ26(p);
  qsaddr mapped_addr = obj_id << 4;
  const qsobj_t * obj = (const qsobj_t*)(qsstore_word_at_const(&(mach->S), mapped_addr));
  if (! obj) return NULL;
  if (! MGMT_IS_USED(obj->mgmt)) return NULL;
  return obj;
}

qsobj_t * qsobj (qsmachine_t * mach, qsptr p)
{
  if (qsobj_const(mach, p))
    {
      return (qsobj_t*)(qsstore_word_at(&(mach->S), COBJ26(p) << 4));
    }
  return NULL;

#if 0
  qsword obj_id = COBJ26(p);
  qsobj_t * retval = NULL;
  qsword addr = (obj_id << 4);
  retval = (qsobj_t*)(qsstore_word_at(&(mach->S), addr));
  return retval;
#endif //0
}


/* QsSym: symbols by id.  See qssymbol for storing name. */
qsptr qssym_make (qsmachine_t * mach, qsword sym_id)
{
  return QSSYM(sym_id);
}

bool qssym_p (const qsmachine_t * mach, qsptr p)
{
  return ISSYM26(p);
}

qsword qssym_id (const qsmachine_t * mach, qsptr p)
{
  return CSYM26(p);
}

int qssym_crepr (qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  return n;
}



/* Heaped object */

/* Heaped prototype: Triplet. */
const qstriplet_t * qstriplet_const (const qsmachine_t * mach, qsptr p)
{
  if (! ISOBJ26(p)) return NULL;
  const qsobj_t * obj = qsobj_const(mach, p);
  if (MGMT_IS_OCT(obj->mgmt)) return NULL;
  if (MGMT_GET_ALLOC(obj->mgmt) != 0) return NULL;
  return (const qstriplet_t*)obj;
}

qstriplet_t * qstriplet (qsmachine_t * mach, qsptr p)
{
  if (qstriplet_const(mach, p))
    {
      return (qstriplet_t*)(qsobj(mach, p));
    }
  return NULL;
}

qsptr qstriplet_make (qsmachine_t * mach, qsptr first, qsptr second, qsptr third)
{
  qsaddr mapped_addr = 0;
  qserr err = qsstore_alloc(&(mach->S), 0, &mapped_addr);
  if (err != QSERR_OK)
    return err;
  qsobj_t * obj = (qsobj_t*)(qsstore_word_at(&(mach->S), mapped_addr));
  qsobj_init(obj, 0, false);
  qstriplet_t * triplet = (qstriplet_t*)obj;
  triplet->first = first;
  triplet->second = second;
  triplet->third = third;
  qsptr retval = qsptr_make(mach, mapped_addr);
  return retval;
}


/* Heaped prototype: WideWord. */
const qswideword_t * qswideword_const (const qsmachine_t * mach, qsptr p)
{
  if (! ISOBJ26(p)) return NULL;
  const qsobj_t * obj = qsobj_const(mach, p);
  if (! MGMT_IS_OCT(obj->mgmt)) return NULL;
  if (MGMT_GET_ALLOC(obj->mgmt) != 0) return NULL;
  return (const qswideword_t*)obj;
}

qswideword_t * qswideword (qsmachine_t * mach, qsptr p)
{
  if (qswideword_const(mach, p))
    {
      return (qswideword_t*)(qsobj(mach, p));
    }
  return NULL;
}

qsptr qswideword_make (qsmachine_t * mach, qsptr subtype)
{
  qsaddr mapped_addr = 0;
  qserr err = qsstore_alloc(&(mach->S), 0, &mapped_addr);
  if (err != QSERR_OK)
    return err;
  qsobj_t * obj = (qsobj_t*)(qsstore_word_at(&(mach->S), mapped_addr));
  qsobj_init(obj, 0, true);
  qswideword_t * wideword = (qswideword_t*)obj;
  wideword->subtype = subtype;
  wideword->payload.l = 0;
  qsptr retval = qsptr_make(mach, mapped_addr);
  return retval;
}


/* Heaped prototype: Pointer Vector. */
const qspvec_t * qspvec_const (const qsmachine_t * mach, qsptr p)
{
  if (! ISOBJ26(p)) return NULL;
  const qsobj_t * obj = qsobj_const(mach, p);
  if (MGMT_IS_OCT(obj->mgmt)) return NULL;
  if (MGMT_GET_ALLOC(obj->mgmt) == 0) return NULL;
  return (const qspvec_t*)obj;
}

qspvec_t * qspvec (qsmachine_t * mach, qsptr p)
{
  if (qspvec_const(mach, p))
    {
      return (qspvec_t*)(qsobj(mach, p));
    }
  return NULL;
}

qsptr qspvec_make (qsmachine_t * mach, qsptr len, qsptr fillval)
{
  qsaddr mapped_addr = 0;
  qserr err = qsstore_alloc_nwords(&(mach->S), len, &mapped_addr);
  if (err != QSERR_OK)
    return err;
  qsobj_t * obj = (qsobj_t*)(qsstore_word_at(&(mach->S), mapped_addr));
  int a = MGMT_GET_ALLOC(obj->mgmt);
  qsobj_init(obj, a, false);
  qspvec_t * pvec = (qspvec_t*)obj;
  pvec->length = len;
  pvec->gcback = QSNIL;
  pvec->gciter = QSNIL;
  qsword i;
  for (i = 0; i < len; i++)
    {
      pvec->elt[i] = fillval;
    }
  qsptr retval = qsptr_make(mach, mapped_addr);
  return retval;
}


/* Heaped prototype: Octet Vector. */
const qsovec_t * qsovec_const (const qsmachine_t * mach, qsptr p)
{
  if (! ISOBJ26(p)) return NULL;
  const qsobj_t * obj = qsobj_const(mach, p);
  if (! MGMT_IS_OCT(obj->mgmt)) return NULL;
  if (MGMT_GET_ALLOC(obj->mgmt) == 0) return NULL;
  return (const qsovec_t*)obj;
}

qsovec_t * qsovec (qsmachine_t * mach, qsptr p)
{
  if (qsovec_const(mach, p))
    {
      return (qsovec_t*)(qsobj(mach, p));
    }
  return NULL;
}

qsptr qsovec_make (qsmachine_t * mach, qsptr len, qsbyte fillval)
{
  qsaddr mapped_addr = 0;
  qserr err = qsstore_alloc_nbytes(&(mach->S), len, &mapped_addr);
  if (err != QSERR_OK)
    return err;
  qsobj_t * obj = (qsobj_t*)(qsstore_word_at_const(&(mach->S), mapped_addr));
  int a = MGMT_GET_ALLOC(obj->mgmt);
  qsobj_init(obj, a, true);
  qsovec_t * ovec = (qsovec_t*)obj;
  ovec->length = len;
  ovec->refcount = 0;
  ovec->reflock = 0;
  qsptr retval = qsptr_make(mach, mapped_addr);
  return retval;
}


/* Heaped object: Pair
   * prototype = qstriplet
   * .first is nil
 */

const qstriplet_t * qspair_const (const qsmachine_t * mach, qsptr p)
{
  const qstriplet_t * triplet = qstriplet_const(mach, p);
  if (! triplet) return NULL;
  if (!ISNIL(triplet->first)) return NULL;
  return triplet;
}

qstriplet_t * qspair (qsmachine_t * mach, qsptr p)
{
  if (qspair_const(mach, p))
    return qstriplet(mach, p);
  return NULL;
}

qsptr qspair_make (qsmachine_t * mach, qsptr a, qsptr d)
{
  qsptr p = qstriplet_make(mach, QSNIL, a, d);
  return p;
}

bool qspair_p (const qsmachine_t * mach, qsptr p)
{
  return (qspair_const(mach, p) != NULL);
}

qsptr qspair_ref_head (const qsmachine_t * mach, qsptr p)
{
  const qstriplet_t * pair = qspair_const(mach, p);
  if (! pair) return QSERR_FAULT;
  return pair->second;
}

qsptr qspair_ref_tail (const qsmachine_t * mach, qsptr p)
{
  const qstriplet_t * pair = qspair_const(mach, p);
  if (! pair) return QSERR_FAULT;
  return pair->third;
}

qsptr qspair_setq_head (qsmachine_t * mach, qsptr p, qsptr a)
{
  qstriplet_t * pair = qspair(mach, p);
  if (! pair) return QSERR_FAULT;
  pair->second = a;
  return p;
}

qsptr qspair_setq_tail (qsmachine_t * mach, qsptr p, qsptr d)
{
  qstriplet_t * pair = qspair(mach, p);
  if (! pair) return QSERR_FAULT;
  pair->third = d;
  return p;
}

int qspair_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  const qstriplet_t * pair = NULL;
  qsptr curr = p;

  n += qs_snprintf(buf+n, buflen-n, "%s", "(");

  while (! ISNIL(curr))
    {
      pair = qspair_const(mach, curr);
      qsptr head = pair->second;
      qsptr tail = pair->third;

      n += qsptr_crepr(mach, head, buf+n, buflen-n);

      if (ISNIL(tail))
	{
	  /* end of list. */
	  curr = QSNIL;
	  break;
	}
      else if (qspair_p(mach, tail))
	{
	  /* proper list. */
	  n += qs_snprintf(buf+n, buflen-n, "%s", " ");
	  curr = tail;
	}
      else
	{
	  /* improper list. */
	  n += qs_snprintf(buf+n, buflen-n, "%s", " . ");
	  n += qsptr_crepr(mach, tail, buf+n, buflen-n);
	  curr = QSNIL;
	  break;
	}
    }

  n += qs_snprintf(buf+n, buflen-n, "%s", ")");
  return n;
}


/* Heaped object: Vector
   * prototype = qspvec
   * .length isa integer
 */

const qspvec_t * qsvector_const (const qsmachine_t * mach, qsptr p)
{
  const qspvec_t * pvec = qspvec_const(mach, p);
  if (! pvec) return NULL;
  if (! ISINT30(pvec->length)) return NULL;
  return pvec;
}

qspvec_t * qsvector (qsmachine_t * mach, qsptr p)
{
  if (qsvector_const(mach, p))
    return (qspvec_t*)(qstriplet(mach, p));
  return NULL;
}

qsptr qsvector_make (qsmachine_t * mach, qsword len, qsptr fill)
{
  qsptr p = qspvec_make(mach, QSINT(len), fill);
  return p;
}

bool qsvector_p (const qsmachine_t * mach, qsptr p)
{
  return (qspvec_const(mach, p) != NULL);
}

qsword qsvector_length (const qsmachine_t * mach, qsptr p)
{
  const qspvec_t * pvec = qspvec_const(mach, p);
  if (! pvec) return QSERR_FAULT;
  return CINT30(pvec->length);
}

qsptr qsvector_ref (const qsmachine_t * mach, qsptr p, qsword k)
{
  const qspvec_t * pvec = qspvec_const(mach, p);
  if (! pvec) return QSERR_FAULT;
  if ((k < 0) || (k >= qsvector_length(mach, p))) return QSERR_FAULT;
  return pvec->elt[k];
}

qsptr qsvector_setq (qsmachine_t * mach, qsptr p, qsword k, qsptr val)
{
  qspvec_t * pvec = qspvec(mach, p);
  if (! pvec) return QSERR_FAULT;
  if ((k < 0) || (k >= qsvector_length(mach, p))) return QSERR_FAULT;
  pvec->elt[k] = val;
  return p;
}

int qsvector_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  qsword i, m;

  n += qs_snprintf(buf+n, buflen-n, "%s", "#(");

  m = qsvector_length(mach, p);
  for (i = 0; i < m; i++)
    {
      qsptr x = qsvector_ref(mach, p, i);
      if (i > 0)
	n += qs_snprintf(buf+n, buflen-n, " ");
      n += qsptr_crepr(mach, x, buf+n, buflen-n);
    }

  n += qs_snprintf(buf+n, buflen-n, "%s", ")");

  return n;
}



/* C-Pointer: prototype 'wideword', 'subtype' == QSWIDE_CPTR. */

const qswideword_t * qscptr_const (const qsmachine_t * mach, qsptr p)
{
  const qswideword_t * cptr = qswideword_const(mach, p);
  if (! cptr) return NULL;
  if (cptr->subtype != QSWIDE_CPTR) return NULL;
  return cptr;
}

qswideword_t * qscptr (qsmachine_t * mach, qsptr p)
{
  if (qscptr_const(mach, p))
    {
      return (qswideword_t*)(qscptr_const(mach, p));
    }
  return NULL;
}

qsptr qscptr_make (qsmachine_t * mach, void * val)
{
  qsptr p = qswideword_make(mach, QSWIDE_CPTR);
  qswideword_t * cptr = qswideword(mach, p);
  cptr->payload.ptr = val;
  return p;
}

bool qscptr_p (const qsmachine_t * mach, qsptr p)
{
  return (qscptr_const(mach, p) != NULL);
}

void * qscptr_get (const qsmachine_t * mach, qsptr p)
{
  const qswideword_t * cptr = qscptr_const(mach, p);
  if (! cptr) return NULL;
  return cptr->payload.ptr;
}

int qscptr_fetch (const qsmachine_t * mach, qsptr p, void ** out)
{
  const qswideword_t * cptr = qscptr_const(mach, p);
  if (! cptr) return 0;
  if (out) *out = cptr->payload.ptr;
  return 1;
}

int qscptr_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  const qswideword_t * cptr = qscptr_const(mach, p);
  if (! cptr)
    {
      n += qs_snprintf(buf+n, buflen-n, "#<(void*)%08x>", 0);
      return n;
    }

  n += qs_snprintf(buf+n, buflen-n, "#<(void*)%08x>", cptr->payload.ptr);

  return n;
}



/* Long (64b): prototype 'wideword', 'subtype' == QSNUM_LONG. */

const qswideword_t * qslong_const (const qsmachine_t * mach, qsptr p)
{
  const qswideword_t * l = qswideword_const(mach, p);
  if (! l) return NULL;
  if (l->subtype != QSNUM_LONG) return NULL;
  return l;
}

qswideword_t * qslong (qsmachine_t * mach, qsptr p)
{
  if (qslong_const(mach, p))
    {
      return (qswideword_t*)(qswideword(mach, p));
    }
  return NULL;
}

qsptr qslong_make (qsmachine_t * mach, int64_t val)
{
  qsptr p = qswideword_make(mach, QSNUM_LONG);
  if (ISOBJ26(p))
    {
      qswideword_t * l = qswideword(mach, p);
      if (l)
	l->payload.l = val;
    }
  return p;
}

bool qslong_p (const qsmachine_t * mach, qsptr p)
{
  return (qslong_const(mach, p) != NULL);
}

int64_t qslong_get (const qsmachine_t * mach, qsptr p)
{
  const qswideword_t * l = qswideword_const(mach, p);
  if (! l) return 0;
  return l->payload.l;
}

int qslong_fetch (const qsmachine_t * mach, qsptr p, int64_t * out)
{
  const qswideword_t * l = qswideword_const(mach, p);
  if (! l) return 0;
  if (out) *out = l->payload.l;
  return 1;
}

int qslong_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  const qswideword_t * l = qswideword_const(mach, p);
  if (! l)
    {
      n += qs_snprintf(buf+n, buflen-n, "%d", 0);
      return n;
    }
  n += qs_snprintf(buf+n, buflen-n, "%ld", l->payload.l);
  return n;
}



/* Double-Precision Float (64b): prototype 'wideword', 'subtype' == QSNUM_DOUBLE. */
const qswideword_t * qsdouble_const (const qsmachine_t * mach, qsptr p)
{
  const qswideword_t * d = qswideword_const(mach, p);
  if (! d) return NULL;
  if (d->subtype != QSNUM_DOUBLE) return NULL;
  return d;
}

qswideword_t * qsdouble (qsmachine_t * mach, qsptr p)
{
  if (qsdouble_const(mach, p))
    {
      return (qswideword_t*)(qswideword(mach, p));
    }
  return NULL;
}

qsptr qsdouble_make (qsmachine_t * mach, double val)
{
  qsptr p = qswideword_make(mach, QSNUM_DOUBLE);
  if (ISOBJ26(p))
    {
      qswideword_t * d = qswideword(mach, p);
      d->payload.d = val;
    }
  return p;
}

bool qsdouble_p (const qsmachine_t * mach, qsptr p)
{
  if (qsdouble_const(mach, p) != NULL);
}

double qsdouble_get (const qsmachine_t * mach, qsptr p)
{
  const qswideword_t * d = qswideword_const(mach, p);
  if (! d) return 0;
  return d->payload.d;
}

int qsdouble_fetch (const qsmachine_t * mach, qsptr p, double * out)
{
  const qswideword_t * d = qswideword_const(mach, p);
  if (! d) return 0;
  if (out) *out = d->payload.d;
  return 1;
}

int qsdouble_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  const qswideword_t * d = qswideword_const(mach, p);
  if (! d)
    {
      n += qs_snprintf(buf+n, buflen-n, "%g", 0);
      return n;
    }
  n += qs_snprintf(buf+n, buflen-n, "%lg", d->payload.d);
  return n;
}


/* QsSymbol: symbol name.  See qssym for comparing symbols by object id. */
/* Heaped object: Symbol
 */
qsptr qssymbol_make (qsmachine_t * mach, qsptr name)
{
}

bool qssymbol_p (const qsmachine_t * mach, qsptr p)
{
}

int qssymbol_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  return n;
}



/* Heaped object: String
   * prototype = ovec
   * .score = 8
   * .length isa integer
 */

const qsovec_t * qsstring_const (const qsmachine_t * mach, qsptr p)
{
  const qsovec_t * s = qsovec_const(mach, p);
  if (! s) return NULL;
  if (! ISINT30(s->length)) return NULL;
  if (qsobj_get_score((qsobj_t*)s) != 8) return NULL;
  return s;
}

qsovec_t * qsstring (qsmachine_t * mach, qsptr p)
{
  if (qsstring_const(mach, p))
    {
      return (qsovec_t*)(qsovec(mach, p));
    }
  return NULL;
}

qsptr qsstring_make (qsmachine_t * mach, qsword len, int fill)
{
  qsptr p = qsovec_make(mach, QSINT(len), fill);
  qsobj_t * obj = qsobj(mach, p);
  qsobj_set_score(obj, 8);
  return p;
}

bool qsstring_p (const qsmachine_t * mach, qsptr p)
{
  return (qsstring_const(mach, p) != NULL);
}

qsword qsstring_length (const qsmachine_t * mach, qsptr p)
{
  const qsovec_t * s = qsovec_const(mach, p);
  if (! s) return 0;
  return CINT30(s->length);
}

int qsstring_ref (const qsmachine_t * mach, qsptr p, qsword k)
{
  const qsovec_t * s = qsovec_const(mach, p);
  if (! s) return 0;
  /* TODO: multi-byte character */
  return s->elt[k];
}

qsptr qsstring_setq (qsmachine_t * mach, qsptr p, qsword k, int ch)
{
  qsovec_t * s = qsovec(mach, p);
  if (! s) return QSERR_FAULT;
  /* TODO: multi-byte character. */

  s->elt[k] = ch;

  return p;
}

int qsstring_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  const qsovec_t * s = qsovec_const(mach, p);
  if (! s)
    {
      n += qs_snprintf(buf+n, buflen-n, "\"\"");
      return n;
    }
  qsword m = qsstring_length(mach, p);
  n += qs_snprintf(buf+n, buflen-n-m, "\"%s\"", s->elt);
  return n;
}


/* Heaped object: Bytevector
   * prototype = ovec
   * .score = 0
   * .length isa integer
 */
const qsovec_t * qsbytevec_const (const qsmachine_t * mach, qsptr p)
{
  const qsovec_t * ovec = qsovec_const(mach, p);
  if (! ovec) return NULL;
  if (! ISINT30(ovec->length)) return NULL;
  return (const qsovec_t*)ovec;
}

qsovec_t * qsbytevec (qsmachine_t * mach, qsptr p)
{
  if (qsbytevec_const(mach, p))
    {
      return (qsovec_t*)(qsovec(mach, p));
    }
  return NULL;
}

qsptr qsbytevec_make (qsmachine_t * mach, qsword len, qsbyte fill)
{
  qsptr p = qsovec_make(mach, QSINT(len), fill);
  qsobj_t * obj = qsobj(mach, p);
  qsobj_set_score(obj, 0);
  return p;
}

bool qsbytevec_p (const qsmachine_t * mach, qsptr p)
{
  return (qsbytevec_const(mach, p) != NULL);
}

qsword qsbytevec_length (const qsmachine_t * mach, qsptr p)
{
  const qsovec_t * ovec = qsovec_const(mach, p);
  if (! ovec) return 0;
  return CINT30(ovec->length);
}

qsbyte qsbytevec_ref (const qsmachine_t * mach, qsptr p, qsword k)
{
  const qsovec_t * ovec = qsovec_const(mach, p);
  if (! ovec) return 0;
  return ovec->elt[k];
}

qsptr qsbytevec_setq (qsmachine_t * mach, qsptr p, qsword k, qsbyte val)
{
  qsovec_t * ovec = qsovec(mach, p);
  if (! ovec) return QSERR_FAULT;
  ovec->elt[k] = val;
  return p;
}

int qsbytevec_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  qsword i, m;
  const qsovec_t * ovec = qsovec_const(mach, p);
  if (! ovec)
    {
      n += qs_snprintf(buf+n, buflen-n, "%s", "#u8()");
      return n;
    }
  n += qs_snprintf(buf+n, buflen-n, "%s", "#u8(");
  m = qsbytevec_length(mach, p);
  for (i = 0; i < m; i++)
    {
      if (i > 0)
	n += qs_snprintf(buf+n, buflen-n, " ");
      n += qs_snprintf(buf+n, buflen-n, "%d", qsbytevec_ref(mach, p, i));
    }
  n += qs_snprintf(buf+n, buflen-n, "%s", ")");

  return n;
}


qsptr qsenv_make (qsmachine_t * mach, qsptr next_env)
{
}

qsptr qsenv_insert (qsmachine_t * mach, qsptr variable, qsptr binding)
{
}

int qsenv_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  return n;
}


qsptr qskont_make (qsmachine_t * mach, qsptr variant, qsptr c, qsptr e, qsptr k, qsptr extra)
{
}

bool qskont_p (const qsmachine_t * mach, qsptr p)
{
}

int qskont_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  return n;
}




int qsptr_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;

  if (ISFLOAT31(p))
    {
      n += qsfloat_crepr(mach, p, buf+n, buflen-n);
    }
  else if (ISINT30(p))
    {
      n += qsint_crepr(mach, p, buf+n, buflen-n);
    }
  else if (ISCHAR24(p))
    {
      n += qschar_crepr(mach, p, buf+n, buflen-n);
    }
  else if (ISCONST20(p))
    {
      n += qsconst_crepr(mach, p, buf+n, buflen-n);
    }
  else if (ISOBJ26(p))
    {
    }
  else if (ISSYM26(p))
    {
    }
  else
    {
      n += qs_snprintf(buf+n, buflen-n, "#<ptr %08x>", p);
    }

  return n;
}

