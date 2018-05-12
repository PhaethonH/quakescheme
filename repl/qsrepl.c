#include <stdio.h>

#include "qsmach.h"
#include "qssexpr.h"


#define SPACELEN 20000
uint8_t _heap1[sizeof(qsheap_t) + SPACELEN*sizeof(qsobj_t)];
qsheap_t *heap1 = (qsheap_t*)&_heap1;

qs_t _scheme1, *scheme1 = &_scheme1;

char line[4096];

int main ()
{
  qsheap_init(heap1, SPACELEN);
  qs_init(scheme1, heap1);
  int looping = 1;

  while (looping)
    {
      printf("> ");
      fflush(stdout);

      *line = 0;
      char * result;
      result = fgets(line, sizeof(line), stdin);
      if (result)
	{
	  qsptr_t xt = qssexpr_parse_cstr(heap1, 1, line, NULL);
	  qs_inject_exp(scheme1, xt);
	  while (!scheme1->halt)
	    {
	      qs_step(scheme1);
	    }
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
