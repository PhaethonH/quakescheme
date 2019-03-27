#ifndef QSVAL_H_
#define QSVAL_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <wchar.h>

#include "qsmach.h"

/* Scheme values, combining single-word values and in-heap values. */

/* identify function, for use in table-of-functions. */
extern qsptr_t qsptr_make (qsmachine_t *, qsptr_t);

/* Directly encoded values. */

extern qsptr_t qsnil_make (qsmachine_t *);
extern bool qsnil_p (const qsmachine_t *, qsptr_t p);

extern qsptr_t qseof_make (const qsmachine_t *);


extern qsptr_t qsbool_make (qsmachine_t *, int val);
extern bool qsbool_p (const qsmachine_t *, qsptr_t p);
extern bool qsbool_get (const qsmachine_t *, qsptr_t p);
extern int qsbool_crepr (const qsmachine_t *, qsptr_t p, char *buf, int buflen);

extern qsptr_t qsfloat_make (qsmachine_t *, float val);
extern bool qsfloat_p (const qsmachine_t *, qsptr_t p);
extern float qsfloat_get (const qsmachine_t *, qsptr_t p);
extern int qsfloat_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

extern qsptr_t qsint_make (qsmachine_t *, int32_t val);
extern bool qsint_p (const qsmachine_t *, qsptr_t p);
extern int32_t qsint_get (const qsmachine_t *, qsptr_t p);
extern int qsint_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

/* Limits due to bit widths. */
#define MAX_INT30 ((int)(~((qsword)0) >> (SHIFT_TAG30 + 1)))
#define MIN_INT30 ((int)(-MAX_INT30 - 1))

extern qsptr_t qschar_make (qsmachine_t *, int val);
extern bool qschar_p (const qsmachine_t *, qsptr_t p);
extern int qschar_get (const qsmachine_t *, qsptr_t p);
extern int qschar_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

extern qsptr_t qsconst_make (qsmachine_t *, int const_id);
extern bool qsconst_p (const qsmachine_t *, qsptr_t p);
int qsconst_get (const qsmachine_t *, qsptr_t p);
extern int qsconst_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

/* File Descriptor directly encoded into Pointer (up to 20b values). */
/*
  Platforms missing Standard C File support may provide a file interface
  based on numeric handles similar to POSIX File Descriptors.
  File state information would be stored in host-space, relieving Scheme heap.
  Presumably 0,1,2 correpond to stdin,stdout,stderr; but not assumed.

  Main use case is to allow writing to fd 2 (stderr) after heap exhaustion.
*/
extern qsptr_t qsfd_make (qsmachine_t *, int fd);
extern qsptr_t qsfd_open (qsmachine_t *, qsptr_t path, int flags, int mode);
extern qsptr_t qsfd_open_c (qsmachine_t *, const char * path, int flags, int mode);
extern bool qsfd_p (const qsmachine_t *, qsptr_t p);
extern int qsfd_id (const qsmachine_t *, qsptr_t p);
extern bool qsfd_eof (const qsmachine_t *, qsptr_t p);
extern int qsfd_read_u8 (const qsmachine_t *, qsptr_t p);
extern bool qsfd_write_u8 (const qsmachine_t *, qsptr_t p, int byte);
extern bool qsfd_close (const qsmachine_t *, qsptr_t p);
extern int qsfd_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

extern qsptr_t qsprim_make (qsmachine_t *, qsword primid);
extern bool qsprim_p (const qsmachine_t *, qsptr_t p);
extern int qsprim_id (const qsmachine_t *, qsptr_t p);
extern int qsprim_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

/* Original goal of QsErr was exceptions in Out-Of-Memory conditions.
   Thus, constructing QsErr does not require modifying the machine state.
 */
extern qsptr_t qserr_make (const qsmachine_t *, qsword errid);
extern qsword qserr_id (const qsmachine_t *, qsptr_t p);
extern bool qserr_p (const qsmachine_t *, qsptr_t p);
extern int qserr_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);


/* Heaped object */

extern qsptr_t qsobj_make (qsmachine_t *, qsword obj_id);
extern qsword qsobj_id (const qsmachine_t *, qsptr_t p);
extern qsaddr_t qsobj_address (const qsmachine_t *, qsptr_t p);

extern qsptr_t qssym_make (qsmachine_t *, qsword sym_id);
extern qsword qssym_id (const qsmachine_t *, qsptr_t p);
extern bool qssym_p (const qsmachine_t *, qsptr_t p);
extern qsptr_t qssym_symbol (const qsmachine_t *, qsptr_t p);
extern qsptr_t qssym_name (const qsmachine_t *, qsptr_t p);
extern int qssym_crepr (qsmachine_t *, qsptr_t p, char * buf, int buflen);

extern qsptr_t qspair_make (qsmachine_t *, qsptr_t a, qsptr_t d);
extern bool qspair_p (const qsmachine_t *, qsptr_t p);
extern qsptr_t qspair_ref_head (const qsmachine_t *, qsptr_t p);
extern qsptr_t qspair_ref_tail (const qsmachine_t *, qsptr_t p);
extern qsptr_t qspair_setq_head (qsmachine_t *, qsptr_t p, qsptr_t a);
extern qsptr_t qspair_setq_tail (qsmachine_t *, qsptr_t p, qsptr_t d);
#define qspair_car qspair_ref_head
#define qspair_cdr qspair_ref_tail
#define qspair_setcarq qspair_setq_head
#define qspair_setcdrq qspair_setq_tail
extern qsptr_t qspair_iter (const qsmachine_t *, qsptr_t p);
extern int qspair_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

extern qsptr_t qsvector_make (qsmachine_t *, qsword len, qsptr_t fill);
extern bool qsvector_p (const qsmachine_t *, qsptr_t p);
extern qsword qsvector_length (const qsmachine_t *, qsptr_t p);
extern qsptr_t qsvector_ref (const qsmachine_t *, qsptr_t p, qsword k);
extern qsptr_t qsvector_setq (qsmachine_t *, qsptr_t p, qsword k, qsptr_t val);
extern int qsvector_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

extern qsptr_t qscharvec_make (qsmachine_t *, qsword len, wchar_t fillch);
extern bool qscharvec_p (const qsmachine_t *, qsptr_t p);
extern qsword qscharvec_length (const qsmachine_t *, qsptr_t p);
extern int qscharvec_ref (const qsmachine_t *, qsptr_t p, qsword k);
extern qsptr_t qscharvec_setq (qsmachine_t *, qsptr_t p, qsword k, int codept);
extern const qsptr_t * qscharvec_get (const qsmachine_t *, qsptr_t p, qsword *out_len);
extern qsword qscharvec_fetch (const qsmachine_t *, qsptr_t p, char * buf, int buflen);
extern qsptr_t qscharvec_inject_charp (qsmachine_t *, const char * s, int slen);
extern qsptr_t qscharvec_inject_wcs (qsmachine_t *, const wchar_t * ws, int wlen);
extern int qscharvec_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

/*
The Array type is inspired by the CDR-coded list.

The original purposes was to allow directly encoding lists into an array of qsptr_t in C, without trying to predict the location of the next pair (cdr).

Unlike CDR-coding, end of list is indicated by sentinel value (leveraging the qsptr_t encoding plan), and improper lists are not supported.
*/
extern qsptr_t qsarray_make (qsmachine_t *, qsword len);
extern qsptr_t qsarray_vinject (qsmachine_t *, va_list vp);
extern qsptr_t qsarray_inject (qsmachine_t *, ...);
extern bool qsarray_p (const qsmachine_t *, qsptr_t p);
extern qsword qsarray_length (const qsmachine_t *, qsptr_t p);
extern qsptr_t qsarray_ref (const qsmachine_t *, qsptr_t p, qsword k);
extern qsptr_t qsarray_setq (qsmachine_t *, qsptr_t p, qsword k, qsptr_t val);
extern qsptr_t qsarray_iter (const qsmachine_t *, qsptr_t p);
extern int qsarray_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

/* C pointers (void pointers). */
/* Considered dangerous, as a mix of dereferencing C pointers and arbitrarily modifying memory may break sandbox. */
extern qsptr_t qscptr_make (qsmachine_t *, void * val);
extern bool qscptr_p (const qsmachine_t *, qsptr_t p);
extern void * qscptr_get (const qsmachine_t *, qsptr_t p);
extern int qscptr_fetch (const qsmachine_t *, qsptr_t p, void ** out);
extern int qscptr_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

extern qsptr_t qslong_make (qsmachine_t *, int64_t val);
extern bool qslong_p (const qsmachine_t *, qsptr_t p);
extern int64_t qslong_get (const qsmachine_t *, qsptr_t p);
extern int qslong_fetch (const qsmachine_t *, qsptr_t p, int64_t * out);
extern int qslong_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

extern qsptr_t qsdouble_make (qsmachine_t *, double val);
extern bool qsdouble_p (const qsmachine_t *, qsptr_t p);
extern double qsdouble_get (const qsmachine_t *, qsptr_t p);
extern int qsdouble_fetch (const qsmachine_t *, qsptr_t p, double * out);
extern int qsdouble_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

/* symbol object links symbol id (qsint) to symbol name (qsstring) */
extern qsptr_t qsname_make (qsmachine_t *, qsword namelen);
extern qsptr_t qsname_bless (qsmachine_t *, qsptr_t s);
extern qsptr_t qsname_inject (qsmachine_t *, const char * cstr, qsword slen);
extern const char * qsname_get (const qsmachine_t *, qsptr_t p);
extern bool qsname_p (const qsmachine_t *, qsptr_t p);
/* get Qssym pointer from Qssymbol object. */
extern qsptr_t qsname_sym (const qsmachine_t *, qsptr_t p);
extern qsword qsname_length (const qsmachine_t *, qsptr_t p);
extern int qsname_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);
extern int qsname_strcmp (const qsmachine_t *, qsptr_t x, const char * s);
extern qscmp_t qsname_cmp (const qsmachine_t *, qsptr_t x, qsptr_t y);

/*
Strings have multiple implementations for different purposes:
  * UTF-8 encoded byte vector, for interfacing with C string handling.
  * Vector of char24 (akin to UTF-32) for mutability.
  * List of char24 for bootstrapping from char24 and pairs.
*/

extern qsptr_t qsutf8_make (qsmachine_t *, qsword len, int fill);
extern qsptr_t qsutf8_inject_charp (qsmachine_t *, const char * s, size_t slen);
extern qsptr_t qsutf8_inject_wcs (qsmachine_t *, const wchar_t * ws, size_t wlen);
extern qsptr_t qsutf8_inject_bytes (qsmachine_t *, uint8_t * buf, qsword buflen);
extern bool qsutf8_p (const qsmachine_t *, qsptr_t p);
extern qsword qsutf8_length (const qsmachine_t *, qsptr_t p);
extern int qsutf8_ref (const qsmachine_t *, qsptr_t p, qsword k);
/* Copy string from Scheme space to C space: number of bytes of copied. */
extern int qsutf8_fetch (const qsmachine_t *, qsptr_t p, char * buf, int buflen);
/* C pointer to Scheme string data (already encoded as C string). */
extern const char * qsutf8_get (const qsmachine_t *, qsptr_t p, qsword * len);
extern qsptr_t qsutf8_setq (qsmachine_t *, qsptr_t p, qsword k, int ch);
/* increment reference count. */
extern qserr_t qsutf8_hold (qsmachine_t *, qsptr_t p);
/* decrement reference count. */
extern qserr_t qsutf8_release (qsmachine_t *, qsptr_t p);
extern int qsutf8_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

extern qsptr_t qsbytevec_make (qsmachine_t *, qsword len, qsbyte fill);
extern bool qsbytevec_p (const qsmachine_t *, qsptr_t p);
extern qsword qsbytevec_length (const qsmachine_t *, qsptr_t p);
extern bool qsbytevec_extract (const qsmachine_t *, qsptr_t p, const uint8_t ** out_uint8ptr, qsword * out_size);
extern qsbyte qsbytevec_ref (const qsmachine_t *, qsptr_t p, qsword k);
extern qsptr_t qsbytevec_setq (qsmachine_t *, qsptr_t p, qsword k, qsbyte val);
extern int qsbytevec_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

extern qsptr_t qsenv_make (qsmachine_t *, qsptr_t next_env);
extern qsptr_t qsenv_insert (qsmachine_t *, qsptr_t env, qsptr_t variable, qsptr_t binding);
extern qsptr_t qsenv_lookup (qsmachine_t *, qsptr_t env, qsptr_t variable);
extern int qsenv_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

extern qsptr_t qskont_make (qsmachine_t *, qsptr_t variant, qsptr_t c, qsptr_t e, qsptr_t k);
extern qsptr_t qskont_make_current (qsmachine_t *);
extern bool qskont_p (const qsmachine_t *, qsptr_t p);
extern qsptr_t qskont_ref_v (const qsmachine_t *, qsptr_t p);
extern qsptr_t qskont_ref_c (const qsmachine_t *, qsptr_t p);
extern qsptr_t qskont_ref_e (const qsmachine_t *, qsptr_t p);
extern qsptr_t qskont_ref_k (const qsmachine_t *, qsptr_t p);
extern int qskont_fetch (const qsmachine_t *, qsptr_t p, qsptr_t * out_v, qsptr_t * out_c, qsptr_t * out_e, qsptr_t * out_k);
extern int qskont_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

extern qsptr_t qslambda_make (qsmachine_t *, qsptr_t parameters, qsptr_t body);
extern bool qslambda_p (const qsmachine_t *, qsptr_t p);
extern qsptr_t qslambda_ref_param (const qsmachine_t *, qsptr_t p);
extern qsptr_t qslambda_ref_body (const qsmachine_t *, qsptr_t p);
extern int qslambda_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

extern qsptr_t qsclosure_make (qsmachine_t *, qsptr_t lambda, qsptr_t env);
extern bool qsclosure_p (const qsmachine_t *, qsptr_t p);
extern qsptr_t qsclosure_ref_lam (const qsmachine_t *, qsptr_t p);
extern qsptr_t qsclosure_ref_env (const qsmachine_t *, qsptr_t p);
extern int qsclosure_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);


/* Heavy Port (ports that have to track their own state inside Scheme) */
extern qsptr_t qscport_make (qsmachine_t * mach, qsptr_t variant, qsptr_t pathspec, bool writeable, qsptr_t host_resource);
extern bool qscport_p (qsmachine_t *, qsptr_t p);
extern bool qscport_get_writeable (qsmachine_t * mach, qsptr_t p);
extern qsptr_t qscport_get_pathspec (qsmachine_t * mach, qsptr_t p);
extern int qscport_get_pos (qsmachine_t * mach, qsptr_t p);
extern int qscport_get_max (qsmachine_t * mach, qsptr_t p);
extern qsptr_t qscport_get_resource (qsmachine_t * mach, qsptr_t p);
extern qsptr_t qscport_set_writeable (qsmachine_t * mach, qsptr_t p, bool val);
extern qsptr_t qscport_set_pathspec (qsmachine_t * mach, qsptr_t p, qsptr_t val);
extern qsptr_t qscport_set_pos (qsmachine_t * mach, qsptr_t p, int pos);
extern qsptr_t qscport_set_max (qsmachine_t * mach, qsptr_t p, int max);
extern qsptr_t qscport_set_resource (qsmachine_t * mach, qsptr_t p, qsptr_t val);

/* C Character Pointer (String) ports. */
extern qsptr_t qscharpport_make (qsmachine_t *, uint8_t * buf, int buflen);
extern bool qscharpport_p (qsmachine_t *, qsptr_t p);
extern bool qscharpport_eof (qsmachine_t *, qsptr_t p);
extern int qscharpport_read_u8 (qsmachine_t *, qsptr_t p);
extern bool qscharpport_write_u8 (qsmachine_t *, qsptr_t p, int byte);
extern bool qscharpport_close (qsmachine_t *, qsptr_t p);
extern int qscharpport_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

/* OctetVector (utf-8 string) ports. */
extern qsptr_t qsovport_make (qsmachine_t *, qsptr_t s);
extern bool qsovport_p (qsmachine_t *, qsptr_t s);
extern int qsovport_read_u8 (qsmachine_t *, qsptr_t p);
extern bool qsovport_write_u8 (qsmachine_t *, qsptr_t p, int byte);
extern bool qsovport_close (qsmachine_t *, qsptr_t p);
extern int qsovport_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

/* Port backed by Standard C File. */
extern qsptr_t qsfport_make (qsmachine_t *, const char * path, const char * mode);
extern bool qsfport_p (qsmachine_t *, qsptr_t p);
extern bool qsfport_eof (qsmachine_t *, qsptr_t p);
extern FILE * qsfport_get (qsmachine_t *, qsptr_t p);
extern int qsfport_read_u8 (qsmachine_t *, qsptr_t p);
extern bool qsfport_write_u8 (qsmachine_t *, qsptr_t p, int byte);
extern qsword qsfport_tell (qsmachine_t *, qsptr_t p);
extern bool qsfport_seek (qsmachine_t *, qsptr_t p, qsword pos);
extern bool qsfport_close (qsmachine_t *, qsptr_t p);
extern int qsfport_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

/* Overall Port wrapper. */
extern qsptr_t qsport_make (qsmachine_t *, qsptr_t variant, qsptr_t path, bool writeable, bool appending);
extern qsptr_t qsport_make_c (qsmachine_t *, qsptr_t variant, const uint8_t * spec, int speclen, bool writeable, bool appending);
extern bool qsport_p (qsmachine_t *, qsptr_t p);
extern bool qsport_eof (qsmachine_t *, qsptr_t p);
extern int qsport_read_u8 (qsmachine_t *, qsptr_t p);
extern bool qsport_write_u8 (qsmachine_t *, qsptr_t p, int byte);
extern qsword qsport_tell (qsmachine_t *, qsptr_t p);
extern bool qsport_seek (qsmachine_t *, qsptr_t p, qsword pos);
extern bool qsport_close (qsmachine_t *, qsptr_t p);


/* the iterator type allows for iterating both pairs and arrays as a list.
*/
extern qsptr_t qsiter_make (const qsmachine_t *, qsaddr_t addr);
extern qsptr_t qsiter_begin (const qsmachine_t *, qsptr_t p);
extern bool qsiter_p (const qsmachine_t *, qsptr_t p);
extern qsptr_t qsiter_head (const qsmachine_t *, qsptr_t p);
extern qsptr_t qsiter_tail (const qsmachine_t *, qsptr_t p);
extern int qsiter_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

/* Abstraction of QsSym and QsName */
extern bool qssymbol_p (const qsmachine_t *, qsptr_t p);
/* convert string object to interned symbol; returns QsSym pointer. */
extern qsptr_t qssymbol_bless (qsmachine_t *, qsptr_t s);
/* intern name object into symbol table; returns QsSym pointer. */
extern qsptr_t qssymbol_intern (qsmachine_t *, qsptr_t p);
/* intern symbol name from C string; returns QsSym pointer. */
extern qsptr_t qssymbol_intern_c (qsmachine_t *, const char * cstr, int slen);
/* gets symbol name from QsName or QsSym. */
extern const char * qssymbol_get (const qsmachine_t *, qsptr_t p);
extern int qssymbol_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);
extern qscmp_t qssymbol_cmp (const qsmachine_t *, qsptr_t x, qsptr_t y);

/* Abstraction of QsCharvec and QsUtf8 */
extern bool qsstring_p (const qsmachine_t *, qsptr_t p);
extern qsword qsstring_length (const qsmachine_t *, qsptr_t p);
extern int qsstring_ref (const qsmachine_t *, qsptr_t p, qsword k);
extern qsptr_t qsstring_setq (qsmachine_t *, qsptr_t p, qsword k, int ch);
extern int qsstring_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);


/* Generalized stringification. */
extern int qsptr_crepr (const qsmachine_t *, qsptr_t p, char * buf, int buflen);

#endif /* QSVAL_H_ */
