#ifndef _QSHEAP_H_
#define _QSHEAP_H_

#include <stdint.h>
#include <stdbool.h>

#include <ctype.h>
#include <wctype.h>
#include <wchar.h>
#include <locale.h>

#include "qsptr.h"

/* Heap memory. */

typedef uint32_t qsheapaddr_t;


typedef struct qsheapcell_s {
    qsptr_t mgmt;
    qsptr_t fields[3];
    qsptr_t _d[];
} qsheapcell_t;


/* mgmt word

 32         21        1
10987654 32109876 54321098 76543210
        |        |        |     011  (sync29)
u       |        |        |          used
 m      |        |        |          marked
  g     |        |        |          grey-marked
   ..   |        |        |          ?
        |TTTTTTTT|        |          tree-balance score
     o  |        |        |          content is octet (else ptr)
      Rr|        |        |          pointer reversals
        |        |        |abcde     alloc-scale
*/
/* Tree-Balance Score
Provided for self-balancing trees.
Most implementations of interest use height or depth of (sub)tree, which is
limited by the log of the tree size.
In a 32b space, the maximum such value would be 31, so 5 bits would suffice.
6 bits for a 64b space.
8 bits allow for 256b space.
8 bits also allow overloading the Score field for the prefix of a utf-8 trie.

Red-Black: 0=black, 1=red
AVL: height (<= 27, as tree nodes occupy a 26b space)
Random Access List: lg(n+1), where n<=2**26, i.e. Score <= 26
*/

#define MGMT_IS_USED(w)		((w) & (1 << 31))
#define MGMT_IS_MARKED(w)	((w) & (1 << 30))
#define MGMT_IS_GREY(w)		((w) & (1 << 29))

#define MGMT_IS_BLACK(w)	(!MGMT_ISRED(w))
#define MGMT_IS_OCTET(w)	((w) & (1 << 26))
#define MGMT_GET_PARENT(w)	(((w) >> 24) & 0x3)
#define MGMT_GET_SCORE(w)	(((w) >> 16) & 0xff)

#define MGMT_SET_USED(w)	((w) |= (1 << 31))
#define MGMT_CLR_USED(w)	((w) &= ~(1 << 31))
#define MGMT_SET_MARKED(w)	((w) |= (1 << 30))
#define MGMT_CLR_MARKED(w)	((w) &= ~(1 << 30))
#define MGMT_SET_GREY(w)	((w) |= (1 << 29))
#define MGMT_CLR_GREY(w)	((w) &= ~(1 << 29))
#define MGMT_SET_OCTET(w)	((w) |= (1 << 26))
#define MGMT_CLR_OCTET(w)	((w) &= ~(1 << 26))
#define MGMT_SET_PARENT(w,v)	((w) = ((w) & ~(0x03000000)) | (((v) & 0x3) << 24))
#define MGMT_CLR_PARENT(w)	((w) &= ~(3 << 24))
#define MGMT_SET_SCORE(w,v)	((w) = ((w) & ~(0x00ff0000)) | (((v) & 0xff) << 16))
#define MGMT_CLR_SCORE(w,v)	((w) &= ~(0xff << 16))

#define MGMT_GET_ALLOCSCALE(w)	((w & 0xf8) >> 3)
#define MGMT_SET_ALLOCSCALE(w,v) (w = (w & ~(0x000000f8) | ((v & 0x1f) << 3)))

// the "not a valid address" value for qsfreelist locations.
#define QSFREE_SENTINEL		((qsheapaddr_t)(0x7fffffff))




/* initialize cell as object or freelist. */
qsheapcell_t * qsheapcell_init (qsheapcell_t *, int used, int red, int allocscale);
int qsheapcell_is_synced (qsheapcell_t *);
int qsheapcell_is_used (qsheapcell_t *);
qsheapcell_t * qsheapcell_set_used (qsheapcell_t *, int);
int qsheapcell_is_marked (qsheapcell_t *);
qsheapcell_t * qsheapcell_set_marked (qsheapcell_t *, int);
//int qsheapcell_is_grey (qsheapcell_t *);
//qsheapcell_t qsheapcell_set_grey (qsheapcell_t *, int);
int qsheapcell_is_octet (qsheapcell_t *);
qsheapcell_t * qsheapcell_set_octet (qsheapcell_t *, int);
int qsheapcell_get_parent (qsheapcell_t *);
qsheapcell_t * qsheapcell_set_parent (qsheapcell_t *, int);
int qsheapcell_get_score (qsheapcell_t *);
qsheapcell_t * qsheapcell_set_score (qsheapcell_t *, int);
int qsheapcell_get_allocscale (qsheapcell_t *);
qsheapcell_t * qsheapcell_set_allocscale (qsheapcell_t *, int);
qsptr_t qsheapcell_get_field (qsheapcell_t *, int);
qsheapcell_t * qsheapcell_set_field (qsheapcell_t *, int, qsptr_t);




/*
   Base unit of store is 4-word "bay".

   Prototypes based on pointer/octet content and allocated size.
*/
struct qsheap_s;
typedef struct qsheap_s qsheap_t;
typedef union qsbay_s {
    struct qsbay_generic_s {
	qsptr_t mgmt;
	qsptr_t fields[3];
    } generic;
    union qsbay_ptr_u {
	/* Single bay, pointer content (e.g. pair). */
	struct qsunibay_ptr_s {
	    qsptr_t mgmt;
	    qsptr_t e;
	    qsptr_t a;
	    qsptr_t d;
	} uni;
	/* Multiple bays, pointer content (e.g. vector). */
	struct qsmultibay_ptr_s {
	    qsptr_t mgmt;
	    qsptr_t len;
	    qsptr_t gc_backtrack;
	    qsptr_t gc_iter;

	    qsptr_t _d[];
	} multi;
    } ptr;
    union qsbay_oct_u {
	/* Single bay, octet content (e.g. widenum, C pointer) */
	struct qsunibay_oct_s {
	    qsptr_t mgmt;
	    qsptr_t variant;
	    uint8_t _d[8];
	} uni;
	/* Multiple bays, octet content (e.g. bytevector, utf-8 string) */
	struct qsmultibay_oct_s {
	    qsptr_t mgmt;
	    qsptr_t len;
	    qsptr_t refcount;
	    qsptr_t lock;

	    uint8_t _d[];
	} multi;
    } oct;
} qsbay_t;
typedef struct qsbay_generic_s qsbay0_t;
typedef struct qsunibay_ptr_s qsunibay_ptr_t;
typedef struct qsmultibay_ptr_s qsmultibay_ptr_t;
typedef struct qsunibay_oct_s qsunibay_oct_t;
typedef struct qsmultibay_oct_s qsmultibay_oct_t;

qsbay_t * qsbay (qsheap_t * mem, qsptr_t p, qsheapaddr_t * out_addr);
bool qsbay_used_p (qsheap_t * mem, qsptr_t p);
bool qsbay_marked_p (qsheap_t * mem, qsptr_t p);
bool qsbay_octetate_p (qsheap_t * mem, qsptr_t p);
int qsbay_ref_score (qsheap_t * mem, qsptr_t p);
int qsbay_ref_parent (qsheap_t * mem, qsptr_t p);
qsword qsbay_ref_allocsize (qsheap_t * mem, qsptr_t p);
int qsbay_ref_allocscale (qsheap_t * mem, qsptr_t p);

/* Arbitrary field access. */
/* Returns pointer field at offset 'ofs' in object 'p'.
     [0] => mgmt word
   Returns QSERROR_* if access denied.

   For unibay_ptr, 0=mgmt, 1=e, 2=a, 3=d
   For multibay_ptr, 0=mgmt, 1=len, 2=gc_backgrack, 3=gc_iter, 4=data[0], ...
   For unibay_oct, only 0 (mgmt) and 1 (variant) are valid.
   For multibay_oct, 0=mgmt, 1=track, 2=refcount, 3=mutex.
*/
qsword qsbay_ref_ptr (qsheap_t * mem, qsptr_t p, qsword ofs);
/* Returns -1 if octet access denied (error). */
int qsbay_ref_oct (qsheap_t * mem, qsptr_t p, qsword ofs);
/* Returns pointer to start of arbitrary data space, the address of ->d[0]
   Primarily intended for writing to/from widenum payloads.
 */
void * qsbay_ref_data (qsheap_t * mem, qsptr_t p, qsword * len);
/* Returns object, or QSERROR_* if mutating failed. */
qsptr_t qsbay_setq_ptr (qsheap_t * mem, qsptr_t p, qsword ofs, qsptr_t val);
/* Returns object, or QSERROR_* if mutating failed. */
qsptr_t qsbay_setq_oct (qsheap_t * mem, qsptr_t p, qsword ofs, int val);

qsptr_t qsbay_setq_score (qsheap_t * mem, qsptr_t p, int val);
qsptr_t qsbay_setq_parent (qsheap_t * mem, qsptr_t p, int val);

/* Mainly for is-a checks. */
qsunibay_ptr_t * qsunibay_ptr (qsheap_t * mem, qsptr_t p);
qsmultibay_ptr_t * qsmultibay_ptr (qsheap_t * mem, qsptr_t p);
qsunibay_oct_t * qsunibay_oct (qsheap_t * mem, qsptr_t p);
qsmultibay_oct_t * qsmultibay_oct (qsheap_t * mem, qsptr_t p);




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
    qsptr_t symstore;
    //qsobj_t space[];
    qsheapcell_t space[];
} qsheap_t;

qsheap_t * qsheap_init (qsheap_t *, uint32_t ncells);
qsheap_t * qsheap_destroy (qsheap_t *);
/* length in number of bays. */
uint32_t qsheap_length (qsheap_t *);
qserror_t qsheap_allocscale (qsheap_t *, qsword allocscale, qsheapaddr_t * out_addr);
qserror_t qsheap_alloc_ncells (qsheap_t *, qsword ncells, qsheapaddr_t * out_addr);
/* Allocate object to occupy 'nbays' total bays. */
qserror_t qsheap_alloc_nbays (qsheap_t *, qsword nbays, qsheapaddr_t * out_addr);
/* Allocate object to hold 'nptrs' additional pointers (e.g. vectors). */
qserror_t qsheap_alloc_with_nptrs (qsheap_t *, qsword nptrs, qsheapaddr_t * out_addr);
/* Allocate object to hold 'nbytes' additional bytes (0 for unibay octetate). */
qserror_t qsheap_alloc_with_nbytes (qsheap_t *, qsword nbytes, qsheapaddr_t * out_addr);
qsheapaddr_t qsheap_free (qsheap_t *, qsheapaddr_t addr);

int qsheap_is_valid (qsheap_t *, qsheapaddr_t addr);
int qsheap_is_synced (qsheap_t *, qsheapaddr_t addr);
int qsheap_get_allocscale (qsheap_t *, qsheapaddr_t addr);
int qsheap_is_octetate (qsheap_t *, qsheapaddr_t addr);
qserror_t qsheap_set_octetate (qsheap_t *, qsheapaddr_t addr, int val);
int qsheap_is_used (qsheap_t *, qsheapaddr_t addr);
qserror_t qsheap_set_used (qsheap_t *, qsheapaddr_t addr, int val);
int qsheap_is_marked (qsheap_t *, qsheapaddr_t addr);
qserror_t qsheap_set_marked (qsheap_t *, qsheapaddr_t addr, int val);
int qsheap_get_score (qsheap_t *, qsheapaddr_t addr);
qserror_t qsheap_set_score (qsheap_t *, qsheapaddr_t addr, int val);
int qsheap_get_parent (qsheap_t *, qsheapaddr_t addr);
qserror_t qsheap_set_parent (qsheap_t *, qsheapaddr_t addr, int val);

qserror_t qsheap_sweep (qsheap_t *);
//qsobj_t * qsheap_ref (qsheap_t *, qsheapaddr_t addr);
qsheapcell_t * qsheap_ref (qsheap_t *, qsheapaddr_t addr);
qserror_t qsheap_word (qsheap_t *, qsheapaddr_t word_addr, qsword * out_word);




/*
Freelist is a linked list of segments,
each segment consists of multiple bays.
Freelist starts at low memory, ends at high memory.
Allocation preferentially starts consuming from high memory.
*/
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




struct qsheapcell_report_s;
struct qsheapcell_report_s qsobj_report (qsheap_t *, qsheapaddr_t obj_addr);

#endif // _QSHEAP_H_
