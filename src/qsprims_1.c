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
static qsptr_t qsprim_boolean_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qsbool_p(mach, arg0)) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_bytevector_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qsbytevec_p(mach, arg0)) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_char_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qschar_p(mach, arg0)) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_eof_object_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qschar_p(mach, arg0)) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_null_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qsnil_p(mach, arg0)) retval = QSTRUE;
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

static qsptr_t qsprim_pair_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qspair_p(mach, arg0)) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_port_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qsport_p(mach, arg0)) retval = QSTRUE;
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

static
struct prims_table_s table1_typepredicates[] = {
      { "boolean?", qsprim_boolean_p },
      { "bytevector?", qsprim_bytevector_p },
      { "char?", qsprim_char_p },
      { "eof-object?", qsprim_eof_object_p },
      { "null?", qsprim_null_p },
      { "number?", qsprim_number_p },
      { "pair?", qsprim_pair_p },
      { "port?", qsprim_port_p },
      { "procedure?", qsprim_procedure_p },
      { "string?", qsprim_string_p },
      { "symbol?", qsprim_symbol_p },
      { "vector?", qsprim_vector_p },
      { NULL, NULL },
};




/* Primitives: Booleans */
/* boolean:not to be implemented in Scheme. */
/* boolean:boolean=? to be implemented in Scheme. */




/* Primitives: Pairs and Lists. */

static qsptr_t qsprim_cons (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  retval = qspair_make(mach, arg0, arg1);
  return retval;
}

static qsptr_t qsprim_car (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  retval = qspair_ref_head(mach, arg0);
  return retval;
}

static qsptr_t qsprim_cdr (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  retval = qspair_ref_tail(mach, arg0);
  return retval;
}

static qsptr_t qsprim_setcarq (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  retval = qspair_setq_head(mach, arg0, arg1);
  return retval; /* object again, or error. */
}

static qsptr_t qsprim_setcdrq (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  retval = qspair_setq_tail(mach, arg0, arg1);
  return retval; /* object again, or error. */
}

/* Pairs:caar..cddddr to be implemented in Scheme. */
/* Lists:list? to be implemented in Scheme. */
/* Lists:list to be implemented in Scheme. */
/* Lists:length to be implemented in Scheme. */
/* Lists:append to be implemented in Scheme. */
/* Lists:reveres to be implemented in Scheme. */
/* Lists:list-tail to be implemented in Scheme. */
/* Lists:list-ref to be implemented in Scheme. */
/* Lists:list-set! to be implemented in Scheme. */
/* Lists:memq to be implemented in Scheme. */
/* Lists:memv to be implemented in Scheme. */
/* Lists:member to be implemented in Scheme. */
/* Lists:assqto be implemented in Scheme. */
/* Lists:assv to be implemented in Scheme. */
/* Lists:assoc to be implemented in Scheme. */
/* Lists:list-copy to be implemented in Scheme. */

static
struct prims_table_s table1_pairs[] = {
      { "cons", qsprim_cons },
      { "car", qsprim_car },
      { "cdr", qsprim_cdr },
      { "set-car!", qsprim_setcarq },
      { "set-cdr!", qsprim_setcdrq },
      { NULL, NULL },
};




/* Primitives: Symbols. */

static qsptr_t qsprim_symbol_to_string (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  retval = qsutf8_inject_charp(mach, qssymbol_get(mach, arg0), 0);
  return retval;
}

static qsptr_t qsprim_string_to_symbol (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  retval = qssymbol_bless(mach, arg0);
  return retval;
}

static
struct prims_table_s table1_symbols[] = {
      { "symbol->string", qsprim_cons },
      { "string->symbol", qsprim_car },
      { NULL, NULL },
};




/* Primitives: Strings. */
/* TODO: Strings:make-string */
/* Strings:string to be implemented in Scheme. */
/* TODO: Strings:string-length */
/* TODO: Strings:string-ref */
/* TODO: Strings:string-set! */
/* TODO: Strings:string=? */
/* TODO: Strings:string-ci=? */
/* TODO: Strings:string<? */
/* TODO: Strings:string>? */
/* TODO: Strings:string<=? */
/* TODO: Strings:string>=? */
/* TODO: Strings:string-ci<? */
/* TODO: Strings:string-ci>? */
/* TODO: Strings:string-ci<=? */
/* TODO: Strings:string-ci>=? */
/* TODO: Strings:string-upcase */
/* TODO: Strings:string-downcase */
/* TODO: Strings:string-foldcase */
/* TODO: Strings:substring */
/* TODO: Strings:string-append */
/* TODO: Strings:string->list */
/* TODO: Strings:list->string */
/* TODO: Strings:string-copy */
/* TODO: Strings:string-copy! */
/* TODO: Strings:string-fill! */




/* Primitives: Vectors */
static qsptr_t qsprim_make_vector (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  qsword k = qsint_get(mach, arg0);
  retval = qsvector_make(mach, k, arg1);
  return retval;
}

static qsptr_t qsprim_vector_length (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsword k = qsvector_length(mach, arg0);
  retval = qsint_make(mach, k);
  return retval;
}

static qsptr_t qsprim_vector_ref (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  qsword k = qsint_get(mach, arg1);
  retval = qsvector_ref(mach, arg0, k);
  return retval;
}

static qsptr_t qsprim_vector_setq (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  qsptr_t arg2 = CAR(CDR(CDR(args)));
  qsword k = qsint_get(mach, arg0);
  retval = qsvector_setq(mach, arg0, k, arg2);
  return retval; /* object itself, or error. */
}

/* TODO: Vector: vector->list */
/* TODO: Vector: list->vector */
/* TODO: Vector: vector->string */
/* TODO: Vector: string->vector */
/* TODO: Vector: vector-copy */
/* TODO: Vector: vector-copy! */
/* TODO: Vector: vector-append */
/* TODO: Vector: vector-fill! */

static
struct prims_table_s table1_vectorss[] = {
      { "make-vector", qsprim_make_vector },
      { "vector-length", qsprim_vector_length },
      { "vector-ref", qsprim_vector_ref },
      { "vector-set!", qsprim_vector_setq },
      { NULL, NULL },
};




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
      { "+", qsprim_add },
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
