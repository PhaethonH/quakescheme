#include <stdio.h>

#include "qsmach.h"
#include "qssexpr.h"


#define SPACELEN 20000
uint8_t _heap1[sizeof(qsstore_t) + SPACELEN*sizeof(qsobj_t)];
qsstore_t *heap1 = (qsstore_t*)&_heap1;

qs_t _scheme1, *scheme1 = &_scheme1;

char line[4096];

int main ()
{
  qsstore_init(heap1, SPACELEN);
  qs_init(scheme1, heap1);
  int looping = 1;

  qsptr_t env0 = qsenv_make(heap1, QSNIL);
  scheme1->E = qsenv_make(heap1, QSNIL);
  qsptr_t sym_toplevel_env = qs_get_symbol(scheme1, "*toplevel-environment*");
  qsenv_setq(heap1, env0, sym_toplevel_env, env0);

  while (looping)
    {
      printf("> ");
      fflush(stdout);

      *line = 0;
      char * result;
      result = fgets(line, sizeof(line), stdin);
      if (result)
	{
	  qsptr_t xt = qssexpr_parse_cstr(heap1, 2, line, NULL);
	  qs_inject_exp(scheme1, xt);
	  scheme1->E = env0;
	  while (!scheme1->halt)
	    {
	      qs_step(scheme1);
	    }
	  *line = 0;
	  qsptr_crepr(heap1, scheme1->A, line, sizeof(line));
	  printf("%s\n", line);
	}
      else
	{
	  looping = 0;
	}
    }
  return 0;
}
