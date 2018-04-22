#ifndef _QSOBJ_H_
#define _QSOBJ_H_

#include <stdio.h>

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


typedef qsheap_t qsmem_t;
typedef qsheapaddr_t qsmemaddr_t;


typedef struct qsobj_s {
    qsptr_t mgmt;
    qsptr_t _0;
    qsptr_t _1;
    qsptr_t _2;
} qsobj_t;

qsobj_t * qsobj (qsmem_t * mem, qsptr_t p, qsmemaddr_t * out_addr);
int qsobj_used_p (qsmem_t * mem, qsptr_t p);
int qsobj_marked_p (qsmem_t * mem, qsptr_t p);
int qsobj_ref_score (qsmem_t * mem, qsptr_t p);
int qsobj_ref_parent (qsmem_t * mem, qsptr_t p);
qsword qsobj_ref_allocsize (qsmem_t * mem, qsptr_t p);   // number of heapcells in allocation unit.
int qsobj_ref_allocscale (qsmem_t * mem, qsptr_t p);
qsptr_t qsobj_setq_marked (qsmem_t * mem, qsptr_t p, qsword val);
qsptr_t qsobj_setq_parent (qsmem_t * mem, qsptr_t p, qsword val);
qsptr_t qsobj_setq_score (qsmem_t * mem, qsptr_t p, qsword val);
qsptr_t qsobj_make (qsmem_t * mem, qsword k, int octetate, qsmemaddr_t * out_addr);
qserror_t qsobj_kmark (qsmem_t * mem, qsptr_t p);
int qsobj_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen);


typedef struct qstree_s {
    qsptr_t mgmt;
    qsptr_t left;
    qsptr_t data;
    qsptr_t right;
} qstree_t;

qstree_t * qstree (qsmem_t * mem, qsptr_t t);
qsptr_t qstree_ref_left (qsmem_t * mem, qsptr_t t);
qsptr_t qstree_ref_data (qsmem_t * mem, qsptr_t t);
qsptr_t qstree_ref_right (qsmem_t * mem, qsptr_t t);
qsptr_t qstree_setq_left (qsmem_t * mem, qsptr_t t, qsptr_t val);
qsptr_t qstree_setq_data (qsmem_t * mem, qsptr_t t, qsptr_t val);
qsptr_t qstree_setq_right (qsmem_t * mem, qsptr_t t, qsptr_t val);
qsptr_t qstree_make (qsmem_t * mem, qsptr_t left, qsptr_t data, qsptr_t right);
qserror_t qstree_kmark (qsmem_t * mem, qsptr_t p, qsptr_t backptr, qsptr_t * nextptr);
int qstree_crepr (qsmem_t * mem, qsptr_t t, char * buf, int buflen);

qsptr_t qstree_find (qsmem_t * mem, qsptr_t t, qsptr_t key, qsptr_t * nearest);
qsptr_t qstree_assoc (qsmem_t * mem, qsptr_t t, qsptr_t key);


/* Manage linked collection of tree nodes as a Red-Black tree. */
typedef struct qsrbtree_s {
    qsptr_t mgmt;   /* ptr, allocscale=1 */
    qsptr_t variant;
    qsptr_t top;    /* Top-most node of tree (i.e. root node). */
    qsptr_t cmp;    /* Comparator function.  Default (nil) => string-cmp */

    qsptr_t reserved4;	/* (set to nil) */
    qsptr_t mutex;	/* Lock for modifying. */
    qsptr_t up;		/* Parent node of a split point. */
    qsptr_t down;	/* Top of subtree that has been split. */
} qsrbtree_t;

/* Perform rotation on arbitrary node (presumably in a red-black tree) */
qsptr_t qsrbnode_rotate_left (qsmem_t * mem, qsptr_t pivot);
qsptr_t qsrbnode_rotate_right (qsmem_t * mem, qsptr_t pivot);

//qsptr_t qsrbtree_make (qsmem_t * mem, qsptr_t top_node);
qsrbtree_t * qsrbtree (qsmem_t * mem, qsptr_t t);
qsptr_t qsrbtree_make (qsmem_t * mem, qsptr_t top_node, qsptr_t cmp);

qsptr_t qsrbtree_ref_top (qsmem_t * mem, qsptr_t rbtree);
qsptr_t qsrbtree_ref_cmp (qsmem_t * mem, qsptr_t rbtree);
qsptr_t qsrbtree_ref_up (qsmem_t * mem, qsptr_t rbtree);
qsptr_t qsrbtree_ref_down (qsmem_t * mem, qsptr_t rbtree);
qsptr_t qsrbtree_setq_top (qsmem_t * mem, qsptr_t rbtree, qsptr_t val);
qsptr_t qsrbtree_setq_cmp (qsmem_t * mem, qsptr_t rbtree, qsptr_t val);
qsptr_t qsrbtree_setq_up (qsmem_t * mem, qsptr_t rbtree, qsptr_t val);
qsptr_t qsrbtree_setq_down (qsmem_t * mem, qsptr_t rbtree, qsptr_t val);

qsptr_t qsrbtree_ref_uncle (qsmem_t * mem, qsptr_t rbtree);
qsptr_t qsrbtree_ref_grandparent (qsmem_t * mem, qsptr_t rbtree);

// (mutably) traverse tree, splitting with reversal pointers.
qsptr_t qsrbtree_split_left (qsmem_t * mem, qsptr_t rbtree);
qsptr_t qsrbtree_split_right (qsmem_t * mem, qsptr_t rbtree);
// go up one level, splicing back together a split tree (undo split).
qsptr_t qsrbtree_mend (qsmem_t * mem, qsptr_t rbtree);

/* Insert data node (association pair) into red-black tree. */
qsptr_t qsrbtree_insert (qsmem_t * mem, qsptr_t root, qsptr_t apair);
/* Find tree node in red-black tree; and/or best match (via comparison) where
   tree node can be attached as a child. */
qsptr_t qsrbtree_find (qsmem_t * mem, qsptr_t rbtree, qsptr_t key, qsptr_t * nearest_match);
/* Find data node (assocation pair) based on key in red-black tree. */
qsptr_t qsrbtree_assoc (qsmem_t * mem, qsptr_t rbtree, qsptr_t key);




/* Indexed-binary tree. */
typedef struct qsibtree_s {
    qsptr_t mgmt;
    qsptr_t filled; /* :int30 */
    qsptr_t idx0;   /* [0] is special case. */
    qsptr_t ones;   /* everything else. */
} qsibtree_t;

qsibtree_t * qsibtree (qsmem_t * mem, qsptr_t t);
qsptr_t qsibtree_make (qsmem_t * mem);
qsword qsibtree_ref_filled (qsmem_t * mem, qsptr_t t);
qsptr_t qsibtree_ref_idx0 (qsmem_t * mem, qsptr_t t);
qsptr_t qsibtree_ref_ones (qsmem_t * mem, qsptr_t t);
qsptr_t qsibtree_setq_filled (qsmem_t * mem, qsptr_t t, qsword val);
qsptr_t qsibtree_setq_idx0 (qsmem_t * mem, qsptr_t t, qsptr_t val);
qsptr_t qsibtree_setq_ones (qsmem_t * mem, qsptr_t t, qsptr_t val);

qsptr_t qsibnode_find (qsmem_t * mem, qsptr_t ibnode, qsword path);

qsptr_t qsibtree_find (qsmem_t * mem, qsptr_t ibnode, qsword idx);

/* Main entry points: (ibtree-ref IBTREE NTH), (ibtree-set! IBTREE NTH VALUE) */
qsword qsibtree_length (qsmem_t * mem, qsptr_t ibnode);
qsptr_t qsibtree_ref (qsmem_t * mem, qsptr_t ibnode, qsword idx);
qsptr_t qsibtree_setq (qsmem_t * mem, qsptr_t ibnode, qsword idx, qsptr_t val);




/* Pair - linked list as degenerate case of tree with no left child */
typedef struct qspair_s {
    qsptr_t mgmt;
    qsptr_t required_null;
    qsptr_t a;
    qsptr_t d;
} qspair_t;

qspair_t * qspair (qsmem_t * mem, qsptr_t p);
qsptr_t qspair_ref_a (qsmem_t * mem, qsptr_t p);
qsptr_t qspair_ref_d (qsmem_t * mem, qsptr_t p);
qsptr_t qspair_setq_a (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qspair_setq_d (qsmem_t * mem, qsptr_t p, qsptr_t val);
qsptr_t qspair_make (qsmem_t * mem, qsptr_t a, qsptr_t d);
int qspair_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen);


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

qsvector_t * qsvector (qsmem_t * mem, qsptr_t v, qsword * out_lim);
qsword qsvector_length (qsmem_t * mem, qsptr_t v);
qsptr_t qsvector_ref (qsmem_t * mem, qsptr_t v, qsword ofs);
qsptr_t qsvector_setq (qsmem_t * mem, qsptr_t v, qsword ofs, qsptr_t val);
qsptr_t qsvector_make (qsmem_t * mem, qsword k, qsptr_t fill);
qserror_t qsvector_mark (qsmem_t * mem, qsword k);
qserror_t qsvector_kmark (qsmem_t * mem, qsptr_t p, qsptr_t backptr, qsptr_t * next);
int qsvector_crepr (qsmem_t * mem, qsptr_t v, char * buf, int buflen);
qsword qsvector_inject (qsmem_t * mem, qsword nelts, qsptr_t * carray);
qsptr_t * qsvector_cptr (qsmem_t * mem, qsptr_t v, qsword * out_len);


/* Immutable List; Read-Only List; Array.  Borrows from CDR-coded lists. */
typedef struct qsimmlist_s {
    qsptr_t mgmt;
    qsptr_t reqnull;
    qsptr_t gc_backtrack;
    qsptr_t gc_iter;
    qsptr_t _d[];
} qsimmlist_t;

qsimmlist_t * qsimmlist (qsmem_t * mem, qsptr_t a);
qsword qsimmlist_length (qsmem_t * mem, qsptr_t a);
qsptr_t qsimmlist_ref (qsmem_t * mem, qsptr_t a, qsword ofs);
qsptr_t qsimmlist_setq (qsmem_t * mem, qsptr_t a, qsword ofs, qsptr_t val);
/* iter() generates an iterator, subject to car/cdr for traversal. */
qsptr_t qsimmlist_iter (qsmem_t * mem, qsptr_t a, qsword ofs);
qsptr_t qsimmlist_make (qsmem_t * mem, qsword k, qsptr_t fill);
int qsimmlist_crepr (qsmem_t * mem, qsptr_t v, char * buf, int buflen);




/* byte vector, C uint8_t[] embedded in Scheme store */
typedef struct qsbytevec_s {
    qsptr_t mgmt;
    qsptr_t len;
    uint32_t refcount;	  // references from outside Scheme memory.
    uint32_t mutex;	  // mutex on refcount.
    uint8_t _d[];
} qsbytevec_t;

qsbytevec_t * qsbytevec (qsmem_t * mem, qsptr_t bv, qsword * out_lim);
qsword qsbytevec_length (qsmem_t * mem, qsptr_t bv);
qsptr_t qsbytevec_ref (qsmem_t * mem, qsptr_t bv, qsword ofs);
qsptr_t qsbytevec_setq (qsmem_t * mem, qsptr_t bv, qsword ofs, qsword val);
qsptr_t qsbytevec_make (qsmem_t * mem, qsword k, qsptr_t fill);
int qsbytevec_crepr (qsmem_t * mem, qsptr_t bv, char * buf, int buflen);
uint8_t * qsbytevec_cptr (qsmem_t * mem, qsptr_t bv, qsword * out_len);
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

qswidenum_t * qswidenum (qsmem_t * mem, qsptr_t n, qsnumtype_t * out_variant);
qsnumtype_t qswidenum_variant (qsmem_t * mem, qsptr_t n);
qswidenum_t * qswidenum_premake (qsmem_t * mem, qsnumtype_t variant, qsptr_t * out_ptr);

qswidenum_t * qslong (qsmem_t * mem, qsptr_t l);
qserror_t qslong_fetch (qsmem_t * mem, qsptr_t l, long * out_long);
long qslong_get (qsmem_t * mem, qsptr_t l);
qsptr_t qslong_make (qsmem_t * mem, long val);
/* construct from high,low pair of 32b integers. */
qsptr_t qslong_make2 (qsmem_t * mem, int32_t high, uint32_t low);
int qslong_crepr (qsmem_t * mem, qsptr_t l, char * buf, int buflen);

qswidenum_t * qsdouble (qsmem_t * mem, qsptr_t d);
qserror_t qsdouble_fetch (qsmem_t * mem, qsptr_t d, double * out_double);
double qsdouble_get (qsmem_t * mem, qsptr_t d);
qsptr_t qsdouble_make (qsmem_t * mem, double val);
int qsdouble_crepr (qsmem_t * mem, qsptr_t l, char * buf, int buflen);

// qsipair (rational)
qswidenum_t * qsipair (qsmem_t * mem, qsptr_t q);
qserror_t qsipair_fetch (qsmem_t * mem, qsptr_t q, int * out_p, int * out_q);
qsptr_t qsipair_make (qsmem_t * mem, int32_t p, int32_t q);
int qsipair_crepr (qsmem_t * mem, qsptr_t q, char * buf, int buflen);

// qsfpair (complex)
qswidenum_t * qsfpair (qsmem_t * mem, qsptr_t z);
qserror_t qsfpair_fetch (qsmem_t * mem, qsptr_t q, float * out_a, float * out_b);
qsptr_t qsfpair_make (qsmem_t * mem, float a, float b);
int qsfpair_crepr (qsmem_t * mem, qsptr_t z, char * buf, int buflen);




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
int qsiter_on_pair (qsmem_t * mem, qsptr_t it, qsptr_t * out_pairptr);
qsword qsiter_get (qsmem_t * mem, qsptr_t it);
qsptr_t qsiter_item (qsmem_t * mem, qsptr_t it);  // also car
qsptr_t qsiter_next (qsmem_t * mem, qsptr_t it);  // also cdr
qsptr_t qsiter_make (qsmem_t * meme, qsmemaddr_t addr);


qsptr_t qsint (qsmem_t * mem, qsptr_t i);
int32_t qsint_get (qsmem_t * mem, qsptr_t i);
qsptr_t qsint_make (qsmem_t * mem, int32_t val);
int qsint_crepr (qsmem_t * mem, qsptr_t i, char * buf, int buflen);


qsptr_t qsfloat (qsmem_t * mem, qsptr_t f);
float qsfloat_get (qsmem_t * mem, qsptr_t f);
qsptr_t qsfloat_make (qsmem_t * mem, float val);
int qsfloat_crepr (qsmem_t * mem, qsptr_t f, char * buf, int buflen);


qsptr_t qschar (qsmem_t * mem, qsptr_t c);
int qschar_get (qsmem_t * mem, qsptr_t c);
qsptr_t qschar_make (qsmem_t * mem, int val);
int qschar_crepr (qsmem_t * mem, qsptr_t c, char * buf, int buflen);


qsptr_t qserr (qsmem_t * mem, qsptr_t e);
int qserr_get (qsmem_t * mem, qsptr_t e);
qsptr_t qserr_make (qsmem_t * mem, int errcode);
int qserr_crepr (qsmem_t * mem, qsptr_t c, char * buf, int buflen);


qsptr_t qsconst (qsmem_t * mem, qsptr_t n);
int qsconst_get (qsmem_t * mem, qsptr_t n);
qsptr_t qsconst_make (qsmem_t * mem, int constcode);
int qsconst_crepr (qsmem_t * mem, qsptr_t c, char * buf, int buflen);




typedef struct qsutf8_s {
    qsptr_t mgmt;
    qsptr_t variant;
    qsptr_t refcount;
    qsptr_t mutex;
    uint8_t _d[];
} qsutf8_t;

qsutf8_t * qsutf8 (qsmem_t * mem, qsptr_t s);
qsword qsutf8_length (qsmem_t * mem, qsptr_t s);
int qsutf8_ref (qsmem_t * mem, qsptr_t s, qsword k);
qsptr_t qsutf8_setq (qsmem_t * mem, qsptr_t s, qsword k, qsword u8);
int qsutf8_crepr (qsmem_t * mem, qsptr_t s, char * buf, int buflen);




qsptr_t qsstr (qsmem_t * mem, qsptr_t s);
qsword qsstr_length (qsmem_t * mem, qsptr_t s);
qsword qsstr_ref (qsmem_t * mem, qsptr_t s, qsword nth);
qsword qsstr_setq (qsmem_t * mem, qsptr_t s, qsword nth, qsword codepoint);
qsptr_t qsstr_make (qsmem_t * mem, qsword k, qsword codepoint_fill);
qsptr_t qsstr_inject (qsmem_t * mem, const char * cstr, qsword slen);
qsptr_t qsstr_inject_wchar (qsmem_t * mem, const wchar_t * ws, qsword wslen);
qsword qsstr_extract (qsmem_t * mem, qsptr_t s, char * cstr, qsword slen);
qsword qsstr_extract_wchar (qsmem_t * mem, qsptr_t s, wchar_t * ws, qsword wslen);
int qsstr_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b);




/* Symbols */
/* qssym = wraps index into interned symbol table, for (eq? ...) use. */
/* qssymbol = object in heap tying together name and id, for name->id lookup. */ 
qsptr_t qssym (qsmem_t * mem, qsptr_t y);
qsword qssym_get (qsmem_t * mem, qsptr_t y);
qsptr_t qssym_make (qsmem_t * mem, qsptr_t symbol_id);
int qssym_crepr (qsmem_t * mem, qsptr_t y, char * buf, int buflen);

typedef struct qssymbol_s {
    qsptr_t mgmt;
    qsptr_t indicator;	  // point to self.
    qsptr_t name;
    qsptr_t id;
} qssymbol_t;

qssymbol_t * qssymbol (qsmem_t * mem, qsptr_t yy);
qsptr_t qssymbol_ref_name (qsmem_t * mem, qsptr_t yy);
qsptr_t qssymbol_ref_id (qsmem_t * mem, qsptr_t yy);
qsptr_t qssymbol_make (qsmem_t * mem, qsptr_t name, qsptr_t symbol_id);

typedef struct qssymstore_s {
    qsptr_t mgmt;
    qsptr_t tag;
    qsptr_t table;
    qsptr_t tree;
} qssymstore_t;

qssymstore_t * qssymstore (qsmem_t * mem, qsptr_t ystore);
qsptr_t qssymstore_make (qsmem_t * mem);




int qsptr_crepr (qsmem_t * mem, qsptr_t c, char * buf, int buflen);

#endif // _QSOBJ_H_
