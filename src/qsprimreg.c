#include <stdlib.h>
#include <string.h>

#include "qsprimreg.h"


qsprimreg_t * qsprimreg_init (qsprimreg_t * qsprims)
{
  memset(qsprims, 0, sizeof(qsprimreg_t));
  qsprims->cap = MAX_PRIMS;
  return qsprims;
}

qsprim_f qsprimreg_find (qsprimreg_t * qsprims, const char * opname)
{
  int i;

  for (i = 0; i < qsprims->len; i++)
    {
      qsprimentry_t * entry = qsprims->entries + i;
      if (0 == strncmp(opname, entry->name, MAX_OPNAME_LEN))
	{
	  return entry->f;
	}
    }
  return NULL;
}


int qsprimreg_install (qsprimreg_t * qsprims, const char * opname, qsprim_f opfunc)
{
  if (qsprims->len >= qsprims->cap)
    return -1;

  qsprimentry_t * entry = qsprims->entries + qsprims->len;
  int i = 0;
  for (i = 0; i < MAX_OPNAME_LEN; i++)
    entry->name[i] = 0;
  strncpy(entry->name, opname, MAX_OPNAME_LEN);
  entry->f = opfunc;
  qsprims->len++;

  return 1;
}


int qsprimreg_install_multi (qsprimreg_t * qsprims, int count, const struct qsprimentry_s entrylist[])
{
  int i = 0;
  /* end after 'count', or a NULL entry. */
  for (i = 0; ((count && i < count) || (entrylist[i].f)); i++)
    {
      const char * opname = entrylist[i].name;
      qsprim_f opfunc = entrylist[i].f;
      qsprimreg_install(qsprims, opname, opfunc);
    }
  return 0;
}

