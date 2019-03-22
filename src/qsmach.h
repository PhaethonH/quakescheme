#ifndef QSMACHINE_H_
#define QSMACHINE_H_

#include "qsptr.h"
#include "qsstore.h"

/* Scheme Machine, CESK. */

#define MAX_PRIMS 160

/* operator (underlying operation for a primitive procedure) */
struct qsmachine_s;  // forward-declaration.
typedef qsptr_t (*qsprim_f)(struct qsmachine_s *, qsptr_t args);

typedef struct qsmachine_s {
    bool halt;
    qsptr_t C;
    qsptr_t E;
    qsptr_t K;

    qsstore_t S;

    qsptr_t A;  /* result from most recent evaluation, "Answer" */

    qsptr_t Y;  /* Symbol table (interned symbols). */

    int n_prims;
    qsprim_f prims[MAX_PRIMS];
} qsmachine_t;


qsmachine_t * qsmachine_init (qsmachine_t *);
qsmachine_t * qsmachine_destroy (qsmachine_t *);

qsstore_t * qsmachine_get_store (qsmachine_t *);

int qsmachine_step (qsmachine_t *);
int qsmachine_load (qsmachine_t *, qsptr_t C, qsptr_t E, qsptr_t K);
int qsmachine_applykont (qsmachine_t *, qsptr_t k, qsptr_t value);
int qsmachine_applyproc (qsmachine_t *, qsptr_t proc, qsptr_t values);
qsptr_t qsmachine_eval_atomic (qsmachine_t *, qsptr_t aexp);

/* Primitives Registry */
int qsprimreg_register (qsmachine_t *, qsprim_f op);
qsprim_f qsprimreg_get (const qsmachine_t *, int nth);
int qsprimreg_find (const qsmachine_t *, qsprim_f cfunc);

/* Symbols Store (Symbol Table) */
qsptr_t qssymstore_find_c (const qsmachine_t *, const char * s);
qsptr_t qssymstore_find_utf8 (const qsmachine_t *, qsptr_t utf8s);
qsptr_t qssymstore_insert (qsmachine_t *, qsptr_t symstore, qsptr_t symobj);

/* Garbage collection. */
qserr_t qsgc_trace (qsmachine_t *, qsptr_t root);
qserr_t qsgc_sweep (qsmachine_t *);

#endif /* QSMACHINE_H_ */
