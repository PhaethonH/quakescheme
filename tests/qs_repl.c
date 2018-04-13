#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"

/* REPL skeleton. */

#define SPACELEN 4100200

uint8_t _heap1[sizeof(qsheap_t) + SPACELEN*sizeof(qsobj_t)];
qsheap_t *heap1 = (qsheap_t*)&_heap1;

//qs_t _scheme1, *scheme1 = &_scheme1;

char buf[131072];


void init ()
{
  qsheap_init(heap1, SPACELEN);
//  qs_init(scheme1, heap1);
}

int main ()
{
  init();

  qsptr_t obj = QSNIL;

  while (1)
    {
      fprintf(stdout, "qs> ");
      fflush(stdout);
      /* Read */
      /* Eval */
      /* Print */
      /* Loop */
    }
  fprintf(stdout, "\n");

  return 0;
}

