#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

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


qsptr_t qsptr_make (qsmachine_t * mach, qsaddr_t mapped_addr)
{
  qsptr_t retval = QSOBJ(mapped_addr >> 4);
  return retval;
}


/* Directly encoded values. */

qsptr_t qsnil_make (qsmachine_t * mach)
{
  return QSNIL;
}

bool qsnil_p (const qsmachine_t * mach, qsptr_t p)
{
  return ISNIL(p);
}


qsptr_t qsbool_make (qsmachine_t * mach, int val)
{
  return val ? QSTRUE : QSFALSE;
}

bool qsbool_p (const qsmachine_t * mach, qsptr_t p)
{
  return (p == QSTRUE) || (p == QSFALSE);
}

bool qsbool_get (const qsmachine_t * mach, qsptr_t p)
{
  if (p == QSFALSE) return false;
  /* N.B. allow for nil to mean false. */
  /*
  if (qsnil_p(mach, p)) return false;
  */
  return true;
}

int qsbool_crepr (const qsmachine_t * mach, qsptr_t p, char *buf, int buflen)
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


qsptr_t qsfloat_make (qsmachine_t * mach, float val)
{
  return QSFLOAT(val);
}

bool qsfloat_p (const qsmachine_t * mach, qsptr_t p)
{
  return ISFLOAT31(p);
}

float qsfloat_get (const qsmachine_t * mach, qsptr_t p)
{
  float fval = CFLOAT31(p);
  return fval;
}

int qsfloat_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  float fval = CFLOAT31(p);

  n += qs_snprintf(buf+n, buflen-n, "%g", fval);
  return n;
}


qsptr_t qsint_make (qsmachine_t * mach, int32_t val)
{
  return QSINT(val);
}

bool qsint_p (const qsmachine_t * mach, qsptr_t p)
{
  return ISINT30(p);
}

int32_t qsint_get (const qsmachine_t * mach, qsptr_t p)
{
  int ival = CINT30(p);
  return ival;
}

int qsint_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  int ival = CINT30(p);

  n += qs_snprintf(buf+n, buflen-n, "%d", ival);
  return n;
}


qsptr_t qschar_make (qsmachine_t * mach, int val)
{
  return QSCHAR(val);
}

bool qschar_p (const qsmachine_t * mach, qsptr_t p)
{
  return ISCHAR24(p);
}

int qschar_get (const qsmachine_t * mach, qsptr_t p)
{
  int chval = CCHAR24(p);
  return chval;
}

int qschar_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
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


qsptr_t qsconst_make (qsmachine_t * mach, int const_id)
{
  return QSCONST(const_id);
}

bool qsconst_p (const qsmachine_t * mach, qsptr_t p)
{
  return ISCONST20(p);
}

int qsconst_id (const qsmachine_t * mach, qsptr_t p)
{
  return CCONST20(p);
}

int qsconst_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
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


qsptr_t qsfd_make (qsmachine_t * mach, int val)
{
  /* TODO: check value too large. */
  return QSFD(val);
}

qsptr_t qsfd_open (qsmachine_t * mach, const char  * path, int flags, int mode)
{
  int fd = open(path, flags, mode);
  if (fd < 0) return QSERR_FAULT;
  return qsfd_make(mach, fd);
}

bool qsfd_p (const qsmachine_t * mach, qsptr_t p)
{
  if (ISFD20(p))
    {
      /* Ensure FD is valid. */
      int fd = CFD20(p);
      off_t res = lseek(fd, 0, SEEK_CUR);
      return (res >= 0);
    }
  return false;
}

int qsfd_id (const qsmachine_t * mach, qsptr_t p)
{
  return CFD20(p);
}

bool qsfd_eof (const qsmachine_t * mach, qsptr_t p)
{
  /* FDs allows for dynamic/variable EOF.
     For normal files, read() returns 0 on EOF.
   */
  return false;
}

int qsfd_read_u8 (const qsmachine_t * mach, qsptr_t p)
{
  if (! qsfd_p(mach, p)) return -1;
  uint8_t byte;
  int fd = qsfd_id(mach, p);
  ssize_t res = read(fd, &byte, 1);
  if (res > 0)
    return byte;
  else
    return -1;
}

bool qsfd_write_u8 (const qsmachine_t * mach, qsptr_t p, int byte)
{
  if (! qsfd_p(mach, p)) return false;
  uint8_t buf = (uint8_t)byte;
  int fd = qsfd_id(mach, p);
  ssize_t res = write(fd, &buf, 1);
  return (res > 0);
}

qsword qsfd_tell (const qsmachine_t * mach, qsptr_t p)
{
  if (! qsfd_p(mach, p)) return 0;
  int fd = qsfd_id(mach, p);
  return lseek(fd, 0, SEEK_CUR);
}

bool qsfd_seek (const qsmachine_t * mach, qsptr_t p, qsword pos)
{
  if (! qsfd_p(mach, p)) return 0;
  int fd = qsfd_id(mach, p);
  off_t res = lseek(fd, pos, SEEK_SET);
  return (res >= 0);
}

bool qsfd_close (const qsmachine_t * mach, qsptr_t p)
{
  int fd = qsfd_id(mach, p);
  int res = close(fd);
  return (res == 0);
}

int qsfd_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  int fdval = CFD20(p);

  n += qs_snprintf(buf+n, buflen-n, "#<fd %d>", fdval);

  return n;
}


qsptr_t qsprim_make (qsmachine_t * mach, qsword primid)
{
  return QSPRIM(primid);
}

bool qsprim_p (const qsmachine_t * mach, qsptr_t p)
{
  return ISPRIM20(p);
}

int qsprim_id (const qsmachine_t * mach, qsptr_t p)
{
  return CPRIM20(p);
}

int qsprim_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  int primid = CPRIM20(p);

  n += qs_snprintf(buf+n, buflen-n, "#<prim %d>", primid);

  return n;
}


qsptr_t qserr_make (qsmachine_t * mach, qsword errid)
{
  return QSERR(errid);
}

qsword qserr_p (const qsmachine_t * mach, qsptr_t p)
{
  return ISERR20(p);
}

qsword qserr_id (const qsmachine_t * mach, qsptr_t p)
{
  return CERR20(p);
}

int qserr_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
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


qsptr_t qsobj_make (qsmachine_t * mach, qsword obj_id)
{
  return QSOBJ(obj_id);
}

qsword qsobj_p (const qsmachine_t * mach, qsptr_t p)
{
  return ISOBJ26(p);
}

qsword qsobj_id (const qsmachine_t * mach, qsptr_t p)
{
  return COBJ26(p);
}

qsaddr_t qsobj_address (const qsmachine_t * mach, qsptr_t p)
{
  return COBJ26(p) << 4;
}

const qsobj_t * qsobj_const (const qsmachine_t * mach, qsptr_t p)
{
  if (! ISOBJ26(p)) return NULL;
  qsaddr_t mapped_addr = qsobj_address(mach, p);
  const qsobj_t * obj = (const qsobj_t*)(qsstore_word_at_const(&(mach->S), mapped_addr));
  if (! obj) return NULL;
  if (! MGMT_IS_USED(obj->mgmt)) return NULL;
  return obj;
}

qsobj_t * qsobj (qsmachine_t * mach, qsptr_t p)
{
  if (qsobj_const(mach, p))
    {
      return (qsobj_t*)(qsstore_word_at(&(mach->S), qsobj_address(mach, p)));
    }
  return NULL;
}

bool _qsobj_is_octetate (const qsobj_t * obj)
{
  return MGMT_IS_OCT(obj->mgmt);
}

int _qsobj_get_allocscale (const qsobj_t * obj)
{
  return MGMT_GET_ALLOC(obj->mgmt);
}

int _qsobj_get_nbounds (const qsobj_t * obj)
{
  return (1 << _qsobj_get_allocscale(obj));
}

int _qsobj_get_allocsize (const qsobj_t * obj)
{
  return _qsobj_get_nbounds(obj) * sizeof(qsobj_t);
}


/* QsSym: symbols by id.  See qssymbol for storing name. */
qsptr_t qssym_make (qsmachine_t * mach, qsword sym_id)
{
  return QSSYM(sym_id);
}

bool qssym_p (const qsmachine_t * mach, qsptr_t p)
{
  return ISSYM26(p);
}

qsword qssym_id (const qsmachine_t * mach, qsptr_t p)
{
  return CSYM26(p);
}

int qssym_crepr (qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  return n;
}

qsptr_t qssym_symbol (const qsmachine_t * mach, qsptr_t p)
{
  return QSOBJ(CSYM26(p));
}

qsptr_t qssym_name (const qsmachine_t * mach, qsptr_t p)
{
  return QSOBJ(CSYM26(p));
}

qscmp_t qssym_cmp (qsmachine_t * mach, qsptr_t x, qsptr_t y)
{
  return (x == y) ? QSCMP_EQ : QSCMP_NE;
}


/* Heaped object */

/* Heaped prototype: Triplet. */
const qstriplet_t * qstriplet_const (const qsmachine_t * mach, qsptr_t p)
{
  const qsobj_t * obj = qsobj_const(mach, p);
  if (! obj) return NULL;
  if (_qsobj_is_octetate(obj)) return NULL;
  if (_qsobj_get_allocscale(obj) != 0) return NULL;
  return (const qstriplet_t*)obj;
}

qstriplet_t * qstriplet (qsmachine_t * mach, qsptr_t p)
{
  if (qstriplet_const(mach, p))
    {
      return (qstriplet_t*)(qsobj(mach, p));
    }
  return NULL;
}

qsptr_t qstriplet_make (qsmachine_t * mach, qsptr_t first, qsptr_t second, qsptr_t third)
{
  qsaddr_t mapped_addr = 0;
  qserr_t err = qsstore_alloc(&(mach->S), 0, &mapped_addr);
  if (err != QSERR_OK)
    return err;
  qsobj_t * obj = (qsobj_t*)(qsstore_word_at(&(mach->S), mapped_addr));
  qsobj_init(obj, 0, false);
  qstriplet_t * triplet = (qstriplet_t*)obj;
  triplet->first = first;
  triplet->second = second;
  triplet->third = third;
  qsptr_t retval = qsptr_make(mach, mapped_addr);
  return retval;
}

qscmp_t qstriplet_cmp (const qsmachine_t * mach, qsptr_t x, qsptr_t y)
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
const qswideword_t * qswideword_const (const qsmachine_t * mach, qsptr_t p)
{
  const qsobj_t * obj = qsobj_const(mach, p);
  if (! obj) return NULL;
  if (! _qsobj_is_octetate(obj)) return NULL;
  if (_qsobj_get_allocscale(obj) != 0) return NULL;
  return (const qswideword_t*)obj;
}

qswideword_t * qswideword (qsmachine_t * mach, qsptr_t p)
{
  if (qswideword_const(mach, p))
    {
      return (qswideword_t*)(qsobj(mach, p));
    }
  return NULL;
}

qsptr_t qswideword_make (qsmachine_t * mach, qsptr_t subtype)
{
  qsaddr_t mapped_addr = 0;
  qserr_t err = qsstore_alloc(&(mach->S), 0, &mapped_addr);
  if (err != QSERR_OK)
    return err;
  qsobj_t * obj = (qsobj_t*)(qsstore_word_at(&(mach->S), mapped_addr));
  qsobj_init(obj, 0, true);
  qswideword_t * wideword = (qswideword_t*)obj;
  wideword->subtype = subtype;
  wideword->payload.l = 0;
  qsptr_t retval = qsptr_make(mach, mapped_addr);
  return retval;
}

qscmp_t qswideword_cmp (const qsmachine_t * mach, qsptr_t x, qsptr_t y)
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
const qspvec_t * qspvec_const (const qsmachine_t * mach, qsptr_t p)
{
  const qsobj_t * obj = qsobj_const(mach, p);
  if (! obj) return NULL;
  if (_qsobj_is_octetate(obj)) return NULL;
  if (_qsobj_get_allocscale(obj) == 0) return NULL;
  return (const qspvec_t*)obj;
}

qspvec_t * qspvec (qsmachine_t * mach, qsptr_t p)
{
  if (qspvec_const(mach, p))
    {
      return (qspvec_t*)(qsobj(mach, p));
    }
  return NULL;
}

qsptr_t qspvec_make (qsmachine_t * mach, qsptr_t payload_words, qsptr_t fillval)
{
  qsaddr_t mapped_addr = 0;
  qserr_t err = qsstore_alloc_nwords(&(mach->S), payload_words, &mapped_addr);
  if (err != QSERR_OK)
    return err;
  qsobj_t * obj = (qsobj_t*)(qsstore_word_at(&(mach->S), mapped_addr));
  int a = _qsobj_get_allocscale(obj);
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
  qsptr_t retval = qsptr_make(mach, mapped_addr);
  return retval;
}

qscmp_t qspvec_cmp (const qsmachine_t * mach, qsptr_t x, qsptr_t y)
{
  const qspvec_t * vx = qspvec_const(mach, x);
  const qspvec_t * vy = qspvec_const(mach, y);
  if (!vx || !vy) return QSCMP_NE;
  if (x == y) return QSCMP_EQ;
  if (vx->length != vy->length) return QSCMP_NE;
  qsword i;
  qsword m = (1 << qsobj_get_allocscale((qsobj_t*)vx)) * sizeof(qsobj_t)/sizeof(qsptr_t) - sizeof(qsobj_t);
  for (i = 0; i < m; i++)
    if (vx->elt[i] != vy->elt[i]) return QSCMP_NE;
  return QSCMP_EQ;
}


/* Heaped prototype: Octet Vector. */
const qsovec_t * qsovec_const (const qsmachine_t * mach, qsptr_t p)
{
  const qsobj_t * obj = qsobj_const(mach, p);
  if (! obj) return NULL;
  if (! _qsobj_is_octetate(obj)) return NULL;
  if (_qsobj_get_allocscale(obj) == 0) return NULL;
  return (const qsovec_t*)obj;
}

qsovec_t * qsovec (qsmachine_t * mach, qsptr_t p)
{
  if (qsovec_const(mach, p))
    {
      return (qsovec_t*)(qsobj(mach, p));
    }
  return NULL;
}

qsptr_t qsovec_make (qsmachine_t * mach, qsptr_t len, qsbyte fillval)
{
  qsaddr_t mapped_addr = 0;
  qserr_t err = qsstore_alloc_nbytes(&(mach->S), len, &mapped_addr);
  if (err != QSERR_OK)
    return err;
  qsobj_t * obj = (qsobj_t*)(qsstore_word_at_const(&(mach->S), mapped_addr));
  int a = _qsobj_get_allocscale(obj);
  qsobj_init(obj, a, true);
  qsovec_t * ovec = (qsovec_t*)obj;
  ovec->length = len;
  ovec->refcount = 0;
  ovec->reflock = 0;
  qsptr_t retval = qsptr_make(mach, mapped_addr);
  return retval;
}

qscmp_t qsovec_cmp (const qsmachine_t * mach, qsptr_t x, qsptr_t y)
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

qserr_t qsovec_hold (qsmachine_t * mach, qsptr_t p)
{
  /* TODO: lock. */
  qsovec_t * ovec = qsovec(mach, p);
  if (! ovec) return QSERR_FAULT;
  if (ovec->refcount >= MAX_INT30) return QSERR_FAULT;
  ++ovec->refcount;
  return QSERR_OK;
}

qserr_t qsovec_release (qsmachine_t * mach, qsptr_t p)
{
  /* TODO: lock. */
  qsovec_t * ovec = qsovec(mach, p);
  if (! ovec) return QSERR_FAULT;
  if (ovec->refcount <= 0) return QSERR_OK;  /* TODO: error on mismatch? */
  --ovec->refcount;
  return QSERR_OK;
}


/* Heaped object: Pair
   * prototype = qstriplet
   * .first is nil
 */

const qstriplet_t * qspair_const (const qsmachine_t * mach, qsptr_t p)
{
  const qstriplet_t * triplet = qstriplet_const(mach, p);
  if (! triplet) return NULL;
  if (! qsnil_p(mach, triplet->first)) return NULL;
  return triplet;
}

qstriplet_t * qspair (qsmachine_t * mach, qsptr_t p)
{
  if (qspair_const(mach, p))
    return qstriplet(mach, p);
  return NULL;
}

qsptr_t qspair_make (qsmachine_t * mach, qsptr_t a, qsptr_t d)
{
  qsptr_t p = qstriplet_make(mach, QSNIL, a, d);
  return p;
}

bool qspair_p (const qsmachine_t * mach, qsptr_t p)
{
  return (qspair_const(mach, p) != NULL);
}

qsptr_t qspair_ref_head (const qsmachine_t * mach, qsptr_t p)
{
  const qstriplet_t * pair = qspair_const(mach, p);
  if (! pair) return QSERR_FAULT;
  return pair->second;
}

qsptr_t qspair_ref_tail (const qsmachine_t * mach, qsptr_t p)
{
  const qstriplet_t * pair = qspair_const(mach, p);
  if (! pair) return QSERR_FAULT;
  return pair->third;
}

qsptr_t qspair_setq_head (qsmachine_t * mach, qsptr_t p, qsptr_t a)
{
  qstriplet_t * pair = qspair(mach, p);
  if (! pair) return QSERR_FAULT;
  pair->second = a;
  return p;
}

qsptr_t qspair_setq_tail (qsmachine_t * mach, qsptr_t p, qsptr_t d)
{
  qstriplet_t * pair = qspair(mach, p);
  if (! pair) return QSERR_FAULT;
  pair->third = d;
  return p;
}

qsptr_t qspair_iter (const qsmachine_t * mach, qsptr_t p)
{
  if (! qspair_const(mach, p)) return QSERR_FAULT;
  qsaddr_t memaddr = qsobj_address(mach, p);
  qsptr_t retval = qsiter_make(mach, memaddr);
  return retval;
}

int qspair_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  const qstriplet_t * pair = NULL;
  qsptr_t curr = p;

  n += qs_snprintf(buf+n, buflen-n, "%s", "(");

  while (! qsnil_p(mach, curr))
    {
      pair = qspair_const(mach, curr);
      qsptr_t head = pair->second;
      qsptr_t tail = pair->third;

      n += qsptr_crepr(mach, head, buf+n, buflen-n);

      if (qsnil_p(mach, tail))
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

const qspvec_t * qsvector_const (const qsmachine_t * mach, qsptr_t p)
{
  const qspvec_t * pvec = qspvec_const(mach, p);
  if (! pvec) return NULL;
  if (! ISINT30(pvec->length)) return NULL;
  return pvec;
}

qspvec_t * qsvector (qsmachine_t * mach, qsptr_t p)
{
  if (qsvector_const(mach, p))
    return (qspvec_t*)(qspvec(mach, p));
  return NULL;
}

qsptr_t qsvector_make (qsmachine_t * mach, qsword len, qsptr_t fill)
{
  qsptr_t p = qspvec_make(mach, len, fill);
  qspvec_t * pvec = qspvec(mach, p);
  pvec->length = QSINT(len);
  return p;
}

bool qsvector_p (const qsmachine_t * mach, qsptr_t p)
{
  return (qsvector_const(mach, p) != NULL);
}

qsword qsvector_length (const qsmachine_t * mach, qsptr_t p)
{
  const qspvec_t * pvec = qsvector_const(mach, p);
  if (! pvec) return 0;
  return CINT30(pvec->length);
}

qsptr_t qsvector_ref (const qsmachine_t * mach, qsptr_t p, qsword k)
{
  const qspvec_t * vec = qsvector_const(mach, p);
  if (! vec) return QSERR_FAULT;
  if ((k < 0) || (k >= qsvector_length(mach, p))) return QSERR_FAULT;
  return vec->elt[k];
}

qsptr_t qsvector_setq (qsmachine_t * mach, qsptr_t p, qsword k, qsptr_t val)
{
  qspvec_t * pvec = qsvector(mach, p);
  if (! pvec) return QSERR_FAULT;
  if ((k < 0) || (k >= qsvector_length(mach, p))) return QSERR_FAULT;
  pvec->elt[k] = val;
  return p;
}

int qsvector_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  qsword i, m;

  n += qs_snprintf(buf+n, buflen-n, "%s", "#(");

  m = qsvector_length(mach, p);
  for (i = 0; i < m; i++)
    {
      qsptr_t x = qsvector_ref(mach, p, i);
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

const qspvec_t * qsarray_const (const qsmachine_t * mach, qsptr_t p)
{
  const qspvec_t * pvec = qspvec_const(mach, p);
  if (! pvec) return NULL;
  if (! qsnil_p(mach, pvec->length)) return NULL;
  return pvec;
}

qspvec_t * qsarray (qsmachine_t * mach, qsptr_t p)
{
  if (qsarray_const(mach, p))
    return (qspvec_t*)(qspvec(mach, p));
  return NULL;
}

qsptr_t qsarray_make (qsmachine_t * mach, qsword len)
{
  qsptr_t p = qspvec_make(mach, len, QSNIL);
  qspvec_t * pvec = qspvec(mach, p);
  pvec->length = QSNIL;
  return p;
}

qsptr_t qsarray_vinject (qsmachine_t * mach, va_list vp)
{
  qsword nptrs = 0;
  va_list vp0;
  qsptr_t elt;

  /* bookmark. */
  va_copy(vp0, vp);

  /* count arguments. */
  int depth = 1;
  while (depth > 0)
    {
      elt = va_arg(vp0, qsptr_t);
      if (elt == QSBOL)
	depth++;
      else if (elt == QSEOL)
	depth--;
      nptrs++;
    }
  va_end(vp0);

  /* prepare memory. */
  qsptr_t retval = qsarray_make(mach, nptrs);
  if (! ISOBJ26(retval))
    return QSERR_FAULT;

  /* populate memory. */
  qsword i;
  for (i = 0; i < nptrs; i++)
    {
      elt = va_arg(vp, qsptr_t);
      qsarray_setq(mach, retval, i, elt);
    }
  qsarray_setq(mach, retval, i, QSEOL);

  return retval;
}

qsptr_t qsarray_inject (qsmachine_t * mach, ...)
{
  va_list vp;
  qsptr_t retval = QSNIL;

  va_start(vp, mach);
  retval = qsarray_vinject(mach, vp);
  va_end(vp);
  return retval;
}

bool qsarray_p (const qsmachine_t * mach, qsptr_t p)
{
  return (qsarray_const(mach, p) != NULL);
}

qsword qsarray_length (const qsmachine_t * mach, qsptr_t p)
{
  const qspvec_t * pvec = qsarray_const(mach, p);
  if (! pvec) return 0;
  const int words_per_boundary = sizeof(qsobj_t) / sizeof(qsptr_t);
  qsaddr_t allocsize = (1 << qsobj_get_allocscale((const qsobj_t*)pvec));
  qsword retval = allocsize * words_per_boundary;
  return retval;
}

qsptr_t qsarray_ref (const qsmachine_t * mach, qsptr_t p, qsword k)
{
  const qspvec_t * vec = qsarray_const(mach, p);
  if (! vec) return QSERR_FAULT;
  if ((k < 0) || (k >= qsarray_length(mach, p))) return QSERR_FAULT;
  return vec->elt[k];
}

qsptr_t qsarray_setq (qsmachine_t * mach, qsptr_t p, qsword k, qsptr_t val)
{
  qspvec_t * pvec = qsarray(mach, p);
  if (! pvec) return QSERR_FAULT;
  if ((k < 0) || (k >= qsarray_length(mach, p))) return QSERR_FAULT;
  pvec->elt[k] = val;
  return p;
}

qsptr_t qsarray_iter (const qsmachine_t * mach, qsptr_t p)
{
  const qspvec_t * pvec = qsarray_const(mach, p);
  if (! pvec) return QSERR_FAULT;
  qsaddr_t memaddr = qsobj_address(mach, p) + sizeof(qsobj_t);  /* next boundary. */
  return qsiter_make(mach, memaddr);
}

int qsarray_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  qsword i, m;

  n += qs_snprintf(buf+n, buflen-n, "%s", "#(");

  m = qsarray_length(mach, p);
  for (i = 0; i < m; i++)
    {
      qsptr_t x = qsarray_ref(mach, p, i);
      if (i > 0)
	n += qs_snprintf(buf+n, buflen-n, " ");
      n += qsptr_crepr(mach, x, buf+n, buflen-n);
    }

  n += qs_snprintf(buf+n, buflen-n, "%s", ")");

  return n;
}


/* C-Pointer: prototype 'wideword', 'subtype' == QSWIDE_CPTR. */

const qswideword_t * qscptr_const (const qsmachine_t * mach, qsptr_t p)
{
  const qswideword_t * cptr = qswideword_const(mach, p);
  if (! cptr) return NULL;
  if (cptr->subtype != QSWIDE_CPTR) return NULL;
  return cptr;
}

qswideword_t * qscptr (qsmachine_t * mach, qsptr_t p)
{
  if (qscptr_const(mach, p))
    {
      return (qswideword_t*)(qscptr_const(mach, p));
    }
  return NULL;
}

qsptr_t qscptr_make (qsmachine_t * mach, void * val)
{
  qsptr_t p = qswideword_make(mach, QSWIDE_CPTR);
  qswideword_t * cptr = qswideword(mach, p);
  cptr->payload.ptr = val;
  return p;
}

bool qscptr_p (const qsmachine_t * mach, qsptr_t p)
{
  return (qscptr_const(mach, p) != NULL);
}

void * qscptr_get (const qsmachine_t * mach, qsptr_t p)
{
  const qswideword_t * cptr = qscptr_const(mach, p);
  if (! cptr) return NULL;
  return cptr->payload.ptr;
}

int qscptr_fetch (const qsmachine_t * mach, qsptr_t p, void ** out)
{
  const qswideword_t * cptr = qscptr_const(mach, p);
  if (! cptr) return 0;
  if (out) *out = cptr->payload.ptr;
  return 1;
}

int qscptr_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
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

const qswideword_t * qslong_const (const qsmachine_t * mach, qsptr_t p)
{
  const qswideword_t * l = qswideword_const(mach, p);
  if (! l) return NULL;
  if (l->subtype != QSNUM_LONG) return NULL;
  return l;
}

qswideword_t * qslong (qsmachine_t * mach, qsptr_t p)
{
  if (qslong_const(mach, p))
    {
      return (qswideword_t*)(qswideword(mach, p));
    }
  return NULL;
}

qsptr_t qslong_make (qsmachine_t * mach, int64_t val)
{
  qsptr_t p = qswideword_make(mach, QSNUM_LONG);
  if (ISOBJ26(p))
    {
      qswideword_t * l = qswideword(mach, p);
      if (l)
	l->payload.l = val;
    }
  return p;
}

bool qslong_p (const qsmachine_t * mach, qsptr_t p)
{
  return (qslong_const(mach, p) != NULL);
}

int64_t qslong_get (const qsmachine_t * mach, qsptr_t p)
{
  const qswideword_t * l = qswideword_const(mach, p);
  if (! l) return 0;
  return l->payload.l;
}

int qslong_fetch (const qsmachine_t * mach, qsptr_t p, int64_t * out)
{
  const qswideword_t * l = qswideword_const(mach, p);
  if (! l) return 0;
  if (out) *out = l->payload.l;
  return 1;
}

int qslong_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
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
const qswideword_t * qsdouble_const (const qsmachine_t * mach, qsptr_t p)
{
  const qswideword_t * d = qswideword_const(mach, p);
  if (! d) return NULL;
  if (d->subtype != QSNUM_DOUBLE) return NULL;
  return d;
}

qswideword_t * qsdouble (qsmachine_t * mach, qsptr_t p)
{
  if (qsdouble_const(mach, p))
    {
      return (qswideword_t*)(qswideword(mach, p));
    }
  return NULL;
}

qsptr_t qsdouble_make (qsmachine_t * mach, double val)
{
  qsptr_t p = qswideword_make(mach, QSNUM_DOUBLE);
  if (ISOBJ26(p))
    {
      qswideword_t * d = qswideword(mach, p);
      d->payload.d = val;
    }
  return p;
}

bool qsdouble_p (const qsmachine_t * mach, qsptr_t p)
{
  if (qsdouble_const(mach, p) != NULL);
}

double qsdouble_get (const qsmachine_t * mach, qsptr_t p)
{
  const qswideword_t * d = qswideword_const(mach, p);
  if (! d) return 0;
  return d->payload.d;
}

int qsdouble_fetch (const qsmachine_t * mach, qsptr_t p, double * out)
{
  const qswideword_t * d = qswideword_const(mach, p);
  if (! d) return 0;
  if (out) *out = d->payload.d;
  return 1;
}

int qsdouble_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
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


/* QsName: symbol name.  See qssym for comparing symbols by object id. */
/* Heaped object: Name
   * prototype = ovec
   * .length isa char24
 */
const qsovec_t * qsname_const (const qsmachine_t * mach, qsptr_t p)
{
//  if (ISSYM26(p)) p = qssym_symbol(mach, p);
  const qsovec_t * y = qsovec_const(mach, p);
  if (! y) return NULL;
  if (! ISCHAR24(y->length)) return NULL;
  return y;
}

qsovec_t * qsname (qsmachine_t * mach, qsptr_t p)
{
//  if (ISSYM26(p)) p = qssym_symbol(mach, p);
  if (qsname_const(mach, p))
    {
      return (qsovec_t*)(qsovec(mach, p));
    }
  return NULL;
}

qsptr_t qsname_make (qsmachine_t * mach, qsword namelen)
{
  qsptr_t p = qsovec_make(mach, namelen+1, 0);
  qsovec_t * y = qsovec(mach, p);
  if (!y) return QSERR_FAULT;
  y->length = QSCHAR(namelen);
  return p;
}

qsptr_t _qsname_make (qsmachine_t * mach, qsword namelen, qsovec_t ** out_sym)
{
  qsptr_t p = qsovec_make(mach, namelen+1, 0);
  qsovec_t * y = qsovec(mach, p);
  if (!y) return QSERR_FAULT;
  if (out_sym) *out_sym = y;
  return p;
}

qsptr_t qsname_inject (qsmachine_t * mach, const char * cstr, qsword slen)
{
  if (0 == slen) slen = strlen(cstr);
  qsptr_t p = qsname_make(mach, slen);
  if (! ISOBJ26(p)) return QSERR_NOMEM;
  qsovec_t * y = qsname(mach, p);
  strcpy(y->elt, cstr);
  return p;
}

const char * qsname_get (const qsmachine_t * mach, qsptr_t p)
{
  const qsovec_t * y = qsname_const(mach, p);
  if (! y) return NULL;
  return y->elt;
}

bool qsname_p (const qsmachine_t * mach, qsptr_t p)
{
  return (qsname_const(mach, p) != NULL);
}

qsptr_t qsname_sym (const qsmachine_t * mach, qsptr_t p)
{
  if (ISSYM26(p)) return p;
  if (! qsname_const(mach, p)) return QSNIL;
  qsword obj_id = qsobj_id(mach, p);
  return QSSYM(obj_id);
}

qsword qsname_length (const qsmachine_t * mach, qsptr_t p)
{
  const qsovec_t * y = qsname_const(mach, p);
  if (!y) return 0;
  qsword retval = CCHAR24(y->length);
  return retval;
}

int qsname_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  const qsovec_t * y = qsovec_const(mach, p);
  if (! y) return n;
  int m = CCHAR24(y->length);
  n += qs_snprintf(buf+n, buflen-n-m, "%s", y->elt);
  return n;
}

int qsname_strcmp (const qsmachine_t * mach, qsptr_t x, const char * s)
{
  const qsovec_t * y = qsname_const(mach, x);
  if (! y) return 0;
  int m = qsname_length(mach, x);
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

qscmp_t qsname_cmp (const qsmachine_t * mach, qsptr_t x, qsptr_t y)
{
  const qsovec_t * y0 = qsname_const(mach, x);
  const qsovec_t * y1 = qsname_const(mach, y);
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

const qsovec_t * qsutf8_const (const qsmachine_t * mach, qsptr_t p)
{
  const qsovec_t * s = qsovec_const(mach, p);
  if (! s) return NULL;
  if (! ISINT30(s->length)) return NULL;
  if (qsobj_get_score((qsobj_t*)s) != 8) return NULL;
  return s;
}

qsovec_t * qsutf8 (qsmachine_t * mach, qsptr_t p)
{
  if (qsutf8_const(mach, p))
    {
      return (qsovec_t*)(qsovec(mach, p));
    }
  return NULL;
}

qsptr_t qsutf8_make (qsmachine_t * mach, qsword len, int fill)
{
  /* Allocate one more byte for terminating guard byte '\0'. */
  qsptr_t p = qsovec_make(mach, len+1, fill);
  qsobj_t * obj = qsobj(mach, p);
  qsobj_set_score(obj, 8);
  qsovec_t * ovec = (qsovec_t*)obj;
  /* Valid bytes discount the terminating '\0'. */
  ovec->length = QSINT(len);
  return p;
}

qsptr_t qsutf8_inject_charp (qsmachine_t * mach, const char * cstr, size_t slen)
{
  if (0 == slen) slen = strlen(cstr);
  qsptr_t retval = qsutf8_make(mach, slen, 0);
  if (ISOBJ26(retval))
    {
      qsovec_t * st = qsutf8(mach, retval);
      strncpy(st->elt, cstr, slen);
    }
  return retval;
}

qsptr_t qsutf8_inject_bytes (qsmachine_t * mach, uint8_t * buf, qsword buflen)
{
  qsptr_t retval = qsutf8_make(mach, buflen, 0);
  if (ISOBJ26(retval))
    {
      qsovec_t * st = qsutf8(mach, retval);
      memcpy(st->elt, buf, buflen);
    }
  return retval;
}

bool qsutf8_p (const qsmachine_t * mach, qsptr_t p)
{
  return (qsutf8_const(mach, p) != NULL);
}

qsword qsutf8_length (const qsmachine_t * mach, qsptr_t p)
{
  const qsovec_t * s = qsutf8_const(mach, p);
  if (! s) return 0;
  return CINT30(s->length);
}

int qsutf8_ref (const qsmachine_t * mach, qsptr_t p, qsword k)
{
  const qsovec_t * s = qsutf8_const(mach, p);
  if (! s) return 0;
  /* TODO: multi-byte character */
  return s->elt[k];
}

const char * qsutf8_get (const qsmachine_t * mach, qsptr_t p, qsword * len)
{
  const qsovec_t * s = qsutf8_const(mach, p);
  if (! s) return NULL;
  if (len) *len = qsutf8_length(mach, p);
  return s->elt;
}

int qsutf8_fetch (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
{
  qsword len = 0;
  const char * t = qsutf8_get(mach, p, &len);
  if (! t) return 0;
  if (len < buflen) len = buflen;
  snprintf(buf, len, "%s", t);
  return len;
}

qsptr_t qsutf8_setq (qsmachine_t * mach, qsptr_t p, qsword k, int ch)
{
  qsovec_t * s = qsutf8(mach, p);
  if (! s) return QSERR_FAULT;
  /* TODO: multi-byte character. */

  if ((k < 0) || (k >= qsutf8_length(mach, p))) return QSERR_FAULT;
  s->elt[k] = ch;
  return p;
}

int qsutf8_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
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

qserr_t qsutf8_hold (qsmachine_t * mach, qsptr_t p)
{
  return qsovec_hold(mach, p);
}

qserr_t qsutf8_release (qsmachine_t * mach, qsptr_t p)
{
  return qsovec_release(mach, p);
}

qscmp_t qsutf8_cmp (const qsmachine_t * mach, qsptr_t x, qsptr_t y)
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
const qsovec_t * qsbytevec_const (const qsmachine_t * mach, qsptr_t p)
{
  const qsovec_t * ovec = qsovec_const(mach, p);
  if (! ovec) return NULL;
  if (! ISINT30(ovec->length)) return NULL;
  return (const qsovec_t*)ovec;
}

qsovec_t * qsbytevec (qsmachine_t * mach, qsptr_t p)
{
  if (qsbytevec_const(mach, p))
    {
      return (qsovec_t*)(qsovec(mach, p));
    }
  return NULL;
}

qsptr_t qsbytevec_make (qsmachine_t * mach, qsword len, qsbyte fill)
{
  qsptr_t p = qsovec_make(mach, QSINT(len), fill);
  qsobj_t * obj = qsobj(mach, p);
  qsobj_set_score(obj, 0);
  return p;
}

bool qsbytevec_p (const qsmachine_t * mach, qsptr_t p)
{
  return (qsbytevec_const(mach, p) != NULL);
}

qsword qsbytevec_length (const qsmachine_t * mach, qsptr_t p)
{
  const qsovec_t * ovec = qsovec_const(mach, p);
  if (! ovec) return 0;
  return CINT30(ovec->length);
}

bool qsbytevec_extract (const qsmachine_t * mach, qsptr_t p, const uint8_t ** out_uint8ptr, qsword * out_size)
{
  const qsovec_t * ovec = qsovec_const(mach, p);
  if (! ovec) return false;
  if (out_uint8ptr) *out_uint8ptr = ovec->elt;
  if (out_size) *out_size = CINT30(ovec->length);
  return true;
}

qsbyte qsbytevec_ref (const qsmachine_t * mach, qsptr_t p, qsword k)
{
  const qsovec_t * ovec = qsovec_const(mach, p);
  if (! ovec) return 0;
  return ovec->elt[k];
}

qsptr_t qsbytevec_setq (qsmachine_t * mach, qsptr_t p, qsword k, qsbyte val)
{
  qsovec_t * ovec = qsovec(mach, p);
  if (! ovec) return QSERR_FAULT;
  ovec->elt[k] = val;
  return p;
}

int qsbytevec_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
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


/* Phase 1: environment is list of ( QsEnvFrame . QsEnv )
   Where QsEnvFrame is alist of ( QsSym . QsValue )
   */

qsptr_t qsenv_make (qsmachine_t * mach, qsptr_t next_env)
{
  qsptr_t retval = qspair_make(mach, QSNIL, next_env);
  return retval;
}

qsptr_t qsenv_insert (qsmachine_t * mach, qsptr_t env, qsptr_t variable, qsptr_t binding)
{
  if (qsnil_p(mach, env))
    {
      env = qsenv_make(mach, QSNIL);
    }
  qsptr_t frame = qspair_car(mach, env);
  qsptr_t bind = qspair_make(mach, variable, binding);
  qsptr_t link = qspair_make(mach, bind, frame);
  qspair_setq_head(mach, env, link);
  return env;
}

qsptr_t qsenv_lookup (qsmachine_t * mach, qsptr_t env, qsptr_t variable)
{
  qsptr_t frameiter;
  qsptr_t binditer;

  frameiter = qsiter_begin(mach, env);
  while (ISITER28(frameiter))
    {
      qsptr_t binditer = qsiter_begin(mach, qsiter_head(mach, frameiter));
      while (ISITER28(binditer))
	{
	  qsptr_t bind = qsiter_head(mach, binditer);
	  if (qspair_ref_head(mach, bind) == variable)
	    {
	      return qspair_ref_tail(mach, bind);
	    }
	  binditer = qsiter_tail(mach, binditer);
	}
      frameiter = qsiter_tail(mach, frameiter);
    }
  return QSERR_UNBOUND;
}

int qsenv_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
{
  return qspair_crepr(mach, p, buf, buflen);
}


const qstriplet_t * qslambda_const (const qsmachine_t * mach, qsptr_t p)
{
  const qstriplet_t * triplet = qstriplet_const(mach, p);
  if (! triplet) return NULL;
  if (triplet->first != QSLAMBDA) return NULL;
  return triplet;
}

qstriplet_t * qslambda (qsmachine_t * mach, qsptr_t p)
{
  if (qslambda_const(mach, p))
    {
      return qstriplet(mach, p);
    }
  return NULL;
}

/* Combine list of formal parameters with body. */
qsptr_t qslambda_make (qsmachine_t * mach, qsptr_t param, qsptr_t body)
{
  qsptr_t p = qstriplet_make(mach, QSLAMBDA, param, body);
  return p;
}

bool qslambda_p (const qsmachine_t * mach, qsptr_t p)
{
  return (qslambda_const(mach, p) != NULL);
}

qsptr_t qslambda_ref_param (const qsmachine_t * mach, qsptr_t p)
{
  const qstriplet_t * lam = qslambda_const(mach, p);
  if (! lam) return QSERR_FAULT;
  return lam->second;
}

qsptr_t qslambda_ref_body (const qsmachine_t * mach, qsptr_t p)
{
  const qstriplet_t * lam = qslambda_const(mach, p);
  if (! lam) return QSERR_FAULT;
  return lam->third;
}

qsptr_t qslambda_setq_param (qsmachine_t * mach, qsptr_t p, qsptr_t val)
{
  qstriplet_t * lam = qslambda(mach, p);
  if (! lam) return QSERR_FAULT;
  lam->second = val;
  return p;
}

qsptr_t qslambda_setq_body (qsmachine_t * mach, qsptr_t p, qsptr_t val)
{
  qstriplet_t * lam = qslambda(mach, p);
  if (! lam) return QSERR_FAULT;
  lam->third = val;
  return p;
}

int qslambda_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
//  n += qs_snprintf(buf+n, buflen-n, "#<lambda 0x%08x>", qsobj-id(mach, p));
  const qstriplet_t * lam = qstriplet_const(mach, p);
  if (! lam) return n;
  n += qs_snprintf(buf+n, buflen-n, "(lambda ");
  qsptr_t param = lam->second;
  qsptr_t body = lam->third;
  n += qsptr_crepr(mach, param, buf+n, buflen-n);
  if (! qsnil_p(mach, body))
    {
      n += qs_snprintf(buf+n, buflen-n, " ");
      n += qsptr_crepr(mach, body, buf+n, buflen-n);
    }
  n += qs_snprintf(buf+n, buflen-n, ")");
  return n;
}


const qstriplet_t * qsclosure_const (const qsmachine_t * mach, qsptr_t p)
{
  const qstriplet_t * triplet = qstriplet_const(mach, p);
  if (! triplet) return NULL;
  if (triplet->first != QSCLO) return NULL;
  return triplet;
}

qstriplet_t * qsclosure (qsmachine_t * mach, qsptr_t p)
{
  if (qsclosure_const(mach, p))
    {
      return qstriplet(mach, p);
    }
  return NULL;
}

/* Combine lambda with environment. */
qsptr_t qsclosure_make (qsmachine_t * mach, qsptr_t lam, qsptr_t env)
{
  qsptr_t p = qstriplet_make(mach, QSCLO, lam, env);
  return p;
}

bool qsclosure_p (const qsmachine_t * mach, qsptr_t p)
{
  return (qsclosure_const(mach, p) != NULL);
}

qsptr_t qsclosure_ref_lam (const qsmachine_t * mach, qsptr_t p)
{
  const qstriplet_t * clo = qsclosure_const(mach, p);
  if (! clo) return QSERR_FAULT;
  return clo->second;
}

qsptr_t qsclosure_ref_env (const qsmachine_t * mach, qsptr_t p)
{
  const qstriplet_t * clo = qsclosure_const(mach, p);
  if (! clo) return QSERR_FAULT;
  return clo->third;
}

qsptr_t qsclosure_setq_lam (qsmachine_t * mach, qsptr_t p, qsptr_t val)
{
  qstriplet_t * clo = qsclosure(mach, p);
  if (! clo) return QSERR_FAULT;
  clo->second = val;
}

qsptr_t qsclosure_setq_env (qsmachine_t * mach, qsptr_t p, qsptr_t val)
{
  qstriplet_t * clo = qsclosure(mach, p);
  if (! clo) return QSERR_FAULT;
  clo->third = val;
}

int qsclosure_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  n += qs_snprintf(buf+n, buflen-n, "#<closure 0x%08x>", qsobj_id(mach,p));
  return n;
}


const qspvec_t * qskont_const (const qsmachine_t * mach, qsptr_t p)
{
  const qspvec_t * pvec = qspvec_const(mach, p);
  if (!pvec) return NULL;
  if (pvec->length != QSKONT) return NULL;
  return pvec;
}

qspvec_t * qskont (qsmachine_t * mach, qsptr_t p)
{
  if (qskont_const(mach, p))
    {
      return qspvec(mach, p);
    }
  return NULL;
}

/* arbitrary continuation */
qsptr_t qskont_make (qsmachine_t * mach, qsptr_t variant, qsptr_t c, qsptr_t e, qsptr_t k)
{
  qsptr_t p = qspvec_make(mach, 4, QSNIL);
  qspvec_t * pvec = qspvec(mach, p);
  if (! pvec) return QSERR_NOMEM;
  pvec->length = QSKONT;
  pvec->elt[0] = variant;
  pvec->elt[1] = c;
  pvec->elt[2] = e;
  pvec->elt[3] = k;
  return p;
}

/* current-continuation */
qsptr_t qskont_make_current (qsmachine_t * mach)
{
  return qskont_make(mach, QSNIL, mach->C, mach->E, mach->K);
}

bool qskont_p (const qsmachine_t * mach, qsptr_t p)
{
  return (qskont_const(mach, p) != NULL);
}

qsptr_t qskont_ref_v (const qsmachine_t * mach, qsptr_t p)
{
  const qspvec_t * kont = qskont_const(mach, p);
  if (!kont) return QSERR_FAULT;
  return kont->elt[0];
}

qsptr_t qskont_ref_c (const qsmachine_t * mach, qsptr_t p)
{
  const qspvec_t * kont = qskont_const(mach, p);
  if (!kont) return QSERR_FAULT;
  return kont->elt[1];
}

qsptr_t qskont_ref_e (const qsmachine_t * mach, qsptr_t p)
{
  const qspvec_t * kont = qskont_const(mach, p);
  if (!kont) return QSERR_FAULT;
  return kont->elt[2];
}

qsptr_t qskont_ref_k (const qsmachine_t * mach, qsptr_t p)
{
  const qspvec_t * kont = qskont_const(mach, p);
  if (!kont) return QSERR_FAULT;
  return kont->elt[3];
}

int qskont_fetch (const qsmachine_t * mach, qsptr_t p, qsptr_t * out_v, qsptr_t * out_c, qsptr_t * out_e, qsptr_t * out_k)
{
  int n = 0;
  if (out_v) { *out_v = qskont_ref_v(mach, p); n++; }
  if (out_c) { *out_c = qskont_ref_c(mach, p); n++; }
  if (out_e) { *out_e = qskont_ref_e(mach, p); n++; }
  if (out_k) { *out_k = qskont_ref_k(mach, p); n++; }
  return n;
}

qsptr_t qskont_setq_v (qsmachine_t * mach, qsptr_t p, qsptr_t val)
{
  qspvec_t * kont = qskont(mach, p);
  if (!kont) return QSERR_FAULT;
  kont->elt[0] = val;
  return p;
}

qsptr_t qskont_setq_c (qsmachine_t * mach, qsptr_t p, qsptr_t val)
{
  qspvec_t * kont = qskont(mach, p);
  if (!kont) return QSERR_FAULT;
  kont->elt[1] = val;
  return p;
}

qsptr_t qskont_setq_e (qsmachine_t * mach, qsptr_t p, qsptr_t val)
{
  qspvec_t * kont = qskont(mach, p);
  if (!kont) return QSERR_FAULT;
  kont->elt[2] = val;
  return p;
}

qsptr_t qskont_setq_k (qsmachine_t * mach, qsptr_t p, qsptr_t val)
{
  qspvec_t * kont = qskont(mach, p);
  if (!kont) return QSERR_FAULT;
  kont->elt[3] = val;
  return p;
}

int qskont_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  const qspvec_t * kont = qskont_const(mach, p);
  n += qs_snprintf(buf+n, buflen-n, "#<kont 0x%08x>", qsobj_id(mach, p));
  return n;
}



/* Heavy Port (base class).
   Ports where state information has to be stored in Scheme objects.
   No const versions as ports can only exist in writeable memory.
 */
qspvec_t * qscport (qsmachine_t * mach, qsptr_t p)
{
  qspvec_t * port = qspvec(mach, p);
  if (! port) return NULL;
  switch (port->length)
    {
    case QSPORT_CFILE:
    case QSPORT_BYTEVEC:
    case QSPORT_CHARP:
      return port;
    default:
      return NULL;
    }
}

qsptr_t qscport_make (qsmachine_t * mach, qsptr_t variant, qsptr_t pathspec, bool writeable, qsptr_t host_resource)
{
  qsptr_t p = qsvector_make(mach, 4, QSNIL);
  qspvec_t * vec = qspvec(mach, p);
  if (! vec) return QSERR_FAULT;
  vec->length = variant;
  if (writeable)
    {
      qsobj_t * obj = qsobj(mach, p);
      if (obj)
	{
	  qsobj_set_score(obj, qsobj_get_score(obj) | 1);
	}
    }
  /* 0: path spec. */
  vec->elt[0] = pathspec;
  /* 1: position. */
  vec->elt[1] = QSINT(0);
  /* 2: maximum (EOF). */
  vec->elt[2] = QSINT(-1);
  /* 3: host resource. */
  vec->elt[3] = host_resource;

  return p;
}

bool qscport_p (qsmachine_t * mach, qsptr_t p)
{
  return (qscport(mach, p) != NULL);
}

bool qscport_get_writeable (qsmachine_t * mach, qsptr_t p)
{
  const qsobj_t * obj = qsobj_const(mach, p);
  if (qsobj_get_score(obj) & 1)
    {
      return true;
    }
  return false;
}

qsptr_t qscport_set_writeable (qsmachine_t * mach, qsptr_t p, bool val)
{
  qsobj_t * obj = qsobj(mach, p);
  if (val)
    {
      qsobj_set_score(obj, qsobj_get_score(obj) | 1);
    }
  else
    {
      qsobj_set_score(obj, qsobj_get_score(obj) & ~1);
    }
  return p;
}

qsptr_t qscport_get_pathspec (qsmachine_t * mach, qsptr_t p)
{
  qspvec_t * port = qscport(mach, p);
  if (! port) return QSNIL;
  return port->elt[0];
}

qsptr_t qscport_set_pathspec (qsmachine_t * mach, qsptr_t p, qsptr_t s)
{
  qspvec_t * port = qscport(mach, p);
  if (! port) return QSNIL;
  port->elt[0] = s;
  return p;
}

int qscport_get_pos (qsmachine_t * mach, qsptr_t p)
{
  qspvec_t * port = qscport(mach, p);
  if (! port) return QSNIL;
  return CINT30(port->elt[1]);
}

qsptr_t qscport_set_pos (qsmachine_t * mach, qsptr_t p, int pos)
{
  qspvec_t * port = qscport(mach, p);
  if (! port) return QSNIL;
  port->elt[1] = QSINT(pos);
  return p;
}

int qscport_get_max (qsmachine_t * mach, qsptr_t p)
{
  qspvec_t * port = qscport(mach, p);
  if (! port) return QSNIL;
  return CINT30(port->elt[2]);
}

qsptr_t qscport_set_max (qsmachine_t * mach, qsptr_t p, int max)
{
  qspvec_t * port = qscport(mach, p);
  if (! port) return QSNIL;
  port->elt[2] = QSINT(max);
  return p;
}

qsptr_t qscport_get_resource (qsmachine_t * mach, qsptr_t p)
{
  qspvec_t * port = qscport(mach, p);
  if (! port) return QSNIL;
  return port->elt[3];
}

qsptr_t qscport_set_resource (qsmachine_t * mach, qsptr_t p, qsptr_t val)
{
  qspvec_t * port = qscport(mach, p);
  if (! port) return QSNIL;
  port->elt[3] = val;
  return p;
}


/* C Character Pointer (String) Port:
   port reading from C char array (string),
   writing to C char array (string).
 */
qspvec_t * qscharpport (qsmachine_t * mach, qsptr_t p)
{
  qspvec_t * port = qscport(mach, p);
  if (! port) return NULL;
  if (port->length != QSPORT_CHARP) return NULL;
  return port;
}

qsptr_t qscharpport_make (qsmachine_t * mach, uint8_t * buf, int buflen)
{
  qsptr_t variant = QSPORT_CHARP;
  bool writeable = false;
  qsptr_t pathspec = QSNIL;
  qsptr_t cptr = qscptr_make(mach, (void*)buf);
  if (! qscptr_p(mach, cptr)) return QSERR_NOMEM;

  qsptr_t p = qscport_make(mach, variant, pathspec, writeable, cptr);
  if (! qscport_p(mach, p)) return QSERR_NOMEM;
  qscport_set_max(mach, p, buflen);
  return p;
}

bool qscharpport_p (qsmachine_t * mach, qsptr_t p)
{
  return (qscharpport(mach, p) != NULL);
}

bool qscharpport_eof (qsmachine_t * mach, qsptr_t p)
{
  int retval = -1;
  if (! qscharpport(mach, p)) return -1;
  int pos = qscport_get_pos(mach, p);
  int max = qscport_get_max(mach, p);
  return (pos >= max);
}

int qscharpport_read_u8 (qsmachine_t * mach, qsptr_t p)
{
  int retval = -1;
  if (! qscharpport(mach, p)) return -1;
  int pos = qscport_get_pos(mach, p);
  int max = qscport_get_max(mach, p);
  qsptr_t cptr = qscport_get_resource(mach, p);
  if (pos >= max) return -1;
  if (pos < 0) return -1;
  uint8_t * s = qscptr_get(mach, cptr);
  if (! s) return -1;
  retval = s[pos];
  ++pos;
  qscport_set_pos(mach, p, pos);
  return retval;
}

bool qscharpport_write_u8 (qsmachine_t * mach, qsptr_t p, int byte)
{
  if (! qscharpport(mach, p)) return false;
  if (! qscport_get_writeable(mach, p)) return false;
  int pos = qscport_get_pos(mach, p);
  int max = qscport_get_max(mach, p);
  qsptr_t cptr = qscport_get_resource(mach, p);
  if (pos >= max) return false;
  if (pos < 0) return false;
  uint8_t * s = qscptr_get(mach, cptr);
  if (! s) return false;
  s[pos] = byte;
  ++pos;
  qscport_set_pos(mach, p, pos);
  return true;
}

qsword qscharpport_tell (qsmachine_t * mach, qsptr_t p)
{
  if (! qscharpport(mach, p)) return 0;
  return qscport_get_pos(mach, p);
}

bool qscharpport_seek (qsmachine_t * mach, qsptr_t p, qsword pos)
{
  if (! qscharpport(mach, p)) return false;
  qscport_set_pos(mach, p, pos);
  return true;
}

bool qscharpport_close (qsmachine_t * mach, qsptr_t p)
{
  if (! qscharpport(mach, p)) return false;
  qscport_set_max(mach, p, 0);
  qscport_set_pos(mach, p, 0);
  qscport_set_resource(mach, p, QSNIL);  /* unlink resource. */
  qscport_set_pathspec(mach, p, QSNIL);
  qscport_set_writeable(mach, p, false);
  /* then wait for garbage collection. */
  return true;
}


/* OctetVector Port:
   port reading from Scheme bytevec.
   writing to Scheme bytevec.
 */

qspvec_t * qsovport (qsmachine_t * mach, qsptr_t p)
{
  qspvec_t * port = qscport(mach, p);
  if (! port) return NULL;
  if (port->length != QSPORT_BYTEVEC) return NULL;
  return port;
}

qsptr_t qsovport_make (qsmachine_t * mach, qsptr_t bv)
{
  qsptr_t variant = QSPORT_BYTEVEC;
  bool writeable = false;
  qsptr_t pathspec = QSNIL;

  if (! qsbytevec_p(mach, bv)) return QSERR_FAULT;
  qsptr_t p = qscport_make(mach, variant, pathspec, writeable, bv);
  if (! qscport_p(mach, p)) return QSERR_FAULT;
  qscport_set_max(mach, p, qsbytevec_length(mach, bv));
  return p;
}

bool qsovport_p (qsmachine_t * mach, qsptr_t p)
{
  return (qsovport(mach, p) != NULL);
}

bool qsovport_eof (qsmachine_t * mach, qsptr_t p)
{
  int retval = -1;
  if (! qsovport(mach, p)) return -1;
  int pos = qscport_get_pos(mach, p);
  int max = qscport_get_max(mach, p);
  return (pos >= max);
}

int qsovport_read_u8 (qsmachine_t * mach, qsptr_t p)
{
  int retval = -1;
  if (! qsovport(mach, p)) return -1;
  int pos = qscport_get_pos(mach, p);
  int max = qscport_get_max(mach, p);
  qsptr_t bv = qscport_get_resource(mach, p);
  if (pos >= max) return -1;
  if (pos < 0) return -1;
  retval = qsbytevec_ref(mach, bv, pos);
  return retval;
}

bool qsovport_write_u8 (qsmachine_t * mach, qsptr_t p, int byte)
{
  if (! qsovport(mach, p)) return false;
  if (! qscport_get_writeable(mach, p)) return false;
  int pos = qscport_get_pos(mach, p);
  int max = qscport_get_max(mach, p);
  qsptr_t bv = qscport_get_resource(mach, p);
  if (pos >= max) return false;
  if (pos < 0) return false;
  /* TODO: expand. */
  qsbytevec_setq(mach, bv, pos, byte);
  ++pos;
  qscport_set_pos(mach, p, pos);
  return true;
}

qsword qsovport_tell (qsmachine_t * mach, qsptr_t p)
{
  if (! qsovport(mach, p)) return 0;
  return qscport_get_pos(mach, p);
}

bool qsovport_seek (qsmachine_t * mach, qsptr_t p, qsword pos)
{
  if (! qsovport(mach, p)) return false;
  qscport_set_pos(mach, p, pos);
  return true;
}

bool qsovport_close (qsmachine_t * mach, qsptr_t p)
{
  if (! qsovport(mach, p)) return false;
  qscport_set_max(mach, p, 0);
  qscport_set_pos(mach, p, 0);
  qscport_set_resource(mach, p, QSNIL);  /* unlink resource. */
  qscport_set_pathspec(mach, p, QSNIL);
  qscport_set_writeable(mach, p, false);
  /* then wait for garbage collection. */
  return true;
}


/* Backed by Standard C File. */
qspvec_t * qsfport (qsmachine_t * mach, qsptr_t p)
{
  qspvec_t * port = qscport(mach, p);
  if (! port) return NULL;
  if (port->length != QSPORT_CFILE) return NULL;
  return port;
}

qsptr_t qsfport_make (qsmachine_t * mach, const char * path, const char * mode)
{
  qsptr_t variant = QSPORT_CFILE;
  bool writeable = false;
  if (mode)
    {
      if (strchr(mode, 'w') || strchr(mode, 'a')) writeable = true;
    }
  else
    {
      mode = "r";
    }
  qsptr_t pathspec = qsutf8_inject_charp(mach, path, 0);
  if (! qsutf8_p(mach, pathspec)) return QSERR_NOMEM;

  FILE * f = fopen(path, mode);
  if (! f) return QSERR_FAULT;
  qsptr_t fptr = qscptr_make(mach, (void*)f);
  if (! qscptr_p(mach, fptr)) return QSERR_NOMEM;
  qsptr_t p = qscport_make(mach, variant, pathspec, writeable, fptr);
  if (! qscport_p(mach, p)) return QSERR_NOMEM;
  return p;
}

bool qsfport_p (qsmachine_t * mach, qsptr_t p)
{
  return (qsfport(mach, p) != NULL);
}

FILE * qsfport_get (qsmachine_t * mach, qsptr_t p)
{
  if (! qsfport(mach, p)) return NULL;
  qsptr_t fptr = qscport_get_resource(mach, p);
  if (! qscptr_p(mach, fptr)) return NULL;
  FILE * f = (FILE*)(qscptr_get(mach, fptr));
  return f;
}

bool qsfport_eof (qsmachine_t * mach, qsptr_t p)
{
  int retval = -1;
  FILE * f = qsfport_get(mach, p);
  if (! f) return false;
  return feof(f);
}

int qsfport_read_u8 (qsmachine_t * mach, qsptr_t p)
{
  int retval = -1;
  FILE * f = qsfport_get(mach, p);
  if (! f) return -1;
  if (feof(f)) return -1;
  retval = fgetc(f);
  return retval;
}

bool qsfport_write_u8 (qsmachine_t * mach, qsptr_t p, int byte)
{
  FILE * f = qsfport_get(mach, p);
  if (! f) return false;
  int res = fputc(byte, f);
  return (res > 0);
}

qsword qsfport_tell (qsmachine_t * mach, qsptr_t p)
{
  FILE * f = qsfport_get(mach, p);
  if (! f) return false;
  return ftell(f);
}

bool qsfport_seek (qsmachine_t * mach, qsptr_t p, qsword pos)
{
  FILE * f = qsfport_get(mach, p);
  if (! f) return false;
  int newpos = fseek(f, pos, SEEK_SET);
  return (newpos == pos);
}

bool qsfport_close (qsmachine_t * mach, qsptr_t p)
{
  if (! qsfport(mach, p)) return false;

  /* Finalizer */
    {
      qsptr_t fptr = qscport_get_resource(mach, p);
      FILE * f = (FILE*)(qscptr_get(mach, p));
      if (f) fclose(f);
    }

  qscport_set_resource(mach, p, QSNIL);  /* unlink resource. */
  qscport_set_pathspec(mach, p, QSNIL);
  qscport_set_writeable(mach, p, false);
  /* then wait for garbage collection. */
  return true;
}


/* Overall Port wrapper. */
qsptr_t qsport_make_c (qsmachine_t * mach, qsptr_t variant, const uint8_t * spec, int speclen, bool writeable, bool appending)
{
  qsptr_t retval = QSERR_FAULT;
  switch (variant)
    {
    case QSPORT_FD:
	{
	  if (spec)
	    {
	      int flags = O_RDONLY;
	      if (writeable) flags = O_RDWR | O_CREAT;
	      if (appending) flags |= O_WRONLY;
	      int mode = 0600;  /* Default permissions on create. */
	      retval = qsfd_open(mach, spec, flags, 0600);
	      if (appending)
		{
		  int fd = qsfd_id(mach, retval);
		  lseek(fd, 0, SEEK_END);
		}
	    }
	  else
	    {
	      /* take fd as-is from speclen */
	      retval = qsfd_make(mach, speclen);
	    }
	}
      break;
    case QSPORT_CFILE:
	{
	  const char * cmode = "r";
	  if (writeable) cmode = "w+";
	  if (appending) cmode = "a";
	  retval = qsfport_make(mach, spec, cmode);
	}
      break;
    case QSPORT_CHARP:
	{
	  if (speclen == 0)
	    speclen = strlen(spec) + 1;
	  uint8_t * spec_rw = (uint8_t*)spec;
	  retval = qscharpport_make(mach, spec_rw, speclen);
	  if (writeable) qscport_set_writeable(mach, retval, true);
	}
      break;
    default:
      break;
    }
  return retval;
}

qsptr_t qsport_make (qsmachine_t * mach, qsptr_t variant, qsptr_t path, bool writeable, bool appending)
{
  qsptr_t retval = QSERR_FAULT;
  switch (variant)
    {
    case QSPORT_FD:
	{
	  if (qsutf8_p(mach, path))
	    {
	      const char * s = NULL;
	      const qsovec_t * st = qsutf8_const(mach, path);
	      if (! st) return QSERR_FAULT;
	      s = st->elt;
	      retval = qsport_make_c(mach, variant, s, 0, writeable, appending);
	    }
	  else if (qsint_p(mach, path))
	    {
	      retval = qsfd_make(mach, CINT30(path));
	    }
	  else if (qsfd_p(mach, path))
	    {
	      retval = path;
	    }
	}
      break;
    case QSPORT_BYTEVEC:
      retval = qsovport_make(mach, path);
      if (writeable) qscport_set_writeable(mach, retval, 1);
      /* TODO: append. */
      break;
    case QSPORT_CFILE:
	{
	  const char * s = NULL;
	  const qsovec_t * st = qsutf8_const(mach, path);
	  if (! st) return QSERR_FAULT;
	  s = st->elt;
	  retval = qsport_make_c(mach, variant, s, 0, writeable, appending);
	}
      break;
    default:
      break;
    }
  return retval;
}

bool qsport_p (qsmachine_t * mach, qsptr_t p)
{
  if (qsfd_p(mach, p)) return true; /* TODO: check still open. */
  if (! qscport_p(mach, p)) return false;
  return true;
}

bool qsport_eof (qsmachine_t * mach, qsptr_t p)
{
  if (qsfd_p(mach, p)) return qsfd_eof(mach, p);
  if (qsfport_p(mach, p)) return qsfport_eof(mach, p);
  if (qsovport_p(mach, p)) return qsovport_eof(mach, p);
  if (qscharpport_p(mach, p)) return qscharpport_eof(mach, p);
  return true; /* not a port => eof. */
}

int qsport_read_u8 (qsmachine_t * mach, qsptr_t p)
{
  if (qsfd_p(mach, p)) return qsfd_read_u8(mach, p);
  if (qsfport_p(mach, p)) return qsfport_read_u8(mach, p);
  if (qsovport_p(mach, p)) return qsovport_read_u8(mach, p);
  if (qscharpport_p(mach, p)) return qscharpport_read_u8(mach, p);
  return -1;
}

bool qsport_write_u8 (qsmachine_t * mach, qsptr_t p, int byte)
{
  if (qsfd_p(mach, p)) return qsfd_write_u8(mach, p, byte);
  if (qsfport_p(mach, p)) return qsfport_write_u8(mach, p, byte);
  if (qsovport_p(mach, p)) return qsovport_write_u8(mach, p, byte);
  if (qscharpport_p(mach, p)) return qscharpport_write_u8(mach, p, byte);
  return -1;
}

qsword qsport_tell (qsmachine_t * mach, qsptr_t p)
{
  if (qsfd_p(mach, p)) return qsfd_tell(mach, p);
  if (qsfport_p(mach, p)) return qsfport_tell(mach, p);
  if (qsovport_p(mach, p)) return qsovport_tell(mach, p);
  if (qscharpport_p(mach, p)) return qscharpport_tell(mach, p);
  return 0;
}

bool qsport_seek (qsmachine_t * mach, qsptr_t p, qsword pos)
{
  if (qsfd_p(mach, p)) return qsfd_seek(mach, p, pos);
  if (qsfport_p(mach, p)) return qsfport_seek(mach, p, pos);
  if (qsovport_p(mach, p)) return qsovport_seek(mach, p, pos);
  if (qscharpport_p(mach, p)) return qscharpport_seek(mach, p, pos);
  return false;
}



/* QsIter: wrapper around Pair and Array. */
/* Make iterator from memory address, no checks.
   For constructing from object, see _iter() for the associated object.
 */
qsptr_t qsiter_make (const qsmachine_t * mach, qsaddr_t addr)
{
  qsptr_t retval = QSITER(addr >> 2);
  return retval;
}

qsptr_t qsiter_begin (const qsmachine_t * mach, qsptr_t p)
{
  if (qsiter_p(mach, p))
    {
      return p;
    }
  else if (qspair_p(mach, p))
    {
      return qspair_iter(mach, p);
    }
  else if (qsarray_p(mach, p))
    {
      return qsarray_iter(mach, p);
    }
  return QSNIL;
}

bool qsiter_p (const qsmachine_t * mach, qsptr_t p)
{
  return ISITER28(p);
}

qsaddr_t _qsiter_memaddr (const qsmachine_t * mach, qsptr_t p)
{
  qsaddr_t memaddr = CITER28(p) << 2;
  return memaddr;
}

bool _qsiter_on_pair (const qsmachine_t * mach, qsptr_t p, qsptr_t * out_pair)
{
  bool retval = false;
  qsaddr_t memaddr = _qsiter_memaddr(mach, p);
  if ((memaddr & 0xf) != 0) return false; /* not aligned, cannot be object. */
  qsptr_t maybe_pair = QSOBJ( memaddr >> 4 );
  if (qspair_const(mach, maybe_pair))
    {
      if (out_pair) *out_pair = maybe_pair;
      retval = true;
    }
  return retval;
}

qsptr_t _qsiter_word (const qsmachine_t * mach, qsptr_t p)
{
  qsaddr_t memaddr = _qsiter_memaddr(mach, p);
  qsptr_t retval = qsstore_get_word(&(mach->S), memaddr);
  return retval;
}

/* returns contents of list head (car). */
qsptr_t qsiter_head (const qsmachine_t * mach, qsptr_t p)
{
  qsptr_t retval;
  qsptr_t pair = QSNIL;

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
qsptr_t qsiter_tail (const qsmachine_t * mach, qsptr_t p)
{
  qsptr_t retval;
  qsptr_t pair = QSNIL;
  int depth = 0;

  if (! qsiter_p(mach, p)) return QSERR_FAULT;
  if (_qsiter_on_pair(mach, p, &pair))
    {
      qsptr_t next = qspair_ref_tail(mach, pair);
      if (qsobj_p(mach, next))
	//retval = QSITER( COBJ26(next) << 2 );
	retval = qsiter_make(mach, qsobj_address(mach, next));
      else if (qsnil_p(mach, next))
	retval = next;
      else
	retval = QSERR_FAULT;
    }
  else
    {
      qsword skip = 1;
      qsptr_t head = _qsiter_word(mach, p);
      if (head == QSBOL)
	{
	  /* skip to matching QSEOL */
	  depth = 1;
	  while (depth > 0)
	    {
	      qsptr_t elt = _qsiter_word(mach, p + (skip << SHIFT_TAG28));
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
      qsptr_t next = QSITER(CITER28(p) + skip);
      /* peek for end of iteration. */
      qsptr_t peek = _qsiter_word(mach, next);
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

int qsiter_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  if (! ISITER28(p)) return 0;
  if (_qsiter_on_pair(mach, p, NULL))
    {
      n += qspair_crepr(mach, p, buf+n, buflen-n);
    }
  else
    {
      qsptr_t it = p;

      n += qs_snprintf(buf+n, buflen-n, "(");
      while (ISITER28(it))
	{
	  qsptr_t x = qsiter_head(mach, it);
	  if (it != p)
	    n += qs_snprintf(buf+n, buflen-n, " ");
	  n += qsptr_crepr(mach, x, buf+n, buflen-n);
	  it = qsiter_tail(mach, it);
	}
      n += qs_snprintf(buf+n, buflen-n, ")");
    }
  return n;
}


/* QsSymbol: wrapper/combiner of QsSym and QsName */

bool qssymbol_p (const qsmachine_t * mach, qsptr_t p)
{
  return (qssym_p(mach,p) || qsname_p(mach,p));
}

/* convert string to (interned) symbol. */
qsptr_t qssymbol_bless (qsmachine_t * mach, qsptr_t s)
{
  if (! qsutf8_p(mach, s))
    return QSERR_FAULT;
  qsword slen = qsutf8_length(mach, s);

  qsptr_t p = qsname_make(mach, slen);
  if (! ISOBJ26(p)) return QSERR_FAULT;

  qsovec_t * y = qsname(mach, p);
  qsword i;
  for (i = 0; i < slen; i++)
    {
      /* TODO convert multi-byte sequence. */
      y->elt[i] = qsutf8_ref(mach, s, i);
    }
  y->elt[i] = 0;

  /* intern symbol. */
  qssymstore_insert(mach, mach->Y, p);

  qsptr_t retval = qsname_sym(mach, p);
  return retval;
}

/* intern a Name object into symbol table. */
qsptr_t qssymbol_intern (qsmachine_t * mach, qsptr_t p)
{
  if (! qsname_p(mach, p)) return QSERR_FAULT;
  qssymstore_insert(mach, mach->Y, p);
  qsptr_t retval = qsname_sym(mach, p);
  return retval;
}

qsptr_t qssymbol_intern_c (qsmachine_t * mach, const char * cstr, int slen)
{
  /* 1. find already interned symbol. */
  qsptr_t extant = qssymstore_find_c(mach, cstr);
  if (qssymbol_p(mach, extant))
    return extant;

  /* 2. fallback to constructing symbol object. */
  qsptr_t p;
  p = qsname_inject(mach, cstr, slen);

  /* 3. then interning symbol object. */
  p = qssymbol_intern(mach, p);

  return p;
}

const char * qssymbol_get (const qsmachine_t * mach, qsptr_t p)
{
  qsptr_t x;
  if (qssym_p(mach, p)) x = qssym_name(mach, p);
  if (! qsname_p(mach, x)) return NULL;
  const qsovec_t * st = qsname_const(mach, x);
  if (! st) return NULL;
  return st->elt;
}

int qssymbol_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  qsptr_t x = p;
  if (qssym_p(mach, p)) x = qssym_name(mach, p);
  if (! qsname_p(mach, x)) return 0;
  const char * symname = qssymbol_get(mach, x);
  n += qs_snprintf(buf+n, buflen-n, "%s", symname);
  return n;
}

qscmp_t qssymbol_cmp (const qsmachine_t * mach, qsptr_t x, qsptr_t y)
{
  return (x == y) ? QSCMP_EQ : QSCMP_NE;
}




int qsptr_crepr (const qsmachine_t * mach, qsptr_t p, char * buf, int buflen)
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
      else if (qsarray_p(mach, p))
	{
	  n += qsarray_crepr(mach, p, buf+n, buflen-n);
	}
      else if (qsutf8_p(mach, p))
	{
	  n += qsutf8_crepr(mach, p, buf+n, buflen-n);
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
      else if (qsname_p(mach, p))
	{
	  n += qsname_crepr(mach, p, buf+n, buflen-n);
	}
      else if (qslambda_p(mach, p))
	{
	  n += qslambda_crepr(mach, p, buf+n, buflen-n);
	}
      else if (qsclosure_p(mach, p))
	{
	  n += qsclosure_crepr(mach, p, buf+n, buflen-n);
	}
      else if (qskont_p(mach, p))
	{
	  n += qskont_crepr(mach, p, buf+n, buflen-n);
	}
    }
  else if (ISSYM26(p))
    {
      qsptr_t symobj = QSOBJ(CSYM26(p));
      n += qsname_crepr(mach, symobj, buf+n, buflen-n);
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

