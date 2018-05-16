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

typedef uint32_t qsstoreaddr_t;

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
#define QSFREE_SENTINEL		((qsstoreaddr_t)(0x7fffffff))




/*
   Base unit of store is 4-word "bay".

   Prototypes based on pointer/octet content and allocated size.
*/
//struct qsheap_s;
//typedef struct qsheap_s qsheap_t;
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


typedef struct qsstore_s {
    int wlock;			/* write-lock into storage. */
    uint32_t cap;		/* maximum number of words. */
    qsstoreaddr_t end_freelist;	/* end of free list. */
    qsptr_t symstore;		/* symbol table. */
    qsbay0_t space[];		/* space is a sequence of bays. */
} qsstore_t;
typedef qsstore_t qsheap_t;

/* Methods on entirety of memory store. */

qsstore_t * qsstore_init (qsstore_t *, uint32_t ncells);
qsstore_t * qsstore_destroy (qsstore_t *);
/* address is in range. */
bool qsstore_is_valid (qsstore_t *, qsstoreaddr_t addr);
/* length, in number of bays */
uint32_t qsstore_length (qsstore_t *);
qserror_t qsstore_allocscale (qsstore_t *, qsword allocscale, qsstoreaddr_t * out_addr);
/* Allocate total number of (contiguous) bays. */
qserror_t qsstore_alloc_nbays (qsstore_t *, qsword nbays, qsstoreaddr_t * out_addr);
/* Allocate enough to hold 'nptrs' additional pointers (e.g. vectors). */
qserror_t qsstore_alloc_with_nptrs (qsstore_t *, qsword nptrs, qsstoreaddr_t * out_addr);
/* Allocate enough to hold 'nbytes' additional bytes (0 for unibay octetate). */
qserror_t qsstore_alloc_with_nbytes (qsstore_t *, qsword nbytes, qsstoreaddr_t * out_addr);
qsbay0_t * qsstore_get (qsstore_t *, qsstoreaddr_t addr);
qserror_t qsstore_fetch_word (qsstore_t *, qsstoreaddr_t word_addr, qsword * out_word);
qserror_t qsstore_sweep (qsstore_t *);

/* Methods on single store bay as part of store. */

int qsstorebay_init (qsstore_t *, qsstoreaddr_t addr);
int qsstorebay_destroy (qsstore_t *, qsstoreaddr_t addr);
int qsstorebay_clear (qsstore_t *, qsstoreaddr_t addr);
qsword qsstorebay_get_ptr (qsstore_t *, qsstoreaddr_t addr, qsword ofs);
qsword qsstorebay_set_ptr (qsstore_t *, qsstoreaddr_t addr, qsword ofs, qsword ptrval);
int qsstorebay_get_oct (qsstore_t *, qsstoreaddr_t addr, qsword ofs);
qsword qsstorebay_set_oct (qsstore_t *, qsstoreaddr_t addr, qsword ofs, qsword byteval);
void * qsbay_ref_data (qsheap_t * mem, qsstoreaddr_t addr, qsword * len);
qsword qsstorebay_get_allocsize (qsstore_t *, qsword addr);
int qsstorebay_get_allocscale (qsstore_t *, qsstoreaddr_t addr);
int qsstorebay_set_allocscale (qsstore_t *, qsstoreaddr_t addr, qsword val);
bool qsstorebay_is_synced (qsstore_t *, qsstoreaddr_t addr);
int qsstorebay_set_synced (qsstore_t *, qsstoreaddr_t addr, bool val);
bool qsstorebay_is_used (qsstore_t *, qsstoreaddr_t addr);
int qsstorebay_set_used (qsstore_t *, qsstoreaddr_t addr, bool val);
bool qsstorebay_is_marked (qsstore_t *, qsstoreaddr_t addr);
int qsstorebay_set_marked (qsstore_t *, qsstoreaddr_t addr, bool val);
bool qsstorebay_is_octetate (qsstore_t *, qsstoreaddr_t addr);
int qsstorebay_set_octetate (qsstore_t *, qsstoreaddr_t addr, bool val);
int qsstorebay_get_parent (qsstore_t *, qsstoreaddr_t addr);
int qsstorebay_set_parent (qsstore_t *, qsstoreaddr_t addr, qsword val);
int qsstorebay_get_score (qsstore_t *, qsstoreaddr_t addr);
int qsstorebay_set_score (qsstore_t *, qsstoreaddr_t addr, qsword val);

bool qstorebay_is_uniptr (qsstore_t *, qsstoreaddr_t addr);
bool qstorebay_is_multiptr (qsstore_t *, qsstoreaddr_t addr);
bool qstorebay_is_unioct (qsstore_t *, qsstoreaddr_t addr);
bool qstorebay_is_multioct (qsstore_t *, qsstoreaddr_t addr);


/*
Freelist is a linked list of segments,
each segment consists of multiple bays.
Freelist starts at high memory, pointing towards low memory.
Allocation preferentially starts consuming from high memory.
*/
typedef struct qsfreelist_s {
    qsptr_t mgmt;
    qsptr_t span; // :int30, number of cells available in this segment.
    qsptr_t prev; // :int30, lower address.
    qsptr_t next; // :int30, higher address.
} qsfreelist_t;

qsfreelist_t * qsfreelist (qsstore_t *, qsptr_t p);
qsfreelist_t * qsfreelist_ref (qsstore_t *, qsstoreaddr_t);
qserror_t qsfreelist_reap (qsstore_t * heap, qsstoreaddr_t addr, qsfreelist_t ** out_freelist);
qserror_t qsfreelist_split (qsstore_t *, qsstoreaddr_t, qsword ncells, qsstoreaddr_t * out_first, qsstoreaddr_t * out_second);
qserror_t qsfreelist_fit_end (qsstore_t *, qsstoreaddr_t, qsword ncells, qsstoreaddr_t * out_addr);
qsword qsfreelist_get_span (qsstore_t *, qsstoreaddr_t cell_addr);
qsword qsfreelist_get_prev (qsstore_t *, qsstoreaddr_t cell_addr);
qsword qsfreelist_get_next (qsstore_t *, qsstoreaddr_t cell_addr);
qserror_t qsfreelist_set_span (qsstore_t *, qsstoreaddr_t cell_addr, qsword val);
qserror_t qsfreelist_set_prev (qsstore_t *, qsstoreaddr_t cell_addr, qsword val);
qserror_t qsfreelist_set_next (qsstore_t *, qsstoreaddr_t cell_addr, qsword val);
int qsfreelist_crepr (qsstore_t *, qsstoreaddr_t cell_addr, char * buf, int buflen);


struct qsstorebay_report_s;
struct qsstorebay_report_s qsobj_report (qsstore_t *, qsstoreaddr_t obj_addr);

#endif // _QSHEAP_H_
