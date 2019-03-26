#include <errno.h>
#include <stdlib.h>
#include "qsmach.h"
#include "qsval.h"

#define LIST_P(x) (qspair_p(mach,x) || qsiter_p(mach,x))
#define CAR(x) (qspair_p(mach,x) ? qspair_ref_head(mach,x) : qsiter_p(mach,x) ?  qsiter_head(mach,x) : QSNIL)
#define CDR(x) (qspair_p(mach,x) ? qspair_ref_tail(mach,x) : qsiter_p(mach,x) ?  qsiter_tail(mach,x) : QSNIL)


qsmachine_t * qsmachine_init (qsmachine_t * mach)
{
  qsstore_init(&(mach->S));
  mach->C = QSNIL;
  mach->E = QSNIL;
  mach->K = QSNIL;
  mach->A = QSNIL;
  mach->Y = QSNIL;
  mach->n_prims = 0;
  int i;
  for (i = 0; i < MAX_PRIMS; i++)
    {
      mach->prims[i] = NULL;
    }
  return mach;
}

qsmachine_t * qsmachine_destroy (qsmachine_t * mach)
{
  return mach;
}

qsstore_t * qsmachine_get_store (qsmachine_t * mach)
{
  return &(mach->S);
}

int qsmachine_display (qsmachine_t * mach, qsptr_t p)
{
  char buf[1024];
  qsptr_crepr(mach, p, buf, sizeof(buf));
  puts(buf);
}

int qsmachine_eval_lambda (qsmachine_t * mach)
{
  qsptr_t args = CDR(mach->C);
  if (qsnil_p(mach, args))
    {
      /* TODO: exception. */
      return -1;
    }
  qsptr_t param = CAR(args);
  qsptr_t body = CDR(args);
  qsptr_t lam = qslambda_make(mach, param, body);
  qsptr_t clo = qsclosure_make(mach, lam, mach->E);
  mach->A = clo;
  return 0;
}

int qsmachine_applyproc (qsmachine_t * mach, qsptr_t proc, qsptr_t values)
{
  if (qsclosure_p(mach, proc))
    {
      qsptr_t env = qsclosure_ref_env(mach, proc);
      qsptr_t frame = qsenv_make(mach, env);
      qsptr_t lam = qsclosure_ref_lam(mach, proc);
      qsptr_t param = qslambda_ref_param(mach, lam);
      qsptr_t body = qslambda_ref_body(mach, lam);
      qsptr_t paramiter = qsiter_begin(mach, param);
      qsptr_t argiter = qsiter_begin(mach, values);
      while (qsiter_p(mach, paramiter) && qsiter_p(mach, argiter))
	{
	  qsptr_t formal = qsiter_head(mach, paramiter);
	  qsptr_t value = qsiter_head(mach, argiter);
	  env = qsenv_insert(mach, env, formal, value);
	  paramiter = qsiter_tail(mach, paramiter);
	  argiter = qsiter_tail(mach, argiter);
	}
      if (qsiter_p(mach, paramiter) || qsiter_p(mach, argiter))
	{
	  /* TODO: parameter mismatch. */
	}
      mach->C = body;
      mach->E = env;
      mach->K = mach->K;
    }
  else
    {
      return QSERR_FAULT;
    }
  return 0;
}

int qsmachine_applykont (qsmachine_t * mach, qsptr_t k, qsptr_t value)
{
  if (qsnil_p(mach, k))
    {
      /* halt */
      mach->A = value;
      mach->halt = true;
      return 0;
    }
  else
    {
      qsptr_t k_v, k_c, k_e, k_k;
      qskont_fetch(mach, k, &k_v, &k_c, &k_e, &k_k);
      if (qssym_p(mach, k_v) || qsname_p(mach, k_v))
	{
	  /* assigned to variable in environment. */
	  if (qssym_p(mach, k_v)) k_v = qssym_symbol(mach, k_v);
	  mach->E = qsenv_insert(mach, k_e, k_v, value);
	}
      else
	{
	  mach->E = k_e;
	}
      mach->C = k_c;
      mach->K = k_k;
    }
  return 1;
}

qsptr_t qsmachine_eval_atomic (qsmachine_t * mach, qsptr_t arg)
{
  qsptr_t retval = QSNIL;
  if (qspair_p(mach,arg) || qsiter_p(mach,arg))
    {
      /* list, lead with Primitive => apply operator */
      qsptr_t head = CAR(arg);
      if (qsprim_p(mach, head))
	{
	  int primid = qsprim_id(mach, head);
	  qsprim_f op = qsprimreg_get(mach, primid);
	  retval = op(mach, CDR(arg));
	}
      else
	{
	  /* not recognized as Primitive call. */
	  return QSERR_FAULT;
	}
    }
  else if (qslambda_p(mach,arg))
    {
      /* lambda => closure. */
      retval = qsclosure_make(mach, arg, mach->E);
    }
  else if (qsname_p(mach,arg) || qssym_p(mach,arg))
    {
      /* variable => look up in environment. */
      qsptr_t variable = arg;
      if (qsname_p(mach, variable)) variable = qsname_sym(mach, variable);
      retval = qsenv_lookup(mach, mach->E, variable);
    }
  else
    {
      /* else => self. */
      retval = arg;
    }
  return retval;
}

int qsmachine_step (qsmachine_t * mach)
{
  qsptr_t C = mach->C;
  if (qspair_p(mach,C) || qsiter_p(mach,C))
    {
      /* check for forms. */
      qsptr_t head = CAR(C);
      qsptr_t args = CDR(C);
      const char * headname = NULL;
      if (qssym_p(mach,head)) head = qssym_symbol(mach, head);
      if (qsname_p(mach, head)) headname = qsname_get(mach, head);
      if (!headname) headname = "";
      if (0 == strcmp(headname, "if"))
	{
	  qsptr_t aexp = CAR(args);
	  qsptr_t eTRUE = CAR(CDR(args));
	  qsptr_t eFALSE = CAR(CDR(CDR(args)));
	  qsptr_t discriminant = qsmachine_eval_atomic(mach, aexp);
	  if (discriminant != QSFALSE)
	    {
	      mach->C = eTRUE;
	    }
	  else
	    {
	      mach->C = eFALSE;
	    }
	  /* preserve/maintain E and K */
	}
      else if (0 == strcmp(headname, "let"))
	{
	  qsptr_t bindings = CAR(args);
	  qsptr_t body = CAR(CDR(args));
	  qsptr_t variable = CAR(CAR(bindings));
	  qsptr_t exp = CAR(CDR(CAR(bindings)));
	  qsptr_t E = mach->E;
	  qsptr_t K = mach->K;
	  qsptr_t letk = qskont_make(mach, variable, body, E, K);
	  mach->C = exp;
	  /* preserve/maintain E */
	  mach->K = letk;
	}
      else if (0 == strcmp(headname, "letrec"))
	{
	  qsptr_t frame = qsenv_make(mach, mach->E);
	  qsptr_t bindings = CAR(args);
	  qsptr_t body = CAR(CDR(args));
	  qsptr_t binditer = qsiter_begin(mach, bindings);
	  while (qsiter_p(mach, binditer))
	    {
	      qsptr_t bind = qsiter_head(mach, binditer);
	      qsptr_t variable = CAR(bind);
	      qsptr_t aexp = CAR(CDR(bind));
	      qsptr_t value = qsmachine_eval_atomic(mach, aexp);
	      frame = qsenv_insert(mach, frame, variable, value);
	      binditer = qsiter_tail(mach, binditer);
	    }
	  mach->C = body;
	  mach->E = frame;
	}
      else if (0 == strcmp(headname, "set!"))
	{
	  qsptr_t variable = CAR(args);
	  qsptr_t aexp = CAR(CDR(args));
	  qsptr_t value = qsmachine_eval_atomic(mach, aexp);
	  mach->E = qsenv_insert(mach, mach->E, variable, value);
	  mach->A = QSBLACKHOLE;
	  qsmachine_applykont(mach, mach->K, QSBLACKHOLE);
	}
      else if (0 == strcmp(headname, "call/cc"))
	{
	  qsptr_t aexp = CAR(args);
	  qsptr_t proc = qsmachine_eval_atomic(mach, aexp);
	  qsptr_t cc = qskont_make_current(mach);
	  qsptr_t args = qspair_make(mach, cc, QSNIL);
	  qsmachine_applyproc(mach, proc, args);
	}
      else
	{
	  /* else maybe procedure call. */
	  qsptr_t A = qsmachine_eval_atomic(mach, C);
	  if (qserr_p(mach, A))
	    {
	      /* evaluate individual list members. */
	      qsptr_t head = QSBLACKHOLE;
	      qsptr_t root = QSNIL, build = QSNIL;
	      qsptr_t curr = qsiter_begin(mach, C);
	      while (qsiter_p(mach, curr))
		{
		  qsptr_t aexp = qsiter_head(mach, curr);
		  qsptr_t a = qsmachine_eval_atomic(mach, aexp);
		  if (head == QSBLACKHOLE)
		    head = a;
		  if (qsnil_p(mach, root))
		    build = root = qspair_make(mach, a, QSNIL);
		  else
		    {
		      /* append */
		      qsptr_t next = qspair_make(mach, a, QSNIL);
		      qspair_setq_tail(mach, build, next);
		      build = next;
		    }
		  curr = qsiter_tail(mach, curr);
		}
	      mach->A = root;
	      if (qsprim_p(mach, head))
		{
		  /* shift to C to evaluate next cycle. */
		  mach->C = mach->A;
		}
	      else if (qsclosure_p(mach, head))
		{
		  /* evaluate closure. */
		  qsptr_t proc = CAR(mach->A);
		  qsptr_t args = CDR(mach->A);
		  qsmachine_applyproc(mach, proc, args);
		}
	      else if (qskont_p(mach, head))
		{
		  /* evaluate continuation. */
		  qsptr_t cc = CAR(mach->A);
		  qsptr_t a = CAR(CDR(mach->A));
		  qsptr_t k = qskont_ref_k(mach, cc);
		  qsmachine_applykont(mach, k, a);
		}
	    }
	  else
	    {
	      /* evaluated as aexp, proceed with continuation (return). */
	      mach->A = A;
	      qsmachine_applykont(mach, mach->K, mach->A);
	    }
	}
    }
  else
    {
      mach->A = qsmachine_eval_atomic(mach, C);
      /* return. */
      qsmachine_applykont(mach, mach->K, mach->A);
    }
  return 0;
}

int qsmachine_load (qsmachine_t * mach, qsptr_t C, qsptr_t E, qsptr_t K)
{
  mach->C = C;
  mach->E = E;
  mach->K = K;
  mach->halt = false;
  return 0;
}




int qsprimreg_register (qsmachine_t * mach, qsprim_f op)
{
  if (mach->n_prims >= MAX_PRIMS) return -1;
  int primid = mach->n_prims;
  mach->prims[mach->n_prims] = op;
  mach->n_prims++;
  return primid;
}

int qsprimreg_install (qsmachine_t * mach, int nth, qsprim_f op)
{
  if (nth >= MAX_PRIMS) return -1;
  int primid = nth;
  mach->prims[primid] = op;
  if (mach->n_prims <= primid)
    mach->n_prims = primid+1;
  return primid;
}

qsprim_f qsprimreg_get (const qsmachine_t * mach, int nth)
{
  if ((0 <= nth) && (nth < mach->n_prims))
    {
      return mach->prims[nth];
    }
  return NULL;
}

int qsprimreg_find (const qsmachine_t * mach, qsprim_f cfunc)
{
  int primid;
  for (primid = 0; primid < mach->n_prims; primid++)
    {
      if (mach->prims[primid] == cfunc) return primid;
    }
  return -1;
}



/* Operations on Proper Lists. */
bool qslist_p (const qsmachine_t * mach, qsptr_t p)
{
  qsptr_t iter = qsiter_begin(mach, p);
  /* TODO: circular list */
  while (qsiter_p(mach, iter))
    {
      iter = qsiter_tail(mach, iter);
    }
  /* Should end on nil. */
  return qsnil_p(mach, iter);
}

qsword qslist_length (const qsmachine_t * mach, qsptr_t p)
{
  qsword n = 0;
  qsptr_t iter = qsiter_begin(mach, p);
  /* TODO: circular list */
  while (qsiter_p(mach, iter))
    {
      iter = qsiter_tail(mach, iter);
      ++n;
    }
  /* TODO: what to do if improper list? */
  return n;
}

qsptr_t qslist_tail (const qsmachine_t * mach, qsptr_t p, qsword k)
{
  qsword n = k;
  qsptr_t iter = qsiter_begin(mach, p);
  /* TODO: circular list */
  while (n > 0)
    {
      iter = qsiter_tail(mach, iter);
      --n;
    }
  return iter;
}

qsptr_t qslist_ref (const qsmachine_t * mach, qsptr_t p, qsword k)
{
  qsptr_t pair = qslist_tail(mach, p, k);
  if (! qsnil_p(mach, pair))
    return qsiter_head(mach, pair);
  else
    return pair;
}


/* if uintptr_t does not exist, #define as void* */
bool _qssymstore_find_cmp_utf8s (const qsmachine_t * mach, qsptr_t probe, void * criterion)
{
  /* bypass warning about casting 64b pointer to 32b int. */
  qsptr_t utf8s = (qsptr_t)(uintptr_t)(criterion);
  return (QSCMP_EQ == qsname_cmp(mach, probe, utf8s));
}

bool _qssymstore_find_cmp_str (const qsmachine_t * mach, qsptr_t probe, void * criterion)
{
  return (0 == qsname_strcmp(mach, probe, (const char *)criterion));
}

/* Find symbol in symbol store based on a search criterion and a comparator
   function.

   Search criterion may be:
   * C string (const char *) cast to void*
     matched with comparator _qssymtore_find_cmp_str
   * Scheme string (qsptr_t) cast to void*
     matched with comparator _qssymstore_find_cmp_utf8s

   This function is a result of combining find-by-C-string and
   find-by-Scheme-string.  The types used to represent a C string and a Scheme
   string are combined into the generalized "void *", and casting back is
   presumed by the comparator.
 */
qsptr_t _qssymstore_find (const qsmachine_t * mach, void * criterion, bool (*eq)(const qsmachine_t *, qsptr_t, void *))
{
  qsptr_t symstore = mach->Y;
  if (qsnil_p(mach, symstore)) return QSNIL;
  qsptr_t symiter = qspair_iter(mach, symstore);
  qsptr_t probe = QSNIL;
  while (qsiter_p(mach, symiter))
    {
      probe = qsiter_head(mach, symiter);
      if (qsname_p(mach, probe))
	{
	  if (eq(mach, probe, criterion))
	    {
	      qsptr_t retval = qsname_sym(mach, probe);
	      return retval;
	    }
	}
      symiter = qsiter_tail(mach, symiter);
    }
  return QSNIL;
}

/* Find symbol in symbol store based on C string. */
qsptr_t qssymstore_find_c (const qsmachine_t * mach, const char * s)
{
  return _qssymstore_find(mach, (void*)s, _qssymstore_find_cmp_str);
}

/* Find symbol in symbol store based on Scheme string (UTF-8) object. */
qsptr_t qssymstore_find_utf8 (const qsmachine_t * mach, qsptr_t utf8s)
{
  /* bypass warning about casting 32b int to 64b pointer. */
  uintptr_t stuffed = (uintptr_t)utf8s;
  return _qssymstore_find(mach, (void*)stuffed, _qssymstore_find_cmp_utf8s);
}

/* Intern symbol: insert symbol object into symbol store. */
qsptr_t qssymstore_insert (qsmachine_t * mach, qsptr_t symstore, qsptr_t symobj)
{
  qsptr_t pair = qspair_make(mach, symobj, symstore);
  mach->Y = pair;
  return pair;
}



qserr_t qsgc_trace (qsmachine_t * mach, qsptr_t root)
{
  qsaddr_t addr = qsobj_address(mach, root);
  qsstore_trace(&(mach->S), addr, 1);
  return QSERR_OK;
}

qserr_t qsgc_sweep (qsmachine_t * mach)
{
  qsstore_sweep(&(mach->S));
  return QSERR_OK;
}


