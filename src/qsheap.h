#ifndef _QSHEAP_H_
#define _QSHEAP_H_

#include <stdint.h>

#include "qsptr.h"

/* Heap memory. */

typedef uint32_t qsheapaddr_t;


typedef struct qsheapcell_s {
    qsptr_t mgmt;
    qsptr_t fields[3];
    qsptr_t _d[];
} qsheapcell_t;


typedef struct qsobj_s {
    qsptr_t mgmt;
    qsptr_t _0;
    qsptr_t _1;
    qsptr_t _2;
} qsobj_t;


/* mgmt word

 32         21        1
10987654 32109876 54321098 76543210
        |        |        |     011  (sync29)
u       |        |        |          used
 m      |        |        |          marked
  g     |        |        |          grey-marked
   .    |        |        |          ?
    T   |        |        |          tree-balance mark (red-black tree)
     o  |        |        |          content is octet (else ptr)
      Rr|        |        |          pointer reversals
        |        |        |abcde     alloc-scale
*/

#define MGMT_IS_USED(w)		(w & (1 << 31))
#define MGMT_IS_MARKED(w)	(w & (1 << 30))
#define MGMT_IS_GREY(w)		(w & (1 << 29))

#define MGMT_IS_RED(w)		(w & (1 << 27))
#define MGMT_IS_BLACK(w)	(!MGMT_ISRED(w))
#define MGMT_IS_OCTET(w)	(w & (1 << 26))
#define MGMT_GET_PARENT(w)	((w >> 24) & 0x3)

#define MGMT_SET_USED(w)	(w |= (1 << 31))
#define MGMT_CLR_USED(w)	(w &= ~(1 << 31))
#define MGMT_SET_MARKED(w)	(w |= (1 << 30))
#define MGMT_CLR_MARKED(w)	(w &= ~(1 << 30))
#define MGMT_SET_GREY(w)	(w |= (1 << 29))
#define MGMT_CLR_GREY(w)	(w &= ~(1 << 29))
#define MGMT_SET_RED(w)		(w |= (1 << 27))
#define MGMT_CLR_RED(w)		(w &= ~(1 << 27))
#define MGMT_SET_OCTET(w)	(w |= (1 << 26))
#define MGMT_CLR_OCTET(w)	(w &= ~(1 << 26))
#define MGMT_SET_PARENT(w,v)	(w = (w & ~(0x03000000)) | ((v & 0x3) << 24))
#define MGMT_CLR_PARENT(w)	(w &= ~(3 << 24))

#define MGMT_GET_ALLOCSCALE(w)	((w & 0xf8) >> 3)
#define MGMT_SET_ALLOCSCALE(w,v) (w = (w & ~(0x000000f8) | ((v & 0x1f) << 3)))

// the "not a valid address" value for qsfreelist locations.
#define QSFREE_SENTINEL		((qsheapaddr_t)(0x7fffffff))

qsobj_t * qsobj_init ();
qsobj_t * qsobj_destroy ();
qsptr_t qsobj_get_mgmt (qsobj_t *);
qsobj_t * qsobj_set_marked (qsobj_t *);
int qsobj_is_used (qsobj_t *);
int qsobj_is_marked (qsobj_t *);
int qsobj_is_octet (qsobj_t *);
int qsobj_get_allocscale (qsobj_t *);
qsptr_t qsobj_get (qsobj_t *, int /* 0, 1, 2 */);
void qsobj_set (qsobj_t *, int /* [012] */, qsptr_t val);
// increment reference count, for byte-objects.
void qsobj_up ();
// decrement reference count, for byte-objects.
void qsobj_down ();



/* initialize cell as object or freelist. */
qsheapcell_t * qsheapcell_init (qsheapcell_t *, int used, int red, int allocscale);
int qsheapcell_is_used (qsheapcell_t *);
qsheapcell_t * qsheapcell_set_used (qsheapcell_t *, int);
int qsheapcell_is_marked (qsheapcell_t *);
qsheapcell_t * qsheapcell_set_marked (qsheapcell_t *, int);
//int qsheapcell_is_grey (qsheapcell_t *);
//qsheapcell_t qsheapcell_set_grey (qsheapcell_t *, int);
int qsheapcell_is_red (qsheapcell_t *);
qsheapcell_t * qsheapcell_set_red (qsheapcell_t *, int);
int qsheapcell_is_octet (qsheapcell_t *);
qsheapcell_t * qsheapcell_set_octet (qsheapcell_t *, int);
int qsheapcell_get_parent (qsheapcell_t *);
qsheapcell_t * qsheapcell_set_parent (qsheapcell_t *, int);
int qsheapcell_get_allocscale (qsheapcell_t *);
qsheapcell_t * qsheapcell_set_allocscale (qsheapcell_t *, int);
qsptr_t qsheapcell_get_field (qsheapcell_t *, int);




/*
typedef union qsheapref_u {
    qsfreelist_t freelist;
    qsheapcell_t cell;
    qsobj_t obj;
} qsheapref_t;
*/

typedef struct qsheap_s {
    int wlock;			/* write-lock into storage. */
    uint32_t cap;		/* maximum number of words. */
    //qsheapaddr_t freelist;	/* start of free list. */
    qsheapaddr_t end_freelist;	/* end of free list. */
    //qsobj_t space[];
    qsheapcell_t space[];
} qsheap_t;

qsheap_t * qsheap_init (qsheap_t *, uint32_t ncells);
qsheap_t * qsheap_destroy (qsheap_t *);
uint32_t qsheap_length (qsheap_t *);
qserror_t qsheap_allocscale (qsheap_t *, qsword allocscale, qsheapaddr_t * out_addr);
qserror_t qsheap_alloc_ncells (qsheap_t *, qsword ncells, qsheapaddr_t * out_addr);
qsheapaddr_t qsheap_free (qsheap_t *, qsheapaddr_t addr);
qserror_t qsheap_set_marked (qsheap_t *, qsheapaddr_t addr, int val);
qserror_t qsheap_set_used (qsheap_t *, qsheapaddr_t addr, int val);
int qsheap_is_marked (qsheap_t *, qsheapaddr_t addr);
int qsheap_is_used (qsheap_t *, qsheapaddr_t addr);
qserror_t qsheap_sweep (qsheap_t *);
//qsobj_t * qsheap_ref (qsheap_t *, qsheapaddr_t addr);
qsheapcell_t * qsheap_ref (qsheap_t *, qsheapaddr_t addr);
qserror_t qsheap_word (qsheap_t *, qsheapaddr_t word_addr, qsword * out_word);




typedef struct qsfreelist_s {
    qsptr_t mgmt;
    qsptr_t span; // :int30, number of cells available in this segment.
    qsptr_t prev; // :int30, lower address.
    qsptr_t next; // :int30, higher address.
} qsfreelist_t;

qsfreelist_t * qsfreelist (qsheap_t *, qsptr_t p);
qsfreelist_t * qsfreelist_ref (qsheap_t *, qsheapaddr_t);
qserror_t qsfreelist_reap (qsheap_t * heap, qsheapaddr_t addr, qsfreelist_t ** out_freelist);
qserror_t qsfreelist_split (qsheap_t *, qsheapaddr_t, qsword ncells, qsheapaddr_t * out_first, qsheapaddr_t * out_second);
qserror_t qsfreelist_fit_end (qsheap_t *, qsheapaddr_t, qsword ncells, qsheapaddr_t * out_addr);
qsword qsfreelist_get_span (qsheap_t *, qsheapaddr_t cell_addr);
qsword qsfreelist_get_prev (qsheap_t *, qsheapaddr_t cell_addr);
qsword qsfreelist_get_next (qsheap_t *, qsheapaddr_t cell_addr);
qserror_t qsfreelist_set_span (qsheap_t *, qsheapaddr_t cell_addr, qsword val);
qserror_t qsfreelist_set_prev (qsheap_t *, qsheapaddr_t cell_addr, qsword val);
qserror_t qsfreelist_set_next (qsheap_t *, qsheapaddr_t cell_addr, qsword val);
int qsfreelist_crepr (qsheap_t *, qsheapaddr_t cell_addr, char * buf, int buflen);




#endif // _QSHEAP_H_
