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

    qsptr Y;  /* Symbol table (interned symbols). */
} qsmachine_t;


qsmachine_t * qsmachine_init (qsmachine_t *);
qsmachine_t * qsmachine_destroy (qsmachine_t *);

qsstore_t * qsmachine_get_store (qsmachine_t *);

int qsmachine_step (qsmachine_t *);
int qsmachine_load (qsmachine_t *, qsptr C, qsptr E, qsptr K);
int qsmachine_applykont (qsmachine_t *, qsptr kont, qsptr value);
int qsmachine_applyproc (qsmachine_t *, qsptr clo, qsptr values);

qsptr qssymstore_find_c (const qsmachine_t *, const char * s);
qsptr qssymstore_find_utf8 (const qsmachine_t *, qsptr utf8s);
qsptr qssymstore_insert (qsmachine_t *, qsptr symstore, qsptr symobj);

#endif /* QSMACHINE_H_ */
