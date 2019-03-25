#ifndef QSVAL_H_
#define QSVAL_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "qsmach.h"

/* Scheme values, combining single-word values and in-heap values. */

/* identify function, for use in table-of-functions. */
qsptr_t qsptr_make (qsmachine_t *, qsptr_t);

/* Directly encoded values. */

qsptr_t qsbool_make (qsmachine_t *, int val);
bool qsbool_p (const qsmachine_t *, qsptr_t p);
int qsbool_crepr (const qsmachine_t *, qsptr_t p, char *buf, int buflen);

qsptr_t qsfloat_make (qsmachine_t *, float val);
bool qsfloat_p (const qsmachine_t *, qsptr_t p);
int qsfloat_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

qsptr_t qsint_make (qsmachine_t *, int32_t val);
bool qsint_p (const qsmachine_t *, qsptr_t p);
int qsint_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

/* Limits due to bit widths. */
#define MAX_INT30 ((int)(~((qsword)0) >> (SHIFT_TAG30 + 1)))
#define MIN_INT30 ((int)(-MAX_INT30 - 1))

qsptr_t qschar_make (qsmachine_t *, int val);
bool qschar_p (const qsmachine_t *, qsptr_t p);
int qschar_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

qsptr_t qsconst_make (qsmachine_t *, int const_id);
bool qsconst_p (const qsmachine_t *, qsptr_t p);
int qsconst_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

/* File Descriptor directly encoded into Pointer (up to 20b values). */
/*
  Platforms missing Standard C File support may provide a file interface
  based on numeric handles similar to POSIX File Descriptors.
  File state information would be stored in host-space, relieving Scheme heap.
  Presumably 0,1,2 correpond to stdin,stdout,stderr; but not assumed.

  Main use case is to allow writing to fd 2 (stderr) after heap exhaustion.
*/
qsptr_t qsfd_make (qsmachine_t *, int fd);
qsptr_t qsfd_open (qsmachine_t *, const char * path, int flags, int mode);
bool qsfd_p (const qsmachine_t *, qsptr_t p);
int qsfd_id (const qsmachine_t *, qsptr_t p);
bool qsfd_eof (const qsmachine_t *, qsptr_t p);
int qsfd_read_u8 (const qsmachine_t *, qsptr_t p);
bool qsfd_write_u8 (const qsmachine_t *, qsptr_t p, int byte);
bool qsfd_close (const qsmachine_t *, qsptr_t p);
int qsfd_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

qsptr_t qsprim_make (qsmachine_t *, qsword primid);
bool qsprim_p (const qsmachine_t *, qsptr_t p);
int qsprim_id (const qsmachine_t *, qsptr_t p);
int qsprim_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);


/* Heaped object */

qsptr_t qsobj_make (qsmachine_t *, qsword obj_id);

qsptr_t qssym_make (qsmachine_t *, qsword sym_id);
qsword qssym_id (const qsmachine_t *, qsptr_t p);
bool qssym_p (const qsmachine_t *, qsptr_t p);
qsptr_t qssym_symbol (const qsmachine_t *, qsptr_t p);
qsptr_t qssym_name (const qsmachine_t *, qsptr_t p);
int qssym_crepr (qsmachine_t *, qsptr_t p, char * buf, int buflen);

qsptr_t qspair_make (qsmachine_t *, qsptr_t a, qsptr_t d);
bool qspair_p (const qsmachine_t *, qsptr_t p);
qsptr_t qspair_ref_head (const qsmachine_t *, qsptr_t p);
qsptr_t qspair_ref_tail (const qsmachine_t *, qsptr_t p);
qsptr_t qspair_setq_head (qsmachine_t *, qsptr_t p, qsptr_t a);
qsptr_t qspair_setq_tail (qsmachine_t *, qsptr_t p, qsptr_t d);
#define qspair_car qspair_ref_head
#define qspair_cdr qspair_ref_tail
#define qspair_setcarq qspair_setq_head
#define qspair_setcdrq qspair_setq_tail
qsptr_t qspair_iter (const qsmachine_t *, qsptr_t p);
int qspair_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

qsptr_t qsvector_make (qsmachine_t *, qsword len, qsptr_t fill);
bool qsvector_p (const qsmachine_t *, qsptr_t p);
qsword qsvector_length (const qsmachine_t *, qsptr_t p);
qsptr_t qsvector_ref (const qsmachine_t *, qsptr_t p, qsword k);
qsptr_t qsvector_setq (qsmachine_t *, qsptr_t p, qsword k, qsptr_t val);
int qsvector_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

/*
The Array type is inspired by the CDR-coded list.

The original purposes was to allow directly encoding lists into an array of qsptr_t in C, without trying to predict the location of the next pair (cdr).

Unlike CDR-coding, end of list is indicated by sentinel value (leveraging the qsptr_t encoding plan), and improper lists are not supported.
*/
qsptr_t qsarray_make (qsmachine_t *, qsword len);
qsptr_t qsarray_vinject (qsmachine_t *, va_list vp);
qsptr_t qsarray_inject (qsmachine_t *, ...);
bool qsarray_p (const qsmachine_t *, qsptr_t p);
qsword qsarray_length (const qsmachine_t *, qsptr_t p);
qsptr_t qsarray_ref (const qsmachine_t *, qsptr_t p, qsword k);
qsptr_t qsarray_setq (qsmachine_t *, qsptr_t p, qsword k, qsptr_t val);
qsptr_t qsarray_iter (const qsmachine_t *, qsptr_t p);
int qsarray_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

/* C pointers (void pointers). */
/* Considered dangerous, as a mix of dereferencing C pointers and arbitrarily modifying memory may break sandbox. */
qsptr_t qscptr_make (qsmachine_t *, void * val);
bool qscptr_p (const qsmachine_t *, qsptr_t p);
void * qscptr_get (const qsmachine_t *, qsptr_t p);
int qscptr_fetch (const qsmachine_t *, qsptr_t p, void ** out);
int qscptr_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

qsptr_t qslong_make (qsmachine_t *, int64_t val);
bool qslong_p (const qsmachine_t *, qsptr_t p);
int64_t qslong_get (const qsmachine_t *, qsptr_t p);
int qslong_fetch (const qsmachine_t *, qsptr_t p, int64_t * out);
int qslong_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

qsptr_t qsdouble_make (qsmachine_t *, double val);
bool qsdouble_p (const qsmachine_t *, qsptr_t p);
double qsdouble_get (const qsmachine_t *, qsptr_t p);
int qsdouble_fetch (const qsmachine_t *, qsptr_t p, double * out);
int qsdouble_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

/* symbol object links symbol id (qsint) to symbol name (qsstring) */
qsptr_t qsname_make (qsmachine_t *, qsword namelen);
qsptr_t qsname_bless (qsmachine_t *, qsptr_t s);
qsptr_t qsname_inject (qsmachine_t *, const char * cstr, qsword slen);
const char * qsname_get (const qsmachine_t *, qsptr_t p);
bool qsname_p (const qsmachine_t *, qsptr_t p);
/* get Qssym pointer from Qssymbol object. */
qsptr_t qsname_sym (const qsmachine_t *, qsptr_t p);
qsword qsname_length (const qsmachine_t *, qsptr_t p);
int qsname_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);
int qsname_strcmp (const qsmachine_t *, qsptr_t x, const char * s);
qscmp_t qsname_cmp (const qsmachine_t *, qsptr_t x, qsptr_t y);

/*
Strings have multiple implementations for different purposes:
  * UTF-8 encoded byte vector, for interfacing with C string handling.
  * Vector of char24 (akin to UTF-32) for mutability.
  * List of char24 for bootstrapping from char24 and pairs.
*/

qsptr_t qsutf8_make (qsmachine_t *, qsword len, int fill);
qsptr_t qsutf8_inject_charp (qsmachine_t *, const char * s, size_t slen);
qsptr_t qsutf8_inject_bytes (qsmachine_t *, uint8_t * buf, qsword buflen);
bool qsutf8_p (const qsmachine_t *, qsptr_t p);
qsword qsutf8_length (const qsmachine_t *, qsptr_t p);
int qsutf8_ref (const qsmachine_t *, qsptr_t p, qsword k);
/* Copy string from Scheme space to C space: number of bytes of copied. */
int qsutf8_fetch (const qsmachine_t *, qsptr_t p, char * buf, int buflen);
/* C pointer to Scheme string data (already encoded as C string). */
const char * qsutf8_get (const qsmachine_t *, qsptr_t p, qsword * len);
qsptr_t qsutf8_setq (qsmachine_t *, qsptr_t p, qsword k, int ch);
/* increment reference count. */
qserr_t qsutf8_hold (qsmachine_t *, qsptr_t p);
/* decrement reference count. */
qserr_t qsutf8_release (qsmachine_t *, qsptr_t p);
int qsutf8_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

qsptr_t qsbytevec_make (qsmachine_t *, qsword len, qsbyte fill);
bool qsbytevec_p (const qsmachine_t *, qsptr_t p);
qsword qsbytevec_length (const qsmachine_t *, qsptr_t p);
bool qsbytevec_extract (const qsmachine_t *, qsptr_t p, const uint8_t ** out_uint8ptr, qsword * out_size);
qsbyte qsbytevec_ref (const qsmachine_t *, qsptr_t p, qsword k);
qsptr_t qsbytevec_setq (qsmachine_t *, qsptr_t p, qsword k, qsbyte val);
int qsbytevec_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

qsptr_t qsenv_make (qsmachine_t *, qsptr_t next_env);
qsptr_t qsenv_insert (qsmachine_t *, qsptr_t env, qsptr_t variable, qsptr_t binding);
qsptr_t qsenv_lookup (qsmachine_t *, qsptr_t env, qsptr_t variable);
int qsenv_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

qsptr_t qskont_make (qsmachine_t *, qsptr_t variant, qsptr_t c, qsptr_t e, qsptr_t k);
qsptr_t qskont_make_current (qsmachine_t *);
bool qskont_p (const qsmachine_t *, qsptr_t p);
qsptr_t qskont_ref_v (const qsmachine_t *, qsptr_t p);
qsptr_t qskont_ref_c (const qsmachine_t *, qsptr_t p);
qsptr_t qskont_ref_e (const qsmachine_t *, qsptr_t p);
qsptr_t qskont_ref_k (const qsmachine_t *, qsptr_t p);
int qskont_fetch (const qsmachine_t *, qsptr_t p, qsptr_t * out_v, qsptr_t * out_c, qsptr_t * out_e, qsptr_t * out_k);
int qskont_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

qsptr_t qslambda_make (qsmachine_t *, qsptr_t parameters, qsptr_t body);
bool qslambda_p (const qsmachine_t *, qsptr_t p);
qsptr_t qslambda_ref_param (const qsmachine_t *, qsptr_t p);
qsptr_t qslambda_ref_body (const qsmachine_t *, qsptr_t p);
int qslambda_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

qsptr_t qsclosure_make (qsmachine_t *, qsptr_t lambda, qsptr_t env);
bool qsclosure_p (const qsmachine_t *, qsptr_t p);
qsptr_t qsclosure_ref_lam (const qsmachine_t *, qsptr_t p);
qsptr_t qsclosure_ref_env (const qsmachine_t *, qsptr_t p);
int qsclosure_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);


/* Heavy Port (ports that have to track their own state inside Scheme) */
qsptr_t qscport_make (qsmachine_t * mach, qsptr_t variant, qsptr_t pathspec, bool writeable, qsptr_t host_resource);
bool qscport_p (qsmachine_t *, qsptr_t p);
bool qscport_get_writeable (qsmachine_t * mach, qsptr_t p);
qsptr_t qscport_get_pathspec (qsmachine_t * mach, qsptr_t p);
int qscport_get_pos (qsmachine_t * mach, qsptr_t p);
int qscport_get_max (qsmachine_t * mach, qsptr_t p);
qsptr_t qscport_get_resource (qsmachine_t * mach, qsptr_t p);
qsptr_t qscport_set_writeable (qsmachine_t * mach, qsptr_t p, bool val);
qsptr_t qscport_set_pathspec (qsmachine_t * mach, qsptr_t p, qsptr_t val);
qsptr_t qscport_set_pos (qsmachine_t * mach, qsptr_t p, int pos);
qsptr_t qscport_set_max (qsmachine_t * mach, qsptr_t p, int max);
qsptr_t qscport_set_resource (qsmachine_t * mach, qsptr_t p, qsptr_t val);

/* C Character Pointer (String) ports. */
qsptr_t qscharpport_make (qsmachine_t *, uint8_t * buf, int buflen);
bool qscharpport_p (qsmachine_t *, qsptr_t p);
bool qscharpport_eof (qsmachine_t *, qsptr_t p);
int qscharpport_read_u8 (qsmachine_t *, qsptr_t p);
bool qscharpport_write_u8 (qsmachine_t *, qsptr_t p, int byte);
bool qscharpport_close (qsmachine_t *, qsptr_t p);
int qscharpport_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

/* OctetVector (utf-8 string) ports. */
qsptr_t qsovport_make (qsmachine_t *, qsptr_t s);
bool qsovport_p (qsmachine_t *, qsptr_t s);
int qsovport_read_u8 (qsmachine_t *, qsptr_t p);
bool qsovport_write_u8 (qsmachine_t *, qsptr_t p, int byte);
bool qsovport_close (qsmachine_t *, qsptr_t p);
int qsovport_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

/* Port backed by Standard C File. */
qsptr_t qsfport_make (qsmachine_t *, const char * path, const char * mode);
bool qsfport_p (qsmachine_t *, qsptr_t p);
bool qsfport_eof (qsmachine_t *, qsptr_t p);
FILE * qsfport_get (qsmachine_t *, qsptr_t p);
int qsfport_read_u8 (qsmachine_t *, qsptr_t p);
bool qsfport_write_u8 (qsmachine_t *, qsptr_t p, int byte);
qsword qsfport_tell (qsmachine_t *, qsptr_t p);
bool qsfport_seek (qsmachine_t *, qsptr_t p, qsword pos);
bool qsfport_close (qsmachine_t *, qsptr_t p);
int qsfport_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

/* Overall Port wrapper. */
qsptr_t qsport_make (qsmachine_t *, qsptr_t variant, qsptr_t path, bool writeable, bool appending);
qsptr_t qsport_make_c (qsmachine_t *, qsptr_t variant, const uint8_t * spec, int speclen, bool writeable, bool appending);
bool qsport_p (qsmachine_t *, qsptr_t p);
bool qsport_eof (qsmachine_t *, qsptr_t p);
int qsport_read_u8 (qsmachine_t *, qsptr_t p);
bool qsport_write_u8 (qsmachine_t *, qsptr_t p, int byte);
qsword qsport_tell (qsmachine_t *, qsptr_t p);
bool qsport_seek (qsmachine_t *, qsptr_t p, qsword pos);
bool qsport_close (qsmachine_t *, qsptr_t p);


/* the iterator type allows for iterating both pairs and arrays as a list.
*/
qsptr_t qsiter_make (const qsmachine_t *, qsaddr_t addr);
qsptr_t qsiter_begin (const qsmachine_t *, qsptr_t p);
bool qsiter_p (const qsmachine_t *, qsptr_t p);
qsptr_t qsiter_head (const qsmachine_t *, qsptr_t p);
qsptr_t qsiter_tail (const qsmachine_t *, qsptr_t p);
int qsiter_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

/* Abstraction of QsSym and QsName */
bool qssymbol_p (const qsmachine_t *, qsptr_t p);
/* convert string object to interned symbol; returns QsSym pointer. */
qsptr_t qssymbol_bless (qsmachine_t *, qsptr_t s);
/* intern name object into symbol table; returns QsSym pointer. */
qsptr_t qssymbol_intern (qsmachine_t *, qsptr_t p);
/* intern symbol name from C string; returns QsSym pointer. */
qsptr_t qssymbol_intern_c (qsmachine_t *, const char * cstr, int slen);
/* gets symbol name from QsName or QsSym. */
const char * qssymbol_get (const qsmachine_t *, qsptr_t p);
int qssymbol_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);
qscmp_t qssymbol_cmp (const qsmachine_t *, qsptr_t x, qsptr_t y);


/* Generalized stringification. */
int qsptr_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

#endif /* QSVAL_H_ */
