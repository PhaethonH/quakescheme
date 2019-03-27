#include <ctype.h>
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

static int _integer_from (qsmachine_t * mach, qsptr_t p)
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
      table1_pairs,
      table1_symbols,
      table1_chars,
      table1_vectors,
      table1_bytevectors,
      table1_konts,
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

