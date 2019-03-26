#include "qsval.h"
#include "qsmach.h"

#define LIST_P(x) (qspair_p(mach,x) || qsiter_p(mach,x))
#define CAR(x) (qspair_p(mach,x) ? qspair_ref_head(mach,x) : qsiter_p(mach,x) ?  qsiter_head(mach,x) : x)
#define CDR(x) (qspair_p(mach,x) ? qspair_ref_tail(mach,x) : qsiter_p(mach,x) ?  qsiter_tail(mach,x) : QSNIL)
#define CAAR(x) CAR(CAR(x))
#define CADR(x) CAR(CDR(x))
#define CDAR(x) CDR(CAR(x))
#define CDDR(x) CDR(CDR(x))
#define CAAAR(x) CAR(CAR(CAR(x)))
#define CAADR(x) CAR(CAR(CDR(x)))
#define CADAR(x) CAR(CDR(CAR(x)))
#define CADDR(x) CAR(CDR(CDR(x)))
#define CDAAR(x) CDR(CAR(CAR(x)))
#define CDADR(x) CDR(CAR(CDR(x)))
#define CDDAR(x) CDR(CDR(CAR(x)))
#define CDDDR(x) CDR(CDR(CDR(x)))


/* associative structure to establish primitives registry and bindings. */
struct prims_table_s {
    const char * name;
    qsprim_f op;
};



/*
Primitives are the gateways from Scheme code into C code.
Inspirations were drawn from *nix system calls.

Primitives are grouped into modules that may be mixed and matched.
The groupings can also be used to provide alternate implementations
(e.g.  complex-aware mathematics functions), or to grant/restrict
features (e.g. file functions).
*/


static
qsptr_t qsprim_halt (qsmachine_t * mach, qsptr_t args)
{
  mach->halt = true;
  return QSERR_FAULT;
}


/* Type Predicates. */
static qsptr_t qsprim_null_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qsnil_p(mach, arg0)) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_boolean_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qsbool_p(mach, arg0)) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_integer_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qsint_p(mach, arg0)) retval = QSTRUE;
  if (qslong_p(mach, arg0)) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_real_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qsfloat_p(mach, arg0)) retval = QSTRUE;
  if (qsdouble_p(mach, arg0)) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_number_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qsint_p(mach, arg0)) retval = QSTRUE;
  if (qslong_p(mach, arg0)) retval = QSTRUE;
  if (qsfloat_p(mach, arg0)) retval = QSTRUE;
  if (qsdouble_p(mach, arg0)) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_string_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qsutf8_p(mach, arg0)) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_symbol_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qsutf8_p(mach, arg0)) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_vector_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qsvector_p(mach, arg0)) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_bytevector_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qsbytevec_p(mach, arg0)) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_pair_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qspair_p(mach, arg0)) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_char_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qschar_p(mach, arg0)) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_procedure_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qsclosure_p(mach, arg0)) retval = QSTRUE;
  if (qsprim_p(mach, arg0)) retval = QSTRUE;
  return retval;
}


static
qsptr_t qsprim_add (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = qsint_make(mach, 0);
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  int ans = 0;

  if (qsnil_p(mach, arg0)) arg0 = qsint_make(mach, 0);
  if (qsnil_p(mach, arg1)) arg1 = qsint_make(mach, 0);

  if (qsint_p(mach, arg0) && qsint_p(mach, arg1))
    {
      ans = qsint_get(mach, arg0) + qsint_get(mach, arg1);
    }
  retval = qsint_make(mach, ans);
  return retval;
}


static
struct prims_table_s table1_typepredicates[] = {
      { "null?", qsprim_null_p },
      { "boolean?", qsprim_boolean_p },
      { "integer?", qsprim_integer_p },
      { "real?", qsprim_real_p },
      { "number?", qsprim_number_p },
      { "char?", qsprim_char_p },
      { "string?", qsprim_string_p },
      { "symbol?", qsprim_symbol_p },
      { "pair?", qsprim_pair_p },
      { "vector?", qsprim_vector_p },
      { "bytevector?", qsprim_bytevector_p },
      { "procedure?", qsprim_procedure_p },
      { NULL, NULL },
};

static
struct prims_table_s table1[] = {
      { "halt", qsprim_halt },
      { "TEST_add", qsprim_add },
      { NULL, NULL },
};

qsptr_t qsprimreg_presets_v1 (qsmachine_t * mach)
{
  qsptr_t primenv = qsenv_make(mach, QSNIL);
  qsptr_t y = QSNIL;
  int primid = 0;
  qsptr_t prim = QSNIL;

  struct prims_table_s * concat[] = {
      table1,
      table1_typepredicates,
      NULL
  };

  struct prims_table_s ** concatiter = NULL;
  for (concatiter = concat; *concatiter; ++concatiter)
    {
      struct prims_table_s * primiter = NULL;
      for (primiter = table1; primiter->name; ++primiter)
	{
	  const char * name = primiter->name;
	  qsprim_f op = primiter->op;
	  /* Avoid double-registering. */
	  primid = qsprimreg_find(mach, op);
	  if (primid < 0)
	    {
	      /* Not found in registray, add now. */
	      primid = qsprimreg_register(mach, op);
	    }
	  y = qssymbol_intern_c(mach, name, 0);
	  prim = qsprim_make(mach, primid);
	  primenv = qsenv_insert(mach, primenv, y, prim);
	}
    }

  return primenv;
}

