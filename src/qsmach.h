#ifndef QSMACH_H_
#define QSMACH_H_

#include "qsptr.h"
#include "qsheap.h"
#include "qsobj.h"

/* quakescheme machine.
CESK, four components:
 1. Prog = set of programs -- provided from outside (or from tests)
 2. Σ = set of machine states
 3. inject : Prog → Σ = injection function, evaluate program to initial sate
 4. step : Σ → Σ = transition function

*/

/* Component 2: set of machine states. */
typedef struct qs_s {
    int halt;
    qsptr_t A;	/* Answer */

    qsptr_t C;  /* Code */
    qsptr_t E;  /* Environment */
    qsptr_t K;  /* Kontinuation */
    qsheap_t * store;  /* Store */
} qs_t;

qs_t * qs_init (qs_t * machine, qsheap_t * heap);
qs_t * qs_destroy (qs_t * machine);
/* Component 3: Injection, load program into machine. */
/*  from expression - store is already prepared with program. */
qs_t * qs_inject_exp (qs_t * machine, qsptr_t exp);
/* Component 4: Transition, determine next machine state (evaluate state) */
qs_t * qs_step (qs_t * machine);

qsptr_t qs_atomic_eval (qs_t * machine, qsptr_t aexp);


#endif // QSMACH_H_
