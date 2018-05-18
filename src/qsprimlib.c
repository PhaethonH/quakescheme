#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fenv.h>

#include "qsprimlib.h"

/*
Prefices for types associated with primitives:

(nul) = machine

B = boolean
F = float31
I = int30
C = char24
K = const16
E = error16
P = portFD16
J = iter28

L = lambda
O = entire store (direct access to store)

o = object
p = pair
s = string
v = vector
w = bytevector
(b = bitvector)
l = list
a = immlist (array)
n = number
 nl = long (fixnum64)
 nd = double (flonum64-inexact)
 nr = rational
 nc = complex
u = utf8 (native string)
e = env
c = closure
k = continuation
y = symbol
t = tree
x = symbol table


Symbols for common object operations:
* = make
@ = ref (value at ...)
! = setq ('!' = mutate in place)
# = length ('#' = number sign; counted number of)
: = to, conversion (':' type indicator)
*/


#define qspair_car0(mem,p) (ISNIL(p) ? QSNIL : qspair_ref_a(mem,p))
#define qspair_cdr0(mem,p) (ISNIL(p) ? QSNIL : qspair_ref_d(mem,p))
#define qspair_cadr0(mem,p) (ISNIL(p) ? QSNIL : qspair_car0(mem,qspair_cdr0(mem,p)))
#define qspair_cddr0(mem,p) (ISNIL(p) ? QSNIL : qspair_cdr0(mem,qspair_cdr0(mem,p)))
#define qspair_caddr0(mem,p) (ISNIL(p) ? QSNIL : qspair_car0(mem,qspair_cddr0(mem,p)))
#define qspair_cdddr0(mem,p) (ISNIL(p) ? QSNIL : qspair_cdr0(mem,qspair_cddr0(mem,p)))

#define ARG(n) qslist_ref(machine->store, args, n)



qsptr_t qsprim_crash (qs_t * machine, qsptr_t args)
{
  puts("Crashing.");
  abort();
}

qsptr_t qsprim_halt (qs_t * machine, qsptr_t args)
{
  machine->halt = 1;
  machine->K = QSNIL;
  return QSTRUE;
}



/*******************/
/* Quark Character */
/*******************/

qsptr_t qsop_char_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  return QSBOOL(ISCHAR24(a));
}

qsptr_t qsop_char_coerce (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  return qschar(machine->store, a);
}

qsptr_t qsop_char_equal_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  qsptr_t res = qschar_cmp(machine->store, a, b);
  return QSBOOL(res == CMP_EQ);
}

qsptr_t qsop_char_to_integer (qs_t * machine, qsptr_t args)
{
  qsptr_t chobj = ARG(0);
  return qsint_make(machine->store, qschar_p(machine->store, chobj));
}

qsptr_t qsop_integer_to_char (qs_t * machine, qsptr_t args)
{
  qsptr_t intobj = ARG(0);
  return qschar_make(machine->store, qsint_get(machine->store, intobj));
}



/*****************/
/* Quark Integer */
/*****************/

qsptr_t qsop_int_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  return QSBOOL(ISINT30(a));
}

qsptr_t qsop_int_eq_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!ISINT30(a) || !ISINT30(b)) return QSFALSE;
  return QSBOOL(a == b);
}

qsptr_t qsop_int_lt_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!ISINT30(a) || !ISINT30(b)) return QSFALSE;
  return QSBOOL(a < b);
}

qsptr_t qsop_int_gt_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!ISINT30(a) || !ISINT30(b)) return QSFALSE;
  return QSBOOL(a > b);
}

qsptr_t qsop_int_coerce (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (ISINT30(a)) return a;
  int ca = 0;
  if (ISCHAR24(a)) ca = CCHAR24(a);
  else if (ISFLOAT31(a)) ca = (int)CFLOAT31(a);
  else if (qslong_p(machine->store, a)) ca = qslong_get(machine->store, a);
  else if (qsdouble_p(machine->store, a))
    ca = (int)(qsdouble_get(machine->store, a));
  else ca = 0;
  return qsint_make(machine->store, ca);
}

qsptr_t qsop_int_neg (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!qsint_p(machine->store, a)) a = QSINT(0);
  return QSINT(-CINT30(a));
}

qsptr_t qsop_int_add (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!ISINT30(a) || !ISINT30(b)) return QSINT(0);
  return QSINT(CINT30(a) + CINT30(b));
}

qsptr_t qsop_int_sub (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!ISINT30(a) || !ISINT30(b)) return QSINT(0);
  return QSINT(CINT30(a) - CINT30(b));
}

qsptr_t qsop_int_mul (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!ISINT30(a) || !ISINT30(b)) return QSINT(0);
  return QSINT(CINT30(a) * CINT30(b));
}

qsptr_t qsop_int_div (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!ISINT30(a) || !ISINT30(b)) return QSINT(0);
  return QSINT(CINT30(a) / CINT30(b));
}

qsptr_t qsop_int_mod (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!ISINT30(a) || !ISINT30(b)) return QSINT(0);
  return QSINT(CINT30(a) % CINT30(b));
}

qsptr_t qsop_int_bnot (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!ISINT30(a)) return QSINT(0);
  return QSINT(~CINT30(a));
}

qsptr_t qsop_int_bor (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!ISINT30(a) || !ISINT30(b)) return QSINT(0);
  return QSINT(CINT30(a) | CINT30(b));
}

qsptr_t qsop_int_band (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!ISINT30(a) || !ISINT30(b)) return QSINT(0);
  return QSINT(CINT30(a) & CINT30(b));
}

qsptr_t qsop_int_bxor (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!ISINT30(a) || !ISINT30(b)) return QSINT(0);
  return QSINT(CINT30(a) ^ CINT30(b));
}

qsptr_t qsop_int_bshl (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!ISINT30(a) || !ISINT30(b)) return QSINT(0);
  return QSINT(CINT30(a) << CINT30(b));
}

qsptr_t qsop_int_bshr (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!ISINT30(a) || !ISINT30(b)) return QSINT(0);
  return QSINT(CINT30(a) >> CINT30(b));
}

qsptr_t qsop_int_sign (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!ISINT30(a)) return QSINT(0);
  int ca = CINT30(a);
  return QSINT((ca < 0) ? -1 : (ca > 0) ? 1 : 0);
}

qsptr_t qsop_int_abs (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!ISINT30(a)) return QSINT(0);
  return QSINT(abs(CINT30(a)));
}

qsptr_t qsop_int_zero_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!ISINT30(a)) return QSFALSE;
  return QSBOOL(CINT30(a) == 0);
}


/* Macros to wrap libm (C math library) functions .*/
/* TODO: error checking */
#define LIBM_WRAP1(scmtype, scmname, ctype, cname, failval) \
qsptr_t qsop_##scmtype##_##scmname (qs_t * machine, qsptr_t args) \
{ \
  qsptr_t a = ARG(0); \
  if (! qs##scmtype##_p(machine->store, a)) return (failval); \
  ctype ca = qs##scmtype##_get(machine->store, a); \
  ctype res = cname(ca); \
  feclearexcept(0); \
  qsptr_t retval = qs##scmtype##_make(machine->store, res); \
  if (fetestexcept(FE_INEXACT | FE_UNDERFLOW | FE_OVERFLOW | FE_DIVBYZERO | FE_INVALID)) return failval; \
  return retval; \
}
#define LIBM_WRAP2(scmtype, scmname, ctype, cname, failval) \
qsptr_t qsop_##scmtype##_##scmname (qs_t * machine, qsptr_t args) \
{ \
  qsptr_t a = ARG(0); \
  qsptr_t b = ARG(1); \
  if (! qs##scmtype##_p(machine->store, a)) return (failval); \
  if (! qs##scmtype##_p(machine->store, b)) return (failval); \
  ctype ca = qs##scmtype##_get(machine->store, a); \
  ctype cb = qs##scmtype##_get(machine->store, b); \
  ctype res = cname(ca, cb); \
  qsptr_t retval = qs##scmtype##_make(machine->store, res); \
  return retval; \
}


/***************/
/* Quark Float */
/***************/

qsptr_t qsop_float_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  return QSBOOL(ISFLOAT31(a));
}

qsptr_t qsop_float_coerce (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (ISFLOAT31(a)) return a;
  float ca = 0.;
  if (ISCHAR24(a)) ca = (float)CCHAR24(a);
  else if (ISINT30(a)) ca = (float)CINT30(a);
  else if (qslong_p(machine->store, a))
    ca = (float)qslong_get(machine->store, a);
  else if (qsdouble_p(machine->store, a))
    ca = (float)qsdouble_get(machine->store, a);
  return qsfloat_make(machine->store, ca);
}

qsptr_t qsop_float_lt_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(0);
  if (!ISFLOAT31(a) || !ISFLOAT31(b)) return QSFALSE;
  return (CFLOAT31(a) < CFLOAT31(b));
}

qsptr_t qsop_float_gt_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(0);
  if (!ISFLOAT31(a) || !ISFLOAT31(b)) return QSFALSE;
  return (CFLOAT31(a) > CFLOAT31(b));
}

qsptr_t qsop_float_eq_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(0);
  if (!ISFLOAT31(a) || !ISFLOAT31(b)) return QSFALSE;
  /* forcibly convert to (float), as NaN and inf should fail comparison. */
  return (CFLOAT31(a) == CFLOAT31(b));
}


qsptr_t qsop_float_neg (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!ISFLOAT31(a)) return QSFLOAT(0);
  return (-CFLOAT31(a));
}

qsptr_t qsop_float_sign (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!ISFLOAT31(a)) return QSFLOAT(0);
  float cf = CFLOAT31(a);
  if (cf < 0) return QSFLOAT(-1);
  else if (cf > 0) return QSFLOAT(1);
  // TODO: what about nan and inf?
  return QSFLOAT(0);
}

qsptr_t qsop_float_abs (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!ISFLOAT31(a)) return QSFLOAT(0);
  return QSFLOAT(fabsf(CFLOAT31(a)));
}

qsptr_t qsop_float_round (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!ISFLOAT31(a)) return QSFLOAT(0);
  return QSFLOAT(roundf(CFLOAT31(a)));
}

qsptr_t qsop_float_floor (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!ISFLOAT31(a)) return QSFLOAT(0);
  return QSFLOAT(floorf(CFLOAT31(a)));
}

qsptr_t qsop_float_ceil (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!ISFLOAT31(a)) return QSFLOAT(0);
  return QSFLOAT(ceilf(CFLOAT31(a)));
}

qsptr_t qsop_float_trunc (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!ISFLOAT31(a)) return QSFLOAT(0);
  return QSFLOAT(truncf(CFLOAT31(a)));
}


qsptr_t qsop_float_add (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(0);
  if (!ISFLOAT31(a) || !ISFLOAT31(b)) return QSFLOAT(0);
  return QSFLOAT(CFLOAT31(a) + CFLOAT31(b));
}

qsptr_t qsop_float_sub (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(0);
  if (!ISFLOAT31(a) || !ISFLOAT31(b)) return QSFLOAT(0);
  return QSFLOAT(CFLOAT31(a) - CFLOAT31(b));
}

qsptr_t qsop_float_mul (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(0);
  if (!ISFLOAT31(a) || !ISFLOAT31(b)) return QSFLOAT(0);
  return QSFLOAT(CFLOAT31(a) * CFLOAT31(b));
}

qsptr_t qsop_float_div (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(0);
  if (!ISFLOAT31(a) || !ISFLOAT31(b)) return QSFLOAT(0);
  return QSFLOAT(CFLOAT31(a) / CFLOAT31(b));
}

qsptr_t qsop_float_zero_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!ISFLOAT31(a)) return QSFALSE;
  return QSBOOL(CFLOAT31(a) == 0);
}


/* Quark Float special case: Not-A-Number */
qsptr_t qsop_nan_make (qs_t * machine, qsptr_t args)
{
  return QSFLOAT(NAN);
}

qsptr_t qsop_nan_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  return QSBOOL(QSFLOAT(a) == NAN);
}


/* Quark Float special case: Infinity */
qsptr_t qsop_inf_make (qs_t * machine, qsptr_t args)
{
  return QSFLOAT(INFINITY);
}

qsptr_t qsop_inf_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  return QSBOOL(QSFLOAT(a) == INFINITY);
}


/***************/
/* Quark: iter */
/***************/
qsptr_t qsop_iter_p (qs_t * machine, qsptr_t args)
{
  qsptr_t j = ARG(0);
  return QSBOOL(ISITER28(j));
}

qsptr_t qsop_iter_eq_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!ISITER28(a) || !ISITER28(b)) return QSFALSE;
  return QSBOOL(a == b);
}

qsptr_t qsop_iter_coerce (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  return qsiter_make(machine->store, a);
}

qsptr_t qsop_iter_item (qs_t * machine, qsptr_t args)
{
  qsptr_t j = ARG(0);
  return qsiter_item(machine->store, j);
}

qsptr_t qsop_iter_next (qs_t * machine, qsptr_t args)
{
  qsptr_t j = ARG(0);
  return qsiter_next(machine->store, j);
}




/***********************/
/* Object Manipulation */
/***********************/

qsptr_t qsop_obj_p (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = ARG(0);
  int retval = qsobj_p(machine->store, obj);
  return (retval ? QSTRUE : QSFALSE);
}

qsptr_t qsop_obj_make (qs_t * machine, qsptr_t args)
{
  qsptr_t arg_k = ARG(0);
  qsptr_t arg_octetate = ARG(1);
  qsword k = qsint_get(machine->store, arg_k);
  //qsword octetate = qsbool_get(machine->store, arg_octetate);
  qsword octetate = (arg_octetate != QSFALSE);
  qsptr_t retval = qsobj_make(machine->store, k, octetate, NULL);
  return retval;
}

qsptr_t qsop_obj_ref_ptr (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = ARG(0);
  qsptr_t arg_nth = ARG(1);
  qsword nth = qsint_get(machine->store, arg_nth);
  qsptr_t retval = qsobj_ref_ptr(machine->store, obj, nth);
  return retval;
}
qsptr_t qsop_obj_setq_ptr (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = ARG(0);
  qsptr_t arg_nth = ARG(1);
  qsptr_t val = ARG(2);
  qsword nth = qsint_get(machine->store, arg_nth);
  qsobj_setq_ptr(machine->store, obj, nth, val);
  return obj;
}

qsptr_t qsop_obj_ref_octet (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = ARG(0);
  qsptr_t arg_nth = ARG(1);
  qsword nth = qsint_get(machine->store, arg_nth);
  int octet = qsobj_ref_octet(machine->store, obj, nth);
  return QSINT(octet);
}
qsptr_t qsop_obj_setq_octet (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = ARG(0);
  qsptr_t arg_nth = ARG(1);
  qsptr_t arg_val = ARG(2);
  qsword nth = qsint_get(machine->store, arg_nth);
  int val = qsint_get(machine->store, arg_val);
  qsobj_setq_octet(machine->store, obj, nth, val);
  return obj;
}

qsptr_t qsop_obj_used_p (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = ARG(0);
  int retval = qsobj_used_p(machine->store, obj);
  return retval ? QSTRUE : QSFALSE;
}

qsptr_t qsop_obj_marked_p (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = ARG(0);
  int retval = qsobj_marked_p(machine->store, obj);
  return retval ? QSTRUE : QSFALSE;
}
qsptr_t qsop_obj_setq_marked (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = ARG(0);
  qsptr_t arg_val = ARG(1);
  int val = qsint_get(machine->store, arg_val);
  qsobj_setq_marked(machine->store, obj, val);
  return obj;
}

qsptr_t qsop_obj_ref_allocsize (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = ARG(0);
  qsword retval = qsobj_ref_allocsize(machine->store, obj);
  return QSINT(retval);
}

qsptr_t qsop_obj_ref_parent (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = ARG(0);
  int retval = qsobj_ref_parent(machine->store, obj);
  return QSINT(retval);
}
qsptr_t qsop_obj_setq_parent (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = ARG(0);
  qsptr_t arg_val = ARG(1);
  int val = qsint_get(machine->store, arg_val);
  qsobj_setq_parent(machine->store, obj, val);
  return obj;
}

qsptr_t qsop_obj_ref_score (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = ARG(0);
  int retval = qsobj_ref_score(machine->store, obj);
  return QSINT(retval);
}
qsptr_t qsop_obj_setq_score (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = ARG(0);
  qsptr_t arg_val = ARG(1);
  int val = qsint_get(machine->store, arg_val);
  qsobj_setq_score(machine->store, obj, val);
  return obj;
}



/******************/
/* object: Vector */
/******************/
qsptr_t qsop_vector_p (qs_t * machine, qsptr_t args)
{
  qsptr_t v = ARG(0);
  return QSBOOL(qsvector_p(machine->store, v));
}

qsptr_t qsop_vector_make (qs_t * machine, qsptr_t args)
{
  qsword k = qsint_get(machine->store, ARG(0));
  qsptr_t fill = ARG(1);

  return qsvector_make(machine->store, k, fill);
}

qsptr_t qsop_vector_length (qs_t * machine, qsptr_t args)
{
  qsptr_t v = ARG(0);
  return qsvector_length(machine->store, v);
}

qsptr_t qsop_vector_ref (qs_t * machine, qsptr_t args)
{
  qsptr_t v = ARG(0);
  qsptr_t ptrofs = ARG(1);
  qsword ofs = qsint_get(machine->store, ptrofs);
  return qsvector_ref(machine->store, v, ofs);
}

qsptr_t qsop_vector_setq (qs_t * machine, qsptr_t args)
{
  qsptr_t v = ARG(0);
  qsptr_t ptrofs = ARG(1);
  qsptr_t ptrval = ARG(2);
  qsword ofs = qsint_get(machine->store, ptrofs);
  return qsvector_setq(machine->store, v, ofs, ptrval);
}


/******************/
/* object: string */
/******************/
qsptr_t qsop_str_make (qs_t * machine, qsptr_t args)
{
  qsword k = qsint_get(machine->store, ARG(0));
  qsptr_t ptrfill = ARG(1);
  qsword codepoint = ISNIL(ptrfill) ? 0 : qschar_get(machine->store, ptrfill);
  return qsstr_make(machine->store, k, codepoint);
}

qsptr_t qsop_str_ref (qs_t * machine, qsptr_t args)
{
  qsptr_t sobj = ARG(0);
  qsptr_t nth = ARG(1);

  qsptr_t retval = qsstr_ref(machine->store, sobj, nth);
  return retval;
}

qsptr_t qsop_str_length (qs_t * machine, qsptr_t args)
{
  qsptr_t sobj = ARG(0);

  qsptr_t retval = QSINT(qsstr_length(machine->store, sobj));
  return retval;
}


/***********************/
/* object: utf8 string */
/***********************/
qsptr_t qsop_utf8_p (qs_t * machine, qsptr_t args)
{
  qsptr_t sobj = ARG(0);
  return QSBOOL(qsutf8_p(machine->store, sobj));
}

qsptr_t qsop_utf8_make (qs_t * machine, qsptr_t args)
{
  qsword k = qsint_get(machine->store, ARG(0));
  qsptr_t ptrfill = ARG(1);
  qsword codepoint = ISNIL(ptrfill) ? 0 : qschar_get(machine->store, ptrfill);
  return qsutf8_make(machine->store, k);
}

qsptr_t qsop_utf8_length (qs_t * machine, qsptr_t args)
{
  qsptr_t sobj = ARG(0);

  qsptr_t retval = QSINT(qsutf8_length(machine->store, sobj));
  return retval;
}

qsptr_t qsop_utf8_ref (qs_t * machine, qsptr_t args)
{
  qsptr_t sobj = ARG(0);
  qsptr_t nth = ARG(1);

  int cnth = CINT30(nth);
  qsptr_t retval = qsutf8_ref(machine->store, sobj, cnth);
  return retval;
}

qsptr_t qsop_utf8_setq (qs_t * machine, qsptr_t args)
{
  qsptr_t sobj = ARG(0);
  qsptr_t nth = ARG(1);
  qsptr_t val = ARG(2);

  int cnth = CINT30(nth);
  int cval = CCHAR24(val);

  qsptr_t retval = qsutf8_setq(machine->store, sobj, cnth, cval);
  return retval;
}


/****************/
/* object: Pair */
/****************/
qsptr_t qsop_pair_p (qs_t * machine, qsptr_t args)
{
  qsptr_t p = ARG(0);
  return QSBOOL(qspair_p(machine->store, p));
}

qsptr_t qsop_pair_make (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t d = ARG(1);
  return qspair_make(machine->store, a, d);
}

qsptr_t qsop_pair_car (qs_t * machine, qsptr_t args)
{
  qsptr_t p = ARG(0);
  if (!qspair_p(machine->store, p)) return QSERROR_INVALID;
  return qspair_ref_a(machine->store, p);
}

qsptr_t qsop_pair_cdr (qs_t * machine, qsptr_t args)
{
  qsptr_t p = ARG(0);
  if (!qspair_p(machine->store, p)) return QSERROR_INVALID;
  return qspair_ref_d(machine->store, p);
}

qsptr_t qsop_pair_setcarq (qs_t * machine, qsptr_t args)
{
  qsptr_t p = ARG(0);
  qsptr_t v = ARG(1);
  if (!qspair_p(machine->store, p)) return QSERROR_INVALID;
  return qspair_setq_a(machine->store, p, v);
}

qsptr_t qsop_pair_setcdrq (qs_t * machine, qsptr_t args)
{
  qsptr_t p = ARG(0);
  qsptr_t v = ARG(1);
  if (!qspair_p(machine->store, p)) return QSERROR_INVALID;
  return qspair_setq_d(machine->store, p, v);
}


/******************/
/* object: lambda */
/******************/
qsptr_t qsop_lambda_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  return QSBOOL(qslambda_p(machine->store, a));
}

qsptr_t qsop_lambda_make (qs_t * machine, qsptr_t args)
{
  qsptr_t param = ARG(0);
  qsptr_t body = ARG(1);
  return qslambda_make(machine->store, param, body);
}

qsptr_t qsop_lambda_ref_param (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  return qslambda_ref_body(machine->store, a);
}

qsptr_t qsop_lambda_ref_body (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  return qslambda_ref_body(machine->store, a);
}


/*******************/
/* object: closure */
/*******************/
qsptr_t qsop_closure_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  return QSBOOL(qsclosure_p(machine->store, a));
}

qsptr_t qsop_closure_make (qs_t * machine, qsptr_t args)
{
  qsptr_t env = ARG(0);
  qsptr_t lambda = ARG(1);
  return qsclosure_make(machine->store, env, lambda);
}

qsptr_t qsop_closure_ref_env (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  return qsclosure_ref_env(machine->store, a);
}

qsptr_t qsop_closure_ref_lambda (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  return qsclosure_ref_lambda(machine->store, a);
}


/***********************/
/* object: environment */
/***********************/
qsptr_t qsop_env_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  return QSBOOL(qsenv_p(machine->store, a));
}

qsptr_t qsop_env_make (qs_t * machine, qsptr_t args)
{
  qsptr_t chained = ARG(0);
  return qsenv_make(machine->store, chained);
}

qsptr_t qsop_env_ref (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t key = ARG(1);
  return qsenv_ref(machine->store, a, key);
}

qsptr_t qsop_env_setq (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t key = ARG(1);
  qsptr_t val = ARG(2);
  return qsenv_setq(machine->store, a, key, val);
}


/************************/
/* object: continuation */
/************************/
qsptr_t qsop_kont_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  return QSBOOL(qskont_p(machine->store, a));
}

qsptr_t qsop_kont_make (qs_t * machine, qsptr_t args)
{
  qsptr_t v = ARG(0);
  qsptr_t k = ARG(1);
  qsptr_t e = ARG(2);
  qsptr_t c = ARG(3);
  qsptr_t o = ARG(4);
  if (ISNIL(v))
    {
      return qskont_make(machine->store,
			 QSKONT_LETK,
			 machine->K,
			 machine->E,
			 machine->C,
			 QSNIL);
    }
  else
    {
      return qskont_make(machine->store, v, k, e, c, o);
    }
}

qsptr_t qsop_kont_make_current (qs_t * machine, qsptr_t args)
{
  return qskont_make(machine->store,
		     QSKONT_LETK,
		     machine->K,
		     machine->E,
		     machine->C,
		     QSNIL);
}

qsptr_t qsop_kont_make_halt (qs_t * machine, qsptr_t args)
{
  return qskont_make(machine->store, QSKONT_HALT, QSNIL, QSNIL, QSNIL, QSNIL);
}

qsptr_t qsop_kont_make_letk (qs_t * machine, qsptr_t args)
{
  qsptr_t v = ARG(0);
  qsptr_t c = ARG(1);
  qsptr_t e = ARG(2);
  qsptr_t k = ARG(3);
  return qskont_make(machine->store, QSKONT_LETK, k, e, c, v);
}

qsptr_t qsop_kont_ref_code (qs_t * machine, qsptr_t args)
{
  return qskont_ref_code(machine->store, ARG(0));
}

qsptr_t qsop_kont_ref_env (qs_t * machine, qsptr_t args)
{
  return qskont_ref_env(machine->store, ARG(0));
}

qsptr_t qsop_kont_ref_kont (qs_t * machine, qsptr_t args)
{
  return qskont_ref_kont(machine->store, ARG(0));
}

qsptr_t qsop_kont_ref_other (qs_t * machine, qsptr_t args)
{
  return qskont_ref_other(machine->store, ARG(0));
}


qsptr_t qsop_kont_setq_code (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t v = ARG(1);
  return qskont_setq_code(machine->store, a, v);
}

qsptr_t qsop_kont_setq_env (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t v = ARG(1);
  return qskont_setq_env(machine->store, a, v);
}

qsptr_t qsop_kont_setq_kont (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t v = ARG(1);
  return qskont_setq_kont(machine->store, a, v);
}

qsptr_t qsop_kont_setq_other (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t v = ARG(1);
  return qskont_setq_other(machine->store, a, v);
}












/******************************/
/* Numeric tower in the store */
/******************************/
/* Numeric tower, fixnum64 */
qsptr_t qsop_widenum_p (qs_t * machine, qsptr_t args)
{
  qsptr_t n = ARG(0);
  return QSBOOL(qswidenum_p(machine->store, n));
}


qsptr_t qsop_long_p (qs_t * machine, qsptr_t args)
{
  qsptr_t n = ARG(0);
  return QSBOOL(qslong_p(machine->store, n));
}

qsptr_t qsop_long_coerce (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (qslong_p(machine->store, a)) return a;
  long ca = 0L;
  if (ISINT30(a)) ca = CINT30(a);
  if (ISFLOAT31(a)) ca = (long)CFLOAT31(a);
  else if (ISCHAR24(a)) ca = (long)CCHAR24(a);
  else if (qsdouble_p(machine->store, a)) 
    ca = (long)qsdouble_get(machine->store, a);
  return qslong_make(machine->store, ca);
}

qsptr_t qsop_long_make (qs_t * machine, qsptr_t args)
{
  qsptr_t feed = ARG(0);
  qsptr_t low = ARG(1);
  long ca = 0L;
  if (ISINT30(feed) && ISINT30(low))
    {
      int hi = CINT30(feed);
      int lo = CINT30(low);
      ca = (hi * 0x4000000L) | lo;  /* use multiplication for sign extend */
      return qslong_make(machine->store, ca);
    }
  else
    {
      return qsop_long_coerce(machine, args);
    }
}

qsptr_t qsop_long_neg (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!qslong_p(machine->store, args)) return QSINT(0);
  long ca = qslong_get(machine->store, a);
  return qslong_make(machine->store, -ca);
}

qsptr_t qsop_long_bnot (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!qslong_p(machine->store, args)) return QSINT(0);
  long ca = qslong_get(machine->store, a);
  return qslong_make(machine->store, ~ca);
}

qsptr_t qsop_long_abs (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!qslong_p(machine->store, args)) return QSINT(0);
  long ca = qslong_get(machine->store, a);
  return qslong_make(machine->store, labs(ca));
}

qsptr_t qsop_long_sign (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!qslong_p(machine->store, args)) return QSINT(0);
  long ca = qslong_get(machine->store, a);
  if (ca < 0) return QSINT(-1);
  if (ca > 0) return QSINT(1);
  return QSINT(0);
}

qsptr_t qsop_long_add (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qslong_p(machine->store, a) || !qslong_p(machine->store, b)) return QSINT(0);
  long ca = qslong_get(machine->store, a);
  long cb = qslong_get(machine->store, b);
  return qslong_make(machine->store, ca + cb);
}

qsptr_t qsop_long_sub (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qslong_p(machine->store, a) || !qslong_p(machine->store, b)) return QSINT(0);
  long ca = qslong_get(machine->store, a);
  long cb = qslong_get(machine->store, b);
  return qslong_make(machine->store, ca - cb);
}

qsptr_t qsop_long_mul (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qslong_p(machine->store, a) || !qslong_p(machine->store, b)) return QSINT(0);
  long ca = qslong_get(machine->store, a);
  long cb = qslong_get(machine->store, b);
  return qslong_make(machine->store, ca * cb);
}

qsptr_t qsop_long_div (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qslong_p(machine->store, a) || !qslong_p(machine->store, b)) return QSINT(0);
  long ca = qslong_get(machine->store, a);
  long cb = qslong_get(machine->store, b);
  return qslong_make(machine->store, ca / cb);
}

qsptr_t qsop_long_mod (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qslong_p(machine->store, a) || !qslong_p(machine->store, b)) return QSINT(0);
  long ca = qslong_get(machine->store, a);
  long cb = qslong_get(machine->store, b);
  return qslong_make(machine->store, ca + cb);
}

qsptr_t qsop_long_bor (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qslong_p(machine->store, a) || !qslong_p(machine->store, b)) return QSINT(0);
  long ca = qslong_get(machine->store, a);
  long cb = qslong_get(machine->store, b);
  return qslong_make(machine->store, ca | cb);
}

qsptr_t qsop_long_band (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qslong_p(machine->store, a) || !qslong_p(machine->store, b)) return QSINT(0);
  long ca = qslong_get(machine->store, a);
  long cb = qslong_get(machine->store, b);
  return qslong_make(machine->store, ca & cb);
}

qsptr_t qsop_long_bxor (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qslong_p(machine->store, a) || !qslong_p(machine->store, b)) return QSINT(0);
  long ca = qslong_get(machine->store, a);
  long cb = qslong_get(machine->store, b);
  return qslong_make(machine->store, ca ^ cb);
}

qsptr_t qsop_long_shl (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qslong_p(machine->store, a)) return QSINT(0);
  if (!qslong_p(machine->store, a) || !qslong_p(machine->store, b)) return QSINT(0);
  long ca = qslong_get(machine->store, a);
  long cb = qslong_p(machine->store, b) ? qslong_get(machine->store, b) : ISINT30(b) ? CINT30(b) : 0;
  return qslong_make(machine->store, ca << cb);
}

qsptr_t qsop_long_shr (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qslong_p(machine->store, a) || !qslong_p(machine->store, b)) return QSINT(0);
  long ca = qslong_get(machine->store, a);
  long cb = qslong_p(machine->store, b) ? qslong_get(machine->store, b) : ISINT30(b) ? CINT30(b) : 0;
  return qslong_make(machine->store, ca >> cb);
}

qsptr_t qsop_long_lt_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qslong_p(machine->store, a) || !qslong_p(machine->store, b)) return QSINT(0);
  long ca = qslong_get(machine->store, a);
  long cb = qslong_p(machine->store, b) ? qslong_get(machine->store, b) : ISINT30(b) ? CINT30(b) : 0;
  return QSBOOL(ca < cb);
}

qsptr_t qsop_long_eq_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qslong_p(machine->store, a) || !qslong_p(machine->store, b)) return QSINT(0);
  long ca = qslong_get(machine->store, a);
  long cb = qslong_p(machine->store, b) ? qslong_get(machine->store, b) : ISINT30(b) ? CINT30(b) : 0;
  return QSBOOL(ca == cb);
}

qsptr_t qsop_long_gt_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qslong_p(machine->store, a) || !qslong_p(machine->store, b)) return QSINT(0);
  long ca = qslong_get(machine->store, a);
  long cb = qslong_p(machine->store, b) ? qslong_get(machine->store, b) : ISINT30(b) ? CINT30(b) : 0;
  return QSBOOL(ca > cb);
}


/* Numeric tower, flonum64 */
qsptr_t qsop_double_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  return QSBOOL(qsdouble_p(machine->store, a));
}

qsptr_t qsop_double_coerce (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (qsdouble_p(machine->store, a)) return a;
  double ca = 0;
  if (ISFLOAT31(a)) ca = (double)CFLOAT31(a);
  else if (ISINT30(a)) ca = (double)CINT30(a);
  else if (qslong_p(machine->store, a))
    ca = (double)qslong_get(machine->store, a);
  return qsdouble_make(machine->store, ca);
}

qsptr_t qsop_double_make (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (qsdouble_p(machine->store, a)) return a;
  double ca = 0.;
  if (ISFLOAT31(a)) ca = CFLOAT31(a);
  return qsdouble_make(machine->store, ca);
}

qsptr_t qsop_double_neg (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!qsdouble_p(machine->store, a)) return QSFLOAT(0);
  double ca = qsdouble_get(machine->store, a);
  return qsdouble_make(machine->store, -ca);
}

/*
qsptr_t qsop_double_abs (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!qsdouble_p(machine->store, a)) return QSFLOAT(0);
  double ca = qsdouble_get(machine->store, a);
  return qsdouble_make(machine->store, fabs(ca));
}
*/

qsptr_t qsop_double_sign (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  if (!qsdouble_p(machine->store, a)) return QSFLOAT(0);
  double ca = qsdouble_get(machine->store, a);
  if (ca < 0) return QSFLOAT(-1);
  else if (ca > 0) return QSFLOAT(+1);
  return QSFLOAT(0);
}

qsptr_t qsop_double_add (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qsdouble_p(machine->store,a)||!qsdouble_p(machine->store,b)) return QSFLOAT(0);
  double ca = qsdouble_get(machine->store, a);
  double cb = qsdouble_get(machine->store, b);
  return qsdouble_make(machine->store, ca+cb);
}

qsptr_t qsop_double_sub (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qsdouble_p(machine->store,a)||!qsdouble_p(machine->store,b)) return QSFLOAT(0);
  double ca = qsdouble_get(machine->store, a);
  double cb = qsdouble_get(machine->store, b);
  return qsdouble_make(machine->store, ca+cb);
}

qsptr_t qsop_double_mul (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qsdouble_p(machine->store,a)||!qsdouble_p(machine->store,b)) return QSFLOAT(0);
  double ca = qsdouble_get(machine->store, a);
  double cb = qsdouble_get(machine->store, b);
  return qsdouble_make(machine->store, ca+cb);
}

qsptr_t qsop_double_div (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qsdouble_p(machine->store,a)||!qsdouble_p(machine->store,b)) return QSFLOAT(0);
  double ca = qsdouble_get(machine->store, a);
  double cb = qsdouble_get(machine->store, b);
  return qsdouble_make(machine->store, ca+cb);
}

qsptr_t qsop_double_lt_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qsdouble_p(machine->store,a)||!qsdouble_p(machine->store,b)) return QSFLOAT(0);
  double ca = qsdouble_get(machine->store, a);
  double cb = qsdouble_get(machine->store, b);
  return QSBOOL(ca < cb);
}

qsptr_t qsop_double_eq_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qsdouble_p(machine->store,a)||!qsdouble_p(machine->store,b)) return QSFLOAT(0);
  double ca = qsdouble_get(machine->store, a);
  double cb = qsdouble_get(machine->store, b);
  return QSBOOL(ca == cb);
}

qsptr_t qsop_double_gt_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (!qsdouble_p(machine->store,a)||!qsdouble_p(machine->store,b)) return QSFLOAT(0);
  double ca = qsdouble_get(machine->store, a);
  double cb = qsdouble_get(machine->store, b);
  return QSBOOL(ca > cb);
}


/* maths functions */
LIBM_WRAP1(double, abs, double, abs, QSFLOAT(0));
LIBM_WRAP1(double, round, double, round, QSFLOAT(0));
LIBM_WRAP1(double, floor, double, floor, QSFLOAT(0));
LIBM_WRAP1(double, ceil, double, ceil, QSFLOAT(0));
LIBM_WRAP1(double, trunc, double, trunc, QSFLOAT(0));


/* (exp z) */
LIBM_WRAP1(float, exp, float, expf, QSFLOAT(0));
LIBM_WRAP1(double, exp, double, exp, QSFLOAT(0));
/* (log z) */ /* (log z1 z2) ? */
LIBM_WRAP1(float, log, float, logf, QSFLOAT(0));
LIBM_WRAP1(double, log, double, log, QSFLOAT(0));
/* (sin z) */
LIBM_WRAP1(float, sin, float, sinf, QSFLOAT(0));
LIBM_WRAP1(double, sin, double, sin, QSFLOAT(0));
/* (cos z) */
LIBM_WRAP1(float, cos, float, cosf, QSFLOAT(0));
LIBM_WRAP1(double, cos, double, cos, QSFLOAT(0));
/* (tan z) */
LIBM_WRAP1(float, tan, float, tanf, QSFLOAT(0));
LIBM_WRAP1(double, tan, double, tan, QSFLOAT(0));
/* (asin z) */
LIBM_WRAP1(float, asin, float, asinf, QSFLOAT(0));
LIBM_WRAP1(double, asin, double, asin, QSFLOAT(0));
/* (acos z) */
LIBM_WRAP1(float, acos, float, acosf, QSFLOAT(0));
LIBM_WRAP1(double, acos, double, acos, QSFLOAT(0));
/* (atan z) (atan y x) */
LIBM_WRAP1(float, atan, float, atanf, QSFLOAT(0));
LIBM_WRAP1(double, atan, double, atan, QSFLOAT(0));
LIBM_WRAP2(float, atan2, float, atan2f, QSFLOAT(0));
LIBM_WRAP2(double, atan2, double, atan2, QSFLOAT(0));
/* (sqrt z) */
LIBM_WRAP1(float, sqrt, float, sqrtf, QSFLOAT(0));
LIBM_WRAP1(double, sqrt, double, sqrt, QSFLOAT(0));


/* exceptions-less comparison. */
qsptr_t qsop_float_noexc_lt_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (! qsfloat_p(machine->store, a)) return QSFALSE;
  if (! qsfloat_p(machine->store, b)) return QSFALSE;
  float ca = qsfloat_get(machine->store, a);
  float cb = qsfloat_get(machine->store, b);
  return QSBOOL( isless(ca, cb) );
}
qsptr_t qsop_float_noexc_gt_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (! qsfloat_p(machine->store, a)) return QSFALSE;
  if (! qsfloat_p(machine->store, b)) return QSFALSE;
  float ca = qsfloat_get(machine->store, a);
  float cb = qsfloat_get(machine->store, b);
  return QSBOOL( isgreater(ca, cb) );
}
qsptr_t qsop_float_noexc_eq_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (! qsfloat_p(machine->store, a)) return QSFALSE;
  if (! qsfloat_p(machine->store, b)) return QSFALSE;
  float ca = qsfloat_get(machine->store, a);
  float cb = qsfloat_get(machine->store, b);
  return QSBOOL( islessequal(ca, cb) && isgreaterequal(ca,cb) );
}

qsptr_t qsop_double_noexc_lt_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (! qsdouble_p(machine->store, a)) return QSFALSE;
  if (! qsdouble_p(machine->store, b)) return QSFALSE;
  double ca = qsdouble_get(machine->store, a);
  double cb = qsdouble_get(machine->store, b);
  return QSBOOL( isless(ca, cb) );
}
qsptr_t qsop_double_noexc_gt_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (! qsdouble_p(machine->store, a)) return QSFALSE;
  if (! qsdouble_p(machine->store, b)) return QSFALSE;
  double ca = qsdouble_get(machine->store, a);
  double cb = qsdouble_get(machine->store, b);
  return QSBOOL( isgreater(ca, cb) );
}
qsptr_t qsop_double_noexc_eq_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  if (! qsdouble_p(machine->store, a)) return QSFALSE;
  if (! qsdouble_p(machine->store, b)) return QSFALSE;
  double ca = qsdouble_get(machine->store, a);
  double cb = qsdouble_get(machine->store, b);
  return QSBOOL( islessequal(ca, cb) && isgreaterequal(ca,cb) );
}




qsprimentry_t qsprims0 [] = {
      { "&&",		qsprim_crash },
      { "&.",		qsprim_halt },

      { "&+",		qsop_int_add },
      { "&-",		qsop_int_sub },
      { "&*",		qsop_int_mul },

      /* quark, integer-30 */
      { "&:I",		qsop_int_coerce },
      { "&I?",		qsop_int_p },
      /*  int30 - unary operators. */
      { "&Ineg",	qsop_int_neg },
      { "&Iabs",	qsop_int_abs },
      { "&Isign",	qsop_int_sign },
      /*  int30 - binary operator */
      { "&I+",		qsop_int_add },
      { "&I-",		qsop_int_sub },
      { "&I*",		qsop_int_mul },
      { "&I/",		qsop_int_div },
      { "&I%",		qsop_int_mod },
      /*  int30 - bitwise and logical operators */
      { "&I~",		qsop_int_bnot },
      { "&I|",		qsop_int_bor },
      { "&I&",		qsop_int_band },
      { "&I^",		qsop_int_bxor },
      { "&Ishl",	qsop_int_bshl },
      { "&Ishr",	qsop_int_bshr },
      /*  int30 - comparison */
      { "&I0?",		qsop_int_zero_p },
      { "&I=?",		qsop_int_eq_p },
      { "&I<?",		qsop_int_lt_p },
      { "&I>?",		qsop_int_gt_p },

      /* quark, float-31 */
      { "&:F",		qsop_float_coerce },
      { "&F?",		qsop_float_p },
      /*  float31 - unary operator */
      { "&Fneg",	qsop_float_neg },
      { "&Fsign",	qsop_float_sign },
      { "&Fabs",	qsop_float_abs },
      { "&Fround",	qsop_float_round },
      { "&Ffloor",	qsop_float_floor },
      { "&Fceil",	qsop_float_ceil },
      { "&Ftrunc",	qsop_float_trunc },
      /*  float31 - binary operator */
      { "&F+",		qsop_float_add },
      { "&F-",		qsop_float_sub },
      { "&F*",		qsop_float_mul },
      { "&F/",		qsop_float_div },
      { "&F0?",		qsop_float_zero_p },
      /*  float31 - comparison */
      /*
      { "&F<?",		qsop_float_lt_p },
      { "&F>?",		qsop_float_gt_p },
      { "&F=?",		qsop_float_eq_p },
      */
      { "&F<?",		qsop_float_noexc_lt_p },
      { "&F>?",		qsop_float_noexc_gt_p },
      { "&F=?",		qsop_float_noexc_eq_p },
      /*  NAN-31 */
      { "&Fnan*",	qsop_nan_make },
      { "&Fnan?",	qsop_nan_p },
      /*  INFINITY-31 */
      { "&Finf*",	qsop_inf_make },
      { "&Finf?",	qsop_inf_p },

      /* quark, character-24 */
      { "&:C",		qsop_char_coerce },
      { "&C?",		qsop_char_p },
      { "&C=?",		qsop_char_equal_p },
      { "&C:I",		qsop_char_to_integer },	// TODO: redundant?
      { "&I:C",		qsop_integer_to_char }, // TODO: redundant?

      /* quark, iterator-28 */
      { "&:J",		qsop_iter_coerce },
      { "&J?",		qsop_iter_p },
      { "&J=?",		qsop_iter_eq_p },
      { "&J@a",		qsop_iter_item },
      { "&J@d",		qsop_iter_next },


      /* object, generic, pointer-26 */
      { "&o?",		qsop_obj_p },
      { "&o*",		qsop_obj_make },
      { "&o@p",		qsop_obj_ref_ptr },
      { "&o!p",		qsop_obj_setq_ptr },
      { "&o@o",		qsop_obj_ref_octet },
      { "&o!o",		qsop_obj_setq_octet },
      { "&o@U",		qsop_obj_used_p },
      { "&o@M",		qsop_obj_marked_p },
      { "&o!M",		qsop_obj_setq_marked },
      { "&o@A",		qsop_obj_ref_allocsize },
      { "&o@P",		qsop_obj_ref_parent },
      { "&o!P",		qsop_obj_setq_parent },
      { "&o@S",		qsop_obj_ref_score },
      { "&o!S",		qsop_obj_setq_score },

      /* object, vector */
      { "&v*",		qsop_vector_make },
      { "&v?",		qsop_vector_p },
      { "&v#",		qsop_vector_length },
      { "&v@",		qsop_vector_ref },
      { "&v!",		qsop_vector_setq },

      /* object, utf8 string */
      { "&u*",		qsop_utf8_make },
      { "&u?",		qsop_utf8_p },
      { "&u#",		qsop_utf8_length },
      { "&u@",		qsop_utf8_ref },
      { "&u!",		qsop_utf8_setq },

      /* object, pair */
      { "&p*",		qsop_pair_make },
      { "&p?",		qsop_pair_p },
      { "&p@a",		qsop_pair_car },
      { "&p@d",		qsop_pair_cdr },
      { "&p!a",		qsop_pair_setcarq },
      { "&p!d",		qsop_pair_setcdrq },

      /* object, lambda */
      { "&L*",		qsop_lambda_make },
      { "&L?",		qsop_lambda_p },
      { "&L@p",		qsop_lambda_ref_param },
      { "&L@b",		qsop_lambda_ref_body },

      /* object, closure */
      { "&c*",		qsop_closure_make },
      { "&c?",		qsop_closure_p },
      { "&c@e",		qsop_closure_ref_env },
      { "&c@l",		qsop_closure_ref_lambda },

      /* object, environment */
      { "&e*",		qsop_env_make },
      { "&e?",		qsop_env_p },
      { "&e@",		qsop_env_ref },
      { "&e!",		qsop_env_setq },

      /* object, continuation */
      { "&k*",		qsop_kont_make },
      { "&k?",		qsop_kont_p },
      { "&k@c",		qsop_kont_ref_code },
      { "&k@e",		qsop_kont_ref_env },
      { "&k@k",		qsop_kont_ref_kont },
      { "&k@o",		qsop_kont_ref_other },
      { "&k!c",		qsop_kont_setq_code },
      { "&k!e",		qsop_kont_setq_env },
      { "&k!k",		qsop_kont_setq_kont },
      { "&k!o",		qsop_kont_setq_other },


      /* object, widenum */
      { "&n?",		qsop_widenum_p },

      /* object, widenum, long-64 */
      { "&:nl",		qsop_long_coerce },
      { "&n*l",		qsop_long_make },
      { "&nl?",		qsop_long_p },
      /*  long64 - unary operator */
      { "&nlneg",	qsop_long_neg },
      { "&nlabs",	qsop_long_abs },
      { "&nl~",		qsop_long_bnot },
      { "&nlsig",	qsop_long_sign },
      /*  long64 - binary operator */
      { "&nl+",		qsop_long_add },
      { "&nl-",		qsop_long_sub },
      { "&nl*",		qsop_long_mul },
      { "&nl/",		qsop_long_div },
      { "&nl%",		qsop_long_mod },
      /*  long64 - bitwise and logical operators */
      { "&nl|",		qsop_long_bor },
      { "&nl&",		qsop_long_band },
      { "&nl^",		qsop_long_bxor },
      { "&nlshl",	qsop_long_shl },
      { "&nlshr",	qsop_long_shr },
      /*  long64 - comparison */
      { "&nl<?",	qsop_long_lt_p },
      { "&nl=?",	qsop_long_eq_p },
      { "&nl>?",	qsop_long_gt_p },

      /* object, widenum, double-64 */
      { "&:nd",		qsop_double_coerce },
      { "&n*d",		qsop_double_make },
      { "&nd?",		qsop_double_p },
      { "&ndneg",	qsop_double_neg },
      /*  double64 - unary operator */
      { "&ndabs",	qsop_double_abs },
      { "&ndsig",	qsop_double_sign },
      { "&ndabs",	qsop_double_abs },
      { "&ndround",	qsop_double_round },
      { "&ndceil",	qsop_double_ceil },
      { "&ndfloor",	qsop_double_floor },
      { "&ndtrunc",	qsop_double_trunc },
      /*  double64 - binary operator */
      { "&nd+",		qsop_double_add },
      { "&nd-",		qsop_double_sub },
      { "&nd*",		qsop_double_mul },
      { "&nd/",		qsop_double_div },
      /* TODO: modulo - returns pair (whole portion, fractional portion) */
      /*  double64 - comparison */
      { "&nd<?",	qsop_double_lt_p },
      { "&nd=?",	qsop_double_eq_p },
      { "&nd>?",	qsop_double_gt_p },


      /* Transcendental functions - float31 */
      { "&F.exp",	qsop_float_exp },
      { "&F.log",	qsop_float_log },
      { "&F.sin",	qsop_float_sin },
      { "&F.cos",	qsop_float_cos },
      { "&F.tan",	qsop_float_tan },
      { "&F.asin",	qsop_float_asin },
      { "&F.acos",	qsop_float_acos },
      { "&F.atan",	qsop_float_atan },
      { "&F.atan2",	qsop_float_atan2 },
      { "&F.sqrt",	qsop_float_sqrt },

      /* Transcendental functions - double64 */
      { "&nd.exp",	qsop_double_exp },
      { "&nd.log",	qsop_double_log },
      { "&nd.sin",	qsop_double_sin },
      { "&nd.cos",	qsop_double_cos },
      { "&nd.tan",	qsop_double_tan },
      { "&nd.asin",	qsop_double_asin },
      { "&nd.acos",	qsop_double_acos },
      { "&nd.atan",	qsop_double_atan },
      { "&nd.atan2",	qsop_double_atan2 },
      { "&nd.sqrt",	qsop_double_sqrt },


      { "&s*",		qsop_str_make },
      { "&s#",		qsop_str_length },
      { "&s@",		qsop_str_ref },

      { 0, 0 },
};

int qsprimlib_init(qsprimreg_t * qsprims)
{
  qsprimreg_install_multi(qsprims, 0, qsprims0);
}

