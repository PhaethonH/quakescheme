#include <stdlib.h>
#include <string.h>

#include "qsmach.h"
#include "qsprim.h"


qs_t * qs_init (qs_t * machine, qsheap_t * store)
{
  machine->C = QSNIL;
  machine->E = QSNIL;
  machine->K = QSNIL;
  machine->A = QSNIL;
  machine->store = store;
//  machine->store->symstore = qssymstore_make(machine->store);
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
  machine->halt = 0;
  machine->C = exp;
  machine->K = QSNIL;
  machine->A = QSNIL;

  if (!qsenv_p(machine->store, machine->E))
    {
      machine->E = qsenv_make(machine->store, QSNIL);
    }
  return machine;
}


#define qspair_car0(mem,p) (ISNIL(p) ? QSNIL : qspair_ref_a(mem,p))
#define qspair_cdr0(mem,p) (ISNIL(p) ? QSNIL : qspair_ref_d(mem,p))
#define qspair_cadr0(mem,p) (ISNIL(p) ? QSNIL : qspair_car0(mem,qspair_cdr0(mem,p)))
#define qspair_cddr0(mem,p) (ISNIL(p) ? QSNIL : qspair_cdr0(mem,qspair_cdr0(mem,p)))
#define qspair_caddr0(mem,p) (ISNIL(p) ? QSNIL : qspair_car0(mem,qspair_cddr0(mem,p)))
#define qspair_cdddr0(mem,p) (ISNIL(p) ? QSNIL : qspair_cdr0(mem,qspair_cddr0(mem,p)))

#define ARG(n) (qslist_ref(machine->store, args, n))
#define HEAD(p) (qslist_ref(mem, p, 0))
#define TAIL(p) (qslist_tail(mem, p, 1))


/* Operator is a function returning qsptr_t, given machine and arguments. */

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
  qsptr_t a = ARG(0);
  qsptr_t b = ARG(1);
  return (a == b);
}




/* Evaluate atomic expression. */
qsptr_t qs_atomic_eval (qs_t * machine, qsptr_t aexp)
{
  qsptr_t retval = QSNIL;
  char symname[32] = { 0, };
  qsheap_t * mem = machine->store;

  if (qslist_p(mem, aexp))
    {
      qsptr_t head = HEAD(aexp);
      qsptr_t tail = TAIL(aexp);
      if (qssymbol_p(mem, head))
	{
	  qsptr_t qssymname = qssymbol_ref_name(mem, head);
	  qsstr_extract(mem, qssymname, symname, sizeof(symname));
	  if ((0 == strcmp(symname, "lambda"))
	       || (0 == strcmp(symname, "λ")))
	    {
	      qsptr_t param = HEAD(tail);
	      qsptr_t body = HEAD(TAIL(tail));
	      qsptr_t lam = qslambda_make(mem, param, body);
	      qsptr_t clo = qsclosure_make(mem, machine->E, lam);
	      retval = clo;
	    }
	  else
	    {
	      // evaluate as primitive.
	      qsprim_f oper = NULL;
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
	      else
		{
		  oper = qsprim_find(machine, symname);
		}

	      if (oper != NULL)
		{
		  /* args = (value1 .. valueN), valueI=A(αI,ρ,κ) */
		  qsptr_t aiter = tail;
		  qsptr_t args = QSNIL;
		  qsptr_t argiter = QSNIL;
		  while (!ISNIL(aiter))
		    {
		      qsptr_t aexpI = HEAD(aiter);
		      qsptr_t val = qs_atomic_eval(machine, aexpI);
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
		      aiter = TAIL(aiter);
		    }

		  /* O(prim) (value1, value2, .., valueN) */
		  if (oper)
		    {
		      retval = oper(machine, args);
		    }
		  else
		    {
		      retval = QSERROR_INVALID;
		    }
		}
	      else
		{
		  retval = QSERROR_INVALID;
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
int qs_applyop (qs_t * machine, qsprim_f op, qsptr_t args)
{
  return 0;
}

/* Apply Continuation. */
int qs_applykont (qs_t * machine, qsptr_t kont, qsptr_t value)
{
  machine->A = value;
  if (ISNIL(kont))
    {
      machine->halt = 1;
      //machine->C = machine->A;
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

/* Apply Procedure. */
int qs_applyproc (qs_t * machine, qsptr_t proc, qsptr_t args)
{
  qsheap_t * mem = machine->store;

  if (qsclosure_p(mem, proc))
    {
      qsptr_t env = qsclosure_ref_env(mem, proc);
      qsptr_t lam = qsclosure_ref_lambda(mem, proc);

      qsptr_t param = qslambda_ref_param(mem, lam);
      qsptr_t body = qslambda_ref_body(mem, lam);

      env = qsenv_make(mem, env);

      qsptr_t piter = param;
      qsptr_t aiter = args;
      while (!ISNIL(piter) && !ISNIL(aiter))
	{
	  qsptr_t pvalue = HEAD(piter);
	  qsptr_t avalue = HEAD(aiter);
	  qsenv_setq(mem, env, pvalue, avalue);
	  piter = TAIL(piter);
	  aiter = TAIL(aiter);
	}

      machine->C = body;
      machine->E = env;
      /*
	 machine->K = machine->K;
       */
    }
  else if (qskont_p(mem, proc) || ISNIL(proc))
    {
      qs_applykont(machine, proc, HEAD(args));
    }
  else
    {
      /* cannot apply as procedure. */
      machine->A = QSERROR_INVALID;
      machine->K = QSNIL;
      machine->halt = 1;
    }

  return 0;
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
      qs_applykont(machine, machine->K, machine->A);
    }
  else if (qslist_p(mem, C))
    {
      /* Try as non-atomic expression. */

      qsptr_t head = HEAD(C);
      qsptr_t tail = TAIL(C);

      if (qssymbol_p(mem, head))
	{
	  /* maybe special form. */
	  qsptr_t qssymname = qssymbol_ref_name(mem, head);
	  qserror_t res = qsstr_extract(mem, qssymname, symname, sizeof(symname));
	  if (0 == strcmp(symname, "if"))
	    {
	      qsptr_t cond = HEAD(tail);
	      qsptr_t consequent = HEAD(TAIL(tail));
	      qsptr_t alternate = HEAD(TAIL(TAIL(tail)));
	      qsptr_t testval = qs_atomic_eval(machine, cond);
	      if (testval == QSFALSE)
		{
		  machine->C = alternate;
		}
	      else
		{
		  machine->C = consequent;
		}
	      return machine;
	    }
	  else if (0 == strcmp(symname, "let"))
	    {
	      qsptr_t decl = HEAD(tail);
	      qsptr_t dec0 = HEAD(decl);
	      qsptr_t var0 = HEAD(dec0);
	      qsptr_t exp0 = HEAD(TAIL(dec0));
	      qsptr_t body = HEAD(TAIL(tail));

	      qsptr_t k = qskont_make(machine->store, QSKONT_LETK, machine->K, machine->E, body, var0);
	      machine->C = exp0;
	      machine->K = k;

	      return machine;
	    }
	  else if (0 == strcmp(symname, "letrec"))
	    {
	      /* extend the environment. */
	      qsptr_t decl = HEAD(tail);

	      qsptr_t extenv = qsenv_make(mem, machine->E);

	      machine->E = extenv; /* evaluate exp's in extended env. */
	      qsptr_t decN = QSNIL;
	      while (!ISNIL(decl))
		{
		  decN = HEAD(decl);

		  qsptr_t varN = HEAD(decN);
		  qsptr_t expN = HEAD(TAIL(decN));
		  qsptr_t valueN = qs_atomic_eval(machine, expN);
		  machine->E = qsenv_setq(mem, machine->E, varN, valueN);

		  decl = TAIL(decl);
		}

	      qsptr_t body = HEAD(TAIL(tail));
	      machine->C = body;
	      /* E already modified. */
	      /* K remains unmodified. */
	    }
	  else if (0 == strcmp(symname, "set!"))
	    {
	      qsptr_t var = HEAD(tail);
	      qsptr_t aexp = HEAD(TAIL(tail));

	      qsptr_t value = qs_atomic_eval(machine, aexp);
	      machine->E = qsenv_setq(machine->store, machine->E, var, value);

	      qs_applykont(machine, machine->K, QSNIL);  /* void */
	      return machine;
	    }
	  else if (0 == strcmp(symname, "call/cc"))
	    {
	      qsptr_t aexp = HEAD(tail);
	      qsptr_t proc = qs_atomic_eval(machine, aexp);
	      qsptr_t valueCC = machine->K;
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
	  qsptr_t aexp = HEAD(aiter);
	  qsptr_t val = qs_atomic_eval(machine, aexp);
	  if (aiter == C)
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
	  aiter = TAIL(aiter);
	}
      qs_applyproc(machine, proc, args);
    }
  else
    {
      machine->halt = 1;
      machine->A = QSERROR_INVALID;
    }

  return machine;
}




int qs_intern (qs_t * machine, qsptr_t sym)
{
  if (ISNIL(machine->store->symstore))
    {
      machine->store->symstore = qssymstore_make(machine->store);
    }
  qssymstore_intern(machine->store, machine->store->symstore, sym);
  return 1;
}


char dumpbuf[131072];
int qs_dump (qs_t * machine)
{
  memset(dumpbuf, sizeof(dumpbuf), 0);
  int n = 0;
  int dumpbuflen = sizeof(dumpbuf);
  n += snprintf(dumpbuf+n, dumpbuflen-n, "{\n");
  n += snprintf(dumpbuf+n, dumpbuflen-n, "  halt=%d\n", machine->halt);
  n += snprintf(dumpbuf+n, dumpbuflen-n, "  A = %08x\n", machine->A);
  n += snprintf(dumpbuf+n, dumpbuflen-n, "  C = ");
  n += qsptr_crepr(machine->store, machine->C, dumpbuf+n, dumpbuflen-n);
  n += snprintf(dumpbuf+n, dumpbuflen-n, "\n");
  n += snprintf(dumpbuf+n, dumpbuflen-n, "  E = [%d] (%08x, %08x)",
		qsenv_length(machine->store, machine->E),
		machine->E,
		qsenv_ref_next(machine->store, machine->E));
  n += snprintf(dumpbuf+n, dumpbuflen-n, "\n");
  n += snprintf(dumpbuf+n, dumpbuflen-n, "  K = ");
  n += qsptr_crepr(machine->store, machine->K, dumpbuf+n, dumpbuflen-n);
  n += snprintf(dumpbuf+n, dumpbuflen-n, "}\n");

  puts(dumpbuf);
  return n;
}

