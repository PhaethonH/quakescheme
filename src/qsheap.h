#ifndef _QSHEAP_H_
#define _QSHEAP_H_

#include <stdint.h>

#include "qsptr.h"

/* Heap memory. */

typedef uint32_t qsheapaddr_t;


typedef struct qsobj_s {
    qsptr_t mgmt;
    qsptr_t _0;
    qsptr_t _1;
    qsptr_t _2;
} qsobj_t;

typedef struct qsfreelist_s {
    qsptr_t mgmt;
    qsptr_t span;
    qsptr_t reserved;
    qsptr_t next;
} qsfreelist_t;


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

qsobj_t * qsobj_init ();
qsobj_t * qsobj_destroy ();
qsptr_t qsobj_get_mgmt (qsobj_t *);
int qsobj_is_used (qsobj_t *);
int qsobj_is_marked (qsobj_t *);
int qsobj_is_octet (qsobj_t *);
qsptr_t qsobj_get (qsobj_t *, int /* 0, 1, 2 */);
void qsobj_set (qsobj_t *, int /* [012] */, qsptr_t val);
// increment reference count, for byte-objects.
void qsobj_up ();
// decrement reference count, for byte-objects.
void qsobj_down ();



typedef struct qsheap_s {
    int wlock;			/* write-lock into storage. */
    uint32_t cap;		/* maximum number of words. */
    qsheapaddr_t freelist;	/* start of free list. */
    qsobj_t space[];
} qsheap_t;

typedef qsptr_t qsheapcell_t[4];


qsheap_t * qsheap_init (qsheap_t *, uint32_t ncells);
qsheap_t * qsheap_destroy (qsheap_t *);
uint32_t qsheap_length (qsheap_t *);
qsheapaddr_t qsheap_alloc (qsheap_t *, int allocscale);
qsheapaddr_t qsheap_alloc_ncells (qsheap_t *, qsword ncells);
qsheapaddr_t qsheap_free (qsheap_t *, qsheapaddr_t addr);
qsobj_t * qsheap_ref (qsheap_t *, qsheapaddr_t addr);
qserror_t qsheap_word (qsheap_t *, qsheapaddr_t word_addr, qsword * out_word);


#endif // _QSHEAP_H_
