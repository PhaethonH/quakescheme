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


static
qsptr_t qsprim_halt (qsmachine_t * mach, qsptr_t args)
{
  mach->halt = true;
  return QSERR_FAULT;
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
struct prims_table_s table1[] = {
      { "halt", qsprim_halt },
      { "add", qsprim_add },
      { NULL, NULL },
};

qsptr_t qsprimreg_presets_v1 (qsmachine_t * mach)
{
  qsptr_t primenv = qsenv_make(mach, QSNIL);
  qsptr_t y = QSNIL;
  int primid = 0;
  qsptr_t prim = QSNIL;

  struct prims_table_s * primiter = NULL;
  for (primiter = table1; primiter->name; ++primiter)
    {
      const char * name = primiter->name;
      qsprim_f op = primiter->op;
      primid = qsprimreg_find(mach, op);
      if (primid < 0)
	{
	  primid = qsprimreg_register(mach, op);
	  y = qssymbol_intern_c(mach, name, 0);
	  prim = qsprim_make(mach, primid);
	}
      primenv = qsenv_insert(mach, primenv, y, prim);
    }

  return primenv;
}

