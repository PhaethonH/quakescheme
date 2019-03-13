#ifndef QSVAL_H_
#define QSVAL_H_

#include <stdarg.h>
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
int qsprim_id (const qsmachine_t *, qsptr p);
int qsprim_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);


/* Heaped object */

qsptr qsobj_make (qsmachine_t *, qsword obj_id);

qsptr qssym_make (qsmachine_t *, qsword sym_id);
qsword qssym_id (const qsmachine_t *, qsptr p);
bool qssym_p (const qsmachine_t *, qsptr p);
qsptr qssym_symbol (const qsmachine_t *, qsptr p);
qsptr qssym_name (const qsmachine_t *, qsptr p);
int qssym_crepr (qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qspair_make (qsmachine_t *, qsptr a, qsptr d);
bool qspair_p (const qsmachine_t *, qsptr p);
qsptr qspair_ref_head (const qsmachine_t *, qsptr p);
qsptr qspair_ref_tail (const qsmachine_t *, qsptr p);
qsptr qspair_setq_head (qsmachine_t *, qsptr p, qsptr a);
qsptr qspair_setq_tail (qsmachine_t *, qsptr p, qsptr d);
#define qspair_car qspair_ref_head
#define qspair_cdr qspair_ref_tail
#define qspair_setcarq qspair_setq_head
#define qspair_setcdrq qspair_setq_tail
qsptr qspair_iter (const qsmachine_t *, qsptr p);
int qspair_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsvector_make (qsmachine_t *, qsword len, qsptr fill);
bool qsvector_p (const qsmachine_t *, qsptr p);
qsword qsvector_length (const qsmachine_t *, qsptr p);
qsptr qsvector_ref (const qsmachine_t *, qsptr p, qsword k);
qsptr qsvector_setq (qsmachine_t *, qsptr p, qsword k, qsptr val);
int qsvector_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

/*
The Array type is inspired by the CDR-coded list.

The original purposes was to allow directly encoding lists into an array of qsptr in C, without trying to predict the location of the next pair (cdr).

Unlike CDR-coding, end of list is indicated by sentinel value (leveraging the qsptr encoding plan), and improper lists are not supported.
*/
qsptr qsarray_make (qsmachine_t *, qsword len);
qsptr qsarray_vinject (qsmachine_t *, va_list vp);
qsptr qsarray_inject (qsmachine_t *, ...);
bool qsarray_p (const qsmachine_t *, qsptr p);
qsword qsarray_length (const qsmachine_t *, qsptr p);
qsptr qsarray_ref (const qsmachine_t *, qsptr p, qsword k);
qsptr qsarray_setq (qsmachine_t *, qsptr p, qsword k, qsptr val);
qsptr qsarray_iter (const qsmachine_t *, qsptr p);
int qsarray_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

/* C pointers (void pointers). */
/* Considered dangerous, as a mix of dereferencing C pointers and arbitrarily modifying memory may break sandbox. */
qsptr qscptr_make (qsmachine_t *, void * val);
bool qscptr_p (const qsmachine_t *, qsptr p);
void * qscptr_get (const qsmachine_t *, qsptr p);
int qscptr_fetch (const qsmachine_t *, qsptr p, void ** out);
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
qsptr qsname_make (qsmachine_t *, qsword namelen);
qsptr qsname_bless (qsmachine_t *, qsptr s);
qsptr qsname_inject (qsmachine_t *, const char * cstr);
const char * qsname_get (const qsmachine_t *, qsptr p);
bool qsname_p (const qsmachine_t *, qsptr p);
/* get Qssym pointer from Qssymbol object. */
qsptr qsname_sym (const qsmachine_t *, qsptr p);
qsword qsname_length (const qsmachine_t *, qsptr p);
int qsname_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);
int qsname_strcmp (const qsmachine_t *, qsptr x, const char * s);
qscmp_t qsname_cmp (const qsmachine_t *, qsptr x, qsptr y);

/*
Strings have multiple implementations for different purposes:
  * UTF-8 encoded byte vector, for interfacing with C string handling.
  * Vector of char24 (akin to UTF-32) for mutability.
  * List of char24 for bootstrapping from char24 and pairs.
*/

qsptr qsutf8_make (qsmachine_t *, qsword len, int fill);
qsptr qsutf8_inject_charp (qsmachine_t *, const char * s);
qsptr qsutf8_inject_bytes (qsmachine_t *, uint8_t * buf, qsword buflen);
bool qsutf8_p (const qsmachine_t *, qsptr p);
qsword qsutf8_length (const qsmachine_t *, qsptr p);
int qsutf8_ref (const qsmachine_t *, qsptr p, qsword k);
qsptr qsutf8_setq (qsmachine_t *, qsptr p, qsword k, int ch);
int qsutf8_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsbytevec_make (qsmachine_t *, qsword len, qsbyte fill);
bool qsbytevec_p (const qsmachine_t *, qsptr p);
qsword qsbytevec_length (const qsmachine_t *, qsptr p);
bool qsbytevec_extract (const qsmachine_t *, qsptr p, const uint8_t ** out_uint8ptr, qsword * out_size);
qsbyte qsbytevec_ref (const qsmachine_t *, qsptr p, qsword k);
qsptr qsbytevec_setq (qsmachine_t *, qsptr p, qsword k, qsbyte val);
int qsbytevec_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qsenv_make (qsmachine_t *, qsptr next_env);
qsptr qsenv_insert (qsmachine_t *, qsptr env, qsptr variable, qsptr binding);
qsptr qsenv_lookup (qsmachine_t *, qsptr env, qsptr variable);
int qsenv_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

qsptr qskont_make (qsmachine_t *, qsptr variant, qsptr c, qsptr e, qsptr k);
qsptr qskont_make_current (qsmachine_t *);
bool qskont_p (const qsmachine_t *, qsptr p);
qsptr qskont_ref_v (const qsmachine_t *, qsptr p);
qsptr qskont_ref_c (const qsmachine_t *, qsptr p);
qsptr qskont_ref_e (const qsmachine_t *, qsptr p);
qsptr qskont_ref_k (const qsmachine_t *, qsptr p);
int qskont_fetch (const qsmachine_t *, qsptr p, qsptr * out_v, qsptr * out_c, qsptr * out_e, qsptr * out_k);
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


/* Heavy Port (ports that have to track their own state inside Scheme) */
qsptr qscport_make (qsmachine_t * mach, qsptr variant, qsptr pathspec, bool writeable, qsptr host_resource);
bool qscport_get_writeable (qsmachine_t * mach, qsptr p);
qsptr qscport_get_pathspec (qsmachine_t * mach, qsptr p);
int qscport_get_pos (qsmachine_t * mach, qsptr p);
int qscport_get_max (qsmachine_t * mach, qsptr p);
qsptr qscport_get_resource (qsmachine_t * mach, qsptr p);
qsptr qscport_set_writeable (qsmachine_t * mach, qsptr p, bool val);
qsptr qscport_set_pathspec (qsmachine_t * mach, qsptr p, qsptr val);
qsptr qscport_set_pos (qsmachine_t * mach, qsptr p, int pos);
qsptr qscport_set_max (qsmachine_t * mach, qsptr p, int max);
qsptr qscport_set_resource (qsmachine_t * mach, qsptr p, qsptr val);

/* C Character Pointer (String) ports. */
qsptr qscharpport_make (qsmachine_t *, uint8_t * buf, int buflen);
bool qscharpport_p (qsmachine_t *, qsptr p);
bool qscharpport_get_writeable (qsmachine_t *, qsptr p);
qsptr qscharpport_set_writeable (qsmachine_t *, qsptr p, bool val);
int qscharpport_read_u8 (qsmachine_t *, qsptr p);
bool qscharpport_write_u8 (qsmachine_t *, qsptr p, int byte);
bool qscharpport_close (qsmachine_t *, qsptr p);
int qscharpport_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

/* OctetVector (utf-8 string) ports. */
qsptr qsovport_make (qsmachine_t *, qsptr s);
bool qsovport_p (qsmachine_t *, qsptr s);
bool qsovport_get_writeable (qsmachine_t *, qsptr p);
qsptr qsovport_set_writeable (qsmachine_t *, qsptr p, bool val);
int qsovport_read_u8 (qsmachine_t *, qsptr p);
bool qsovport_write_u8 (qsmachine_t *, qsptr p, int byte);
bool qsovport_close (qsmachine_t *, qsptr p);
int qsovport_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);


/* the iterator type allows for iterating both pairs and arrays as a list.
*/
qsptr qsiter_make (const qsmachine_t *, qsaddr addr);
qsptr qsiter_begin (const qsmachine_t *, qsptr p);
bool qsiter_p (const qsmachine_t *, qsptr p);
qsptr qsiter_head (const qsmachine_t *, qsptr p);
qsptr qsiter_tail (const qsmachine_t *, qsptr p);
int qsiter_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

/* Abstraction of QsSym and QsName */
bool qssymbol_p (const qsmachine_t *, qsptr p);
/* convert string object to interned symbol; returns QsSym pointer. */
qsptr qssymbol_bless (qsmachine_t *, qsptr s);
/* intern name object into symbol table; returns QsSym pointer. */
qsptr qssymbol_intern (qsmachine_t *, qsptr p);
/* intern symbol name from C string; returns QsSym pointer. */
qsptr qssymbol_intern_c (qsmachine_t *, const char * cstr);
/* gets symbol name from QsName or QsSym. */
const char * qssymbol_get (const qsmachine_t *, qsptr p);
int qssymbol_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);
qscmp_t qssymbol_cmp (const qsmachine_t *, qsptr x, qsptr y);


/* Generalized stringification. */
int qsptr_crepr (const qsmachine_t *, qsptr p, char * buf, int buflen);

#endif /* QSVAL_H_ */
