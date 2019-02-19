#ifndef QSMACHINE_H_
#define QSMACHINE_H_

#include "qsptr.h"
#include "qsstore.h"

/* Scheme Machine, CESK. */

typedef struct qsmachine_s {
    qsptr C;
    qsptr E;
    qsptr K;

    qsstore_t S;

    qsptr A;  /* result from most recent evaluation, "Answer" */
} qsmachine_t;


qsmachine_t * qsmachine_init (qsmachine_t *);
qsmachine_t * qsmachine_destroy (qsmachine_t *);

int qsmachine_step (qsmachine_t *);
int qsmachine_load (qsmachine_t *, qsptr C, qsptr E, qsptr K);
int qsmachine_applykont (qsmachine_t *, qsptr kont, qsptr value);
int qsmachine_applyproc (qsmachine_t *, qsptr clo, qsptr values);

#endif /* QSMACHINE_H_ */
