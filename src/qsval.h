#ifndef QSVAL_H_
#define QSVAL_H_

#include <stdbool.h>
#include <stdint.h>

#include "qsmach.h"

/* Scheme values, combining single-word values and in-heap values. */

/* identify function, for use in table-of-functions. */
qsptr qsptr_make (qsmachine_t *, qsptr);

/* Directly encoded values. */

qsptr qsbool_make (qsmachine_t *, int val);
bool qsbool_p (const qsmachine_t *, qsptr p);
int qsbool_crepr (const qsmachine_t *, qsptr p, char *buf, int buflen);

qsptr qsfloat_make (qsmachine_t *, float val);
bool qsfloat_p (const qsmachine_t *, qsptr p);
int qsfloat_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsint_make (qsmachine_t *, int32_t val);
bool qsint_p (const qsmachine_t *, qsptr p);
int qsint_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qschar_make (qsmachine_t *, int val);
bool qschar_p (const qsmachine_t *, qsptr p);
int qschar_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsconst_make (qsmachine_t *, int const_id);
bool qsconst_p (const qsmachine_t *, qsptr p);
int qsconst_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsfd_make (qsmachine_t *, int val);
bool qsfd_p (const qsmachine_t *, qsptr p);
int qsfd_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsprim_make (qsmachine_t *, qsword primid);
bool qsprim_p (const qsmachine_t *, qsptr p);
int qsprim_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);


/* Heaped object */

qsptr qsobj_make (qsmachine_t *, qsword obj_id);

qsptr qssym_make (qsmachine_t *, qsword sym_id);
bool qssym_p (const qsmachine_t *, qsptr p);
int qssym_crepr (qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qspair_make (qsmachine_t *, qsptr a, qsptr d);
bool qspair_p (const qsmachine_t *, qsptr p);
qserr qspair_ref_head (const qsmachine_t *, qsptr p);
qserr qspair_ref_tail (const qsmachine_t *, qsptr p);
qserr qspair_setq_head (qsmachine_t *, qsptr p, qsptr a);
qserr qspair_setq_tail (qsmachine_t *, qsptr p, qsptr d);
#define qspair_car qspair_ref_head
#define qspair_cdr qspair_ref_tail
#define qspair_setcarq qspair_setq_head
#define qspair_setcdrq qspair_setq_tail
int qspair_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsvector_make (qsmachine_t *, qsword len, qsptr fill);
bool qsvector_p (const qsmachine_t *, qsptr p);
qsptr qsvector_length (const qsmachine_t *, qsptr p);
qsptr qsvector_ref (const qsmachine_t *, qsptr p, qsword k);
qsptr qsvector_setq (qsmachine_t *, qsptr p, qsword k, qsptr val);
int qsvector_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qscptr_make (qsmachine_t *, void * val);
bool qscptr_p (const qsmachine_t *, qsptr p);
void * qsctpr_get (const qsmachine_t *, qsptr p);
int qsctpr_fetch (const qsmachine_t *, qsptr p, void * out);
int qscptr_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qslong_make (qsmachine_t *, int64_t val);
bool qslong_p (const qsmachine_t *, qsptr p);
int64_t qslong_get (const qsmachine_t *, qsptr p);
int qslong_fetch (const qsmachine_t *, qsptr p, int64_t * out);
int qslong_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsdouble_make (qsmachine_t *, double val);
bool qsdouble_p (const qsmachine_t *, qsptr p);
double qsdouble_get (const qsmachine_t *, qsptr p);
int qsdouble_fetch (const qsmachine_t *, qsptr p, double * out);
int qsdouble_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

/* symbol object links symbol id (qsint) to symbol name (qsstring) */
qsptr qssymbol_make (qsmachine_t *, qsptr name);
bool qssymbol_p (const qsmachine_t *, qsptr p);
int qssymbol_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsstring_make (qsmachine_t *, qsword len, int fill);
bool qsstring_p (const qsmachine_t *, qsptr p);
int qsstring_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsbytevec_make (qsmachine_t *, qsword len, qsbyte fill);
bool qsbytevec_p (const qsmachine_t *, qsptr p);
int qsbytevec_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsenv_make (qsmachine_t *, qsptr next_env);
qsptr qsenv_insert (qsmachine_t *, qsptr variable, qsptr binding);
int qsenv_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qskont_make (qsmachine_t *, qsptr variant, qsptr c, qsptr e, qsptr k, qsptr extra);
bool qskont_p (const qsmachine_t *, qsptr p);
qsptr qskont_ref_c (const qsmachine_t *, qsptr p);
qsptr qskont_ref_e (const qsmachine_t *, qsptr p);
qsptr qskont_ref_k (const qsmachine_t *, qsptr p);
qsptr qskont_ref_extra (const qsmachine_t *, qsptr p);
int qskont_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qslambda_make (qsmachine_t *, qsptr parameters, qsptr body);
bool qslambda_p (const qsmachine_t *, qsptr p);
qsptr qslambda_ref_param (const qsmachine_t *, qsptr p);
qsptr qslambda_ref_body (const qsmachine_t *, qsptr p);
int qslambda_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsclosure_make (qsmachine_t *, qsptr lambda, qsptr env);
bool qsclosure_p (const qsmachine_t *, qsptr p);
qsptr qsclosure_ref_lam (const qsmachine_t *, qsptr p);
qsptr qsclosure_ref_env (const qsmachine_t *, qsptr p);
int qsclosure_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

#endif /* QSVAL_H_ */
