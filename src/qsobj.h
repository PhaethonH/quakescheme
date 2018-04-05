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


/* template:
int cOBJ_p (union qsobjform_u * obju)
int cOBJ_get (union qsobjform_u * obju, )
int cOBJ_set (union qsobjform_u * obju, )

qsptr_t QSOBJ_p (qsheap_t * heap, qsptr_t q)
qsptr_t QSOBJ_alloc (qsheap_t * heap, qsptr_t q)
qsptr_t QSOBJ_make (qsheap_t * heap, qsptr_t q)
qsptr_t QSOBJ_get (qsheap_t * heap, qsptr_t q, )
qsptr_t QSOBJ_set (qsheap_t * heap, qsptr_t q, )
int QSOBJ_crepr (qsheap_t * heap, qsptr_t q, const char * buf, int buflen)
*/


int ccons_p (union qsobjform_u * obju);
int ccons_get_a (union qsobjform_u * obju);
int ccons_get_d (union qsobjform_u * obju);
void ccons_set_a (union qsobjform_u * obju, qsptr_t val);
void ccons_set_d (union qsobjform_u * obju, qsptr_t val);

qsptr_t qscons_p (qsheap_t * heap, qsptr_t q);
qsptr_t qscons_alloc (qsheap_t * heap, qsptr_t a, qsptr_t d);
qsptr_t qscons_make (qsheap_t * heap, qsptr_t a, qsptr_t d);
qsptr_t qscons_get_a (qsheap_t * heap, qsptr_t q);
qsptr_t qscons_get_d (qsheap_t * heap, qsptr_t q);
int qscons_crepr (qsheap_t * heap, qsptr_t q, const char *buf, int buflen);



int cvector_p (union qsobjform_u * obju);
int cvector_lim (union qsobjform_u * obju); // get maximum size.
int cvector_get (union qsobjform_u * obju, int ofs);
int cvector_set (union qsobjform_u * obju, int ofs);

qsptr_t qsvector_p (qsheap_t * heap, qsptr_t q);
qsptr_t qsvector_alloc (qsheap_t * heap, qsptr_t q);
qsptr_t qsvector_make (qsheap_t * heap, qsptr_t q);
qsptr_t qsvector_get (qsheap_t * heap, qsptr_t q, int ofs);
qsptr_t qsvector_set (qsheap_t * heap, qsptr_t q, int ofs, qsptr_t val);
int qsvector_crepr (qsheap_t * heap, qsptr_t q, const char * buf, int buflen);

#endif // _QSOBJ_H_
