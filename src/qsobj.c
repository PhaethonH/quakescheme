#include "qsobj.h"


int ccons_p (union qsobjform_u * obju)
{
  qsobj_t * obj = &(obju->generic);
  /* cons = pointer-content && _0 is null */
  return (!qsobj_is_octet(obj) && (qsobj_get(obj, 0) == QSNULL));
}

int ccons_get_a (union qsobjform_u * obju)
{
  return qsobj_get(&(obju->generic), 1);
}

int ccons_get_d (union qsobjform_u * obju)
{
  return qsobj_get(&(obju->generic), 2);
}

void ccons_set_a (union qsobjform_u * obju, qsptr_t val)
{
  qsobj_set(&(obju->generic), 1, val);
  return;
}

void ccons_set_d (union qsobjform_u * obju, qsptr_t val)
{
  qsobj_set(&(obju->generic), 2, val);
  return;
}


qsptr_t qscons_p (qsheap_t * heap, qsptr_t q)
{
  return QSNULL;
}

qsptr_t qscons_alloc (qsheap_t * heap, qsptr_t a, qsptr_t d)
{
  return QSNULL;
}

qsptr_t qscons_make (qsheap_t * heap, qsptr_t a, qsptr_t d)
{
  return QSNULL;
}

qsptr_t qscons_get_a (qsheap_t * heap, qsptr_t q)
{
  return QSNULL;
}

qsptr_t qscons_get_d (qsheap_t * heap, qsptr_t q)
{
  return QSNULL;
}

int qscons_crepr (qsheap_t * heap, qsptr_t q, const char *buf, int buflen)
{
  return 0;
}




int cvector_p (union qsobjform_u * obju)
{
  return QSNULL;
}

int cvector_lim (union qsobjform_u * obju)
{
}

int cvector_get (union qsobjform_u * obju, int ofs)
{
}

int cvector_set (union qsobjform_u * obju, int ofs);
{
}

qsptr_t qsvector_p (qsheap_t * heap, qsptr_t q)
{
  return QSNULL;
}

qsptr_t qsvector_alloc (qsheap_t * heap, qsptr_t q)
{
  return QSNULL;
}

qsptr_t qsvector_make (qsheap_t * heap, qsptr_t q)
{
  return QSNULL;
}

qsptr_t qsvector_get (qsheap_t * heap, qsptr_t q, int ofs)
{
  return QSNULL;
}

qsptr_t qsvector_set (qsheap_t * heap, qsptr_t q, int ofs, qsptr_t val)
{
  return QSNULL;
}

int qsvector_crepr (qsheap_t * heap, qsptr_t q, const char * buf, int buflen)
{
  return 0;
}


#endif // _QSOBJ_H_
