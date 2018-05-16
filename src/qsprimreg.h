#ifndef _QSPRIM_H_
#define _QSPRIM_H_

#include "qsptr.h"

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
struct qs_s;
typedef qsptr_t (*qsprim_f)(struct qs_s * machine, qsptr_t args);


#define MAX_OPNAME_LEN 16

/* list of mapping from name to primitive. */
struct qsprimentry_s {
    char name[MAX_OPNAME_LEN];
    qsprim_f f;
};
typedef struct qsprimentry_s qsprimentry_t;


#define MAX_PRIMS (256)

/* Primitives registry. */
struct qsprimreg_s {
    int cap;
    int len;
    struct qsprimentry_s entries[MAX_PRIMS];
};
typedef struct qsprimreg_s qsprimreg_t;


qsprimreg_t * qsprimreg_init (qsprimreg_t * primreg);
qsprim_f qsprimreg_find (qsprimreg_t * primreg, const char * opname);
int qsprimreg_install (qsprimreg_t * primreg, const char * name, qsprim_f f);
int qsprimreg_install_multi (qsprimreg_t * primreg, int count, const struct qsprimentry_s[]);


#endif // _QSPRIM_H_
