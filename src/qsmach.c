#include <stdlib.h>
#include <string.h>

#include "qsmach.h"


qs_t * qs_init (qs_t * machine, qsheap_t * store)
{
  machine->C = QSNIL;
  machine->E = QSNIL;
  machine->K = QSNIL;
  machine->A = QSNIL;
  machine->store = store;
  return machine;
}

qs_t * qs_destroy (qs_t * machine)
{
  // TODO
  return machine;
}




/*
  CESK Machine Steps, summary of http://matt.might.net/articles/cesk-machines/

# Atomic Expressions (α)
A : AExp × Env × Store → Value
## Integer, eval to self		A(z,ρ,σ) = z
## Booleans, eval to self		A(#t,ρ,σ) = #t ; A(#f,ρ,σ) = #f
## Lambdas, eval to closure		A(lam,ρ,σ) = clo(lam,ρ)
## Variables, looked up in environment	A(v,ρ,σ) = σ(ρ(v))
## Primitives, evaluated recursively
  A( [prim aexp1 aexp2 ...], ρ, σ) =
    O(prim) ( A(aexp1,ρ,σ), A(exp2,ρ,σ), ... )
  where O : Prim → (Value* → Value)
    ; operation takes multiple arguments, returns one value.

# Machine Steps
step: Σ → Σ   ; result of evaluating a machine state is another machine state
## Procedure call
  step( '(α0 α1 … αN), ρ, σ, κ ) = applyproc(proc, [v1 ... vN], σ, κ)
  : proc = A(α0,ρ,σ), vI=A(αI,ρ,σ), I=1..N  ; head=proc, tail=arguments
## Return
  step(α, ρ, σ, κ) = applykont(κ, A(α,ρ,σ), σ)  ; atomic's result applied
## Conditional				-- if
  step( '(if α eT eF), ρ, σ, κ ) = 
    (eT, ρ, σ, κ) : A(α,ρ,σ) ≠ #f  ||  (eF, ρ, σ, κ) : otherwise
## Let					-- let
  step( '(let ([v exp]) body), ρ, σ, κ ) = (exp, ρ, σ, κ')
  : k' = letk(v, body, ρ, κ)	-- eval exp, pass result to this kont
## Mutation				-- set!
  step( '(set! v α), ρ, σ, κ ) = applykont(κ, void, σ')
  : σ' = σ[ρ(v) := A(α,ρ,σ)]	  ; next continuation is given value of void
## Recursion				-- letrec
emulated by extending environment first, then eval each expr in extended env.
  step( '(letrec ([v1 α1] … [vN αN]) body), ρ, σ, κ) = (body, ρ', σ', κ)
  : ρ' = ρ[vI → αI], a1..aN are fresh addresses in σ  ; extend environment.
  : σ' = σ[aI → valueI], valueI=A(αI, ρ', σ')	      ; bind in ext.env.
## First-class continuation		-- call/cc
  step( '(call/cc α), ρ, σ, κ ) = applyproc(proc, (vCC), σ, κ)
  : proc = A(α,ρ,σ), vCC = cont(κ)
## Apply Procedure			-- applyproc
applyproc : Value × Value* × Store × Kont → Σ  -- apply procedure to value
  applyproc( clo( '(λ (v1 … vN) body),ρ ) (vI … vN), σ, κ ) = (body, ρ', σ', κ)
  : ρ' = ρ[vI → aI], a1 .. aN are fresh address in σ
  : σ' = σ[aI → vI]	  ; bind formal parameters to arguments
## Apply Continuation			-- applykont
appylykont : Kont × Value × Store → State
  applykont( letk(v,e,ρ,κ), value, σ) = (e, ρ[v→α], σ[a→value], κ)
  : a not in dom(σ), a fresh address
*/


/*
Implementation Modifications

# Hidden register A
Holds most recently evaluated A()
# Call Evaluation		-- callk
-- avoid evalauting all arguments in a single step.
  step( '(α0 α1 … αN), ρ, σ, κ) = (α0, ρ, σ, κ')
    : α0 is symbol
    : κ' = callk( ø, '(α1 … αN), ρ, κ)
  applykont( callk([R0…RN], [P0…PN], ρ, ,κ), value, σ ) =
    ( αI, ρ, σ, κI ) : κI = callk(ready, pending, ρ, κJ)
      : ready = [value R0 … RN]	    ; prepend (cons) R
      : pending = [P1 … PN], N>0    ; tail of P
      : pending = void, N=0	    ; list P exhausted
  applykont( callk([R0..RN], void, ρ, κ), value, σ ) =
    ( [v0…vN], ρ, σ, κ)	    ; resume with eval Prim or eval Proc
      : v0..vN = RN .. R0   ; reverse(R)
# Selection			-- selk
  step( '(if exp eT eF), ρ, σ, κ ) = (exp, ρ, σ, κ')
    : κ' = selk(eT, eF, ρ, κ)
  applykont( selk(eT, eF, σ, κ), value, σ ) =
    (eF, ρ, σ, κ), value == #f  ||  (eT, ρ, σ, κ), value ≠ #f
# Mutation			-- setqk
  step( '(set! v exp), ρ, σ, κ ) = (exp, ρ, σ, κ')
    : κ' = setqk(v, ρ, κ)
  applykont( setqk(v, ρ, κ), value, σ ) = ( `(set! v ,value), ρ, σ, κ)
*/

/*
Hard-coded rewrite rules.
(Should eventually be replaced with Scheme macros)


# Conditional, support complex expression
 * base: (if α eT eF)
 * complex: (if list eT eF) => (let ((_ list)) (if _ eT eF))
# Call/cc, support complex expression
 * base: (call/cc α)
 * complex: (call/cc exp) => (let ((_ exp)) (call/cc _))

# Define
 * (define v exp) => (set! v exp)
 * (define (p0 ...) exp) => (set! p0 (lambda (p1 ...) exp))
*/



/*
Get or make symbol from C space.
*/
qsptr_t qs_symbol (qs_t * machine, const char * symname)
{
  return QSNIL;
}


/*
Machine store is already prepared with static version of program.
So, use entry point for machine program injection.
*/
qs_t * qs_inject_exp (qs_t * machine, qsptr_t exp)
{
  machine->C = exp;
  machine->K = QSNIL;
  machine->A = QSNIL;

  machine->E = qsenv_make(machine->store, QSNIL);
  return machine;
}


#define qspair_car0(mem,p) (ISNIL(p) ? QSNIL : qspair_ref_a(mem,p))
#define qspair_cdr0(mem,p) (ISNIL(p) ? QSNIL : qspair_ref_d(mem,p))
#define qspair_cadr0(mem,p) (ISNIL(p) ? QSNIL : qspair_car0(mem,qspair_cdr0(mem,p)))
#define qspair_cddr0(mem,p) (ISNIL(p) ? QSNIL : qspair_cdr0(mem,qspair_cdr0(mem,p)))
#define qspair_caddr0(mem,p) (ISNIL(p) ? QSNIL : qspair_car0(mem,qspair_cddr0(mem,p)))
#define qspair_cdddr0(mem,p) (ISNIL(p) ? QSNIL : qspair_cdr0(mem,qspair_cddr0(mem,p)))


/* Operator is a function returning qsptr_t, given machine and arguments. */
typedef qsptr_t (*qsop_f)(qs_t *, qsptr_t args);

qsptr_t qsop_crash (qs_t * machine, qsptr_t args)
{
  puts("Crashing.");
  abort();
}

qsptr_t qsop_halt (qs_t * machine, qsptr_t args)
{
  machine->halt = 1;
  machine->K = QSNIL;
  return QSTRUE;
}

qsptr_t qsop_equals (qs_t * machine, qsptr_t args)
{
  qsptr_t a = qspair_car0(machine->store, args);
  qsptr_t b = qspair_cadr0(machine->store, args);
  return (a == b);
}

qsptr_t qsop_obj_p (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = qspair_car0(machine->store, args);
  int retval = qsobj_p(machine->store, obj);
  return (retval ? QSTRUE : QSFALSE);
}

qsptr_t qsop_obj_make (qs_t * machine, qsptr_t args)
{
  qsptr_t arg_k = qspair_car0(machine->store, args);
  qsptr_t arg_octetate = qspair_cadr0(machine->store, args);
  qsword k = qsint_get(machine->store, arg_k);
  //qsword octetate = qsbool_get(machine->store, arg_octetate);
  qsword octetate = (arg_octetate != QSFALSE);
  qsptr_t retval = qsobj_make(machine->store, k, octetate, NULL);
  return retval;
}

qsptr_t qsop_obj_ref_ptr (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = qspair_car0(machine->store, args);
  qsptr_t arg_nth = qspair_cadr0(machine->store, args);
  qsword nth = qsint_get(machine->store, arg_nth);
  qsptr_t retval = qsobj_ref_ptr(machine->store, obj, nth);
  return retval;
}
qsptr_t qsop_obj_setq_ptr (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = qspair_car0(machine->store, args);
  qsptr_t arg_nth = qspair_cadr0(machine->store, args);
  qsptr_t val = qspair_caddr0(machine->store, args);
  qsword nth = qsint_get(machine->store, arg_nth);
  return qsobj_setq_ptr(machine->store, obj, nth, val);
}

qsptr_t qsop_obj_ref_octet (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = qspair_car0(machine->store, args);
  qsptr_t arg_nth = qspair_cadr0(machine->store, args);
  qsword nth = qsint_get(machine->store, arg_nth);
  int octet = qsobj_ref_octet(machine->store, obj, nth);
  return QSINT(octet);
}
qsptr_t qsop_obj_setq_octet (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = qspair_car0(machine->store, args);
  qsptr_t arg_nth = qspair_cadr0(machine->store, args);
  qsptr_t arg_val = qspair_caddr0(machine->store, args);
  qsword nth = qsint_get(machine->store, arg_nth);
  int val = qsint_get(machine->store, arg_val);
  return qsobj_setq_octet(machine->store, obj, nth, val);
}

qsptr_t qsop_obj_used_p (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = qspair_car0(machine->store, args);
  int retval = qsobj_used_p(machine->store, obj);
  return retval ? QSTRUE : QSFALSE;
}

qsptr_t qsop_obj_marked_p (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = qspair_car0(machine->store, args);
  int retval = qsobj_marked_p(machine->store, obj);
  return retval ? QSTRUE : QSFALSE;
}
qsptr_t qsop_obj_setq_marked (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = qspair_car0(machine->store, args);
  qsptr_t arg_val = qspair_cadr0(machine->store, args);
  int val = qsint_get(machine->store, arg_val);
  return qsobj_setq_marked(machine->store, obj, val);
}

qsptr_t qsop_obj_ref_allocsize (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = qspair_car0(machine->store, args);
  qsword retval = qsobj_ref_allocsize(machine->store, obj);
  return QSINT(retval);
}

qsptr_t qsop_obj_ref_parent (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = qspair_car0(machine->store, args);
  int retval = qsobj_ref_parent(machine->store, obj);
  return QSINT(retval);
}
qsptr_t qsop_obj_setq_parent (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = qspair_car0(machine->store, args);
  qsptr_t arg_val = qspair_cadr0(machine->store, args);
  int val = qsint_get(machine->store, arg_val);
  return qsobj_setq_parent(machine->store, obj, val);
}

qsptr_t qsop_obj_ref_score (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = qspair_car0(machine->store, args);
  int retval = qsobj_ref_score(machine->store, obj);
  return QSINT(retval);
}
qsptr_t qsop_obj_setq_score (qs_t * machine, qsptr_t args)
{
  qsptr_t obj = qspair_car0(machine->store, args);
  qsptr_t arg_val = qspair_cadr0(machine->store, args);
  int val = qsint_get(machine->store, arg_val);
  return qsobj_setq_score(machine->store, obj, val);
}





/* Evaluate atomic expression. */
qsptr_t qs_atomic_eval (qs_t * machine, qsptr_t aexp)
{
  qsptr_t retval = QSNIL;
  char symname[32];
  qsheap_t * mem = machine->store;

  if (qspair_p(mem, aexp))
    {
      qsptr_t head = qspair_car0(mem, aexp);
      qsptr_t tail = qspair_car0(mem, qspair_cdr0(mem, aexp));
      if (qssymbol_p(mem, head))
	{
	  qsptr_t qssymname = qssymbol_ref_name(mem, head);
	  qsstr_extract(mem, qssymname, symname, sizeof(symname));
	  if ((0 == strcmp(symname, "lambda"))
	       || (0 == strcmp(symname, "λ")))
	    {
	      qsptr_t param = qspair_car0(mem, tail);
	      qsptr_t body = qspair_car0(mem, qspair_cdr0(mem, tail));
	      qsptr_t lam = qslambda_make(mem, param, body);
	      qsptr_t clo = qsclosure_make(mem, machine->E, lam);
	      retval = clo;
	    }
	  else
	    {
	      // evaluate as primitive.
	      qsop_f oper = NULL;
	      qsptr_t prim = head;
	      if (0 == strcmp(symname, "+"))
		{
		}
	      else if (0 == strcmp(symname, "-"))
		{
		}
	      else if (0 == strcmp(symname, "*"))
		{
		}
	      else if (0 == strcmp(symname, "="))
		{
		  oper = qsop_equals;
		}
	      else if (0 == strcmp(symname, "&&"))
		{
		  oper = qsop_crash;
		}
	      else if (0 == strcmp(symname, "&."))
		{
		  oper = qsop_halt;
		}
	      else if (0 == strcmp(symname, "&o*"))
		{
		  oper = qsop_obj_make;
		}
	      else if (0 == strcmp(symname, "&o?"))
		{
		  oper = qsop_obj_p;
		}
	      else if (0 == strcmp(symname, "&o@p"))
		{
		  oper = qsop_obj_ref_ptr;
		}
	      else if (0 == strcmp(symname, "&o!p"))
		{
		  oper = qsop_obj_setq_ptr;
		}
	      else if (0 == strcmp(symname, "&o@o"))
		{
		  oper = qsop_obj_ref_octet;
		}
	      else if (0 == strcmp(symname, "&o!o"))
		{
		  oper = qsop_obj_setq_octet;
		}
	      else if (0 == strcmp(symname, "&o@M"))
		{
		  oper = qsop_obj_marked_p;
		}
	      else if (0 == strcmp(symname, "&o!M"))
		{
		  oper = qsop_obj_setq_marked;
		}
	      else if (0 == strcmp(symname, "&o@P"))
		{
		  oper = qsop_obj_ref_parent;
		}
	      else if (0 == strcmp(symname, "&o!P"))
		{
		  oper = qsop_obj_setq_parent;
		}
	      else if (0 == strcmp(symname, "&o@S"))
		{
		  oper = qsop_obj_ref_score;
		}
	      else if (0 == strcmp(symname, "&o!S"))
		{
		  oper = qsop_obj_setq_score;
		}

	      if (oper != NULL)
		{
		  /* args = (value1 .. valueN), valueI=A(αI,ρ,κ) */
		  qsptr_t aiter = tail;
		  qsptr_t args = QSNIL;
		  qsptr_t argiter = QSNIL;
		  while (!ISNIL(aiter))
		    {
		      qsptr_t aexp = qspair_car0(mem, aiter);
		      qsptr_t val = qs_atomic_eval(machine, aexp);
		      qsptr_t next = qspair_make(mem, val, QSNIL);
		      if (ISNIL(args))
			{
			  argiter = args = next;
			}
		      else
			{
			  qspair_setq_d(mem, argiter, next);
			  argiter = next;
			}
		    }

		  /* O(prim) (value1, value2, .., valueN) */
		  retval = oper(machine, args);
		}
	    }
	}
      // else if qsprimitive_p(mem, head)
    }
  else if (qssymbol_p(mem, aexp))
    {
      /* look up in environment; maybe error. */
      retval = qsenv_ref(mem, machine->E, aexp);
    }
  else
    {
      /* evaluate to self. */
      retval = aexp;
    }

  return retval;
}


/* Apply Operation/Primitive. */
int qs_applyop (qs_t * machine, qsop_f op, qsptr_t args)
{
  return 0;
}

/* Apply Procedure. */
int qs_applyproc (qs_t * machine, qsptr_t proc, qsptr_t args)
{
  qsheap_t * mem = machine->store;

  qsptr_t env = qsclosure_ref_env(mem, proc);
  qsptr_t lam = qsclosure_ref_lambda(mem, proc);

  qsptr_t param = qslambda_ref_param(mem, lam);
  qsptr_t body = qslambda_ref_body(mem, lam);

  env = qsenv_make(mem, env);

  qsptr_t piter = param;
  qsptr_t aiter = args;
  while (!ISNIL(piter) && !ISNIL(aiter))
    {
      qsptr_t pvalue = qspair_car0(mem, piter);
      qsptr_t avalue = qspair_car0(mem, aiter);
      qsenv_setq(mem, env, pvalue, avalue);
    }

  machine->C = body;
  machine->E = env;
  /*
  machine->K = machine->K;
  */

  return 0;
}

/* Apply Continuation. */
int qs_applykont (qs_t * machine, qsptr_t kont, qsptr_t value)
{
  //machine->A = value;
  if (ISNIL(kont))
    {
      machine->halt = 1;
      return 0;
    }
  else
    {
      qsheap_t * mem = machine->store;
      qsptr_t varname = qskont_ref_other(mem, kont);
      qsptr_t body = qskont_ref_code(mem, kont);
      qsptr_t env = qskont_ref_env(mem, kont);
      qsptr_t nextk = qskont_ref_kont(mem, kont);

      /*
## Apply Continuation			-- applykont
appylykont : Kont × Value × Store → State
  applykont( letk(v,e,ρ,κ), value, σ) = (e, ρ[v→α], σ[a→value], κ)
  : a not in dom(σ), a fresh address
  */

      env = qsenv_make(mem, env);
      env = qsenv_setq(mem, env, varname, value);

      machine->C = body;
      machine->E = env;
      machine->K = nextk;
      return 0;
    }
}


/* State transition. */
qs_t * qs_step (qs_t * machine)
{
  qsheap_t * mem = machine->store;
  char symname[32] = { 0, };
  qsptr_t C = machine->C;

  /* Try to evaluate as atom. */
  qsptr_t atomic = qs_atomic_eval(machine, C);
  if (atomic != QSERROR_INVALID)
    {
      machine->A = atomic;
    }
  else if (qspair_p(mem, C))
    {
      /* Try as non-atomic expression. */

      qsptr_t head = qspair_car0(mem, C);
      qsptr_t tail = qspair_cdr0(mem, C);

      if (qssymbol_p(mem, head))
	{
	  /* maybe special form. */
	  qsptr_t qssymname = qssymbol_ref_name(mem, head);
	  qserror_t res = qsstr_extract(mem, qssymname, symname, sizeof(symname));
	  if (0 == strcmp(symname, "if"))
	    {
	      qsptr_t cond = qspair_car0(mem, tail);
	      qsptr_t consequent = qspair_car0(mem, qspair_cdr0(mem, tail));
	      qsptr_t alternate = qspair_car0(mem, qspair_cdr0(mem, qspair_cdr0(mem, tail)));
	      if (cond == QSFALSE)
		{
		  machine->C = alternate;
		}
	      else
		{
		  machine->C = consequent;
		}
	      return machine;
	    }
	  else if (0 == strcmp(symname, "set!"))
	    {
	      qsptr_t varname = qspair_car0(mem, tail);
	      qsptr_t aexp = qspair_car0(mem, qspair_cdr0(mem, tail));
	      qs_applykont(machine, machine->K, QSNIL);  /* void */
	      return machine;
	    }
	  else if (0 == strcmp(symname, "call/cc"))
	    {
	      qsptr_t proc = qspair_car0(mem, tail);
	      qsptr_t valueCC = qskont_make(mem, QST_KONT, machine->E, machine->K, machine->C, QSNIL);
	      qsptr_t args = qspair_make(mem, valueCC, QSNIL);
	      qs_applyproc(machine, proc, args);
	      return machine;
	    }
	}

      /* procedure call form. */
      qsptr_t proc = QSNIL;
      qsptr_t aiter = C;
      qsptr_t args = QSNIL;
      qsptr_t argiter = QSNIL;
      while (!ISNIL(aiter))
	{
	  qsptr_t aexp = qspair_car0(mem, aiter);
	  qsptr_t val = qs_atomic_eval(machine, aexp);
	  if (ISNIL(proc))
	    {
	      proc = val;
	    }
	  else if (ISNIL(args))
	    {
	      argiter = args = qspair_make(mem, val, QSNIL);
	    }
	  else
	    {
	      qsptr_t next = qspair_make(mem, val, QSNIL);
	      qspair_setq_d(mem, argiter, next);
	      argiter = next;
	    }
	  aiter = qspair_cdr0(mem, aiter);
	}
      qs_applyproc(machine, proc, args);
    }

  return machine;
}



