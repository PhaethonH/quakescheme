#include <ctype.h>
#include <math.h>
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

static
struct prims_table_s table1_ports[] = {
      { "eof-object", qsprim_eof_object },
      { "port-read-u8", qsprim_bytevector_length },
      { "port-write-u8", qsprim_bytevector_u8_ref },
      { "port-close", qsprim_bytevector_u8_setq },
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
      { "port-mem-open", qsprim_port_file_open },
      { NULL, NULL },
};




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
  retval = qskont_make(mach, arg0, arg1, arg2, arg3);
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
    NUMTYPE_INT,
    NUMTYPE_LONG,
    NUMTYPE_FLOAT,
    NUMTYPE_DOUBLE,
    NUMTYPE_RATIONAL,
    NUMTYPE_COMPLEX,
    NUMTYPE_INF,
    NUMTYPE_NAN,
};

static enum numtower_e _numtype (const qsmachine_t * mach, qsptr_t x)
{
  if (qsint_p(mach, x)) return NUMTYPE_INT;
  if (qslong_p(mach, x)) return NUMTYPE_LONG;
  if (qsfloat_p(mach, x)) return NUMTYPE_FLOAT;
  if (qsdouble_p(mach, x)) return NUMTYPE_DOUBLE;
  /* TODO: rational */
  /* TODO: complex */
  if (qspinf_p(mach, x) || qsninf_p(mach, x)) return NUMTYPE_INF;
  return NUMTYPE_NAN;
}

enum numtower_e _common_numtype (const qsmachine_t * mach, qsptr_t x, qsptr_t y)
{
  enum numtower_e xtype = _numtype(mach, x);
  enum numtower_e ytype = _numtype(mach, y);

  /* promote to more complex type. */
  if (xtype < ytype) xtype = ytype;
  return xtype;
}

static float _float_from (const qsmachine_t * mach, qsptr_t p)
{
  if (qsint_p(mach, p)) return (float)(qsint_get(mach, p));
  if (qsfloat_p(mach, p)) return (float)qsfloat_get(mach, p);
  if (qslong_p(mach, p)) return (float)(qslong_get(mach, p));
  if (qsdouble_p(mach, p)) return (float)(qsdouble_get(mach, p));
  return QSNAN;
}

static long _long_from (const qsmachine_t * mach, qsptr_t p)
{
  if (qsint_p(mach, p)) return (long)(qsint_get(mach, p));
  if (qsfloat_p(mach, p)) return (long)qsfloat_get(mach, p);
  if (qslong_p(mach, p)) return (long)(qslong_get(mach, p));
  if (qsdouble_p(mach, p)) return (long)(qsdouble_get(mach, p));
  return QSNAN;
}

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
      int i = _integer_from(mach, p);
      return (i < 0) ? -1 : (i > 0) ? 1 : 0;
    }
  if (qsfloat_p(mach, p))
    {
      float f = _float_from(mach, p);
      return (f < 0) ? -1 : (f > 0) ? 1 : 0;
    }
  if (qslong_p(mach, p))
    {
      float l = _long_from(mach, p);
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
    struct {
	int32_t i;
	int64_t l;
	float f;
	double d;
    } x, y, z;
} alu_t;


/* Mathematics: numeric predicates. */
static qsptr_t qsprim_complex_p (qsmachine_t * mach, qsptr_t args)
{
  return QSFALSE;
}

static qsptr_t qsprim_real_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qsfloat_p(mach, arg0)) retval = QSTRUE;
  if (qsdouble_p(mach, arg0)) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_rational_p (qsmachine_t * mach, qsptr_t args)
{
  return QSFALSE;
}

static qsptr_t qsprim_integer_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qsint_p(mach, arg0)) retval = QSTRUE;
  if (qslong_p(mach, arg0)) retval = QSTRUE;
  return retval;
}

static qsptr_t qsprim_exact_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qsint_p(mach, arg0)) retval = QSTRUE;
  if (qslong_p(mach, arg0)) retval = QSTRUE;
  /* TODO: rational. */
  return retval;
}

static qsptr_t qsprim_inexact_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qsfloat_p(mach, arg0)) retval = QSTRUE;
  if (qsdouble_p(mach, arg0)) retval = QSTRUE;
  /* TODO: complex. */
  return retval;
}

static qsptr_t qsprim_exact_integer_p (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = QSFALSE;
  qsptr_t arg0 = CAR(args);
  if (qsint_p(mach, arg0)) retval = QSTRUE;
  if (qslong_p(mach, arg0)) retval = QSTRUE;
  return retval;
}

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
  qsptr_t arg0 = CAR(args);
  qsptr_t arg1 = CAR(CDR(args));
  int res = -2;
  alu_t a;
  switch (_common_numtype(mach, arg0, arg1))
    {
    case NUMTYPE_INT:
      a.x.i = _integer_from(mach, arg0);
      a.y.i = _integer_from(mach, arg1);
      res = (a.x.i < a.y.i) ? -1 : (a.x.i > a.y.i) ? 1 : 0;
      break;
    case NUMTYPE_FLOAT:
      a.x.f = _integer_from(mach, arg0);
      a.y.f = _integer_from(mach, arg1);
      res = (a.x.f < a.y.f) ? -1 : (a.x.f > a.y.f) ? 1 : 0;
      break;
    case NUMTYPE_LONG:
      a.x.l = _integer_from(mach, arg0);
      a.y.l = _integer_from(mach, arg1);
      res = (a.x.l < a.y.l) ? -1 : (a.x.l > a.y.l) ? 1 : 0;
      break;
    case NUMTYPE_DOUBLE:
      a.x.d = _integer_from(mach, arg0);
      a.y.d = _integer_from(mach, arg1);
      res = (a.x.d < a.y.d) ? -1 : (a.x.d > a.y.d) ? 1 : 0;
      break;
    /* TODO: rational */
    case NUMTYPE_INF:
      a.x.i = _sign_of(mach, arg0);
      a.y.i = _sign_of(mach, arg1);
      if ((a.x.i == -2) || (a.y.i == -2)) return -2;  /* nope */
      else if (a.x.i < a.y.i) res = -1;  /* - <=> 0, - <=> +, 0 <=> + */
      else if (a.x.i > a.y.i) res = 1;  /* + <=> 0, 0 <=> -, + <=> - */
      else
	{
	  if (qspinf_p(mach, arg0))
	    {
	      if (qspinf_p(mach, arg1)) res = -2;  /* +inf <=> +inf = nope */
	      else res = +1;  /* +inf <=> anything else = +1 */
	    }
	  else if (qsninf_p(mach, arg0))
	    {
	      if (qsninf_p(mach, arg1)) res = -2;  /* -inf <=> -inf = nope */
	      else res = -1;  /* -inf <=> anything else = -1 */
	    }
	  else if (qspinf_p(mach, arg1)) res = -1;  /* fin <=> +inf = -1 */
	  else if (qsninf_p(mach, arg1)) res = +1;  /* fin <=> -inf = +1 */
	}
      break;
    /* TODO: complex. */
    case NUMTYPE_NAN:
    default:
      res = -2;  /* nope */
      break;
    }
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
      { "complex?", qsprim_complex_p },
      { "real?", qsprim_real_p },
      { "rational?", qsprim_rational_p },
      { "integer?", qsprim_integer_p },
      { "exact?", qsprim_exact_p },
      { "inexact?", qsprim_inexact_p },
      { "exact-integer?", qsprim_exact_integer_p },
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

  switch (_common_numtype(mach, arg0, arg1))
    {
    case NUMTYPE_INT:
      a.x.i = _integer_from(mach, arg0);
      a.y.i = _integer_from(mach, arg1);
      a.z.i = a.x.i + a.y.i;
      retval = qsint_make(mach, a.z.i);
      break;
    case NUMTYPE_FLOAT:
      a.x.f = _float_from(mach, arg0);
      a.y.f = _float_from(mach, arg1);
      a.z.f = a.x.f + a.y.f;
      retval = qsfloat_make(mach, a.z.f);
      break;
    case NUMTYPE_LONG:
      a.x.l = _long_from(mach, arg0);
      a.y.l = _long_from(mach, arg1);
      a.z.l = a.x.l + a.y.l;
      retval = qslong_make(mach, a.z.l);
      break;
    case NUMTYPE_DOUBLE:
      a.x.d = _long_from(mach, arg0);
      a.y.d = _long_from(mach, arg1);
      a.z.d = a.x.d + a.y.d;
      retval = qsdouble_make(mach, a.z.d);
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

  switch (_common_numtype(mach, arg0, arg1))
    {
    case NUMTYPE_INT:
      a.x.i = _integer_from(mach, arg0);
      a.y.i = _integer_from(mach, arg1);
      a.z.i = a.x.i - a.y.i;
      retval = qsint_make(mach, a.z.i);
      break;
    case NUMTYPE_FLOAT:
      a.x.f = _float_from(mach, arg0);
      a.y.f = _float_from(mach, arg1);
      a.z.f = a.x.f - a.y.f;
      retval = qsfloat_make(mach, a.z.f);
      break;
    case NUMTYPE_LONG:
      a.x.l = _long_from(mach, arg0);
      a.y.l = _long_from(mach, arg1);
      a.z.l = a.x.l - a.y.l;
      retval = qslong_make(mach, a.z.l);
      break;
    case NUMTYPE_DOUBLE:
      a.x.d = _long_from(mach, arg0);
      a.y.d = _long_from(mach, arg1);
      a.z.d = a.x.d - a.y.d;
      retval = qsdouble_make(mach, a.z.d);
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

  switch (_common_numtype(mach, arg0, arg1))
    {
    case NUMTYPE_INT:
      a.x.i = _integer_from(mach, arg0);
      a.y.i = _integer_from(mach, arg1);
      a.z.i = a.x.i * a.y.i;
      retval = qsint_make(mach, a.z.i);
      break;
    case NUMTYPE_FLOAT:
      a.x.f = _float_from(mach, arg0);
      a.y.f = _float_from(mach, arg1);
      a.z.f = a.x.f * a.y.f;
      retval = qsfloat_make(mach, a.z.f);
      break;
    case NUMTYPE_LONG:
      a.x.l = _long_from(mach, arg0);
      a.y.l = _long_from(mach, arg1);
      a.z.l = a.x.l * a.y.l;
      retval = qslong_make(mach, a.z.l);
      break;
    case NUMTYPE_DOUBLE:
      a.x.d = _long_from(mach, arg0);
      a.y.d = _long_from(mach, arg1);
      a.z.d = a.x.d * a.y.d;
      retval = qsdouble_make(mach, a.z.d);
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

  switch (_common_numtype(mach, arg0, arg1))
    {
    case NUMTYPE_INT:
      /* TODO: cast to rational. */
      a.x.i = _integer_from(mach, arg0);
      a.y.i = _integer_from(mach, arg1);
      a.z.i = a.x.i / a.y.i;
      retval = qsint_make(mach, a.z.i);
      break;
    case NUMTYPE_FLOAT:
      a.x.f = _float_from(mach, arg0);
      a.y.f = _float_from(mach, arg1);
      a.z.f = a.x.f / a.y.f;
      retval = qsfloat_make(mach, a.z.f);
      break;
    case NUMTYPE_LONG:
      /* TODO: cast to rational. */
      a.x.l = _long_from(mach, arg0);
      a.y.l = _long_from(mach, arg1);
      a.z.l = a.x.l / a.y.l;
      retval = qslong_make(mach, a.z.l);
      break;
    case NUMTYPE_DOUBLE:
      a.x.d = _long_from(mach, arg0);
      a.y.d = _long_from(mach, arg1);
      a.z.d = a.x.d / a.y.d;
      retval = qsdouble_make(mach, a.z.d);
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

  switch (_common_numtype(mach, arg0, arg1))
    {
    /* Force integer division on the integer types. */
    case NUMTYPE_INT:
      a.x.i = _integer_from(mach, arg0);
      a.y.i = _integer_from(mach, arg1);
      a.z.i = a.x.i / a.y.i;
      retval = qsint_make(mach, a.z.i);
      break;
    case NUMTYPE_LONG:
      a.x.l = _long_from(mach, arg0);
      a.y.l = _long_from(mach, arg1);
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

  switch (_common_numtype(mach, arg0, arg1))
    {
    /* Force integer division on the integer types. */
    case NUMTYPE_INT:
      a.x.i = _integer_from(mach, arg0);
      a.y.i = _integer_from(mach, arg1);
      a.z.i = a.x.i % a.y.i;
      retval = qsint_make(mach, a.z.i);
      break;
    case NUMTYPE_LONG:
      a.x.l = _long_from(mach, arg0);
      a.y.l = _long_from(mach, arg1);
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

  switch (_numtype(mach, arg0))
    {
    case NUMTYPE_INT:
    case NUMTYPE_LONG:
      retval = arg0;
      break;
    case NUMTYPE_FLOAT:
      a.x.f = _float_from(mach, arg0);
      a.z.f = floorf(a.x.f);
      retval = qsfloat_make(mach, a.z.f);
      break;
    case NUMTYPE_DOUBLE:
      a.x.d = _double_from(mach, arg0);
      a.z.d = floor(a.x.d);
      retval = qsdouble_make(mach, a.z.d);
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

  switch (_numtype(mach, arg0))
    {
    case NUMTYPE_INT:
    case NUMTYPE_LONG:
      retval = arg0;
      break;
    case NUMTYPE_FLOAT:
      a.x.f = _float_from(mach, arg0);
      a.z.f = ceilf(a.x.f);
      retval = qsfloat_make(mach, a.z.f);
      break;
    case NUMTYPE_DOUBLE:
      a.x.d = _double_from(mach, arg0);
      a.z.d = ceil(a.x.d);
      retval = qsdouble_make(mach, a.z.d);
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

  switch (_numtype(mach, arg0))
    {
    case NUMTYPE_INT:
    case NUMTYPE_LONG:
      retval = arg0;
      break;
    case NUMTYPE_FLOAT:
      a.x.f = _float_from(mach, arg0);
      a.z.f = truncf(a.x.f);
      retval = qsfloat_make(mach, a.z.f);
      break;
    case NUMTYPE_DOUBLE:
      a.x.d = _double_from(mach, arg0);
      a.z.d = trunc(a.x.d);
      retval = qsdouble_make(mach, a.z.d);
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

  switch (_numtype(mach, arg0))
    {
    case NUMTYPE_INT:
    case NUMTYPE_LONG:
      retval = arg0;
      break;
    case NUMTYPE_FLOAT:
      a.x.f = _float_from(mach, arg0);
      a.z.f = roundf(a.x.f);
      retval = qsfloat_make(mach, a.z.f);
      break;
    case NUMTYPE_DOUBLE:
      a.x.d = _double_from(mach, arg0);
      a.z.d = round(a.x.d);
      retval = qsdouble_make(mach, a.z.d);
      break;
    default:
      retval = arg0; /* +/-inf => same; +/-nan => same */
      break;
    }
  return retval;
}

static qsptr_t qsprim_rationalize (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = qsint_make(mach, 0);
  qsptr_t arg0 = CAR(args);

  /* TODO */

  return arg0;
}

static qsptr_t qsprim_exp (qsmachine_t * mach, qsptr_t args)
{
  qsptr_t retval = qsint_make(mach, 0);
  qsptr_t arg0 = CAR(args);
  alu_t a;

  switch (_numtype(mach, arg0))
    {
    case NUMTYPE_INT:
      a.x.i = _integer_from(mach, arg0);
      a.z.f = expf(a.x.i);
      retval = qsfloat_make(mach, a.z.f);
      break;
    case NUMTYPE_LONG:
      a.x.f = _integer_from(mach, arg0);
      a.z.f = expf(a.x.f);
      retval = qsfloat_make(mach, a.z.f);
      break;
    case NUMTYPE_FLOAT:
      a.x.l = _integer_from(mach, arg0);
      a.z.f = expf((float)(a.x.l));
      retval = qsfloat_make(mach, a.z.f);
      break;
    case NUMTYPE_DOUBLE:
      a.x.d = _double_from(mach, arg0);
      a.z.d = expf(a.x.d);
      retval = qsfloat_make(mach, a.z.d);
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
  qsptr_t retval = qsint_make(mach, 0);
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
  switch (a.xtype)
    {
    case NUMTYPE_INT:
      a.x.i = _integer_from(mach, arg0);
      if (a.x.i > 0)
	{
	  a.z.f = logf(a.x.i);
	  retval = qsfloat_make(mach, a.z.f);
	}
      else
	{
	  retval = QSNAN;
	}
      break;
    case NUMTYPE_LONG:
      a.x.l = _long_from(mach, arg0);
      if (a.x.l > 0)
	{
	  a.z.f = logf(a.x.l);
	  retval = qsfloat_make(mach, a.z.f);
	}
      else
	{
	  retval = QSNAN;
	}
      break;
    case NUMTYPE_FLOAT:
      a.y.f = _float_from(mach, arg0);
      if (a.x.f > 0)
	{
	  a.z.f = logf(a.x.f);
	  retval = qsfloat_make(mach, a.z.f);
	}
      else
	{
	  retval = QSNAN;
	}
      break;
    case NUMTYPE_DOUBLE:
      a.x.f = _double_from(mach, arg0);
      if (a.x.d > 0)
	{
	  a.z.d = log(a.x.d);
	  retval = qsdouble_make(mach, a.z.d);
	}
      else
	{
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

      { NULL, NULL },
};




static
struct prims_table_s table1[] = {
      { "halt", qsprim_halt },
      { "+", qsprim_add2 },
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

