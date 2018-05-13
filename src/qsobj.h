#ifndef _QSOBJ_H_
#define _QSOBJ_H_

#include <stdio.h>
#include <stdbool.h>

#include "qsptr.h"
#include "qsheap.h"

/* Objects stored in memory/heap */

/*

struct qsobj_s {
  qsptr_t mgmt;  // handled by qsheap.
  qsptr_t _0;
  qsptr_t _1;
  qsptr_t _2;
};


Pointer types (mgmt.o=0):
type	| allocscale	| _0 (discrim)	| _1		| _2
cons	| 0		| =QSNIL	| car		| cdr
tree	| 0		| :ptr   	| :ptr   	| :ptr
ibtree	| 0             | :int30	| idx0:ptr	| _1:ptr
symbol	| 0		| self:ptr	| name:ptr	| val:int30
immlist	| >0		| =QSNIL	| gc_backtrack	| gc_iter
vector	| >0		| :int30	| gc_backtrack	| gc_iter
rbtree	| =1		| QSRBTREE	| top_node:ptr	| key_cmp:ptr
			| mutex:int30	| up:ptr	| down:ptr

Octet types (mgmt.o=1):
type	| allocscale	| _0 (discrim)	| _1		| _2
long	| 0		| 'QSFIXNUM	| int64_t
double	| 0		| 'QSFLONUM	| double
portFILE| 1		| 'QSFILE	| FILE*
	    _3=name:ptr     _4=attrs
portFD	| 0		| 'QSFD		| fd:int32_t	| mode:int
portMEM	| 0		| str:ptr26	| ofs:int30	| max:int30
bytevec	| >0		| :int30	| refcount	| mutex
utf8	| >0		| =QSNIL	| refcount	| mutex
utf32	| >0		| =QSCHAR24(0)	| refcount	| mutex
*/

/*
String subtypes:
1. character vector (utf-24): vector, elements are :char24
2. character list (utf-24): cons list, car are :char24
3. character immlist (utf-24): immlist, elements are :char24
4. C string (utf-8): bytevec
*/


/* types that extend qsobj_t */
#if 0
union qsobjform_u {
  struct qsobj_s generic;  // defined in qsheap.h
  struct qscons_s { qsptr_t mgmt; qsptr_t _N; qsptr_t a; qsptr_t d; } cons;
  struct qstree_s { qsptr_t mgmt; qsptr_t left; qsptr_t data; qsptr_t right; } tree;
  struct qsimmlist_s { qsptr_t mgmt; qsptr_t _N; qsptr_t gc_backtrack; qsptr_t gc_iter; qsptr_t _d[]; } immlist;
  struct qsvector_s { qsptr_t mgmt; qsptr_t len; qsptr_t gc_backtrack; qsptr_t gc_iter; qsptr_t _d[]; } vector;

  struct qswidefixnum_s { qsptr_t mgmt; qsptr_t variant; int64_t l; } widefixnum;
  struct qswideflonum_s { qsptr_t mgmt; qsptr_t variant; double d; } wideflonum;
#ifdef FILE
  struct qsportFILE_s { qsptr_t mgmt; qsptr_t variant; FILE * f; } portFILE;
#endif // FILE
  struct qsportFD_s { qsptr_t mgmt; qsptr_t variant; int fd; } portFD;
  struct qsbytevec_s { qsptr_t mgmt; qsptr_t len; qsptr_t refcount; qsptr_t mutex; } bytevec;
};
#endif //0


/* template:
qsOBJ_t * OBJ (qsmem_t * mem, qsptr_t PTR);
qsptr_t qsOBJ_get (qsmem_t * mem, qsptr_t PTR);
qsptr_t qsOBJ_setq (qsmem_t * mem, qsptr_t PTR,  );
qsptr_t qsOBJ_make (qsmem_t * mem,  )
int qsOBJ_alloc (qsmem_t * mem, qsptr_t * out_ptr, qsmemaddr_t * out_addr,  )
int qsOBJ_crepr (qsmem_t * mem, qsptr_t * PTR, char * buf, int buflen)
*/

/* Function naming convention:

  object indicators are qsptr_t (usually store + object as qsptr_t)
    _p (...): predicate
    _ref... (...): accessor
    _setq... (...): mutator

  object indicators are native-C types (usually pointer to object).
    _is_... (...): predicate
    _get_... (...): accessor
    _fetch_... (..., *): accessor using out-parameter (returns error code).
    _set_... (...): mutator
*/


typedef qsheap_t qsmem_t;
typedef qsheapaddr_t qsmemaddr_t;



/*
   bit pattern
   210
     l  less-than
    e   equal-to
   g    greater-than
*/

typedef enum cmp_e {
    CMP_NE    = 0,  /* Also for incomparable? */
    CMP_LT    = 1,
    CMP_GT    = 2,
    CMP_EQ    = 3,
} cmp_t;



/* deprecated in favor of qsbay */
typedef struct qsobj_s {
    qsptr_t mgmt;
    qsptr_t _0;
    qsptr_t _1;
    qsptr_t _2;
} qsobj_t;

qsobj_t * qsobj (qsmem_t * mem, qsptr_t p, qsmemaddr_t * out_addr);
int qsobj_p (qsmem_t * mem, qsptr_t p);
int qsobj_used_p (qsmem_t * mem, qsptr_t p);
int qsobj_marked_p (qsmem_t * mem, qsptr_t p);
int qsobj_ref_score (qsmem_t * mem, qsptr_t p);
int qsobj_ref_parent (qsmem_t * mem, qsptr_t p);
qsword qsobj_ref_allocsize (qsmem_t * mem, qsptr_t p);   // number of heapcells in allocation unit.
int qsobj_ref_allocscale (qsmem_t * mem, qsptr_t p);
qsptr_t qsobj_setq_marked (qsmem_t * mem, qsptr_t p, qsword val);
qsptr_t qsobj_setq_parent (qsmem_t * mem, qsptr_t p, qsword val);
qsptr_t qsobj_setq_score (qsmem_t * mem, qsptr_t p, qsword val);

/* Fetch arbitrary pointer field from start of object.

   single-bay pointer: 0=mgmt, 1=e, 2=a, 3=d
   multi-bay pointer: 0=mgmt, 1=len, 2=gc_backtrack, 3=gc_iter, 4=data[0],...
   single-bay octet: 0=mgmt, 1=variant
   multi-bay octet: 0=mgmt, 1=len, 2=refcount, 3=mutex

   Returns QSERROR if access denied (typically out-of-bounds).
*/
qsptr_t qsobj_ref_ptr (qsmem_t * mem, qsptr_t p, qsword ofs);
/* Fetch arbitrary octet (byte) from start of octet array.
   single-bay octet: 0..7
   multi-bay octet: 0...

   Return -1 if access denied (out-of-bounds).
*/
int qsobj_ref_octet (qsmem_t * mem, qsptr_t p, qsword ofs);
/* Returns object on success, QSERROR_* on error. */
qsptr_t qsobj_setq_ptr (qsmem_t *, qsptr_t p, qsword ofs, qsptr_t val);
/* Returns object on success, QSERROR_* on error. */
qsptr_t qsobj_setq_octet (qsmem_t *, qsptr_t p, qsword ofs, int val);
/* Returns pointer to start of arbitrary data (->_d[0]).
   NULL if no such field available.
   Writes out total valid size (bytes) to '*size', ignored if NULL.
*/
void * qsobj_ref_data (qsmem_t *, qsptr_t p, size_t * size);

qsptr_t qsobj_make (qsmem_t * mem, qsword k, int octetate, qsmemaddr_t * out_addr);
cmp_t qsobj_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);
qserror_t qsobj_kmark (qsmem_t * mem, qsptr_t p);
int qsobj_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen);


typedef struct qstree_s {
    qsptr_t mgmt;
    qsptr_t left;
    qsptr_t data;
    qsptr_t right;
} qstree_t;

bool qstree_p (qsmem_t * mem, qsptr_t p);
qsptr_t qstree_ref_left (qsmem_t * mem, qsptr_t p);
qsptr_t qstree_ref_data (qsmem_t * mem, qsptr_t p);
qsptr_t qstree_ref_right (qsmem_t * mem, qsptr_t p);
qsptr_t qstree_setq_left (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qstree_setq_data (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qstree_setq_right (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qstree_make (qsmem_t * mem, qsptr_t left, qsptr_t data, qsptr_t right);
qserror_t qstree_kmark (qsmem_t * mem, qsptr_t p, qsptr_t backptr, qsptr_t * nextptr);
cmp_t qstree_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);
int qstree_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen);

qsptr_t qstree_find (qsmem_t * mem, qsptr_t p, qsptr_t key, qsptr_t * nearest);
qsptr_t qstree_assoc (qsmem_t * mem, qsptr_t p, qsptr_t key);


/* Manage linked collection of tree nodes as a Red-Black tree. */
typedef struct qsrbtree_s {
    qsptr_t mgmt;   /* ptr, allocscale=1 */
    qsptr_t variant;  /* tag QSRBTREE */
    qsptr_t gc_backtrack;
    qsptr_t gc_iter;

    qsptr_t top;    /* Top-most node of tree (i.e. root node). */
    qsptr_t cmp;    /* Comparator function.  Default (nil) => string-cmp */
    qsptr_t up;	    /* Parent node of a split point. */
    qsptr_t down;   /* Top of subtree that has been split. */
} qsrbtree_t;

/* Perform rotation on arbitrary node (presumably in a red-black tree) */
qsptr_t qsrbnode_rotate_left (qsmem_t * mem, qsptr_t pivot);
qsptr_t qsrbnode_rotate_right (qsmem_t * mem, qsptr_t pivot);

//qsptr_t qsrbtree_make (qsmem_t * mem, qsptr_t top_node);
bool qsrbtree_p (qsmem_t * mem, qsptr_t p);

qsptr_t qsrbtree_ref_top (qsmem_t * mem, qsptr_t p);
qsptr_t qsrbtree_ref_cmp (qsmem_t * mem, qsptr_t p);
qsptr_t qsrbtree_ref_up (qsmem_t * mem, qsptr_t p);
qsptr_t qsrbtree_ref_down (qsmem_t * mem, qsptr_t p);
qsptr_t qsrbtree_setq_top (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qsrbtree_setq_cmp (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qsrbtree_setq_up (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qsrbtree_setq_down (qsmem_t * mem, qsptr_t p, qsptr_t val);

qsptr_t qsrbtree_ref_uncle (qsmem_t * mem, qsptr_t p);
qsptr_t qsrbtree_ref_grandparent (qsmem_t * mem, qsptr_t p);

// (mutably) traverse tree, splitting with reversal pointers.
qsptr_t qsrbtree_split_left (qsmem_t * mem, qsptr_t p);
qsptr_t qsrbtree_split_right (qsmem_t * mem, qsptr_t p);
// go up one level, splicing back together a split tree (undo split).
qsptr_t qsrbtree_mend (qsmem_t * mem, qsptr_t p);

/* Insert data node (association pair) into red-black tree. */
qsptr_t qsrbtree_insert (qsmem_t * mem, qsptr_t p, qsptr_t apair);
/* Find tree node in red-black tree; and/or best match (via comparison) where
   tree node can be attached as a child. */
qsptr_t qsrbtree_find (qsmem_t * mem, qsptr_t p, qsptr_t key, qsptr_t * nearest_match);
/* Find data node (assocation pair) based on key in red-black tree. */
qsptr_t qsrbtree_assoc (qsmem_t * mem, qsptr_t p, qsptr_t key);

qsptr_t qsrbtree_make (qsmem_t * mem, qsptr_t top_node, qsptr_t cmp);
cmp_t qsrbtree_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);
int qsrbtree_crepr (qsmem_t * mem, qsptr_t rbtree, char * buf, int buflen);




/* Indexed-binary tree. */
typedef struct qsibtree_s {
    qsptr_t mgmt;
    qsptr_t filled; /* :int30 */
    qsptr_t idx0;   /* [0] is special case. */
    qsptr_t ones;   /* everything else. */
} qsibtree_t;

bool qsibtree_p (qsmem_t * mem, qsptr_t p);
qsword qsibtree_ref_filled (qsmem_t * mem, qsptr_t p);
qsptr_t qsibtree_ref_idx0 (qsmem_t * mem, qsptr_t t);
qsptr_t qsibtree_ref_ones (qsmem_t * mem, qsptr_t p);
qsptr_t qsibtree_setq_filled (qsmem_t * mem, qsptr_t p, qsword val);
qsptr_t qsibtree_setq_idx0 (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qsibtree_setq_ones (qsmem_t * mem, qsptr_t p, qsptr_t val);

qsptr_t qsibnode_find (qsmem_t * mem, qsptr_t p, qsword path);

qsptr_t qsibtree_find (qsmem_t * mem, qsptr_t p, qsword idx);

/* Main entry points: (ibtree-ref IBTREE NTH), (ibtree-set! IBTREE NTH VALUE) */
qsword qsibtree_length (qsmem_t * mem, qsptr_t p);
qsptr_t qsibtree_ref (qsmem_t * mem, qsptr_t p, qsword idx);
qsptr_t qsibtree_setq (qsmem_t * mem, qsptr_t p, qsword idx, qsptr_t val);
qsptr_t qsibtree_make (qsmem_t * mem);
cmp_t qsibtree_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);




/* Pair - linked list as degenerate case of tree with no left child */
typedef struct qspair_s {
    qsptr_t mgmt;
    qsptr_t required_null;
    qsptr_t a;
    qsptr_t d;
} qspair_t;

bool qspair_p (qsmem_t * mem, qsptr_t p);
qsptr_t qspair_ref_a (qsmem_t * mem, qsptr_t p);
qsptr_t qspair_ref_d (qsmem_t * mem, qsptr_t p);
qsptr_t qspair_setq_a (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qspair_setq_d (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qspair_make (qsmem_t * mem, qsptr_t a, qsptr_t d);
cmp_t qspair_cmp (qsmem_t * mem, qsptr_t x, qsptr_t y);
int qspair_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen);


bool qslist_p (qsmem_t * mem, qsptr_t p);
qsword qslist_length (qsmem_t * mem, qsptr_t p);
qsword qslist_tail (qsmem_t * mem, qsptr_t p, qsword k);
qsword qslist_ref (qsmem_t * mem, qsptr_t p, qsword k);
qsptr_t qslist_make (qsmem_t * mem, ...);


/* Vector [of poiners] */
typedef struct qsvector_s {
    qsptr_t mgmt;
    qsptr_t len;
    qsptr_t gc_backtrack;
    qsptr_t gc_iter;
    qsptr_t _d[];
} qsvector_t;

bool qsvector_p (qsmem_t * mem, qsptr_t p);
qsword qsvector_length (qsmem_t * mem, qsptr_t p);
qsptr_t qsvector_ref (qsmem_t * mem, qsptr_t p, qsword ofs);
qsptr_t qsvector_setq (qsmem_t * mem, qsptr_t p, qsword ofs, qsptr_t val);
qsptr_t qsvector_make (qsmem_t * mem, qsword k, qsptr_t fill);
cmp_t qsvector_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);
qserror_t qsvector_mark (qsmem_t * mem, qsword k);
qserror_t qsvector_kmark (qsmem_t * mem, qsptr_t p, qsptr_t backptr, qsptr_t * next);
int qsvector_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen);
qsword qsvector_inject (qsmem_t * mem, qsword nelts, qsptr_t * carray);
qsptr_t qsvector_iter (qsmem_t * mem, qsptr_t p, qsword ofs);
qsptr_t * qsvector_cptr (qsmem_t * mem, qsptr_t p, qsword * out_len);


/* Immutable List; Read-Only List; Array.  Borrows from CDR-coded lists. */
typedef struct qsimmlist_s {
    qsptr_t mgmt;
    qsptr_t reqnull;
    qsptr_t gc_backtrack;
    qsptr_t gc_iter;
    qsptr_t _d[];
} qsimmlist_t;

bool qsimmlist_p (qsmem_t * mem, qsptr_t p);
/* Raw size, including encoding details. */
qsword qsimmlist_len (qsmem_t * mem, qsptr_t p);
/* Raw access to list contents, including encoding details. */
qsptr_t qsimmlist_at (qsmem_t * mem, qsptr_t p, qsword ofs);
/* Adjusted size, accounting for nested lists, in the sense of (length). */
qsword qsimmlist_length (qsmem_t * mem, qsptr_t p);
/* Adjust for nested lists, thus in the sense of (list-ref) */
qsptr_t qsimmlist_ref (qsmem_t * mem, qsptr_t p, qsword ofs);
/*
qsptr_t qsimmlist_setq (qsmem_t * mem, qsptr_t a, qsword ofs, qsptr_t val);
*/
/* iter() generates an iterator, subject to car/cdr for traversal. */
qsptr_t qsimmlist_iter (qsmem_t * mem, qsptr_t p, qsword ofs);
qsptr_t qsimmlist_make (qsmem_t * mem, qsword k, qsptr_t fill);
/* Create immlist from an image in C's space. */
qsptr_t qsimmlist_inject (qsmem_t * mem, qsptr_t * cmem, qsword nptrs);
/* Specify elements as arguments, terminated with QSEOL */
qsptr_t qsimmlist_injectv (qsmem_t * mem, va_list vp);
qsptr_t qsimmlist_injectl (qsmem_t * mem, ...);
int qsimmlist_crepr (qsmem_t * mem, qsptr_t v, char * buf, int buflen);


/* Environment (bindings for free variables). */
typedef struct qsenv_s {
    qsptr_t mgmt;
    qsptr_t tag;
    qsptr_t dict;
    qsptr_t next;
} qsenv_t;

bool qsenv_p (qsmem_t * mem, qsptr_t p);
qsword qsenv_length (qsmem_t * mem, qsptr_t p);
qsptr_t qsenv_ref_dict (qsmem_t * mem, qsptr_t p);
qsptr_t qsenv_ref_next (qsmem_t * mem, qsptr_t p);
qsptr_t qsenv_setq_dict (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qsenv_setq_next (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qsenv_assoc (qsmem_t * mem, qsptr_t p, qsptr_t key);
qsptr_t qsenv_setq (qsmem_t * mem, qsptr_t p, qsptr_t key, qsptr_t val);
qsptr_t qsenv_ref (qsmem_t * mem, qsptr_t p, qsptr_t key);
qsptr_t qsenv_make (qsmem_t * mem, qsptr_t next);
cmp_t qsenv_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);
int qsenv_crepr (qsmem_t * mem, qsptr_t e, char * buf, int buflen);


typedef struct qslambda_s {
    qsptr_t mgmt;
    qsptr_t variant;
    qsptr_t param;
    qsptr_t body;
} qslambda_t;

bool qslambda_p (qsmem_t * mem, qsptr_t p);
qsptr_t qslambda_ref_param (qsmem_t * mem, qsptr_t p);
qsptr_t qslambda_ref_body (qsmem_t * mem, qsptr_t p);
qsptr_t qslambda_setq_param (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qslambda_setq_body (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qslambda_make (qsmem_t * mem, qsptr_t param, qsptr_t body);
cmp_t qslambda_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);
int qslambda_crepr (qsmem_t * mem, qsptr_t lam, char * buf, int buflen);


typedef struct qsclosure_s {
    qsptr_t mgmt;
    qsptr_t variant;
    qsptr_t env;
    qsptr_t lambda;
} qsclosure_t;

bool qsclosure_p (qsmem_t * mem, qsptr_t p);
qsptr_t qsclosure_ref_env (qsmem_t * mem, qsptr_t p);
qsptr_t qsclosure_ref_lambda (qsmem_t * mem, qsptr_t p);
qsptr_t qsclosure_setq_env (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qsclosure_setq_lambda (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qsclosure_make (qsmem_t * mem, qsptr_t env, qsptr_t lambda);
cmp_t qsclosure_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);
int qsclosure_crepr (qsmem_t * mem, qsptr_t clo, char * buf, int buflen);


typedef union qskont_u {
    struct qskont_halt_s {
	qsptr_t mgmt;
	qsptr_t variant;
	qsptr_t gc_backtrack;
	qsptr_t gc_iter;

	qsptr_t _0;
	qsptr_t _1;
	qsptr_t _2;
	qsptr_t _3;
    } halt;
    /* First-class continuation. */
    struct qskont_cont_s {
	qsptr_t mgmt;
	qsptr_t variant;
	qsptr_t gc_backtrack;
	qsptr_t gc_iter;

	qsptr_t e;
	qsptr_t k;
	qsptr_t c;
	qsptr_t _3;
    } cont;
    struct qskont_letk_s {
	qsptr_t mgmt;
	qsptr_t variant;
	qsptr_t gc_backtrack;
	qsptr_t gc_iter;

	qsptr_t e;
	qsptr_t k;
	qsptr_t body;
	qsptr_t v;
    } letk;
} qskont_t;

bool qskont_p (qsmem_t * mem, qsptr_t p);
qsptr_t qskont_ref_env (qsmem_t * mem, qsptr_t p);
qsptr_t qskont_ref_kont (qsmem_t * mem, qsptr_t p);
qsptr_t qskont_ref_code (qsmem_t * mem, qsptr_t p);
qsptr_t qskont_ref_other (qsmem_t * mem, qsptr_t p);
qsptr_t qskont_setq_env (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qskont_setq_kont (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qskont_setq_code (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qskont_setq_other (qsmem_t * mem, qsptr_t p, qsptr_t val);
/* arguments listed in order of likeliness of being nil. */
qsptr_t qskont_make (qsmem_t * mem, qsptr_t variant, qsptr_t k, qsptr_t e, qsptr_t c, qsptr_t o);
qsptr_t qskont_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);
int qskont_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen);




/* byte vector, C uint8_t[] embedded in Scheme store */
typedef struct qsbytevec_s {
    qsptr_t mgmt;
    qsptr_t len;
    uint32_t refcount;	  // references from outside Scheme memory.
    uint32_t mutex;	  // mutex on refcount.
    uint8_t _d[];
} qsbytevec_t;

bool qsbytevec_p (qsmem_t * mem, qsptr_t p);
qsword qsbytevec_length (qsmem_t * mem, qsptr_t p);
int qsbytevec_ref (qsmem_t * mem, qsptr_t p, qsword ofs);
qsptr_t qsbytevec_setq (qsmem_t * mem, qsptr_t p, qsword ofs, qsword val);
qsptr_t qsbytevec_make (qsmem_t * mem, qsword k, qsptr_t fill);
cmp_t qsbytevec_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);
int qsbytevec_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen);
uint8_t * qsbytevec_cptr (qsmem_t * mem, qsptr_t p, qsword * out_len);
qsptr_t qsbytevec_inject (qsmem_t * mem, qsword nbytes, uint8_t * carray);




/* String variants:
  * strL: cons list of qschar24
  * strV: vector of qschar24
  * strI: immlist of qschar24
  * strC: bytevector of uint8_t (utf-8)
  * strW: bytevector of wchar_t (utf-32?)
*/

#if 0
qsstrL_t * qsstrL (qsmem_t * mem, qsptr_t s);
qsword qsstrL_length (qsmem_t * mem, qsptr_t s);
qsptr_t qsstrL_ref (qsmem_t * mem, qsptr_t s, qsword nth);
qsptr_t qsstrL_setq (qsmem_t * mem, qsptr_t s, qsword nth, qsword ch);
qsptr_t qsstrL_append (qsmem_t * mem, qsptr_t s, qsword nth, qsword ch);
qsptr_t qsstrL_make (qsmem_t * mem);
int qsstrL_crepr (qsmem_t * mem, qsptr_t v, char * buf, int buflen);
#endif //0




/* Wide numbers - numbers stored in heap.
*/

typedef struct qswidenum_s {
    qsptr_t mgmt;
    qsptr_t variant;
    union {
	int64_t l;
	double d;
	struct {
	    int32_t p;
	    int32_t q;
	} ipair;
	struct {
	    float a;
	    float b;
	} fpair;
    } payload;
} qswidenum_t;

bool qswidenum_p (qsmem_t * mem, qsptr_t p);
qsnumtype_t qswidenum_variant (qsmem_t * mem, qsptr_t p);
qswidenum_t * qswidenum_premake (qsmem_t * mem, qsnumtype_t variant, qsptr_t * out_ptr);
cmp_t qswidenum_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);

bool qslong_p (qsmem_t * mem, qsptr_t p);
qserror_t qslong_fetch (qsmem_t * mem, qsptr_t p, long * out_long);
long qslong_get (qsmem_t * mem, qsptr_t p);
qsptr_t qslong_make (qsmem_t * mem, long val);
/* construct from high,low pair of 32b integers. */
qsptr_t qslong_make2 (qsmem_t * mem, int32_t high, uint32_t low);
int qslong_crepr (qsmem_t * mem, qsptr_t l, char * buf, int buflen);
cmp_t qslong_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);

bool qsdouble_p (qsmem_t * mem, qsptr_t p);
qserror_t qsdouble_fetch (qsmem_t * mem, qsptr_t p, double * out_double);
double qsdouble_get (qsmem_t * mem, qsptr_t p);
qsptr_t qsdouble_make (qsmem_t * mem, double val);
int qsdouble_crepr (qsmem_t * mem, qsptr_t d, char * buf, int buflen);
cmp_t qsdouble_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);

// qsipair (rational)
qserror_t qsipair_fetch (qsmem_t * mem, qsptr_t p, int * out_p, int * out_q);
qsptr_t psipair_make (qsmem_t * mem, int32_t p, int32_t q);
int qsipair_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen);
cmp_t qsipair_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);

// qsfpair (complex)
qserror_t qsfpair_fetch (qsmem_t * mem, qsptr_t q, float * out_a, float * out_b);
qsptr_t qsfpair_make (qsmem_t * mem, float a, float b);
int qsfpair_crepr (qsmem_t * mem, qsptr_t z, char * buf, int buflen);
cmp_t qsfpair_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);




/* word-sized objects:
  * qsfloat (float31)
  * qsint (int30)
  * qsiter (iter28)
  * qsobj (obj26)
  * qschar (char24)
  * qserr (error16)
  * qsconst (const16)
*/


/* iterator pointer

Intended to allow list-style access (CAR, CDR) to array elements (immlist).
Also usable on vector.
Numeric value is word-level access to heap.
iter_next() keeps returning iterator to its next word address until content is EOL (then it returns null pointer).


bug: does not respect weak referencing -- iterator will still exist even if
iterated object is freed.
*/

qsptr_t qsiter (qsmem_t * mem, qsptr_t it);
bool qsiter_p (qsmem_t * mem, qsptr_t p);
qsword qsiter_length (qsmem_t * mem, qsptr_t p);
int qsiter_on_pair (qsmem_t * mem, qsptr_t it, qsptr_t * out_pairptr);
qsword qsiter_get (qsmem_t * mem, qsptr_t it);
qsptr_t qsiter_item (qsmem_t * mem, qsptr_t it);  // also car
qsptr_t qsiter_next (qsmem_t * mem, qsptr_t it);  // also cdr
qsptr_t qsiter_make (qsmem_t * mem, qsmemaddr_t addr);
cmp_t qsiter_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);


qsptr_t qsint (qsmem_t * mem, qsptr_t i);
bool qsint_p (qsmem_t * mem, qsptr_t p);
int32_t qsint_get (qsmem_t * mem, qsptr_t i);
qsptr_t qsint_make (qsmem_t * mem, int32_t val);
int qsint_crepr (qsmem_t * mem, qsptr_t i, char * buf, int buflen);
cmp_t qsint_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);


qsptr_t qsfloat (qsmem_t * mem, qsptr_t f);
bool qsfloat_p (qsmem_t * mem, qsptr_t p);
float qsfloat_get (qsmem_t * mem, qsptr_t f);
qsptr_t qsfloat_make (qsmem_t * mem, float val);
int qsfloat_crepr (qsmem_t * mem, qsptr_t f, char * buf, int buflen);
cmp_t qsfloat_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);


qsptr_t qschar (qsmem_t * mem, qsptr_t c);
bool qschar_p (qsmem_t * mem, qsptr_t c);
int qschar_get (qsmem_t * mem, qsptr_t c);
qsptr_t qschar_make (qsmem_t * mem, int val);
int qschar_crepr (qsmem_t * mem, qsptr_t c, char * buf, int buflen);
cmp_t qschar_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);


qsptr_t qserr (qsmem_t * mem, qsptr_t e);
bool qserr_p (qsmem_t * mem, qsptr_t e);
int qserr_get (qsmem_t * mem, qsptr_t e);
qsptr_t qserr_make (qsmem_t * mem, int errcode);
int qserr_crepr (qsmem_t * mem, qsptr_t c, char * buf, int buflen);
cmp_t qserr_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);


qsptr_t qsconst (qsmem_t * mem, qsptr_t n);
bool qsconst_p (qsmem_t * mem, qsptr_t n);
int qsconst_get (qsmem_t * mem, qsptr_t n);
qsptr_t qsconst_make (qsmem_t * mem, int constcode);
int qsconst_crepr (qsmem_t * mem, qsptr_t c, char * buf, int buflen);
cmp_t qsconst_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);




typedef struct qsutf8_s {
    qsptr_t mgmt;
    qsptr_t variant;
    qsptr_t refcount;
    qsptr_t mutex;
    uint8_t _d[];
} qsutf8_t;

bool qsutf8_p (qsmem_t * mem, qsptr_t p);
qsword qsutf8_length (qsmem_t * mem, qsptr_t p);
int qsutf8_ref (qsmem_t * mem, qsptr_t p, qsword k);
qsptr_t qsutf8_setq (qsmem_t * mem, qsptr_t p, qsword k, qsword u8);
qsptr_t qsutf8_make (qsmem_t * mem, qsword slen);
int qsutf8_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen);
cmp_t qsutf8_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);




bool qsstr_p (qsmem_t * mem, qsptr_t p);
qsword qsstr_length (qsmem_t * mem, qsptr_t p);
qsword qsstr_ref (qsmem_t * mem, qsptr_t p, qsword nth);
qsword qsstr_setq (qsmem_t * mem, qsptr_t p, qsword nth, qsword codepoint);
qsptr_t qsstr_make (qsmem_t * mem, qsword k, qsword codepoint_fill);
qsptr_t qsstr_inject (qsmem_t * mem, const char * cstr, qsword slen);
qsptr_t qsstr_inject_wchar (qsmem_t * mem, const wchar_t * ws, qsword wslen);
qsword qsstr_extract (qsmem_t * mem, qsptr_t p, char * cstr, qsword slen);
qsword qsstr_extract_wchar (qsmem_t * mem, qsptr_t p, wchar_t * ws, qsword wslen);
cmp_t qsstr_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);
int qsstr_crepr (qsmem_t * mem, qsptr_t s, char * buf, int buflen);




/* Symbols */
/* qssym = wraps index into interned symbol table, for (eq? ...) use. */
/* qssymbol = object in heap tying together name and id, for name->id lookup. */ 
qsptr_t qssym (qsmem_t * mem, qsptr_t y);
bool qsssym_p (qsmem_t * mem, qsptr_t p);
qsword qssym_get (qsmem_t * mem, qsptr_t y);
qsptr_t qssym_make (qsmem_t * mem, qsptr_t symbol_id);
int qssym_crepr (qsmem_t * mem, qsptr_t y, char * buf, int buflen);
cmp_t qssym_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);


typedef struct qssymbol_s {
    qsptr_t mgmt;
    qsptr_t indicator;	  // point to self.
    qsptr_t name;
    qsptr_t id;
} qssymbol_t;

bool qssymbol_p (qsmem_t * mem, qsptr_t p);
qsptr_t qssymbol_ref_name (qsmem_t * mem, qsptr_t p);
qsptr_t qssymbol_ref_id (qsmem_t * mem, qsptr_t p);
qsptr_t qssymbol_make (qsmem_t * mem, qsptr_t name);
cmp_t qssymbol_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);


typedef struct qssymstore_s {
    qsptr_t mgmt;
    qsptr_t tag;
    qsptr_t table;
    qsptr_t tree;
} qssymstore_t;

bool qssymstore_p (qsmem_t * mem, qsptr_t p);
qsptr_t qssymstore_make (qsmem_t * mem);
qsptr_t qssymstore_ref_table (qsmem_t * mem, qsptr_t p);
qsptr_t qssymstore_ref_tree (qsmem_t * mem, qsptr_t p);
qsptr_t qssymstore_setq_table (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qssymstore_setq_tree (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qssymstore_intern (qsmem_t * mem, qsptr_t p, qsptr_t y);
qsptr_t qssymstore_ref (qsmem_t * mem, qsptr_t p, qsptr_t key);
qsptr_t qssymstore_assoc (qsmem_t * mem, qsptr_t p, qsptr_t key);
cmp_t qssymstore_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);



/* Ports */
/*  Scheme standard (default) input/output, distinct from C stdin/stdout. */
bool qsSTDIO_p (qsmem_t * mem, qsptr_t p);
qsptr_t qsSTDIO_make (qsmem_t * mem, qsptr_t stdioe);
qsptr_t qsSTDIO_peek_u8 (qsmem_t * mem, qsptr_t p);
qsptr_t qsSTDIO_read_u8 (qsmem_t * mem, qsptr_t p);
qsptr_t qsSTDIO_write_u8 (qsmem_t * mem, qsptr_t p, int octet);
/*
qsptr_t qsSTDIO_read_bytevector (qsmem_t * mem, qsptr_t p);
qsptr_t qsSTDIO_write_bytevector (qsmem_t * mem, qsptr_t p);
*/


qsptr_t qsatom_parse_cstr (qsmem_t * mem, const char * s, int slen);

int qsptr_crepr (qsmem_t * mem, qsptr_t c, char * buf, int buflen);

#endif // _QSOBJ_H_
