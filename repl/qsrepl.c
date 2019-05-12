#include <stdio.h>

#include "qsmach.h"
#include "qssexpr.h"

/* Read-Eval-Print-Loop */

#define SPACELEN 20000
qsword _heap0[SPACELEN * sizeof(qsword)] = { 0, };
void * heap0 = &_heap0;

qsmachine_t _machine, *machine = &_machine;

char line[4096];
int looping = 1;

qsptr_t qsprim_quit (qsmachine_t * mach, qsptr_t args)
{
  looping = 0;
  return QSTRUE;
}

int main ()
{
  qsmachine_init(machine);

  qsptr_t env1 = qsprimreg_presets_v1(machine);

  int o_quit = qsprimreg_register(machine, qsprim_quit);
  qsptr_t y_quit = qssymbol_intern_c(machine, "quit", 0);
  env1 = qsenv_insert(machine, env1, y_quit, QSPRIM(o_quit));

  qsptr_t repl_env = qsenv_make(machine, env1);
  machine->E = repl_env;
  qsptr_t sym_toplevel_env = qssymbol_intern_c(machine, "*toplevel-environment*", 0);
  qsenv_insert(machine, repl_env, sym_toplevel_env, env1);

  looping = 1;
  while (looping)
    {
      printf("> ");
      fflush(stdout);

      *line = 0;
      char * result;
      result = fgets(line, sizeof(line), stdin);
      if (result)
	{
	  qsptr_t xt = qssexpr_parse_cstr(machine, 1, line, NULL);
          qsmachine_load(machine, xt, repl_env, QSNIL);
	  while (!machine->halt)
	    {
	      qsmachine_step(machine);
	    }
	  *line = 0;
	  qsptr_crepr(machine, machine->A, line, sizeof(line));
	  printf("%s\n", line);
	}
      else
	{
	  looping = 0;
	}
    }
  return 0;
}
