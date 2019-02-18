#include "qsmach.h"

machine_t * machine_init (machine_t * mach)
{
  return mach;
}

machine_t * machine_destroy (machine_t * mach)
{
  return mach;
}

int machine_step (machine_t * mach)
{
  return 0;
}

int machine_load (machine_t * mach, qsptr C, qsptr E, qsptr K)
{
  return 0;
}

int machine_applykont (machine_t * mach, qsptr kont, qsptr value)
{
  return 0;
}

int machine_applyproc (machine_t * mach, qsptr clo, qsptr values)
{
  return 0;
}
