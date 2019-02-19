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
qsptr qsbool_sprint (const qsmachine_t *, qsptr p, char *buf, int buflen);

qsptr qsfloat_make (qsmachine_t *, float val);
bool qsfloat_p (const qsmachine_t *, qsptr p);
qsptr qsfloat_sprint (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsint_make (qsmachine_t *, int32_t val);
bool qsint_p (const qsmachine_t *, qsptr p);
qsptr qsint_sprint (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qschar_make (qsmachine_t *, int val);
bool qschar_p (const qsmachine_t *, qsptr p);
qsptr qschar_sprint (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsfd_make (qsmachine_t *, int val);
bool qsfd_p (const qsmachine_t *, qsptr p);
qsptr qsfd_sprint (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsprim_make (qsmachine_t *, qsword primid);
bool qsprim_p (const qsmachine_t *, qsptr p);
qsptr qsprim_sprint (const qsmachine_t *, qsptr p, char * buf, int buflen);


/* Heaped object */

qsptr qsobj_make (qsmachine_t *, qsword obj_id);

qsptr qssym_make (qsmachine_t *, qsword sym_id);
bool qssym_p (const qsmachine_t *, qsptr p);
int qssym_print (qsmachine_t *, qsptr p, char * buf, int buflen);

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
int qspair_print (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsvector_make (qsmachine_t *, qsword len, qsptr fill);
bool qsvector_p (const qsmachine_t *, qsptr p);
qsptr qsvector_length (const qsmachine_t *, qsptr p);
qsptr qsvector_ref (const qsmachine_t *, qsptr p, qsword k);
qsptr qsvector_setq (qsmachine_t *, qsptr p, qsword k, qsptr val);
int qsvector_print (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qscptr_make (qsmachine_t *, void * val);
bool qscptr_p (const qsmachine_t *, qsptr p);
void * qsctpr_get (const qsmachine_t *, qsptr p);
int qsctpr_fetch (const qsmachine_t *, qsptr p, void * out);
int qscptr_print (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qslong_make (qsmachine_t *, int64_t val);
bool qslong_p (const qsmachine_t *, qsptr p);
int64_t qslong_get (const qsmachine_t *, qsptr p);
int qslong_fetch (const qsmachine_t *, qsptr p, int64_t * out);
int qslong_print (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsdouble_make (qsmachine_t *, double val);
bool qsdouble_p (const qsmachine_t *, qsptr p);
double qsdouble_get (const qsmachine_t *, qsptr p);
int qsdouble_fetch (const qsmachine_t *, qsptr p, double * out);
int qsdouble_print (const qsmachine_t *, qsptr p, char * buf, int buflen);

/* symbol object links symbol id (qsint) to symbol name (qsstring) */
qsptr qssymbol_make (qsmachine_t *, qsptr name);
bool qssymbol_p (const qsmachine_t *, qsptr p);
int qssymbol_print (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsstring_make (qsmachine_t *, qsword len, int fill);
bool qsstring_p (const qsmachine_t *, qsptr p);
int qsstring_print (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsbytevec_make (qsmachine_t *, qsword len, qsbyte fill);
bool qsbytevec_p (const qsmachine_t *, qsptr p);
int qsbytevec_print (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsenv_make (qsmachine_t *, qsptr next_env);
qsptr qsenv_insert (qsmachine_t *, qsptr variable, qsptr binding);
int qsenv_print (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qskont_make (qsmachine_t *, qsptr variant, qsptr c, qsptr e, qsptr k, qsptr extra);
bool qskont_p (const qsmachine_t *, qsptr p);
int qskont_print (const qsmachine_t *, qsptr p, char * buf, int buflen);

#endif /* QSVAL_H_ */
