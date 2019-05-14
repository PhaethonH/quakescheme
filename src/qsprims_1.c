#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <complex.h>
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
  (void)args;
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
      { "symbol->string", qsprim_symbol_to_string },
      { "string->symbol", qsprim_string_to_symbol },
      { NULL, NULL },
};




/* Primitives: Characters. */

static int _integer_from (const qsmachine_t * mach, qsptr_t p)
{
  if (qschar_p(mach, p)) return qschar_get(mach, p);
  if (qsint_p(mach, p)) return qsint_get(mach, p);
  if (qslong_p(mach, p)) return qslong_get(mach, p);
  if (qsfloat_p(mach, p)) return (int)(qsfloat_get(mach, p));
  if (qsdouble_p(mach, p)) return (int)(qsdouble_get(mach, p));
  return 0;
}

static qsptr_t qsprim_char_eq_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  qsword codept0 = qschar_get(mach, arg0);
  qsword codept1 = qschar_get(mach, arg1);
  retval = qsbool_make(mach, (codept0 == codept1));
  return retval;
}

static qsptr_t qsprim_char_lt_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  qsword codept0 = qschar_get(mach, arg0);
  qsword codept1 = qschar_get(mach, arg1);
  retval = qsbool_make(mach, (codept0 < codept1));
  return retval;
}

static qsptr_t qsprim_char_gt_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  qsword codept0 = qschar_get(mach, arg0);
  qsword codept1 = qschar_get(mach, arg1);
  retval = qsbool_make(mach, (codept0 > codept1));
  return retval;
}

/* Characters:char<=? */
/* Characters:char>=? */
/* Characters:char-ci=? */
/* Characters:char-ci<? */
/* Characters:char-ci>? */
/* Characters:char-ci<=? */
/* Characters:char-ci>=? */

static qsptr_t qsprim_char_alphabetic_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsword codepoint = qschar_get(mach, arg0);
  retval = qsbool_make(mach, isalpha(codepoint));
  return retval;
}

static qsptr_t qsprim_char_numeric_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsword codepoint = qschar_get(mach, arg0);
  retval = qsbool_make(mach, isdigit(codepoint));
  return retval;
}

static qsptr_t qsprim_char_whitespace_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsword codepoint = qschar_get(mach, arg0);
  retval = qsbool_make(mach, isspace(codepoint));
  return retval;
}

static qsptr_t qsprim_char_upper_case_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsword codepoint = qschar_get(mach, arg0);
  retval = qsbool_make(mach, isupper(codepoint));
  return retval;
}

static qsptr_t qsprim_char_lower_case_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsword codepoint = qschar_get(mach, arg0);
  retval = qsbool_make(mach, islower(codepoint));
  return retval;
}

static qsptr_t qsprim_digit_value (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsword codepoint = qschar_get(mach, arg0);
  if (isdigit(codepoint))
    {
      /* TODO: better mapping to value. */
      retval = qsint_make(mach, codepoint-'0');
    }
  return retval;
}

static qsptr_t qsprim_char_to_integer (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsword codepoint = qschar_get(mach, arg0);
  retval = qsint_make(mach, codepoint);
  return retval;
}

static qsptr_t qsprim_integer_to_char (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsword codepoint = _integer_from(mach, arg0);
  retval = qschar_make(mach, codepoint);
  return retval;
}

static qsptr_t qsprim_char_upcase (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsword codepoint = _integer_from(mach, arg0);
  retval = qschar_make(mach, toupper(codepoint));
  return retval;
}

static qsptr_t qsprim_char_downcase (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsword codepoint = _integer_from(mach, arg0);
  retval = qschar_make(mach, tolower(codepoint));
  return retval;
}

static qsptr_t qsprim_char_foldcase (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsword codepoint = _integer_from(mach, arg0);
  if (isupper(codepoint))
    {
      retval = qschar_make(mach, tolower(codepoint));
    }
  else if (islower(codepoint))
    {
      retval = qschar_make(mach, toupper(codepoint));
    }
  else
    {
      retval = arg0;
    }
  return retval;
}


static
struct prims_table_s table1_chars[] = {
      { "char=?", qsprim_char_eq_p },
      { "char<?", qsprim_char_lt_p },
      { "char>?", qsprim_char_gt_p },
      { "char-alphabetic?", qsprim_char_alphabetic_p },
      { "char-numeric?", qsprim_char_numeric_p },
      { "char-whitespace?", qsprim_char_whitespace_p },
      { "char-upper-case?", qsprim_char_upper_case_p },
      { "char-lower-case?", qsprim_char_lower_case_p },
      { "digit-value", qsprim_digit_value },
      { "char->integer", qsprim_char_to_integer },
      { "integer->char", qsprim_integer_to_char },
      { "char-upcase", qsprim_char_upcase },
      { "char-downcase", qsprim_char_downcase },
      { "char-foldcase", qsprim_char_foldcase },
      { NULL, NULL },
};





/* Primitives: Strings. */

static qsptr_t qsprim_make_string (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSERR_FAULT;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  qsword len = qsint_get(mach, arg0);
  int codepoint = qschar_get(mach, arg1);
  retval = qscharvec_make(mach, len, codepoint);
  return retval;
}

/* Strings:string to be implemented in Scheme. */

static qsptr_t qsprim_string_length (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSERR_FAULT;
  qsptr_t arg0 = CAR(args);
  qsword len = qsstring_length(mach, arg0);
  retval = qsint_make(mach, len);
  return retval;
}

static qsptr_t qsprim_string_ref (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSERR_FAULT;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  qsword k = qsint_get(mach, arg1);
  retval = qsstring_ref(mach, arg0, k);
  return retval;
}

static qsptr_t qsprim_string_setq (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSERR_FAULT;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  qsptr_t arg2 = CAR(CDR(CDR(args)));
  qsword k = qsint_get(mach, arg1);
  qsword codept = qsint_get(mach, arg2);
  retval = qsstring_setq(mach, arg0, k, codept);
  return retval;
}

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

static
struct prims_table_s table1_strings[] = {
      { "make-string", qsprim_make_string },
      { "string-length", qsprim_string_length },
      { "string-ref", qsprim_string_ref },
      { "string-set!", qsprim_string_setq },
      { NULL, NULL },
};




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
  qsword k = qsint_get(mach, arg1);
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
struct prims_table_s table1_vectors[] = {
      { "make-vector", qsprim_make_vector },
      { "vector-length", qsprim_vector_length },
      { "vector-ref", qsprim_vector_ref },
      { "vector-set!", qsprim_vector_setq },
      { NULL, NULL },
};




/* Primitives: Bytevectors. */

static int _byte_from (qsmachine_t * mach, qsptr_t p)
{
  if (qschar_p(mach, p)) return qschar_get(mach, p) && 0xff;
  if (qsint_p(mach, p)) return qsint_get(mach, p) && 0xff;
  if (qslong_p(mach, p)) return qslong_get(mach, p) && 0xff;
  return 0;
}

static qsptr_t qsprim_make_bytevector (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  qsword k = qsint_get(mach, arg0);
  qsword o = _byte_from(mach, arg1);
  retval = qsbytevec_make(mach, k, o);
  return retval;
}

/* Bytevectors:bytevector */

static qsptr_t qsprim_bytevector_length (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsword k = qsbytevec_length(mach, arg0);
  retval = qsint_make(mach, k);
  return retval;
}

static qsptr_t qsprim_bytevector_u8_ref (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  qsword k = qsint_get(mach, arg1);
  qsword o = qsbytevec_ref(mach, arg0, k);
  retval = qsint_make(mach, o);
  return retval;
}

static qsptr_t qsprim_bytevector_u8_setq (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  qsptr_t arg2 = CAR(CDR(CDR(args)));
  qsword k = qsint_get(mach, arg1);
  qsword o = _byte_from(mach, arg2);
  retval = qsbytevec_setq(mach, arg0, k, o);
  return retval; /* object itself, or error. */
}

/* Bytevectors:bytevector-copy */
/* Bytevectors:bytevector-copy! */
/* Bytevectors:bytevector-append */
/* Bytevectors:utf8->string */
/* Bytevectors:string->utf8 */

static
struct prims_table_s table1_bytevectors[] = {
      { "make-bytevector", qsprim_make_bytevector },
      { "bytevector-length", qsprim_bytevector_length },
      { "bytevector-u8-ref", qsprim_bytevector_u8_ref },
      { "bytevector-u8-set!", qsprim_bytevector_u8_setq },
      { NULL, NULL },
};




/* Primitives: Ports */

static qsptr_t qsprim_eof_object (qsmachine_t * mach, qsptr_t args)
{
  (void)mach;
  (void)args;
  return QSEOF;
}

static qsptr_t qsprim_port_read_u8 (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSEOF;
  qsptr_t arg0 = CAR(args);
  if (qsport_p(mach, arg0))
    {
      int byte = qsport_read_u8(mach, arg0);
      retval = qsint_make(mach, byte);
    }
  else
    {
      /* TODO: incompatible type. */
      retval = QSERR_FAULT;
    }
  return retval;
}

static qsptr_t qsprim_port_write_u8 (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSEOF;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  if (qsport_p(mach, arg0))
    {
      int byte = _integer_from(mach, arg1);
      bool b = qsport_write_u8(mach, arg0, byte);
      retval = qsbool_make(mach, b);
    }
  else
    {
      /* TODO: incompatible type. */
      retval = QSERR_FAULT;
    }
  return retval;
}

static qsptr_t qsprim_port_close (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSERR_FAULT;
  qsptr_t arg0 = CAR(args);
  bool b = qsport_close(mach, arg0);
  retval = qsbool_make(mach, b);
  return retval;
}

static qsptr_t qsprim_current_input_port (qsmachine_t * mach, qsptr_t args)
{
  (void)args;
  qsptr_t y_stdin = qssymbol_intern_c(mach, "*current-input-port*", 0);
  qsptr_t retval = qsenv_lookup(mach, mach->E, y_stdin);
  if (qsnil_p(mach, retval)) retval = qsfd_make(mach, 0);
  return retval;
}

static qsptr_t qsprim_current_output_port (qsmachine_t * mach, qsptr_t args)
{
  (void)args;
  qsptr_t y_stdout = qssymbol_intern_c(mach, "*current-output-port*", 0);
  qsptr_t retval = qsenv_lookup(mach, mach->E, y_stdout);
  if (qsnil_p(mach, retval)) retval = qsfd_make(mach, 1);
  return retval;
}

static qsptr_t qsprim_current_error_port (qsmachine_t * mach, qsptr_t args)
{
  (void)args;
  qsptr_t y_stderr = qssymbol_intern_c(mach, "*current-error-port*", 0);
  qsptr_t retval = qsenv_lookup(mach, mach->E, y_stderr);
  if (qsnil_p(mach, retval)) retval = qsfd_make(mach, 2);
  return retval;
}


static
struct prims_table_s table1_ports[] = {
      { "eof-object", qsprim_eof_object },
      { "port-read-u8", qsprim_port_read_u8 },
      { "port-write-u8", qsprim_port_write_u8 },
      { "port-close", qsprim_port_close },
      { "current-input-port", qsprim_current_input_port },
      { "current-output-port", qsprim_current_output_port },
      { "current-error-port", qsprim_current_error_port },
      { NULL, NULL },
};


/* Primitives, sub-section: Ports, File Descriptor. */

static qsptr_t qsprim_port_fd_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSERR_FAULT;
  qsptr_t arg0 = CAR(args);
  bool b = qsfd_p(mach, arg0);
  retval = qsbool_make(mach, b);
  return retval;
}

static qsptr_t qsprim_make_fd (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSERR_FAULT;
  qsptr_t arg0 = CAR(args);
  int fdnum = _integer_from(mach, arg0);
  retval = qsfd_make(mach, fdnum);
  return retval;
}

static qsptr_t qsprim_port_fd_open (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSERR_FAULT;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  qsptr_t arg2 = CAR(CDR(CDR(args)));
  int flags = _integer_from(mach, arg1);
  int mode = _integer_from(mach, arg2);
  retval = qsfd_open(mach, arg0, flags, mode);
  return retval;
}

static
struct prims_table_s table1_ports_fd[] = {
      { "port-fd?", qsprim_port_fd_p },
      { "make-fd", qsprim_make_fd },
      { "port-fd-open", qsprim_port_fd_open },
      { NULL, NULL },
};


/* Primitives, sub-section: Ports, Standard C File. */

static qsptr_t qsprim_port_file_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSERR_FAULT;
  qsptr_t arg0 = CAR(args);
  bool b = qsfport_p(mach, arg0);
  retval = qsbool_make(mach, b);
  return retval;
}

static qsptr_t qsprim_port_file_open (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSERR_FAULT;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  const char * path = qsutf8_get(mach, arg0, 0);
  if (! path) return retval;
  const char * mode = qsutf8_get(mach, arg1, 0);
  if (! mode) mode = "rt";

  retval = qsfport_make(mach, path, mode);
  return retval;
}

static qsptr_t qsprim_port_file_seek (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSERR_FAULT;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  size_t pos = _integer_from(mach, arg1);
  retval = qsfport_seek(mach, arg0, pos);
  return retval;
}

static qsptr_t qsprim_port_file_tell (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSERR_FAULT;
  qsptr_t arg0 = CAR(args);
  size_t pos = qsfport_tell(mach, arg0);
  retval = qsint_make(mach, pos);
  return retval;
}

static qsptr_t qsprim_port_file_eof_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSERR_FAULT;
  qsptr_t arg0 = CAR(args);
  bool b = qsfport_eof(mach, arg0);
  retval = qsbool_make(mach, b);
  return retval;
}

static
struct prims_table_s table1_ports_file[] = {
      { "port-file?", qsprim_port_file_p },
      { "port-file-open", qsprim_port_file_open },
      { "port-file_seek", qsprim_port_file_seek },
      { "port-file-tell", qsprim_port_file_tell },
      { "port-file-eof?", qsprim_port_file_eof_p },
      { NULL, NULL },
};


/* Primitives, sub-section: Memory-port */

static qsptr_t qsprim_port_mem_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSERR_FAULT;
  qsptr_t arg0 = CAR(args);
  bool b = qsovport_p(mach, arg0);
  retval = qsbool_make(mach, b);
  return retval;
}

static qsptr_t qsprim_port_mem_open (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSERR_FAULT;
  qsptr_t arg0 = CAR(args);
  retval = qsovport_make(mach, arg0);
  return retval;
}

static
struct prims_table_s table1_ports_mem[] = {
      { "port-mem?", qsprim_port_mem_p },
      { "port-mem-open", qsprim_port_mem_open },
      { NULL, NULL },
};


/* C-String-Ports are not open to primitives.
   These ports are to be created in C space and injected into Scheme.
 */




/* Primitives: Kontinuation */

static qsptr_t qsprim_kont_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  retval = qsbool_make(mach, qskont_p(mach, arg0));
  return retval;
}

static qsptr_t qsprim_make_kont (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  qsptr_t arg2 = CAR(CDR(CDR(args)));
  qsptr_t arg3 = CAR(CDR(CDR(CDR(args))));
  retval = qskont_make(mach, QSKONT_LETK, arg0, arg1, arg2, arg3);
  return retval;
}

static qsptr_t qsprim_kont_ref_v (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  retval = qskont_ref_v(mach, arg0);
  return retval;
}

static qsptr_t qsprim_kont_ref_c (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  retval = qskont_ref_c(mach, arg0);
  return retval;
}

static qsptr_t qsprim_kont_ref_e (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  retval = qskont_ref_e(mach, arg0);
  return retval;
}

static qsptr_t qsprim_kont_ref_k (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  retval = qskont_ref_k(mach, arg0);
  return retval;
}

static
struct prims_table_s table1_konts[] = {
      { "kont?", qsprim_kont_p },
      { "make-kont", qsprim_make_kont },
      { "kont-ref-v", qsprim_kont_ref_v },
      { "kont-ref-c", qsprim_kont_ref_c },
      { "kont-ref-e", qsprim_kont_ref_e },
      { "kont-ref-k", qsprim_kont_ref_k },
      { NULL, NULL },
};



enum numtower_e {
    NUMTYPE_NONE,
    NUMTYPE_INT,
    NUMTYPE_LONG,
    NUMTYPE_FLOAT,
    NUMTYPE_DOUBLE,
    NUMTYPE_RATIONAL,
    NUMTYPE_COMPLEX,
    NUMTYPE_QUAT,
    NUMTYPE_INF,
    NUMTYPE_NAN,
};

static enum numtower_e _numtype (const qsmachine_t * mach, qsptr_t x)
{
  if (qsint_p(mach, x)) return NUMTYPE_INT;
  if (qslong_p(mach, x)) return NUMTYPE_LONG;
  if (qsfloat_p(mach, x)) return NUMTYPE_FLOAT;
  if (qsdouble_p(mach, x)) return NUMTYPE_DOUBLE;
  if (qsquat_p(mach, x)) return NUMTYPE_QUAT;
  /* TODO: rational */
  /* TODO: complex */
  if (qspinf_p(mach, x) || qsninf_p(mach, x)) return NUMTYPE_INF;
  return NUMTYPE_NAN;
}

enum numtower_e _promote_numtype (const qsmachine_t * mach, qsptr_t x, qsptr_t y)
{
  enum numtower_e xtype = _numtype(mach, x);
  enum numtower_e ytype = _numtype(mach, y);

  /* promote to more complex type. */
  if (xtype < ytype) xtype = ytype;
  return xtype;
}

__attribute__((unused))
static float _float_from (const qsmachine_t * mach, qsptr_t p)
{
  if (qsint_p(mach, p)) return (float)(qsint_get(mach, p));
  if (qsfloat_p(mach, p)) return (float)qsfloat_get(mach, p);
  if (qslong_p(mach, p)) return (float)(qslong_get(mach, p));
  if (qsdouble_p(mach, p)) return (float)(qsdouble_get(mach, p));
  return QSNAN;
}

__attribute__((unused))
static long _long_from (const qsmachine_t * mach, qsptr_t p)
{
  if (qsint_p(mach, p)) return (long)(qsint_get(mach, p));
  if (qsfloat_p(mach, p)) return (long)qsfloat_get(mach, p);
  if (qslong_p(mach, p)) return (long)(qslong_get(mach, p));
  if (qsdouble_p(mach, p)) return (long)(qsdouble_get(mach, p));
  return QSNAN;
}

__attribute__((unused))
static long _double_from (const qsmachine_t * mach, qsptr_t p)
{
  if (qsint_p(mach, p)) return (double)(qsint_get(mach, p));
  if (qsfloat_p(mach, p)) return (double)qsfloat_get(mach, p);
  if (qslong_p(mach, p)) return (double)(qslong_get(mach, p));
  if (qsdouble_p(mach, p)) return (double)(qsdouble_get(mach, p));
  return QSNAN;
}

/*
Returns:
  -1 : negative magnitude
  0 : zero
  1 : positive magnitude
  -2 : magnitude inapplicable
*/
static int _sign_of (const qsmachine_t * mach, qsptr_t p)
{
  if (qsint_p(mach, p))
    {
      int i = qsint_get(mach, p);
      return (i < 0) ? -1 : (i > 0) ? 1 : 0;
    }
  if (qsfloat_p(mach, p))
    {
      float f = qsfloat_get(mach, p);
      return (f < 0) ? -1 : (f > 0) ? 1 : 0;
    }
  if (qslong_p(mach, p))
    {
      float l = qslong_get(mach, p);
      return (l < 0) ? -1 : (l > 0) ? 1 : 0;
    }
  if (qsdouble_p(mach, p))
    {
      double d = qsdouble_get(mach, p);
      return (d < 0) ? -1 : (d > 0) ? 1 : 0;
    }
  if (qspinf_p(mach, p))
    {
      return 1;
    }
  if (qsninf_p(mach, p))
    {
      return -1;
    }
  return -2;
}

typedef struct alu_s {
    enum numtower_e xtype, ytype, ztype;
    struct alureg_s {
	int32_t i;
	int64_t l;
	float f;
	double d;
	float q[4];
    } x, y, z;
} alu_t;

static int alu_load_reg (enum numtower_e * out_type, struct alureg_s * out_reg,
			 const qsmachine_t * mach, qsptr_t p)
{
  enum numtower_e numtype = NUMTYPE_NAN;
  struct alureg_s reg;
  if (qsint_p(mach, p))
    {
      numtype = NUMTYPE_INT;
      reg.i = qsint_get(mach, p);
    }
  else if (qslong_p(mach, p))
    {
      numtype = NUMTYPE_LONG;
      reg.l = qslong_get(mach, p);
    }
  else if (qsfloat_p(mach, p))
    {
      numtype = NUMTYPE_FLOAT;
      reg.f = qsfloat_get(mach, p);
    }
  else if (qsdouble_p(mach, p))
    {
      numtype = NUMTYPE_DOUBLE;
      reg.d = qsdouble_get(mach, p);
    }
  else if (qsquat_p(mach, p))
    {
      numtype = NUMTYPE_QUAT;
      qsquat_fetch(mach, p, reg.q);
    }
  else if (qspinf_p(mach, p))
    {
      numtype = NUMTYPE_INF;
      reg.i = +1;
    }
  else if (qsninf_p(mach, p))
    {
      numtype = NUMTYPE_INF;
      reg.i = -1;
    }
  else if (qspnan_p(mach, p) || qsnnan_p(mach, p))
    {
      numtype = NUMTYPE_NAN;
      reg.i = 0;
    }
  if (out_type) *out_type = numtype;
  if (out_reg) *out_reg = reg;
  return 0;
}

__attribute__((unused))
static int alu_load_x (alu_t * a, qsmachine_t * mach, qsptr_t p)
{
  alu_load_reg(&(a->xtype), &(a->x), mach, p);
  return p;
}

__attribute__((unused))
static int alu_load_y (alu_t * a, qsmachine_t * mach, qsptr_t p)
{
  alu_load_reg(&(a->ytype), &(a->y), mach, p);
  return p;
}

__attribute__((unused))
static int alu_load_z (alu_t * a, qsmachine_t * mach, qsptr_t p)
{
  alu_load_reg(&(a->ztype), &(a->z), mach, p);
  return p;
}

static qsptr_t alu_store_reg (qsmachine_t * mach,
			      enum numtower_e numtype,
			      const struct alureg_s * reg)
{
  qsptr_t retval = QSNIL;
  switch (numtype)
    {
    case NUMTYPE_INT:
      retval = qsint_make(mach, reg->i);
      break;
    case NUMTYPE_LONG:
      retval = qslong_make(mach, reg->l);
      break;
    case NUMTYPE_FLOAT:
      retval = qsfloat_make(mach, reg->f);
      break;
    case NUMTYPE_DOUBLE:
      retval = qsdouble_make(mach, reg->d);
      break;
    case NUMTYPE_QUAT:
      retval = qsquat_make_f4(mach, reg->q);
      break;
    case NUMTYPE_INF:
      if (reg->i < 0)
	{
	  retval = qsninf_make(mach);
	}
      else
	{
	  retval = qspinf_make(mach);
	}
      break;
    case NUMTYPE_NAN:
    default:
      retval = qspnan_make(mach);
      break;
    }
  return retval;
}

__attribute__((unused))
static qsptr_t alu_store_x (alu_t * a, qsmachine_t * mach)
{
  return alu_store_reg(mach, a->xtype, &(a->x));
}

__attribute__((unused))
static qsptr_t alu_store_y (alu_t * a, qsmachine_t * mach)
{
  return alu_store_reg(mach, a->ytype, &(a->y));
}

__attribute__((unused))
static qsptr_t alu_store_z (alu_t * a, qsmachine_t * mach)
{
  return alu_store_reg(mach, a->ztype, &(a->z));
}

static int alu_cast_reg (enum numtower_e regtype, struct alureg_s * reg,
			 enum numtower_e casttype)
{
  switch (regtype)
    {
    case NUMTYPE_INT:
      switch (casttype)
	{
	case NUMTYPE_INT: /* cast int30 to int30 */
	  /* change nothing. */
	  break;
	case NUMTYPE_LONG: /* cast int30 to long */
	  reg->l = (long)(reg->i);
	  break;
	case NUMTYPE_FLOAT: /* cast int30 to float */
	  reg->f = (float)(reg->i);
	  break;
	case NUMTYPE_DOUBLE: /* cast int30 to double */
	  reg->d = (double)(reg->i);
	  break;
	case NUMTYPE_QUAT: /* cast int30 to quat */
	  reg->q[0] = reg->i;
	  reg->q[1] = 0;
	  reg->q[2] = 0;
	  reg->q[3] = 0;
	  break;
	case NUMTYPE_INF: /* cast int30 to inf; preserve sign. */
	  if (reg->i < 0)
	    reg->i = -1;
	  else
	    reg->i = 1;
	  break;
	case NUMTYPE_NAN: /* cast int30 to NaN */
	  reg->i = 0;
	default:
	  break;
	}
      break;
    case NUMTYPE_LONG:
      switch (casttype)
	{
	case NUMTYPE_INT: /* cast long to int30 */
	  reg->i = (int)(reg->l);
	  break;
	case NUMTYPE_LONG: /* cast long to long */
	  /* change nothing. */
	  break;
	case NUMTYPE_FLOAT: /* cast long to float */
	  reg->f = (float)(reg->l);
	  break;
	case NUMTYPE_DOUBLE: /* cast long to double */
	  reg->d = (double)(reg->l);
	  break;
	case NUMTYPE_QUAT: /* cast long to quat */
	  reg->q[0] = (float)(reg->l);
	  reg->q[1] = 0;
	  reg->q[2] = 0;
	  reg->q[3] = 0;
	  break;
	case NUMTYPE_INF: /* cast long to inf; preserve sign. */
	  if (reg->l < 0)
	    reg->i = -1;
	  else
	    reg->i = 1;
	  break;
	case NUMTYPE_NAN: /* cast long to NaN */
	  reg->i = 0;
	default:
	  break;
	}
      break;
    case NUMTYPE_FLOAT:
      switch (casttype)
	{
	case NUMTYPE_INT: /* cast float31 to int30 */
	  reg->i = (int)(reg->f);
	  break;
	case NUMTYPE_LONG: /* cast float31 to long */
	  reg->l = (long)(reg->f);
	  break;
	case NUMTYPE_FLOAT: /* cast float31 to float31 */
	  /* change nothing. */
	  break;
	case NUMTYPE_DOUBLE: /* cast float31 to double */
	  reg->d = (double)(reg->f);
	  break;
	case NUMTYPE_QUAT: /* cast float31 to quat */
	  reg->q[0] = reg->f;
	  reg->q[1] = 0;
	  reg->q[2] = 0;
	  reg->q[3] = 0;
	  break;
	case NUMTYPE_INF: /* cast to infinity */
	  if (reg->f < 0)
	    reg->i = -1;
	  else
	    reg->i = 1;
	  break;
	case NUMTYPE_NAN: /* cast to NaN */
	  reg->i = 0;
	default:
	  break;
	}
      break;
    case NUMTYPE_DOUBLE:
      switch (casttype)
	{
	case NUMTYPE_INT: /* cast double to int30 */
	  reg->i = (int)(reg->d);
	  break;
	case NUMTYPE_LONG: /* cast double to long */
	  reg->l = (long)(reg->d);
	  break;
	case NUMTYPE_FLOAT: /* cast double to float */
	  reg->f = (float)(reg->d);
	  break;
	case NUMTYPE_DOUBLE: /* cast double to double */
	  /* change nothing. */
	  break;
	case NUMTYPE_QUAT: /* cast double to quat */
	  reg->q[0] = (float)(reg->d);
	  reg->q[1] = 0;
	  reg->q[2] = 0;
	  reg->q[3] = 0;
	  break;
	case NUMTYPE_INF: /* cast to infinity */
	  if (reg->d < 0)
	    reg->i = -1;
	  else
	    reg->i = 1;
	  break;
	case NUMTYPE_NAN: /* cast to NaN */
	  reg->i = 0;
	default:
	  break;
	}
      break;
    case NUMTYPE_QUAT:
      switch (casttype)
	{
	case NUMTYPE_INT: /* cast quat to int30 */
	  reg->i = (int)(reg->q[0]);
	  break;
	case NUMTYPE_LONG: /* cast quat to long */
	  reg->l = (long)(reg->q[0]);
	  break;
	case NUMTYPE_FLOAT: /* cast quat to float31 */
	  reg->f = (float)(reg->q[0]);
	  break;
	case NUMTYPE_DOUBLE: /* cast quat to double */
	  reg->d = (double)(reg->q[0]);
	  break;
	case NUMTYPE_QUAT: /* cast quat to quat */
	  /* change nothing. */
	  break;
	case NUMTYPE_INF: /* cast to infinity */
	  if (reg->q[0] < 0)
	    reg->i = -1;
	  else
	    reg->i = 1;
	  break;
	case NUMTYPE_NAN: /* cast to NaN */
	  reg->i = 0;
	default:
	  break;
	}
      break;
    case NUMTYPE_INF:
    case NUMTYPE_NAN:
      switch (casttype)
	{
	case NUMTYPE_INT: /* cast NaN to int30 */
	  reg->i = 0;
	  break;
	case NUMTYPE_LONG: /* cast NaN to long */
	  reg->l = 0;
	  break;
	case NUMTYPE_FLOAT: /* cast NaN to float31 */
	  reg->f = 0;
	  break;
	case NUMTYPE_DOUBLE: /* cast NaN to double */
	  reg->d = 0;
	  break;
	case NUMTYPE_QUAT: /* cast NaN to quat */
	  reg->q[0] = 0;
	  reg->q[1] = 0;
	  reg->q[2] = 0;
	  reg->q[3] = 0;
	  break;
	case NUMTYPE_INF: /* cast NaN to infinity */
	  reg->i = 0;
	  break;
	case NUMTYPE_NAN: /* cast NaN to NaN */
	default:
	  break;
	}
      break;
    default:
      break;
    }
  return 0;
}

__attribute__((unused))
static int alu_cast_x (alu_t * a, enum numtower_e casttype)
{
  alu_cast_reg(a->xtype, &(a->x), casttype);
  return 0;
}

__attribute__((unused))
static int alu_cast_y (alu_t * a, enum numtower_e casttype)
{
  alu_cast_reg(a->ytype, &(a->y), casttype);
  return 0;
}

__attribute__((unused))
static int alu_cast_z (alu_t * a, enum numtower_e casttype)
{
  alu_cast_reg(a->ztype, &(a->z), casttype);
  return 0;
}


/* Call one-argument libm: a.z = func(a.x), 'func' depends on a.ztype */
static enum numtower_e _delegate_libm1 (alu_t * a,
					enum numtower_e preferred_type,
					float (*funcf)(float),
					double (*funcd)(double),
					float complex (*funcc)(float complex))
{
  if (preferred_type != NUMTYPE_NONE)
    a->ztype = preferred_type;
  switch (a->ztype)
    {
    case NUMTYPE_INT:
      if (funcf)  a->z.f = funcf(a->x.i);
      break;
    case NUMTYPE_LONG:
      if (funcf)  a->z.f = funcf(a->x.l);
      break;
    case NUMTYPE_FLOAT:
      if (funcf)  a->z.f = funcf(a->x.f);
      break;
    case NUMTYPE_DOUBLE:
      if (funcd)  a->z.d = funcd(a->x.d);
      break;
    default:
      (void)funcc;
      a->ztype = NUMTYPE_NAN;
      break;
    }
  return a->ztype;
}

/* Call two-argument libm: a.z = func(a.x, a.y), 'func' depends on a.ztype */
static enum numtower_e _delegate_libm2 (alu_t * a,
					enum numtower_e preferred_type,
					float (*funcf)(float, float),
					double (*funcd)(double, double),
					float complex (*funcc)(float complex, float complex))
{
  if (preferred_type != NUMTYPE_NONE)
    a->ztype = preferred_type;
  switch (a->ztype)
    {
    case NUMTYPE_INT:
      if (funcf)  a->z.f = funcf(a->x.i, a->y.i);
      break;
    case NUMTYPE_LONG:
      if (funcf)  a->z.f = funcf(a->x.l, a->y.l);
      break;
    case NUMTYPE_FLOAT:
      if (funcf)  a->z.f = funcf(a->x.f, a->y.f);
      break;
    case NUMTYPE_DOUBLE:
      if (funcd)  a->z.d = funcd(a->x.d, a->y.d);
      break;
    default:
      (void)funcc;
      a->ztype = NUMTYPE_NAN;
      break;
    }
  return a->ztype;
}


/* Mathematics: numeric predicates. */

/* predicates for numeric bits format:
   float31 (direct encoding)
   int30   (direct encoding)
   long    (widenum long)
   double  (widenum double)
   ipair   (widenum, int32 pair - rational)
   fpair   (widenum, float pair - complex)
   quat    (octvec, quad float - quaternion)
 */
static qsptr_t qsprim_numbits_int30_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t arg0 = CAR(args);
  bool b = qsint_p(mach, arg0);
  qsptr_t retval = qsbool_make(mach, b);
  return retval;
}

static qsptr_t qsprim_numbits_float31_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t arg0 = CAR(args);
  bool b = qsfloat_p(mach, arg0);
  qsptr_t retval = qsbool_make(mach, b);
  return retval;
}

static qsptr_t qsprim_numbits_long_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t arg0 = CAR(args);
  bool b = qslong_p(mach, arg0);
  qsptr_t retval = qsbool_make(mach, b);
  return retval;
}

static qsptr_t qsprim_numbits_double_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t arg0 = CAR(args);
  bool b = qsdouble_p(mach, arg0);
  qsptr_t retval = qsbool_make(mach, b);
  return retval;
}

__attribute__((unused))
static qsptr_t qsprim_numbits_ipair_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t arg0 = CAR(args);
  bool b = false;
  /* TODO */
  (void)arg0;
  qsptr_t retval = qsbool_make(mach, b);
  return retval;
}

__attribute__((unused))
static qsptr_t qsprim_numbits_fpair_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t arg0 = CAR(args);
  /* TODO */
  (void)arg0;
  bool b = false;
  qsptr_t retval = qsbool_make(mach, b);
  return retval;
}

__attribute__((unused))
static qsptr_t qsprim_numbits_quat_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t arg0 = CAR(args);
  /* TODO */
  (void)arg0;
  bool b = qsquat_p(mach, arg0);
  qsptr_t retval = qsbool_make(mach, b);
  return retval;
}

static qsptr_t qsprim_numbits_cast_int30 (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  switch (_numtype(mach, arg0))
    {
    case NUMTYPE_INT:
      retval = arg0;
      break;
    case NUMTYPE_LONG:
      retval = qsint_make(mach, qslong_get(mach, arg0));
      break;
    case NUMTYPE_FLOAT:
      retval = qsint_make(mach, qsfloat_get(mach, arg0));
      break;
    case NUMTYPE_DOUBLE:
      retval = qsint_make(mach, qsdouble_get(mach, arg0));;
      break;
    case NUMTYPE_QUAT:
      retval = qsint_make(mach, qsquat_ref(mach, arg0, 0));
      break;
    default:
      retval = QSNAN;
      break;
    }
  return retval;
}

static qsptr_t qsprim_numbits_cast_float31 (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  switch (_numtype(mach, arg0))
    {
    case NUMTYPE_INT:
      retval = qsfloat_make(mach, qsint_get(mach, arg0));
      retval = arg0;
      break;
    case NUMTYPE_LONG:
      retval = qsfloat_make(mach, qslong_get(mach, arg0));
      break;
    case NUMTYPE_FLOAT:
      retval = arg0;
      break;
    case NUMTYPE_DOUBLE:
      retval = qsfloat_make(mach, qsdouble_get(mach, arg0));;
      break;
    case NUMTYPE_QUAT:
      retval = qsfloat_make(mach, qsquat_ref(mach, arg0, 0));
      break;
    default:
      retval = QSNAN;
      break;
    }
  return retval;
}

static qsptr_t qsprim_numbits_cast_long (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  switch (_numtype(mach, arg0))
    {
    case NUMTYPE_INT:
      retval = qslong_make(mach, qsint_get(mach, arg0));
      break;
    case NUMTYPE_LONG:
      retval = arg0;
      break;
    case NUMTYPE_FLOAT:
      retval = qslong_make(mach, qsfloat_get(mach, arg0));
      break;
    case NUMTYPE_DOUBLE:
      retval = qslong_make(mach, qsdouble_get(mach, arg0));;
      break;
    case NUMTYPE_QUAT:
      retval = qslong_make(mach, qsquat_ref(mach, arg0, 0));
      break;
    default:
      retval = QSNAN;
      break;
    }
  return retval;
}

static qsptr_t qsprim_numbits_cast_double (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  switch (_numtype(mach, arg0))
    {
    case NUMTYPE_INT:
      retval = qsdouble_make(mach, qsint_get(mach, arg0));
      break;
    case NUMTYPE_LONG:
      retval = qsdouble_make(mach, qslong_get(mach, arg0));
      break;
    case NUMTYPE_FLOAT:
      retval = qsdouble_make(mach, qsfloat_get(mach, arg0));
      break;
    case NUMTYPE_DOUBLE:
      retval = arg0;
      break;
    case NUMTYPE_QUAT:
      retval = qsdouble_make(mach, qsquat_ref(mach, arg0, 0));
      break;
    default:
      retval = QSNAN;
      break;
    }
  return retval;
}

__attribute__((unused))
static qsptr_t qsprim_numbits_cast_ipair (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  switch (_numtype(mach, arg0))
    {
    case NUMTYPE_INT:
      retval = QSNAN;
      break;
    case NUMTYPE_LONG:
      retval = QSNAN;
      break;
    case NUMTYPE_FLOAT:
      retval = QSNAN;
      break;
    case NUMTYPE_DOUBLE:
      retval = QSNAN;
      break;
    case NUMTYPE_QUAT:
      retval = QSNAN;
      break;
    default:
      retval = QSNAN;
      break;
    }
  return retval;
}

__attribute__((unused))
static qsptr_t qsprim_numbits_cast_fpair (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  switch (_numtype(mach, arg0))
    {
    case NUMTYPE_INT:
      retval = QSNAN;
      break;
    case NUMTYPE_LONG:
      retval = QSNAN;
      break;
    case NUMTYPE_FLOAT:
      retval = QSNAN;
      break;
    case NUMTYPE_DOUBLE:
      retval = QSNAN;
      break;
    case NUMTYPE_QUAT:
      retval = QSNAN;
      break;
    default:
      retval = QSNAN;
      break;
    }
  return retval;
}

__attribute__((unused))
static qsptr_t qsprim_numbits_cast_quat (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  switch (_numtype(mach, arg0))
    {
    case NUMTYPE_INT:
      retval = qsquat_make(mach, qsint_get(mach, arg0), 0, 0, 0);
      break;
    case NUMTYPE_LONG:
      retval = qsquat_make(mach, qslong_get(mach, arg0), 0, 0, 0);
      break;
    case NUMTYPE_FLOAT:
      retval = qsquat_make(mach, qsfloat_get(mach, arg0), 0, 0, 0);
      break;
    case NUMTYPE_DOUBLE:
      retval = qsquat_make(mach, qsdouble_get(mach, arg0), 0, 0, 0);
      break;
    case NUMTYPE_QUAT:
      retval = arg0;
      break;
    default:
      retval = QSNAN;
      break;
    }
  return retval;
}

static qsptr_t qsprim_make_inf (qsmachine_t * mach, qsptr_t args)
{
  (void)args;
  qsptr_t retval = qspinf_make(mach);
  return retval;
}

static qsptr_t qsprim_make_ninf (qsmachine_t * mach, qsptr_t args)
{
  (void)args;
  qsptr_t retval = qsninf_make(mach);
  return retval;
}

static qsptr_t qsprim_make_nan (qsmachine_t * mach, qsptr_t args)
{
  (void)args;
  qsptr_t retval = qspnan_make(mach);
  return retval;
}

/* Mathematics:complex? */
/* Mathematics:real? */
/* Mathematics:rational? */
/* Mathematics:integer? */
/* Mathematics:exact? */
/* Mathematics:inexact? */
/* Mathematics:exact-integer? */

static qsptr_t qsprim_finite_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (_numtype(mach, arg0) != NUMTYPE_INF) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_infinite_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (_numtype(mach, arg0) == NUMTYPE_INF) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_nan_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (_numtype(mach, arg0) == NUMTYPE_NAN) retval = QSTRUE;
  return retval;
}

/* Mathematics:= */
/* Mathematics:< */
/* Mathematics:> */
/* Mathematics:<= */
/* Mathematics:>= */

/* Generic comparator: (cmp x y)
   Returns -1 if x < y
   Returns 0 if x == y
   Returns 1 if x > y
   Returns -2 if comparison impossible
 */
static qsptr_t qsprim_cmp2 (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSERR_FAULT;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  alu_t a;
  a.ztype = NUMTYPE_INT;
  switch (_promote_numtype(mach, arg0, arg1))
    {
    case NUMTYPE_INT:
      a.x.i = _integer_from(mach, arg0);
      a.y.i = _integer_from(mach, arg1);
      a.z.i = (a.x.i < a.y.i) ? -1 : (a.x.i > a.y.i) ? 1 : 0;
      break;
    case NUMTYPE_FLOAT:
      a.x.f = _integer_from(mach, arg0);
      a.y.f = _integer_from(mach, arg1);
      a.z.i = (a.x.f < a.y.f) ? -1 : (a.x.f > a.y.f) ? 1 : 0;
      break;
    case NUMTYPE_LONG:
      a.x.l = _integer_from(mach, arg0);
      a.y.l = _integer_from(mach, arg1);
      a.z.i = (a.x.l < a.y.l) ? -1 : (a.x.l > a.y.l) ? 1 : 0;
      break;
    case NUMTYPE_DOUBLE:
      a.x.d = _integer_from(mach, arg0);
      a.y.d = _integer_from(mach, arg1);
      a.z.i = (a.x.d < a.y.d) ? -1 : (a.x.d > a.y.d) ? 1 : 0;
      break;
    /* TODO: rational */
    case NUMTYPE_INF:
      a.x.i = _sign_of(mach, arg0);
      a.y.i = _sign_of(mach, arg1);
      if ((a.x.i == -2) || (a.y.i == -2)) return -2;  /* nope */
      else if (a.x.i < a.y.i) a.z.i = -1;  /* - <=> 0, - <=> +, 0 <=> + */
      else if (a.x.i > a.y.i) a.z.i = 1;  /* + <=> 0, 0 <=> -, + <=> - */
      else
	{
	  if (qspinf_p(mach, arg0))
	    {
	      if (qspinf_p(mach, arg1)) a.z.i = -2;  /* +inf <=> +inf = nope */
	      else a.z.i = +1;  /* +inf <=> anything else = +1 */
	    }
	  else if (qsninf_p(mach, arg0))
	    {
	      if (qsninf_p(mach, arg1)) a.z.i = -2;  /* -inf <=> -inf = nope */
	      else a.z.i = -1;  /* -inf <=> anything else = -1 */
	    }
	  else if (qspinf_p(mach, arg1)) a.z.i = -1;  /* fin <=> +inf = -1 */
	  else if (qsninf_p(mach, arg1)) a.z.i = +1;  /* fin <=> -inf = +1 */
	}
      break;
    /* TODO: complex. */
    case NUMTYPE_NAN:
    default:
      a.z.i = -2;  /* nope */
      break;
    }
  retval = alu_store_z(&a, mach);
  return retval;
}

static qsptr_t qsprim_zero_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t arg0 = CAR(args);
  bool b = (_sign_of(mach, arg0) == 0);
  qsptr_t retval = qsbool_make(mach, b);
  return retval;
}

static qsptr_t qsprim_positive_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t arg0 = CAR(args);
  bool b = (_sign_of(mach, arg0) > 0);
  qsptr_t retval = qsbool_make(mach, b);
  return retval;
}

static qsptr_t qsprim_negative_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t arg0 = CAR(args);
  bool b = (_sign_of(mach, arg0) < 0);
  qsptr_t retval = qsbool_make(mach, b);
  return retval;
}

/* Mathematics:odd? */
/* Mathematics:even? */
/* Mathematics:max */
/* Mathematics:min */


static
struct prims_table_s table1_numtypes[] = {
      { "numbits-int30?", qsprim_numbits_int30_p },
      { "numbits-float31?", qsprim_numbits_float31_p },
      { "numbits-long?", qsprim_numbits_long_p },
      { "numbits-double?", qsprim_numbits_double_p },
      { "numbits-ipair?", qsprim_numbits_ipair_p },
      { "numbits-fpair?", qsprim_numbits_fpair_p },

      { "numbits-cast-int30", qsprim_numbits_cast_int30 },
      { "numbits-cast-float31", qsprim_numbits_cast_float31 },
      { "numbits-cast-long", qsprim_numbits_cast_long },
      { "numbits-cast-double", qsprim_numbits_cast_double },
      { "numbits-cast-ipair", qsprim_numbits_cast_ipair },
      { "numbits-cast-fpair", qsprim_numbits_cast_fpair },

      { "make-inf", qsprim_make_inf },
      { "make-ninf", qsprim_make_ninf },
      { "make-nan", qsprim_make_nan },

      { "finite?", qsprim_finite_p },
      { "infinite?", qsprim_infinite_p },
      { "nan?", qsprim_nan_p },
      { "<=>", qsprim_cmp2 },
      { "zero?", qsprim_zero_p },
      { "positive?", qsprim_positive_p },
      { "negative?", qsprim_negative_p },
      { NULL, NULL },
};


/* Mathematics: Arithmetic. */

static qsptr_t qsprim_add2 (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = qsint_make(mach, 0);
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  alu_t a;

  enum numtower_e promoted_type = _promote_numtype(mach, arg0, arg1);
  alu_load_x(&a, mach, arg0);
  alu_load_y(&a, mach, arg1);
  alu_cast_x(&a, promoted_type);
  alu_cast_y(&a, promoted_type);

  switch (promoted_type)
    {
    case NUMTYPE_INT:
      a.z.i = a.x.i + a.y.i;
      retval = qsint_make(mach, a.z.i);
      break;
    case NUMTYPE_FLOAT:
      a.z.f = a.x.f + a.y.f;
      retval = qsfloat_make(mach, a.z.f);
      break;
    case NUMTYPE_LONG:
      a.z.l = a.x.l + a.y.l;
      retval = qslong_make(mach, a.z.l);
      break;
    case NUMTYPE_DOUBLE:
      a.z.d = a.x.d + a.y.d;
      retval = qsdouble_make(mach, a.z.d);
      break;
    case NUMTYPE_QUAT:
	{
	  int i;
	  for (i = 0; i < 4; ++i)
	    {
	      a.z.q[i] = a.x.q[i] + a.y.q[i];
	    }
	  retval = qsquat_make_f4(mach, a.z.q);
	}
      break;
    case NUMTYPE_INF:
      if (qspinf_p(mach, arg0))
	{
	  if (qsninf_p(mach, arg1)) retval = QSNAN; /* +inf + -inf = NaN */
	  else retval = arg0; /* +inf + fin = +inf */
	}
      else if (qsninf_p(mach, arg0))
	{
	  if (qspinf_p(mach, arg1)) retval = QSNAN; /* -inf + +inf = NaN */
	  else retval = arg0; /* -inf + fin = -inf */
	}
      else
	{
	  /* y is +inf or -inf */
	  retval = arg1;
	}
      break;
    case NUMTYPE_NAN:
    default:
      retval = QSNAN;
      break;
    }
  return retval;
}

static qsptr_t qsprim_sub2 (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = qsint_make(mach, 0);
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  alu_t a;

  enum numtower_e promoted_type = _promote_numtype(mach, arg0, arg1);
  alu_load_x(&a, mach, arg0);
  alu_load_y(&a, mach, arg1);
  alu_cast_x(&a, promoted_type);
  alu_cast_y(&a, promoted_type);

  switch (promoted_type)
    {
    case NUMTYPE_INT:
      a.z.i = a.x.i - a.y.i;
      retval = qsint_make(mach, a.z.i);
      break;
    case NUMTYPE_FLOAT:
      a.z.f = a.x.f - a.y.f;
      retval = qsfloat_make(mach, a.z.f);
      break;
    case NUMTYPE_LONG:
      a.z.l = a.x.l - a.y.l;
      retval = qslong_make(mach, a.z.l);
      break;
    case NUMTYPE_DOUBLE:
      a.z.d = a.x.d - a.y.d;
      retval = qsdouble_make(mach, a.z.d);
      break;
    case NUMTYPE_QUAT:
	{
	  int i;
	  for (i = 0; i < 4; ++i)
	    {
	      a.z.q[i] = a.x.q[i] - a.y.q[i];
	    }
	  retval = qsquat_make_f4(mach, a.z.q);
	}
      break;
    case NUMTYPE_INF:
      if (qspinf_p(mach, arg0))
	{
	  if (qspinf_p(mach, arg1)) retval = QSNAN; /* +inf - +inf = NaN */
	  else retval = arg0; /* +inf - any = +inf */
	}
      else if (qsninf_p(mach, arg0))
	{
	  if (qsninf_p(mach, arg1)) retval = QSNAN; /* -inf - -inf = NaN */
	  else retval = arg0; /* -inf - any = -inf */
	}
      else if (qspinf_p(mach, arg1)) retval = QSNINF;  /* fin - +inf = -inf */
      else if (qsninf_p(mach, arg1)) retval = QSINF;  /* fin - -inf = +inf */
      break;
    case NUMTYPE_NAN:
    default:
      retval = QSNAN;
      break;
    }
  return retval;
}

static qsptr_t qsprim_mul2 (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = qsint_make(mach, 0);
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  alu_t a;

  enum numtower_e promoted_type = _promote_numtype(mach, arg0, arg1);
  alu_load_x(&a, mach, arg0);
  alu_load_y(&a, mach, arg1);
  alu_cast_x(&a, promoted_type);
  alu_cast_y(&a, promoted_type);

  switch (promoted_type)
    {
    case NUMTYPE_INT:
      a.z.i = a.x.i * a.y.i;
      retval = qsint_make(mach, a.z.i);
      break;
    case NUMTYPE_FLOAT:
      a.z.f = a.x.f * a.y.f;
      retval = qsfloat_make(mach, a.z.f);
      break;
    case NUMTYPE_LONG:
      a.z.l = a.x.l * a.y.l;
      retval = qslong_make(mach, a.z.l);
      break;
    case NUMTYPE_DOUBLE:
      a.z.d = a.x.d * a.y.d;
      retval = qsdouble_make(mach, a.z.d);
      break;
    case NUMTYPE_QUAT:
	{
	  float a1, b1, c1, d1;
	  float a2, b2, c2, d2;
	  a1 = a.x.q[0]; b1 = a.x.q[1]; c1 = a.x.q[2]; d1 = a.x.q[3];
	  a2 = a.y.q[0]; b2 = a.y.q[1]; c2 = a.y.q[2]; d2 = a.y.q[3];
	  a.z.q[0] = a1*a2 - b1*b2 - c1*c2 - d1*d2;
	  a.z.q[1] = a1*b2 + b1*a2 + c1*d2 - d1*c2;
	  a.z.q[2] = a1*c2 - b1*d2 + c1*a2 + d1*b2;
	  a.z.q[3] = a1*d2 + b1*c2 - c1*b2 + d1*a2;
	  retval = qsquat_make_f4(mach, a.z.q);
	}
      break;
    case NUMTYPE_INF:
      if (qspinf_p(mach, arg0))
	{
	  if (qspinf_p(mach, arg1)) retval = QSNAN; /* +inf * +inf = NaN */
	  else
	    {
	      int sign = _sign_of(mach, arg1);
	      switch (sign)
		{
		case -1: retval = QSNINF; break; /* +inf * -fin = -inf */
		case 1: retval = arg0; break; /* +inf * +fin = +inf */
		case 0: default: retval = QSNAN; break; /* +inf * 0 = NaN */
		}
	    }
	}
      else if (qsninf_p(mach, arg0))
	{
	  if (qsninf_p(mach, arg1)) retval = QSNAN; /* -inf * -inf = NaN */
	  else
	    {
	      int sign = _sign_of(mach, arg1);
	      switch (sign)
		{
		case -1: retval = QSINF; break; /* -inf * -fin = +inf */
		case 1: retval = arg0; break; /* -inf * +fin = -inf */
		case 0: default: retval = QSNAN; break; /* -inf * 0 = NaN */
		}
	    }
	}
      else if (qspinf_p(mach, arg1))
	{
	  int sign = _sign_of(mach, arg0);
	  switch (sign)
	    {
	    case -1: retval = QSNINF; break; /* -fin * +inf = -inf */
	    case 1: retval = arg1; break; /* +fin * +inf = +inf */
	    case 0: default: retval = QSNAN; break; /* 0 * +inf = NaN */
	    }
	}
      else if (qsninf_p(mach, arg1))
	{
	  int sign = _sign_of(mach, arg0);
	  switch (sign)
	    {
	    case -1: retval = QSINF; break; /* -fin * -inf = +inf */
	    case 1: retval = arg1; break; /* +fin * -inf = -inf */
	    case 0: default: retval = QSNAN; break; /* 0 * -inf = NaN */
	    }
	}
      break;
    case NUMTYPE_NAN:
    default:
      retval = QSNAN;
      break;
    }
  return retval;
}

static qsptr_t qsprim_div2 (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = qsint_make(mach, 0);
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  alu_t a;

  enum numtower_e promoted_type = _promote_numtype(mach, arg0, arg1);
  alu_load_x(&a, mach, arg0);
  alu_load_y(&a, mach, arg1);
  alu_cast_x(&a, promoted_type);
  alu_cast_y(&a, promoted_type);

  switch (promoted_type)
    {
    case NUMTYPE_INT:
      /* TODO: cast to rational. */
      a.z.i = a.x.i / a.y.i;
      retval = qsint_make(mach, a.z.i);
      break;
    case NUMTYPE_FLOAT:
      a.z.f = a.x.f / a.y.f;
      retval = qsfloat_make(mach, a.z.f);
      break;
    case NUMTYPE_LONG:
      /* TODO: cast to rational. */
      a.z.l = a.x.l / a.y.l;
      retval = qslong_make(mach, a.z.l);
      break;
    case NUMTYPE_DOUBLE:
      a.z.d = a.x.d / a.y.d;
      retval = qsdouble_make(mach, a.z.d);
      break;
    case NUMTYPE_QUAT:
	{
	  float a1, b1, c1, d1;
	  float a2, b2, c2, d2;
	  /* Get inverse of y. */
	  a1 = a.y.q[0], b1 = a.y.q[1], c1 = a.y.q[2], d1 = a.y.q[3];
	  float denom = (a1*a1 + b1*b1 + c1*c1 + d1*d1);
	  a.y.q[0] /= denom;
	  a.y.q[1] /= -denom;
	  a.y.q[2] /= -denom;
	  a.y.q[3] /= -denom;
	  /* Then multiply. */
	  a1 = a.x.q[0]; b1 = a.x.q[1]; c1 = a.x.q[2]; d1 = a.x.q[3];
	  a2 = a.y.q[0]; b2 = a.y.q[1]; c2 = a.y.q[2]; d2 = a.y.q[3];
	  a.z.q[0] = a1*a2 - b1*b2 - c1*c2 - d1*d2;
	  a.z.q[1] = a1*b2 + b1*a2 + c1*d2 - d1*c2;
	  a.z.q[2] = a1*c2 - b1*d2 + c1*a2 + d1*b2;
	  a.z.q[3] = a1*d2 + b1*c2 - c1*b2 + d1*a2;
	  retval = qsquat_make_f4(mach, a.z.q);
	}
      break;
    case NUMTYPE_INF:
      if (qspinf_p(mach, arg0))
	{
	  if (qspinf_p(mach, arg1)) retval = QSNAN; /* +inf / +inf = NaN */
	  else
	    {
	      int sign = _sign_of(mach, arg1);
	      switch (sign)
		{
		case -1: retval = QSNINF; break; /* +inf / -fin = -inf */
		case 1: retval = arg0; break; /* +inf / +fin = +inf */
		case 0: default: retval = QSNAN; break; /* +inf / 0 = NaN */
		}
	    }
	}
      else if (qsninf_p(mach, arg0))
	{
	  if (qsninf_p(mach, arg1)) retval = QSNAN; /* -inf / -inf = NaN */
	  else
	    {
	      int sign = _sign_of(mach, arg1);
	      switch (sign)
		{
		case -1: retval = QSINF; break; /* -inf / -fin = +inf */
		case 1: retval = arg0; break; /* -inf / +fin = -inf */
		case 0: default: retval = QSNAN; break; /* -inf / 0 = NaN */
		}
	    }
	}
      else
	{
	  /* any / +inf = 0. */
	  /* any / -inf = 0. */
	  retval = QSFLOAT(0); /* inexact 0 */
	}
      break;
    case NUMTYPE_NAN:
    default:
      retval = QSNAN;
      break;
    }
  return retval;
}

/* quotient */
static qsptr_t qsprim_quo2 (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = qsint_make(mach, 0);
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  alu_t a;

  enum numtower_e promoted_type = _promote_numtype(mach, arg0, arg1);
  alu_load_x(&a, mach, arg0);
  alu_load_y(&a, mach, arg1);
  alu_cast_x(&a, promoted_type);
  alu_cast_y(&a, promoted_type);

  switch (_promote_numtype(mach, arg0, arg1))
    {
    /* Force integer division on the integer types. */
    case NUMTYPE_INT:
      a.z.i = a.x.i / a.y.i;
      retval = qsint_make(mach, a.z.i);
      break;
    case NUMTYPE_LONG:
      a.z.l = a.x.l / a.y.l;
      retval = qslong_make(mach, a.z.l);
      break;
    default:
      /* Fall back to division. */
      return qsprim_div2(mach, args);
      break;
    }
  return retval;
}

/* remainder */
static qsptr_t qsprim_rem2 (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = qsint_make(mach, 0);
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  alu_t a;

  enum numtower_e promoted_type = _promote_numtype(mach, arg0, arg1);
  alu_load_x(&a, mach, arg0);
  alu_load_y(&a, mach, arg1);
  alu_cast_x(&a, promoted_type);
  alu_cast_y(&a, promoted_type);

  switch (promoted_type)
    {
    /* Force integer division on the integer types. */
    case NUMTYPE_INT:
      a.z.i = a.x.i % a.y.i;
      retval = qsint_make(mach, a.z.i);
      break;
    case NUMTYPE_LONG:
      a.z.l = a.x.l % a.y.l;
      retval = qslong_make(mach, a.z.l);
      break;
    default:
      return QSFLOAT(0); /* inexact zero */
      break;
    }
  return retval;
}

/* Mathematics:abs */

/* Mathematics: Arithmetic yielding to libm. */

static qsptr_t qsprim_floor (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = qsint_make(mach, 0);
  qsptr_t arg0 = CAR(args);
  alu_t a;

  alu_load_x(&a, mach, arg0);
  switch (a.xtype)
    {
    case NUMTYPE_INT:
    case NUMTYPE_LONG:
      retval = arg0;
      break;
    case NUMTYPE_FLOAT:
    case NUMTYPE_DOUBLE:
      _delegate_libm1(&a, a.xtype, floorf, floor, NULL);
      retval = alu_store_z(&a, mach);
      break;
    default:
      retval = arg0; /* +/-inf => same; +/-nan => same */
      break;
    }
  return retval;
}

static qsptr_t qsprim_ceiling (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = qsint_make(mach, 0);
  qsptr_t arg0 = CAR(args);
  alu_t a;

  alu_load_x(&a, mach, arg0);
  switch (a.xtype)
    {
    case NUMTYPE_INT:
    case NUMTYPE_LONG:
      retval = arg0;
      break;
    case NUMTYPE_FLOAT:
    case NUMTYPE_DOUBLE:
      _delegate_libm1(&a, a.xtype, ceilf, ceil, NULL);
      retval = alu_store_z(&a, mach);
      break;
    default:
      retval = arg0; /* +/-inf => same; +/-nan => same */
      break;
    }
  return retval;
}

static qsptr_t qsprim_truncate (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = qsint_make(mach, 0);
  qsptr_t arg0 = CAR(args);
  alu_t a;

  alu_load_x(&a, mach, arg0);
  switch (a.xtype)
    {
    case NUMTYPE_INT:
    case NUMTYPE_LONG:
      retval = arg0;
      break;
    case NUMTYPE_FLOAT:
    case NUMTYPE_DOUBLE:
      _delegate_libm1(&a, a.xtype, ceilf, ceil, NULL);
      retval = alu_store_z(&a, mach);
      break;
    default:
      retval = arg0; /* +/-inf => same; +/-nan => same */
      break;
    }
  return retval;
}

static qsptr_t qsprim_round (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = qsint_make(mach, 0);
  qsptr_t arg0 = CAR(args);
  alu_t a;

  alu_load_x(&a, mach, arg0);
  switch (a.xtype)
    {
    case NUMTYPE_INT:
    case NUMTYPE_LONG:
      retval = arg0;
      break;
    case NUMTYPE_FLOAT:
    case NUMTYPE_DOUBLE:
      _delegate_libm1(&a, a.xtype, roundf, round, NULL);
      retval = alu_store_z(&a, mach);
      break;
    default:
      retval = arg0; /* +/-inf => same; +/-nan => same */
      break;
    }
  return retval;
}

static qsptr_t qsprim_rationalize (qsmachine_t * mach, qsptr_t args)
{
  /* TODO */
  qsptr_t arg0 = CAR(args);
  qsptr_t retval = arg0;
  (void)arg0;

  return retval;
}


static qsptr_t qsprim_exp (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  alu_t a;

  switch (_numtype(mach, arg0))
    {
    case NUMTYPE_INT:
    case NUMTYPE_LONG:
    case NUMTYPE_FLOAT:
    case NUMTYPE_DOUBLE:
      alu_load_x(&a, mach, arg0);
      _delegate_libm1(&a, a.xtype, expf, exp, cexpf);
      retval = alu_store_z(&a, mach);
      break;
    case NUMTYPE_INF:
      if (qspinf_p(mach, arg0))
	{
	  retval = QSINF;  /* exp(inf) => inf */
	}
      else if (qsninf_p(mach, arg0))
	{
	  retval = QSFLOAT(0);  /* exp(-inf) => 0. */
	}
      break;
    default:
      retval = QSNAN;
      break;
    }
  return retval;
}

static qsptr_t qsprim_log (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  alu_t a;

  a.xtype = _numtype(mach, arg0);
  a.ytype = _numtype(mach, arg1);
  if (qsnil_p(mach, arg1))
    {
      a.ytype = NUMTYPE_INT;
      a.y.i = 0;
    }
  else switch (a.ytype)
    {
    case NUMTYPE_INT:
    case NUMTYPE_LONG:
      a.y.i = _integer_from(mach, arg1);
      break;
    case NUMTYPE_NAN:
      a.y.i = 0; /* indicate natural log. */
      break;
    default:
      return QSNAN;
    }
  int xsign = _sign_of(mach, arg0);
  switch (a.xtype)
    {
    case NUMTYPE_INT:
    case NUMTYPE_LONG:
    case NUMTYPE_FLOAT:
    case NUMTYPE_DOUBLE:
      if ((xsign > 0) || (xsign == -2))
	{
	  switch (a.y.i)
	    {
	    case 2: /* base-2 logarithm. */
	      _delegate_libm1(&a, a.xtype, log2f, log2, NULL);
	      break;
	    case 10: /* base-10 logarithm. */
	      _delegate_libm1(&a, a.xtype, log10f, log10, NULL);
	      break;
	    case 0: /* natural logarithm. */
	    default:
	      _delegate_libm1(&a, a.xtype, logf, log, clogf);
	      break;
	    }
	  retval = alu_store_z(&a, mach);
	}
      else if (xsign == 0)
	{
	  /* logarithm of 0. */
	  retval = QSNAN;
	}
      else
	{
	  /* logarithm of negative. */
	  retval = QSNAN;
	}
      break;
    case NUMTYPE_INF:
      if (qspinf_p(mach, arg0)) retval = QSINF;
      else if (qsninf_p(mach, arg0))
	{
	  /* TODO: complex. */
	  retval = QSNAN; /* until complex numbers implemented. */
	}
      break;
    case NUMTYPE_NAN:
    default:
      retval = QSNAN;
      break;
    }
  return retval;
}

static qsptr_t qsprim_sin (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  alu_t a;

  a.xtype = _numtype(mach, arg0);
  switch (a.xtype)
    {
    case NUMTYPE_INT:
    case NUMTYPE_LONG:
    case NUMTYPE_FLOAT:
    case NUMTYPE_DOUBLE:
      alu_load_x(&a, mach, arg0);
      _delegate_libm1(&a, a.xtype, sinf, sin, csinf);
      retval = alu_store_z(&a, mach);
      break;
    default:
      retval = QSNAN;
      break;
    }
  return retval;
}

static qsptr_t qsprim_cos (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  alu_t a;

  a.xtype = _numtype(mach, arg0);
  switch (a.xtype)
    {
    case NUMTYPE_INT:
    case NUMTYPE_LONG:
    case NUMTYPE_FLOAT:
    case NUMTYPE_DOUBLE:
      alu_load_x(&a, mach, arg0);
      _delegate_libm1(&a, a.xtype, cosf, cos, ccosf);
      retval = alu_store_z(&a, mach);
      break;
    default:
      retval = QSNAN;
      break;
    }
  return retval;
}

static qsptr_t qsprim_tan (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  alu_t a;

  a.xtype = _numtype(mach, arg0);
  switch (a.xtype)
    {
    case NUMTYPE_INT:
    case NUMTYPE_LONG:
    case NUMTYPE_FLOAT:
    case NUMTYPE_DOUBLE:
      alu_load_x(&a, mach, arg0);
      _delegate_libm1(&a, a.xtype, tanf, tan, ctanf);
      retval = alu_store_z(&a, mach);
      break;
    default:
      retval = QSNAN;
      break;
    }
  return retval;
}

static qsptr_t qsprim_asin (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  alu_t a;

  a.xtype = _numtype(mach, arg0);
  switch (a.xtype)
    {
    case NUMTYPE_INT:
    case NUMTYPE_LONG:
    case NUMTYPE_FLOAT:
    case NUMTYPE_DOUBLE:
      alu_load_x(&a, mach, arg0);
      _delegate_libm1(&a, a.xtype, asinf, asin, casinf);
      retval = alu_store_z(&a, mach);
      break;
    default:
      retval = QSNAN;
      break;
    }
  return retval;
}

static qsptr_t qsprim_acos (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  alu_t a;

  a.xtype = _numtype(mach, arg0);
  switch (a.xtype)
    {
    case NUMTYPE_INT:
    case NUMTYPE_LONG:
    case NUMTYPE_FLOAT:
    case NUMTYPE_DOUBLE:
      alu_load_x(&a, mach, arg0);
      _delegate_libm1(&a, a.xtype, acosf, acos, cacosf);
      retval = alu_store_z(&a, mach);
      break;
    default:
      retval = QSNAN;
      break;
    }
  return retval;
}

static qsptr_t qsprim_atan (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  alu_t a;

  a.xtype = _numtype(mach, arg0);
  if (qsnil_p(mach, arg1))
    {
      /* single-argument atan(). */
      alu_load_x(&a, mach, arg0);
      _delegate_libm1(&a, a.xtype, atanf, atan, NULL);
      retval = alu_store_z(&a, mach);
    }
  else
    {
      /* two-argument atan2().  Order same as C call. */
      alu_load_x(&a, mach, arg0);
      alu_load_y(&a, mach, arg1);
      _delegate_libm2(&a, a.xtype, atan2f, atan2, NULL);
      retval = alu_store_z(&a, mach);
    }
  return retval;
}

static qsptr_t qsprim_expt (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  alu_t a;

  alu_load_x(&a, mach, arg0);
  alu_load_y(&a, mach, arg1);
  _delegate_libm2(&a, a.xtype, powf, pow, NULL);
  retval = alu_store_z(&a, mach);
  return retval;
}

/* Arithmetic:square */

static qsptr_t qsprim_sqrt (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  alu_t a;

  alu_load_x(&a, mach, arg0);
  _delegate_libm1(&a, a.xtype, sqrtf, sqrt, csqrtf);
  retval = alu_store_z(&a, mach);
  return retval;
}

/* Arithmetic:exact-integer-sqrt */
/* Arithmetic:make-rectangular */
/* Arithmetic:make-polar */

static qsptr_t qsprim_real_part (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = qsfloat_make(mach, 0);
  qsptr_t arg0 = CAR(args);
  alu_t a;

  alu_load_x(&a, mach, arg0);
  switch (a.xtype)
    {
    case NUMTYPE_INT:
      retval = qsfloat_make(mach, a.x.i);
      break;
    case NUMTYPE_LONG:
      retval = qsfloat_make(mach, a.x.l);
      break;
    case NUMTYPE_FLOAT:
      retval = arg0;
      break;
    case NUMTYPE_DOUBLE:
      retval = arg0;
      break;
    /* TODO: complex. */
    case NUMTYPE_QUAT:
	{
	  retval = qsfloat_make(mach, qsquat_ref(mach, arg0, 0));
	}
      break;
    default:
      retval = arg0;
      break;
    }
  return retval;
}

static qsptr_t qsprim_imag_part (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = qsfloat_make(mach, 0);
  qsptr_t arg0 = CAR(args);
  alu_t a;

  alu_load_x(&a, mach, arg0);
  switch (a.xtype)
    {
    case NUMTYPE_INT:
    case NUMTYPE_LONG:
    case NUMTYPE_FLOAT:
    case NUMTYPE_DOUBLE:
      retval = qsfloat_make(mach, 0);
      break;
    /* TODO: complex. */
    case NUMTYPE_QUAT:
	{
	  qsquat_fetch(mach, arg0, a.x.q);
	  a.z.q[0] = 0;
	  a.z.q[1] = a.x.q[1];
	  a.z.q[2] = a.x.q[2];
	  a.z.q[3] = a.x.q[3];
	  retval = qsquat_make_f4(mach, a.z.q);
	}
      break;
    default:
      retval = arg0;
      break;
    }
  return retval;
}


static qsptr_t qsprim_magnitude (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  alu_t a;

  alu_load_x(&a, mach, arg0);
  switch (a.xtype)
    {
    case NUMTYPE_INT:
      retval = qsfloat_make(mach, abs(a.x.i));
      break;
    case NUMTYPE_LONG:
      retval = qsfloat_make(mach, labs(a.x.l));
      break;
    case NUMTYPE_FLOAT:
      retval = qsfloat_make(mach, fabsf(a.x.f));
      break;
    case NUMTYPE_DOUBLE:
      if (a.x.d < 0)
	{
	  retval = qsdouble_make(mach, fabs(a.x.d));
	}
      else
	{
	  retval = arg0;
	}
      break;
    /* TODO: complex. */
    case NUMTYPE_QUAT:
	{
	  qsquat_fetch(mach, arg0, a.x.q);
	  float a1 = a.x.q[0], b1 = a.x.q[1], c1 = a.x.q[2], d1 = a.x.q[3];
	  a.z.f = sqrt(a1*a1 + b1*b1 + c1*c1 + d1*d1);
	  retval = qsfloat_make(mach, a.z.f);
	}
      break;
    default:
      retval = arg0;
      break;
    }
  return retval;
}

/* Arithmetic:angle */

static qsptr_t qsprim_inexact (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  alu_t a;

  alu_load_x(&a, mach, arg0);
  switch (a.xtype)
    {
    case NUMTYPE_INT:
      retval = qsfloat_make(mach, a.x.i);
      break;
    case NUMTYPE_LONG:
      retval = qsfloat_make(mach, a.x.l);
      break;
    case NUMTYPE_FLOAT:
    case NUMTYPE_DOUBLE:
      retval = arg0;
      break;
    default:
      retval = arg0;
      break;
    }
  return retval;
}

/* Not be confused with "exact?" */
static qsptr_t qsprim_exact (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSNAN;
  qsptr_t arg0 = CAR(args);
  alu_t a;

  alu_load_x(&a, mach, arg0);
  switch (a.xtype)
    {
    case NUMTYPE_INT:
    case NUMTYPE_LONG:
      retval = arg0;
      break;
    case NUMTYPE_FLOAT:
      retval = qsint_make(mach, a.x.f);
      break;
    case NUMTYPE_DOUBLE:
      retval = qsint_make(mach, a.x.d);
      break;
    default:
      retval = arg0;
      break;
    }
  return retval;
}


static
struct prims_table_s table1_arithmetic[] = {
      { "add2", qsprim_add2 },
      { "sub2", qsprim_sub2 },
      { "mul2", qsprim_mul2 },
      { "div2", qsprim_div2 },
      { "quo2", qsprim_quo2 },
      { "rem2", qsprim_rem2 },
      { "floor", qsprim_floor },
      { "ceiling", qsprim_ceiling },
      { "truncate", qsprim_truncate },
      { "round", qsprim_round },
      { "rationalize", qsprim_rationalize },

      { "exp", qsprim_exp },
      { "log", qsprim_log },
      { "sin", qsprim_sin },
      { "cos", qsprim_cos },
      { "tan", qsprim_tan },
      { "asin", qsprim_asin },
      { "acos", qsprim_acos },
      { "atan", qsprim_atan },
      { "expt", qsprim_expt },
      { "sqrt", qsprim_sqrt },
      { "magnitude", qsprim_magnitude },
      { "inexact", qsprim_inexact },
      { "exact", qsprim_exact }, /* not to be confused with "exact?". */
      { "imag-part", qsprim_imag_part },
      { "real-part", qsprim_real_part },

      { NULL, NULL },
};




/* for testing invocation of primitives. */
static
struct prims_table_s table0[] = {
      { "halt", qsprim_halt },
      { "+", qsprim_add2 },
      { NULL, NULL },
};




qsptr_t qsprimreg_presets_v0 (qsmachine_t * mach)
{
  qsptr_t primenv = qsenv_make(mach, QSNIL);
  qsptr_t y = QSNIL;
  int primid = 0;
  qsptr_t prim = QSNIL;

  struct prims_table_s * concat[] = {
      table0,
      table1_typepredicates,
      table1_pairs,
      table1_symbols,
      table1_chars,
      table1_strings,
      table1_vectors,
      table1_bytevectors,
      table1_ports,
      table1_ports_fd,
      table1_ports_file,
      table1_ports_mem,
      table1_konts,
      table1_numtypes,
      table1_arithmetic,
      NULL
  };

  struct prims_table_s ** concatiter = NULL;
  for (concatiter = concat; *concatiter; ++concatiter)
    {
      struct prims_table_s * primiter = NULL;
      for (primiter = *concatiter; primiter->name; ++primiter)
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

  qsptr_t y_stdin = qssymbol_intern_c(mach, "*current-input-port*", 0);
  qsptr_t y_stdout = qssymbol_intern_c(mach, "*current-output-port*", 0);
  qsptr_t y_stderr = qssymbol_intern_c(mach, "*current-error-port*", 0);
  qsptr_t p_stdin = qsfd_make(mach, 0);
  qsptr_t p_stdout = qsfd_make(mach, 1);
  qsptr_t p_stderr = qsfd_make(mach, 0);
  primenv = qsenv_insert(mach, primenv, y_stdin, p_stdin);
  primenv = qsenv_insert(mach, primenv, y_stdout, p_stdout);
  primenv = qsenv_insert(mach, primenv, y_stderr, p_stderr);

  return primenv;
}

qsptr_t qsprimreg_presets_v1 (qsmachine_t * mach)
{
  qsptr_t primenv = qsenv_make(mach, QSNIL);
  qsptr_t y = QSNIL;
  int primid = 0;
  qsptr_t prim = QSNIL;

  struct prims_table_s * concat[] = {
      table1_typepredicates,
      table1_pairs,
      table1_symbols,
      table1_chars,
      table1_strings,
      table1_vectors,
      table1_bytevectors,
      table1_ports,
      table1_ports_fd,
      table1_ports_file,
      table1_ports_mem,
      table1_konts,
      table1_numtypes,
      table1_arithmetic,
      NULL
  };

  struct prims_table_s ** concatiter = NULL;
  for (concatiter = concat; *concatiter; ++concatiter)
    {
      struct prims_table_s * primiter = NULL;
      for (primiter = *concatiter; primiter->name; ++primiter)
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

  qsptr_t y_stdin = qssymbol_intern_c(mach, "*current-input-port*", 0);
  qsptr_t y_stdout = qssymbol_intern_c(mach, "*current-output-port*", 0);
  qsptr_t y_stderr = qssymbol_intern_c(mach, "*current-error-port*", 0);
  qsptr_t p_stdin = qsfd_make(mach, 0);
  qsptr_t p_stdout = qsfd_make(mach, 1);
  qsptr_t p_stderr = qsfd_make(mach, 0);
  primenv = qsenv_insert(mach, primenv, y_stdin, p_stdin);
  primenv = qsenv_insert(mach, primenv, y_stdout, p_stdout);
  primenv = qsenv_insert(mach, primenv, y_stderr, p_stderr);

  return primenv;
}

