#include <stdarg.h>
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


qsptr qsptr_make (qsmachine_t * mach, qsptr p)
{
  return p;
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

qsobj_t * qsobj (qsmachine_t * mach, qsptr p)
{
  qsword obj_id = COBJ26(p);
  qsobj_t * retval = NULL;
  qsword addr = (obj_id << 4);
  retval = (qsobj_t*)(qsstore_word_at(&(mach->S), addr));
  return retval;
}


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

qsptr qspair_make (qsmachine_t * mach, qsptr a, qsptr d)
{
}

bool qspair_p (const qsmachine_t * mach, qsptr p)
{
}

qserr qspair_ref_head (const qsmachine_t * mach, qsptr p)
{
}

qserr qspair_ref_tail (const qsmachine_t * mach, qsptr p)
{
}

qserr qspair_setq_head (qsmachine_t * mach, qsptr p, qsptr a)
{
}

qserr qspair_setq_tail (qsmachine_t * mach, qsptr p, qsptr d)
{
}

int qspair_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  return n;
}


qsptr qsvector_make (qsmachine_t * mach, qsword len, qsptr fill)
{
}

bool qsvector_p (const qsmachine_t * mach, qsptr p)
{
}

qsptr qsvector_length (const qsmachine_t * mach, qsptr p)
{
}

qsptr qsvector_ref (const qsmachine_t * mach, qsptr p, qsword k)
{
}

qsptr qsvector_setq (qsmachine_t * mach, qsptr p, qsword k, qsptr val)
{
}

int qsvector_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  return n;
}


qsptr qscptr_make (qsmachine_t * mach, void * val)
{
}

bool qscptr_p (const qsmachine_t * mach, qsptr p)
{
}

void * qsctpr_get (const qsmachine_t * mach, qsptr p)
{
}

int qsctpr_fetch (const qsmachine_t * mach, qsptr p, void * out)
{
}

int qscptr_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  return n;
}


qsptr qslong_make (qsmachine_t * mach, int64_t val)
{
}

bool qslong_p (const qsmachine_t * mach, qsptr p)
{
}

int64_t qslong_get (const qsmachine_t * mach, qsptr p)
{
}

int qslong_fetch (const qsmachine_t * mach, qsptr p, int64_t * out)
{
}

int qslong_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  return n;
}


qsptr qsdouble_make (qsmachine_t * mach, double val)
{
}

bool qsdouble_p (const qsmachine_t * mach, qsptr p)
{
}

double qsdouble_get (const qsmachine_t * mach, qsptr p)
{
}

int qsdouble_fetch (const qsmachine_t * mach, qsptr p, double * out)
{
}

int qsdouble_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  return n;
}


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


qsptr qsstring_make (qsmachine_t * mach, qsword len, int fill)
{
}

bool qsstring_p (const qsmachine_t * mach, qsptr p)
{
}

int qsstring_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
  return n;
}


qsptr qsbytevec_make (qsmachine_t * mach, qsword len, qsbyte fill)
{
}

bool qsbytevec_p (const qsmachine_t * mach, qsptr p)
{
}

int qsbytevec_crepr (const qsmachine_t * mach, qsptr p, char * buf, int buflen)
{
  int n = 0;
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


