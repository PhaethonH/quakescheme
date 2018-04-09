#ifndef QSMACH_H_
#define QSMACH_H_

#include "qsptr.h"
#include "qsheap.h"

/* quakescheme machine.
CESK
*/

typedef struct qs_s {
    qsptr_t C;  /* Code */
    qsptr_t E;  /* Environment */
    qsptr_t K;  /* Kontinuation */
    qsheap_t * store;  /* Store */
} qs_t;

qs_t * qs_init (qs_t * machine, qsheap_t * heap);
qs_t * qs_destroy (qs_t * machine);


#endif // QSMACH_H_
