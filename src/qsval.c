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
  const char * symbolic = "";

  switch (err_id)
    {
    case QSERR_OK:
      symbolic = " OK";
      break;
    case QSERR_NOMEM:
      symbolic = " NOMEM";
      break;
    case QSERR_UNBOUND:
      symbolic = " UNBOUND";
      break;
    case QSERR_FAULT:
      symbolic = " FAULT";
      break;
    default:
      break;
    }

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

qscmp_t qssym_cmp (qsmachine_t * mach, qsptr x, qsptr y)
{
  return (x == y) ? QSCMP_EQ : QSCMP_NE;
}


/* Heaped object */

/* Heaped prototype: Triplet. */
const qstriplet_t * qstriplet_const (const qsmachine_t * mach, qsptr p)
{
  if (! ISOBJ26(p)) return NULL;
  const qsobj_t * obj = qsobj_const(mach, p);
  if (! obj) return NULL;
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

qscmp_t qstriplet_cmp (const qsmachine_t * mach, qsptr x, qsptr y)
{
  const qstriplet_t * tx = qstriplet_const(mach, x);
  const qstriplet_t * ty = qstriplet_const(mach, y);
  if (!tx || !ty) return QSCMP_NE;
  if (x == y) return QSCMP_EQ;
  if (tx->first != ty->first) return QSCMP_NE;
  if (tx->second != ty->second) return QSCMP_NE;
  if (tx->third != ty->third) return QSCMP_NE;
  return QSCMP_EQ;
}


/* Heaped prototype: WideWord. */
const qswideword_t * qswideword_const (const qsmachine_t * mach, qsptr p)
{
  if (! ISOBJ26(p)) return NULL;
  const qsobj_t * obj = qsobj_const(mach, p);
  if (! obj) return NULL;
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

qscmp_t qswideword_cmp (const qsmachine_t * mach, qsptr x, qsptr y)
{
  const qswideword_t * wx = qswideword_const(mach, x);
  const qswideword_t * wy = qswideword_const(mach, y);
  if (!wx || !wy) return QSCMP_NE;
  if (x == y) return QSCMP_EQ;
  if (wx->subtype != wy->subtype) return QSCMP_NE;
  /* widest member possible that can be compared. */
  if (wx->payload.l != wy->payload.l) return QSCMP_NE;
  return QSCMP_EQ;
}


/* Heaped prototype: Pointer Vector. */
const qspvec_t * qspvec_const (const qsmachine_t * mach, qsptr p)
{
  if (! ISOBJ26(p)) return NULL;
  const qsobj_t * obj = qsobj_const(mach, p);
  if (! obj) return NULL;
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

qsptr qspvec_make (qsmachine_t * mach, qsptr payload_words, qsptr fillval)
{
  qsaddr mapped_addr = 0;
  qserr err = qsstore_alloc_nwords(&(mach->S), payload_words, &mapped_addr);
  if (err != QSERR_OK)
    return err;
  qsobj_t * obj = (qsobj_t*)(qsstore_word_at(&(mach->S), mapped_addr));
  int a = MGMT_GET_ALLOC(obj->mgmt);
  qsobj_init(obj, a, false);
  qspvec_t * pvec = (qspvec_t*)obj;
  pvec->length = QSNIL;
  pvec->gcback = QSNIL;
  pvec->gciter = QSNIL;
  qsword i;
  for (i = 0; i < payload_words; i++)
    {
      pvec->elt[i] = fillval;
    }
  qsptr retval = qsptr_make(mach, mapped_addr);
  return retval;
}

qscmp_t qspvec_cmp (const qsmachine_t * mach, qsptr x, qsptr y)
{
  const qspvec_t * vx = qspvec_const(mach, x);
  const qspvec_t * vy = qspvec_const(mach, y);
  if (!vx || !vy) return QSCMP_NE;
  if (x == y) return QSCMP_EQ;
  if (vx->length != vy->length) return QSCMP_NE;
  qsword i;
  qsword m = (1 << qsobj_get_allocscale((qsobj_t*)vx)) * sizeof(qsobj_t)/sizeof(qsptr) - sizeof(qsobj_t);
  for (i = 0; i < m; i++)
    if (vx->elt[i] != vy->elt[i]) return QSCMP_NE;
  return QSCMP_EQ;
}


/* Heaped prototype: Octet Vector. */
const qsovec_t * qsovec_const (const qsmachine_t * mach, qsptr p)
{
  if (! ISOBJ26(p)) return NULL;
  const qsobj_t * obj = qsobj_const(mach, p);
  if (! obj) return NULL;
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

qscmp_t qsovec_cmp (const qsmachine_t * mach, qsptr x, qsptr y)
{
  const qsovec_t * ox = qsovec_const(mach, x);
  const qsovec_t * oy = qsovec_const(mach, y);
  if (!ox || !oy) return QSCMP_NE;
  if (x == y) return QSCMP_EQ;
  if (ox->length != oy->length) return QSCMP_NE;
  qsword i;
  qsword m = (1 << qsobj_get_allocscale((qsobj_t*)ox)) * sizeof(qsobj_t) - sizeof(qsobj_t);
  for (i = 0; i < m; i++)
    if (ox->elt[i] != oy->elt[i]) return QSCMP_NE;
  return QSCMP_EQ;
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

qsptr qspair_iter (const qsmachine_t * mach, qsptr p)
{
  if (! qspair_const(mach, p)) return QSERR_FAULT;
  qsaddr memaddr = COBJ26(p) << 4;
  qsptr retval = qsiter_make(mach, memaddr);
  return retval;
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
    return (qspvec_t*)(qspvec(mach, p));
  return NULL;
}

qsptr qsvector_make (qsmachine_t * mach, qsword len, qsptr fill)
{
  qsptr p = qspvec_make(mach, len, fill);
  qspvec_t * pvec = qspvec(mach, p);
  pvec->length = QSINT(len);
  return p;
}

bool qsvector_p (const qsmachine_t * mach, qsptr p)
{
  return (qsvector_const(mach, p) != NULL);
}

qsword qsvector_length (const qsmachine_t * mach, qsptr p)
{
  const qspvec_t * pvec = qsvector_const(mach, p);
  if (! pvec) return 0;
  return CINT30(pvec->length);
}

qsptr qsvector_ref (const qsmachine_t * mach, qsptr p, qsword k)
{
  const qspvec_t * vec = qsvector_const(mach, p);
  if (! vec) return QSERR_FAULT;
  if ((k < 0) || (k >= qsvector_length(mach, p))) return QSERR_FAULT;
  return vec->elt[k];
}

qsptr qsvector_setq (qsmachine_t * mach, qsptr p, qsword k, qsptr val)
{
  qspvec_t * pvec = qsvector(mach, p);
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


/* Heaped object: Array
   * prototype = qspvec
   * .length is nil
 */

const qspvec_t * qsarray_const (const qsmachine_t * mach, qsptr p)
{
  const qspvec_t * pvec = qspvec_const(mach, p);
  if (! pvec) return NULL;
  if (! ISNIL(pvec->length)) return NULL;
  return pvec;
}

qspvec_t * qsarray (qsmachine_t * mach, qsptr p)
{
  if (qsarray_const(mach, p))
    return (qspvec_t*)(qspvec(mach, p));
  return NULL;
}

qsptr qsarray_make (qsmachine_t * mach, qsword len)
{
  qsptr p = qspvec_make(mach, len, QSNIL);
  qspvec_t * pvec = qspvec(mach, p);
  pvec->length = QSNIL;
  return p;
}

qsptr qsarray_vinject (qsmachine_t * mach, va_list vp)
{
  qsword nptrs = 0;
  va_list vp0;
  qsptr elt;

  /* bookmark. */
  va_copy(vp0, vp);

  /* count arguments. */
  int depth = 1;
  while (depth > 0)
    {
      elt = va_arg(vp0, qsptr);
      if (elt == QSBOL)
	depth++;
      else if (elt == QSEOL)
	depth--;
      nptrs++;
    }
  va_end(vp0);

  /* prepare memory. */
  qsptr retval = qsarray_make(mach, nptrs);
  if (! ISOBJ26(retval))
    return QSERR_FAULT;

  /* populate memory. */
  qsword i;
  for (i = 0; i < nptrs; i++)
    {
      elt = va_arg(vp, qsptr);
      qsarray_setq(mach, retval, i, elt);
    }
  qsarray_setq(mach, retval, i, QSEOL);

  return retval;
}

qsptr qsarray_inject (qsmachine_t * mach, ...)
{
  va_list vp;
  qsptr retval = QSNIL;

  va_start(vp, mach);
  retval = qsarray_vinject(mach, vp);
  va_end(vp);
  return retval;
}

bool qsarray_p (const qsmachine_t * mach, qsptr p)
{
  return (qsarray_const(mach, p) != NULL);
}

qsword qsarray_length (const qsmachine_t * mach, qsptr p)
{
  const qspvec_t * pvec = qsarray_const(mach, p);
  if (! pvec) return 0;
  const int words_per_boundary = sizeof(qsobj_t) / sizeof(qsptr);
  qsaddr allocsize = (1 << qsobj_get_allocscale((const qsobj_t*)pvec));
  qsword retval = allocsize * words_per_boundary;
  return retval;
}

qsptr qsarray_ref (const qsmachine_t * mach, qsptr p, qsword k)
{
  const qspvec_t * vec = qsarray_const(mach, p);
  if (! vec) return QSERR_FAULT;
  if ((k < 0) || (k >= qsarray_length(mach, p))) return QSERR_FAULT;
  return vec->elt[k];
}

qsptr qsarray_setq (qsmachine_t * mach, qsptr p, qsword k, qsptr val)
{
  qspvec_t * pvec = qsarray(mach, p);
  if (! pvec) return QSERR_FAULT;
  if ((k < 0) || (k >= qsarray_length(mach, p))) return QSERR_FAULT;
  pvec->elt[k] = val;
  return p;
}

qsptr qsarray_iter (const qsmachine_t * mach, qsptr p)
{
  const qspvec_t * pvec = qsarray_const(mach, p);
  if (! pvec) return QSERR_FAULT;
  qsaddr memaddr = (COBJ26(p) + 1) << 4;  /* next boundary. */
  return qsiter_make(mach, memaddr);
}

int qsarray_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  qsword i, m;

  n += qs_snprintf(buf+n, buflen-n, "%s", "#(");

  m = qsarray_length(mach, p);
  for (i = 0; i < m; i++)
    {
      qsptr x = qsarray_ref(mach, p, i);
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
   * prototype = ovec
   * .length isa char24
 */
const qsovec_t * qssymbol_const (const qsmachine_t * mach, qsptr p)
{
  const qsovec_t * y = qsovec_const(mach, p);
  if (! y) return NULL;
  if (! ISCHAR24(y->length)) return NULL;
  return y;
}

qsovec_t * qssymbol (qsmachine_t * mach, qsptr p)
{
  if (qssymbol_const(mach, p))
    {
      return (qsovec_t*)(qsovec(mach, p));
    }
  return NULL;
}

qsptr _qssymbol_make (qsmachine_t * mach, qsword namelen, qsovec_t ** out_sym)
{
  qsptr p = qsovec_make(mach, namelen+1, 0);
  qsovec_t * y = qsovec(mach, p);
  if (!y) return QSERR_FAULT;
  if (out_sym) *out_sym = y;
  return p;
}

qsptr qssymbol_bless (qsmachine_t * mach, qsptr s)
{
  if (! qsutf8_p(mach, s))
    return QSERR_FAULT;
  qsword slen = qsutf8_length(mach, s);

  qsovec_t * y = NULL;
  qsptr p = _qssymbol_make(mach, slen, &y);
  if (! ISOBJ26(p)) return QSERR_FAULT;

  y->length = QSCHAR(slen);
  qsword i;
  for (i = 0; i < slen; i++)
    {
      /* TODO convert multi-byte sequence. */
      y->elt[i] = qsutf8_ref(mach, s, i);
    }
  y->elt[i] = 0;

  /* intern symbol. */
  qssymstore_insert(mach, mach->Y, p);

  return p;
}

qsptr qssymbol_intern_c (qsmachine_t * mach, const char * cstr)
{
  /* 1. find already interned symbol. */
  qsptr extant = qssymstore_find_c(mach, cstr);
  if (ISOBJ26(extant))
    return extant;

  /* 2. fallback to constructing symbol object. */
  qsovec_t * y = NULL;
  qsword slen = strlen(cstr);
  qsptr p = _qssymbol_make(mach, slen, &y);
  if (! ISOBJ26(p)) return QSERR_NOMEM;
  y->length = QSCHAR(slen);
  qsword i;
  strcpy(y->elt, cstr);

  /* 3. then interning symbol object. */
  qssymstore_insert(mach, mach->Y, p);

  return p;
}

bool qssymbol_p (const qsmachine_t * mach, qsptr p)
{
  return (qssymbol_const(mach, p) != NULL);
}

qsptr qssymbol_sym (const qsmachine_t * mach, qsptr p)
{
  if (! qssymbol_const(mach, p)) return QSNIL;
  return QSSYM(COBJ26(p));
}

qsword qssymbol_length (const qsmachine_t * mach, qsptr p)
{
  const qsovec_t * y = qssymbol_const(mach, p);
  if (!y) return 0;
  qsword retval = CCHAR24(y->length);
  return retval;
}

int qssymbol_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  const qsovec_t * y = qsovec_const(mach, p);
  if (! y) return n;
  int m = CCHAR24(y->length);
  n += qs_snprintf(buf+n, buflen-n-m, "%s", y->elt);
  return n;
}

int qssymbol_strcmp (const qsmachine_t * mach, qsptr x, const char * s)
{
  const qsovec_t * y = qssymbol_const(mach, x);
  if (! y) return 0;
  int m = qssymbol_length(mach, x);
  /* Use C string comparison, up to the length of the symbol content.
     If they don't match, strcmp() result stands.
     If mostly match, but symbol is too long, strcmp() returns Greater Than.
     If mostly match, but symbol is too short, should return Less Than.
     */
  int res = strncmp(y->elt, s, m);
  if (res != 0) return res;  /* unequal, shortcut return. */
  if (s[m] != 0) return -1;  /* matching prefix, but s is longer. */
  /* matching prefix but s is shorter covered in strncmp(). */
  return 0; /* equal */
}

qscmp_t qssymbol_cmp (const qsmachine_t * mach, qsptr x, qsptr y)
{
  const qsovec_t * y0 = qssymbol_const(mach, x);
  const qsovec_t * y1 = qssymbol_const(mach, y);
  if (!y0) return QSCMP_NE;
  if (!y1) return QSCMP_NE;
  if (y0->length != y1->length) return QSCMP_NE;
  qsword m = CCHAR24(y0->length);
  if (0 != strncmp(y0->elt, y1->elt, m))
    return QSCMP_NE;
  return QSCMP_EQ;
}



/* Heaped object: UTF-8 String
   * prototype = ovec
   * .score = 8
   * .length isa integer
 */

const qsovec_t * qsutf8_const (const qsmachine_t * mach, qsptr p)
{
  const qsovec_t * s = qsovec_const(mach, p);
  if (! s) return NULL;
  if (! ISINT30(s->length)) return NULL;
  if (qsobj_get_score((qsobj_t*)s) != 8) return NULL;
  return s;
}

qsovec_t * qsutf8 (qsmachine_t * mach, qsptr p)
{
  if (qsutf8_const(mach, p))
    {
      return (qsovec_t*)(qsovec(mach, p));
    }
  return NULL;
}

qsptr qsutf8_make (qsmachine_t * mach, qsword len, int fill)
{
  /* Allocate one more byte for terminating guard byte '\0'. */
  qsptr p = qsovec_make(mach, len+1, fill);
  qsobj_t * obj = qsobj(mach, p);
  qsobj_set_score(obj, 8);
  qsovec_t * ovec = (qsovec_t*)obj;
  /* Valid bytes discount the terminating '\0'. */
  ovec->length = QSINT(len);
  return p;
}

qsptr qsutf8_inject_charp (qsmachine_t * mach, const char * cstr)
{
  size_t slen = strlen(cstr);
  qsptr retval = qsutf8_make(mach, slen, 0);
  if (ISOBJ26(retval))
    {
      qsovec_t * st = qsutf8(mach, retval);
      strncpy(st->elt, cstr, slen);
    }
  return retval;
}

qsptr qsutf8_inject_bytes (qsmachine_t * mach, uint8_t * buf, qsword buflen)
{
  qsptr retval = qsutf8_make(mach, buflen, 0);
  if (ISOBJ26(retval))
    {
      qsovec_t * st = qsutf8(mach, retval);
      memcpy(st->elt, buf, buflen);
    }
  return retval;
}

bool qsutf8_p (const qsmachine_t * mach, qsptr p)
{
  return (qsutf8_const(mach, p) != NULL);
}

qsword qsutf8_length (const qsmachine_t * mach, qsptr p)
{
  const qsovec_t * s = qsovec_const(mach, p);
  if (! s) return 0;
  return CINT30(s->length);
}

int qsutf8_ref (const qsmachine_t * mach, qsptr p, qsword k)
{
  const qsovec_t * s = qsovec_const(mach, p);
  if (! s) return 0;
  /* TODO: multi-byte character */
  return s->elt[k];
}

qsptr qsutf8_setq (qsmachine_t * mach, qsptr p, qsword k, int ch)
{
  qsovec_t * s = qsovec(mach, p);
  if (! s) return QSERR_FAULT;
  /* TODO: multi-byte character. */

  if ((k < 0) || (k >= qsutf8_length(mach, p))) return QSERR_FAULT;
  s->elt[k] = ch;
  return p;
}

int qsutf8_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  const qsovec_t * s = qsovec_const(mach, p);
  if (! s)
    {
      n += qs_snprintf(buf+n, buflen-n, "\"\"");
      return n;
    }
  qsword m = qsutf8_length(mach, p);
  n += qs_snprintf(buf+n, buflen-n-m, "\"%s\"", s->elt);
  return n;
}

qscmp_t qsutf8_cmp (const qsmachine_t * mach, qsptr x, qsptr y)
{
  /* TODO: UTF-8 string-equivalents handling (e.g. overlong encoding). */
  bool bx = qsutf8_p(mach, x);
  bool by = qsutf8_p(mach, y);
  if (!bx && !by) return QSCMP_NE;
  if (!by) return QSCMP_GT;
  if (!bx) return QSCMP_LT;

  const qsovec_t * sx = qsutf8_const(mach, x);
  const qsovec_t * sy = qsutf8_const(mach, y);
  int res = strcmp(sx->elt, sy->elt);
  if (res < 0) return QSCMP_LT;
  if (res > 0) return QSCMP_GT;
  if (res == 0) return QSCMP_EQ;
  return QSCMP_NE;
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


/* Make iterator from memory address, no checks.
   For constructing from object, see __iter() for the associated object.
 */
qsptr qsiter_make (const qsmachine_t * mach, qsaddr addr)
{
  qsptr retval = QSITER(addr >> 2);
  return retval;
}

bool qsiter_p (const qsmachine_t * mach, qsptr p)
{
  return ISITER28(p);
}

qsaddr _qsiter_memaddr (const qsmachine_t * mach, qsptr p)
{
  qsaddr memaddr = CITER28(p) << 2;
  return memaddr;
}

bool _qsiter_on_pair (const qsmachine_t * mach, qsptr p, qsptr * out_pair)
{
  bool retval = false;
  qsaddr memaddr = _qsiter_memaddr(mach, p);
  if ((memaddr & 0xf) != 0) return false; /* not aligned, cannot be object. */
  qsptr maybe_pair = QSOBJ( memaddr >> 4 );
  if (qspair_const(mach, maybe_pair))
    {
      if (out_pair) *out_pair = maybe_pair;
      retval = true;
    }
  return retval;
}

qsptr _qsiter_word (const qsmachine_t * mach, qsptr p)
{
  qsaddr memaddr = _qsiter_memaddr(mach, p);
  qsptr retval = qsstore_get_word(&(mach->S), memaddr);
  return retval;
}

/* returns contents of list head (car). */
qsptr qsiter_head (const qsmachine_t * mach, qsptr p)
{
  qsptr retval;
  qsptr pair = QSNIL;

  if (! qsiter_p(mach, p)) return QSERR_FAULT;
  if (_qsiter_on_pair(mach, p, &pair))
    {
      retval = qspair_ref_head(mach, pair);
    }
  else
    {
      retval = _qsiter_word(mach, p);
      if (retval == QSBOL)
	{
	  /* nested list, form iterator into next word. */
	  retval = QSITER( (CITER28(p) + 1) );
	}
    }
  return retval;
}

/* returns next iter in the list (cdr). */
qsptr qsiter_tail (const qsmachine_t * mach, qsptr p)
{
  qsptr retval;
  qsptr pair = QSNIL;
  int depth = 0;

  if (! qsiter_p(mach, p)) return QSERR_FAULT;
  if (_qsiter_on_pair(mach, p, &pair))
    {
      qsptr next = qspair_ref_tail(mach, pair);
      if (ISOBJ26(next))
	retval = QSITER( COBJ26(next) << 2 );
      else if (ISNIL(next))
	retval = next;
      else
	retval = QSERR_FAULT;
    }
  else
    {
      qsword skip = 1;
      qsptr head = _qsiter_word(mach, p);
      if (head == QSBOL)
	{
	  /* skip to matching QSEOL */
	  depth = 1;
	  while (depth > 0)
	    {
	      qsptr elt = _qsiter_word(mach, p + (skip << SHIFT_TAG28));
	      if (elt == QSBOL)
		{
		  depth++;
		}
	      else if (elt == QSEOL)
		{
		  depth--;
		}
	      skip++;
	    }
	}
      qsptr next = QSITER(CITER28(p) + skip);
      /* peek for end of iteration. */
      qsptr peek = _qsiter_word(mach, next);
      if ((CITER28(next) & 0x3) == 0)
	{
	  /* aligned (4th word); is-sync => start of new object, ends list. */
	  if (ISSYNC29(peek))
	    return QSNIL;
	}
      if (peek == QSEOL)
	{
	  /* end of list. */
	  return QSNIL;
	}
      retval = next;
    }

  return retval;
}

int qsiter_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  return 0;
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
  else if (ISITER28(p))
    {
      n += qsiter_crepr(mach, p, buf+n, buflen-n);
    }
  else if (ISCHAR24(p))
    {
      n += qschar_crepr(mach, p, buf+n, buflen-n);
    }
  else if (ISOBJ26(p))
    {
      if (qspair_p(mach, p))
	{
	  n += qspair_crepr(mach, p, buf+n, buflen-n);
	}
      else if (qsvector_p(mach, p))
	{
	  n += qsvector_crepr(mach, p, buf+n, buflen-n);
	}
      else if (qsbytevec_p(mach, p))
	{
	  n += qsbytevec_crepr(mach, p, buf+n, buflen-n);
	}
      else if (qslong_p(mach, p))
	{
	  n += qslong_crepr(mach, p, buf+n, buflen-n);
	}
      else if (qsdouble_p(mach, p))
	{
	  n += qsdouble_crepr(mach, p, buf+n, buflen-n);
	}
      else if (qscptr_p(mach, p))
	{
	  n += qscptr_crepr(mach, p, buf+n, buflen-n);
	}
    }
  else if (ISSYM26(p))
    {
    }
  else if (ISFD20(p))
    {
      n += qsfd_crepr(mach, p, buf+n, buflen-n);
    }
  else if (ISERR20(p))
    {
      n += qserr_crepr(mach, p, buf+n, buflen-n);
    }
  else if (ISPRIM20(p))
    {
      n += qsprim_crepr(mach, p, buf+n, buflen-n);
    }
  else if (ISCONST20(p))
    {
      n += qsconst_crepr(mach, p, buf+n, buflen-n);
    }
  else
    {
      n += qs_snprintf(buf+n, buflen-n, "#<ptr %08x>", p);
    }

  return n;
}

