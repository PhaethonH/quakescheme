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
cons	| 0		| =QSNULL	| car		| cdr
tree	| 0		| :ptr   	| :ptr   	| :ptr
immlist	| >0		| =QSNULL	| gc_backtrac	| gc_iter
vector	| >0		| :int30	| gc_backtrack	| gc_iter

Octet types (mgmt.o=1):
type	| allocscale	| _0 (discrim)	| _1		| _2
widefix	| 0		| 'QSFIXNUM	| int64_t
wideflo | 0		| 'QSFLONUM	| double
portFILE| 1		| 'QSFILE	| FILE*
	    _3=name:ptr     _4=attrs
portFD	| 0		| 'QSFD		| fd:int32_t	| mode:int
portMEM	| 0		| str:ptr26	| ofs:int30	| max:int30
bytevec	| >0		| :int30	| refcount	| mutex
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
int cOBJ_p (union qsobjform_u * obju)
int cOBJ_get (union qsobjform_u * obju, )
int cOBJ_set (union qsobjform_u * obju, )

qsptr_t QSOBJ_p (qsheap_t * heap, qsptr_t q)
qsptr_t QSOBJ_alloc (qsheap_t * heap, qsptr_t q)
qsptr_t QSOBJ_make (qsheap_t * heap, qsptr_t q)
qsptr_t QSOBJ_get (qsheap_t * heap, qsptr_t q, )
qsptr_t QSOBJ_set (qsheap_t * heap, qsptr_t q, )
int QSOBJ_crepr (qsheap_t * heap, qsptr_t q, char * buf, int buflen)
*/


typedef qsheap_t qsmem_t;
typedef qsheapaddr_t qsmemaddr_t;


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
int qspair_alloc (qsmem_t * mem, qsptr_t * out_obj, qsmemaddr_t * out_addr);
int qspair_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen);


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
int qstree_alloc (qsmem_t * mem, qsptr_t * out_ptr, qsmemaddr_t * out_addr);
int qstree_crepr (qsmem_t * mem, qsptr_t t, char * buf, int buflen);


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
int qsvector_alloc (qsmem_t * mem, qsptr_t * out_ptr, qsmemaddr_t * out_addr, qsword cap);
int qsvector_crepr (qsmem_t * mem, qsptr_t v, char * buf, int buflen);


#endif // _QSOBJ_H_
