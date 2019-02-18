#ifndef QSMACHINE_H_
#define QSMACHINE_H_

#include "qsptr.h"
#include "qsstore.h"

/* Scheme Machine, CESK. */

typedef struct machine_s {
    qsptr C;
    qsptr E;
    qsptr K;

    qsstore_t S;

    qsptr A;  /* result from most recent evaluation, "Answer" */
} machine_t;


machine_t * machine_init (machine_t *);
machine_t * machine_destroy (machine_t *);

int machine_step (machine_t *);
int machine_load (machine_t *, qsptr C, qsptr E, qsptr K);
int machine_applykont (machine_t *, qsptr kont, qsptr value);
int machine_applyproc (machine_t *, qsptr clo, qsptr values);

#endif /* QSMACHINE_H_ */
