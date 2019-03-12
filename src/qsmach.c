#include "qsmach.h"
#include "qsval.h"

qsmachine_t * qsmachine_init (qsmachine_t * mach)
{
  qsstore_init(&(mach->S));
  mach->C = QSNIL;
  mach->E = QSNIL;
  mach->K = QSNIL;
  mach->A = QSNIL;
  mach->Y = QSNIL;
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

int qsmachine_step (qsmachine_t * mach)
{
  return 0;
}

int qsmachine_load (qsmachine_t * mach, qsptr C, qsptr E, qsptr K)
{
  return 0;
}

int qsmachine_applykont (qsmachine_t * mach, qsptr kont, qsptr value)
{
  return 0;
}

int qsmachine_applyproc (qsmachine_t * mach, qsptr clo, qsptr values)
{
  return 0;
}



/* if uintptr_t does not exist, #define as void* */
bool _qssymstore_find_cmp_utf8s (const qsmachine_t * mach, qsptr probe, void * criterion)
{
  /* bypass warning about casting 64b pointer to 32b int. */
  qsptr utf8s = (qsptr)(uintptr_t)(criterion);
  return (QSCMP_EQ == qssymbol_cmp(mach, probe, utf8s));
}

bool _qssymstore_find_cmp_str (const qsmachine_t * mach, qsptr probe, void * criterion)
{
  return (0 == qssymbol_strcmp(mach, probe, (const char *)criterion));
}

qsptr _qssymstore_find (const qsmachine_t * mach, void * criterion, bool (*cmp)(const qsmachine_t *, qsptr, void *))
{
  qsptr symstore = mach->Y;
  if (ISNIL(symstore)) return QSNIL;
  qsptr symiter = qspair_iter(mach, symstore);
  qsptr probe = QSNIL;
  while (ISITER28(symiter))
    {
      probe = qsiter_head(mach, symiter);
      if (qssymbol_p(mach, probe))
	{
	  if (cmp(mach, probe, criterion))
	    {
	      return probe;
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

