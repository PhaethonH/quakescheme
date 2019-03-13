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

int qsmachine_display (qsmachine_t * mach, qsptr p)
{
  char buf[1024];
  qsptr_crepr(mach, p, buf, sizeof(buf));
  puts(buf);
}

int qsmachine_eval_lambda (qsmachine_t * mach)
{
  qsptr args = CDR(mach->C);
  if (ISNIL(args))
    {
      /* TODO: exception. */
      return -1;
    }
  qsptr param = CAR(args);
  qsptr body = CDR(args);
  qsptr lam = qslambda_make(mach, param, body);
  qsptr clo = qsclosure_make(mach, lam, mach->E);
  mach->A = clo;
  return 0;
}

int qsmachine_applyproc (qsmachine_t * mach, qsptr proc, qsptr values)
{
  if (qsclosure_p(mach, proc))
    {
      qsptr env = qsclosure_ref_env(mach, proc);
      qsptr frame = qsenv_make(mach, env);
      qsptr lam = qsclosure_ref_lam(mach, proc);
      qsptr param = qslambda_ref_param(mach, lam);
      qsptr body = qslambda_ref_body(mach, lam);
      qsptr paramiter = qsiter_begin(mach, param);
      qsptr argiter = qsiter_begin(mach, values);
      while (ISITER28(paramiter) && ISITER28(argiter))
	{
	  qsptr formal = qsiter_head(mach, paramiter);
	  qsptr value = qsiter_head(mach, argiter);
	  env = qsenv_insert(mach, env, formal, value);
	  paramiter = qsiter_tail(mach, paramiter);
	  argiter = qsiter_tail(mach, argiter);
	}
      if (ISITER28(paramiter) || ISITER28(argiter))
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

int qsmachine_applykont (qsmachine_t * mach, qsptr k, qsptr value)
{
  if (ISNIL(k))
    {
      /* halt */
      mach->halt = true;
      return 0;
    }
  else
    {
      qsptr k_v, k_c, k_e, k_k;
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

qsptr qsmachine_eval_atomic (qsmachine_t * mach, qsptr arg)
{
  qsptr retval = QSNIL;
  if (qspair_p(mach,arg) || qsiter_p(mach,arg))
    {
      /* list, lead with Primitive => apply operator */
      qsptr head = CAR(arg);
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
      qsptr variable = arg;
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
  qsptr C = mach->C;
  if (qspair_p(mach,C) || qsiter_p(mach,C))
    {
      /* check for forms. */
      qsptr head = CAR(C);
      qsptr args = CDR(C);
      const char * headname = NULL;
      if (qssym_p(mach,head)) head = qssym_symbol(mach, head);
      if (qsname_p(mach, head)) headname = qsname_get(mach, head);
      if (!headname) headname = "";
      if (0 == strcmp(headname, "if"))
	{
	  qsptr aexp = CAR(args);
	  qsptr eTRUE = CAR(CDR(args));
	  qsptr eFALSE = CAR(CDR(CDR(args)));
	  qsptr discriminant = qsmachine_eval_atomic(mach, aexp);
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
	  qsptr bindings = CAR(args);
	  qsptr body = CAR(CDR(args));
	  qsptr variable = CAR(CAR(bindings));
	  qsptr exp = CAR(CDR(CAR(bindings)));
	  qsptr E = mach->E;
	  qsptr K = mach->K;
	  qsptr letk = qskont_make(mach, variable, body, E, K);
	  mach->C = exp;
	  /* preserve/maintain E */
	  mach->K = letk;
	}
      else if (0 == strcmp(headname, "letrec"))
	{
	  qsptr frame = qsenv_make(mach, mach->E);
	  qsptr bindings = CAR(args);
	  qsptr body = CAR(CDR(args));
	  qsptr binditer = qsiter_begin(mach, bindings);
	  while (ISITER28(binditer))
	    {
	      qsptr bind = qsiter_head(mach, binditer);
	      qsptr variable = CAR(bind);
	      qsptr aexp = CAR(CDR(bind));
	      qsptr value = qsmachine_eval_atomic(mach, aexp);
	      frame = qsenv_insert(mach, frame, variable, value);
	      binditer = qsiter_tail(mach, binditer);
	    }
	  mach->C = body;
	  mach->E = frame;
	}
      else if (0 == strcmp(headname, "set!"))
	{
	  qsptr variable = CAR(args);
	  qsptr aexp = CAR(CDR(args));
	  qsptr value = qsmachine_eval_atomic(mach, aexp);
	  mach->E = qsenv_insert(mach, mach->E, variable, value);
	  mach->A = QSBLACKHOLE;
	  qsmachine_applykont(mach, mach->K, QSBLACKHOLE);
	}
      else if (0 == strcmp(headname, "call/cc"))
	{
	  qsptr aexp = args;
	  qsptr proc = qsmachine_eval_atomic(mach, aexp);
	  qsptr cc = qskont_make_current(mach);
	  qsptr args = qspair_make(mach, cc, QSNIL);
	  mach->A = qsmachine_applyproc(mach, proc, args);
	}
      else
	{
	  /* else maybe procedure call. */
	  qsptr A = qsmachine_eval_atomic(mach, C);
	  if (ISERR20(A))
	    {
	      /* evaluate individual list members. */
	      qsptr head = QSBLACKHOLE;
	      qsptr root = QSNIL, build = QSNIL;
	      qsptr curr = qsiter_begin(mach, C);
	      while (ISITER28(curr))
		{
		  qsptr aexp = qsiter_head(mach, curr);
		  qsptr a = qsmachine_eval_atomic(mach, aexp);
		  if (head == QSBLACKHOLE)
		    head = a;
		  if (ISNIL(root))
		    build = root = qspair_make(mach, a, QSNIL);
		  else
		    {
		      /* append */
		      qsptr next = qspair_make(mach, a, QSNIL);
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
		  qsptr proc = CAR(mach->A);
		  qsptr args = CDR(mach->A);
		  qsmachine_applyproc(mach, proc, args);
		}
	    }
	  else
	    {
	      /* evaluated as aexp, proceed with continuation. */
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

int qsmachine_load (qsmachine_t * mach, qsptr C, qsptr E, qsptr K)
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




/* if uintptr_t does not exist, #define as void* */
bool _qssymstore_find_cmp_utf8s (const qsmachine_t * mach, qsptr probe, void * criterion)
{
  /* bypass warning about casting 64b pointer to 32b int. */
  qsptr utf8s = (qsptr)(uintptr_t)(criterion);
  return (QSCMP_EQ == qsname_cmp(mach, probe, utf8s));
}

bool _qssymstore_find_cmp_str (const qsmachine_t * mach, qsptr probe, void * criterion)
{
  return (0 == qsname_strcmp(mach, probe, (const char *)criterion));
}

qsptr _qssymstore_find (const qsmachine_t * mach, void * criterion, bool (*eq)(const qsmachine_t *, qsptr, void *))
{
  qsptr symstore = mach->Y;
  if (ISNIL(symstore)) return QSNIL;
  qsptr symiter = qspair_iter(mach, symstore);
  qsptr probe = QSNIL;
  while (ISITER28(symiter))
    {
      probe = qsiter_head(mach, symiter);
      if (qsname_p(mach, probe))
	{
	  if (eq(mach, probe, criterion))
	    {
	      qsptr retval = qsname_sym(mach, probe);
	      return retval;
	    }
	}
      symiter = qsiter_tail(mach, symiter);
    }
  return QSNIL;
}

qsptr qssymstore_find_c (const qsmachine_t * mach, const char * s)
{
  return _qssymstore_find(mach, (void*)s, _qssymstore_find_cmp_str);
}

qsptr qssymstore_find_utf8 (const qsmachine_t * mach, qsptr utf8s)
{
  /* bypass warning about casting 32b int to 64b pointer. */
  uintptr_t stuffed = (uintptr_t)utf8s;
  return _qssymstore_find(mach, (void*)stuffed, _qssymstore_find_cmp_utf8s);
}

qsptr qssymstore_insert (qsmachine_t * mach, qsptr symstore, qsptr symobj)
{
  qsptr pair = qspair_make(mach, symobj, symstore);
  mach->Y = pair;
  return pair;
}

