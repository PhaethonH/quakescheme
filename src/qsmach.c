#include "qsmach.h"

qsmachine_t * qsmachine_init (qsmachine_t * mach)
{
  return mach;
}

qsmachine_t * qsmachine_destroy (qsmachine_t * mach)
{
  return mach;
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
