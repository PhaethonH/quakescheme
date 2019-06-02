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
    /* Control */
    qsptr_t C;
    /* Environment */
    qsptr_t E;
    /* Kontinuation */
    qsptr_t K;

    /* Store */
    qsstore_t S;

    qsptr_t A;  /* result from most recent evaluation, "Answer" */

    qsptr_t Y;  /* Symbol table (interned symbols). */

    /* Registry of Primitives.
       The Primitive-Operation objects hold an index into this table.
     */
    int n_prims;
    qsprim_f prims[MAX_PRIMS];
} qsmachine_t;


extern qsmachine_t * qsmachine_init (qsmachine_t *);
extern qsmachine_t * qsmachine_destroy (qsmachine_t *);

/* Retrieve reference to the Store (main memory). */
extern qsstore_t * qsmachine_get_store (qsmachine_t *);

/* Display Scheme value to stdout; primarily for debugging purposes. */
extern int qsmachine_display (qsmachine_t *, qsptr_t p);
/* One evaluation cycle. */
extern int qsmachine_step (qsmachine_t *);
/* Allow for injecting programs into the machine. */
extern int qsmachine_load (qsmachine_t *, qsptr_t C, qsptr_t E, qsptr_t K);
/* applykont() - apply pending continuation. */
extern int qsmachine_applykont (qsmachine_t *, qsptr_t k, qsptr_t value);
/* applyproc() - apply procedure call. */
extern int qsmachine_applyproc (qsmachine_t *, qsptr_t proc, qsptr_t values);
/* eval_atomic() - evaluate an expression atomically (fails if not atomic). */
extern qsptr_t qsmachine_eval_atomic (qsmachine_t *, qsptr_t aexp);

/* Primitives Registry */
extern int qsprimreg_register (qsmachine_t *, qsprim_f op);
extern int qsprimreg_install (qsmachine_t *, int nth, qsprim_f op);
extern qsprim_f qsprimreg_get (const qsmachine_t *, int nth);
extern int qsprimreg_find (const qsmachine_t *, qsprim_f cfunc);
extern qsptr_t qsprimreg_presets_v0 (qsmachine_t *);
extern qsptr_t qsprimreg_presets_v1 (qsmachine_t *);

/* Operations on (proper) Lists. */
extern bool qslist_p (const qsmachine_t *, qsptr_t p);
extern qsword qslist_length (const qsmachine_t *, qsptr_t p);
extern qsptr_t qslist_tail (const qsmachine_t *, qsptr_t p, qsword k);
extern qsptr_t qslist_ref (const qsmachine_t *, qsptr_t p, qsword k);

/* Symbols Store (Symbol Table) */
extern qsptr_t qssymstore_find_c (const qsmachine_t *, const char * s);
extern qsptr_t qssymstore_find_utf8 (const qsmachine_t *, qsptr_t utf8s);
extern qsptr_t qssymstore_insert (qsmachine_t *, qsptr_t symstore, qsptr_t symobj);

/* Garbage collector. */
extern qserr_t qsgc_trace (qsmachine_t *, qsptr_t root);
extern qserr_t qsgc_sweep (qsmachine_t *);

#endif /* QSMACHINE_H_ */
