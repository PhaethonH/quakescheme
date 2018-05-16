#ifndef _QSPRIM_H_
#define _QSPRIM_H_

#include "qsmach.h"

/* QuakeScheme Primitives. */

/* available argument types:
   * w: qsword
   * p: qsptr_t (tagged pointer)
   * i: int
   * s: char*
   * f: float

   Indicates what decoder to use for arguments in calling C function (qsobj.*).
*/



/* Pointer to function providing a primitive. */
typedef qsptr_t (*qsprim_f)(qs_t * machine, qsptr_t args);


#define MAX_OPNAME_LEN 16

/* list of mapping from name to primitive. */
struct qsprimmap_s {
    char name[MAX_OPNAME_LEN];
    qsprim_f f;
};
typedef struct qsprimmap_s qsprimmap_t;

#define MAX_PRIMS 256

extern struct qsprimmap_s qsprims[MAX_PRIMS];


qsprim_f qsprim_find (qs_t * machine, const char * opname);


int qsprim_install (qs_t * machine, const char * name, qsprim_f f);
int qsprim_install_multi (qs_t * achine, int count, struct qsprimmap_s[]);


#endif // _QSPRIM_H_
