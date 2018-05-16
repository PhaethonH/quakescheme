#include <stdlib.h>
#include <string.h>

#include "qsprim.h"

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




qsptr_t qsop_int_eq_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  return QSBOOL(a == b);
}

qsptr_t qsop_int_lt_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  return QSBOOL(a < b);
}

qsptr_t qsop_int_gt_p (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  return QSBOOL(a > b);
}

qsptr_t qsop_int_add (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  return QSINT(CINT30(a) + CINT30(b));
}

qsptr_t qsop_int_sub (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  return QSINT(CINT30(a) - CINT30(b));
}

qsptr_t qsop_int_mul (qs_t * machine, qsptr_t args)
{
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  return QSINT(CINT30(a) * CINT30(b));
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




qsprimmap_t qsprims [MAX_PRIMS] = {
      { "&&",	  qsprim_crash },
      { "&.",	  qsprim_halt },

      { "&+",	  qsop_int_add },
      { "&-",	  qsop_int_sub },
      { "&*",	  qsop_int_mul },

      { "&o?",	  qsop_obj_p },
      { "&o*",	  qsop_obj_make },
      { "&o@p",	  qsop_obj_ref_ptr },
      { "&o!p",	  qsop_obj_setq_ptr },
      { "&o@o",	  qsop_obj_ref_octet },
      { "&o!o",	  qsop_obj_setq_octet },
      { "&o@U",	  qsop_obj_used_p },
      { "&o@M",	  qsop_obj_marked_p },
      { "&o!M",	  qsop_obj_setq_marked },
      { "&o@A",	  qsop_obj_ref_allocsize },
      { "&o@P",	  qsop_obj_ref_parent },
      { "&o!P",	  qsop_obj_setq_parent },
      { "&o@S",	  qsop_obj_ref_score },
      { "&o!S",	  qsop_obj_setq_score },

      { "&C=?",	  qsop_char_equal_p },
      { "&C:I",	  qsop_char_to_integer },

      { "&I=?",	  qsop_int_eq_p },
      { "&I<?",	  qsop_int_lt_p },
      { "&I>?",	  qsop_int_gt_p },
      { "&I+",	  qsop_int_add },
      { "&I-",	  qsop_int_sub },
      { "&I*",	  qsop_int_mul },

      { "&s@",	  qsop_str_ref },
      { "&s#",	  qsop_str_length },

      { 0, 0 },
};

qsprim_f qsprim_find (qs_t * machine, const char * opname)
{
  int i;
  for (i = 0; qsprims[i].f != NULL; i++)
    {
      if (0 == strncmp(opname, qsprims[i].name, MAX_OPNAME_LEN))
	{
	  return qsprims[i].f; }
    }
  return NULL;
}


















int qsprim_install (qs_t * machine, const char * opname, qsprim_f opfunc)
{
  return 0;
}


int qsprim_install_multi (qs_t * machine, int count, qsprimmap_t * primlist)
{
  int i = 0;
  /* end after 'count', or a NULL entry. */
  while ((count && i < count) || (primlist[i].f))
    {
      const char * opname = primlist[i].name;
      qsprim_f opfunc = primlist[i].f;
      qsprim_install(machine, opname, opfunc);
    }
  return 0;
}

