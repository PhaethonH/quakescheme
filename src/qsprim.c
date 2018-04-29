#include <stdlib.h>
#include <string.h>

#include "qsprim.h"


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
  return qsobj_setq_ptr(machine->store, obj, nth, val);
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
  return qsobj_setq_octet(machine->store, obj, nth, val);
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
  return qsobj_setq_marked(machine->store, obj, val);
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
  return qsobj_setq_parent(machine->store, obj, val);
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
  return qsobj_setq_score(machine->store, obj, val);
}






qsprimmap_t qsprims [MAX_PRIMS] = {
      { "&&",	  qsprim_crash },
      { "&.",	  qsprim_halt },

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

