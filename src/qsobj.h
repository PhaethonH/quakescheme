#ifndef QSOBJ_H_
#define QSOB_H_

#include <stdbool.h>
#include <stdio.h>

#include "qsstore.h"


/* header (management) word manipulation. */
#define MGMT_SHIFT_USED	      (31)
#define MGMT_SHIFT_MARK	      (30)
#define MGMT_SHIFT_GREY	      (29)
#define MGMT_SHIFT_OCT        (26)
#define MGMT_SHIFT_REVERS     (24)
#define MGMT_SHIFT_SCORE      (16)
#define MGMT_SHIFT_ALLOC      (3)

#define MGMT_MASK_USED	      (1 << MGMT_SHIFT_USED)
#define MGMT_MASK_MARK	      (1 << MGMT_SHIFT_MARK)
#define MGMT_MASK_GREY	      (1 << MGMT_SHIFT_GREY)
#define MGMT_MASK_OCT         (1 << MGMT_SHIFT_OCT)
#define MGMT_MASK_REVERS      (3 << MGMT_SHIFT_REVERS)
#define MGMT_MASK_SCORE       ( ((1<<8)-1) << MGMT_SHIFT_SCORE )
#define MGMT_MASK_ALLOC       ( ((1<<5)-1) << MGMT_SHIFT_ALLOC )

#define MGMT_IS_USED(x)	      ((x & MGMT_MASK_USED) == MGMT_MASK_USED)
#define MGMT_IS_MARK(x)	      ((x & MGMT_MASK_MARK) == MGMT_MASK_MARK)
#define MGMT_IS_GREY(x)	      ((x & MGMT_MASK_GREY) == MGMT_MASK_GREY)
#define MGMT_IS_OCT(x)        ((x & MGMT_MASK_OCT) == MGMT_MASK_OCT)
#define MGMT_GET_REVERS(x)    ((x & MGMT_MASK_REVERS) >> MGMT_SHIFT_REVERS)
#define MGMT_GET_SCORE(x)     ((x & MGMT_MASK_SCORE) >> MGMT_SHIFT_SCORE)
#define MGMT_GET_ALLOC(x)     ((x & MGMT_MASK_ALLOC) >> MGMT_SHIFT_ALLOC)

#define MGMT_SET_USED(x)      (x |= MGMT_MASK_USED)
#define MGMT_CLR_USED(x)      (x &= ~MGMT_MASK_USED)
#define MGMT_SET_MARK(x)      (x |= MGMT_MASK_MARK)
#define MGMT_CLR_MARK(x)      (x &= ~MGMT_MASK_MARK)
#define MGMT_SET_GREY(x)      (x |= MGMT_MASK_GREY)
#define MGMT_CLR_GREY(x)      (x &= ~MGMT_MASK_GREY)
#define MGMT_SET_OCT(x)	      (x |= MGMT_MASK_OCT)
#define MGMT_CLR_OCT(x)	      (x &= ~MGMT_MASK_OCT)
#define MGMT_SET_REVERS(x,v)  (x = (x & ~MGMT_MASK_REVERS) | ((v << MGMT_SHIFT_REVERS) & MGMT_MASK_REVERS))
#define MGMT_SET_SCORE(x,v)   (x = (x & ~MGMT_MASK_SCORE) | ((v << MGMT_SHIFT_SCORE) & MGMT_MASK_SCORE))
#define MGMT_SET_ALLOC(x,v)   (x = (x & ~MGMT_MASK_ALLOC) | ((v << MGMT_SHIFT_ALLOC) & MGMT_MASK_ALLOC))


/* Generic object manipulation. */

typedef struct qsobj_s {
    qsword mgmt;
    qsword fields[3];
} qsobj_t;

qsobj_t * qsobj_init (qsobj_t * obj, int alloscale, bool octetate);
bool qsobj_is_used (const qsobj_t * obj);
bool qsobj_is_marked (const qsobj_t * obj);
bool qsobj_is_grey (const qsobj_t * obj);
bool qsobj_is_octetate (const qsobj_t * obj);
int qsobj_get_reversal (const qsobj_t * obj);
int qsobj_get_score (const qsobj_t * obj);
int qsobj_get_allocscale (const qsobj_t * obj);

void qsobj_set_used (qsobj_t * obj, bool val);
void qsobj_set_marked (qsobj_t * obj, bool val);
void qsobj_set_grey (qsobj_t * obj, bool val);
void qsobj_set_octetate (qsobj_t * obj, bool val);
void qsobj_set_reversal (qsobj_t * obj, int val);
void qsobj_set_score (qsobj_t * obj, int val);
void qsobj_set_allocscale (qsobj_t * obj, int val);



/* Prototype 0: Free list. */
#define QSFREE_SENTINEL (0x3fffffff)
typedef struct qsfreelist_s {
    qsword mgmt;
    qsword length;
    /* double-linked list. */
    qsaddr prev;
    qsaddr next;
} qsfreelist_t;

qsfreelist_t * qsfreelist_init (qsfreelist_t *, qsword length, qsaddr prev, qsaddr next);
qsfreelist_t * qsfreelist_destroy (qsfreelist_t *);
qsword qsfreelist_get_length (qsfreelist_t *);
qsaddr qsfreelist_get_prev (qsfreelist_t *);
qsaddr qsfreelist_get_next (qsfreelist_t *);


/* Prototype 1: Single-bounds pointer-content (Triplet). */
typedef struct qstriplet_s {
    qsword mgmt;
    qsptr first;
    qsptr second;
    qsptr third;
} qstriplet_t;

qstriplet_t * qstriplet_init (qstriplet_t * triplet, qsptr first, qsptr second, qsptr third);
qsptr qstriplet_ref_first (const qstriplet_t * triplet);
qsptr qstriplet_ref_second (const qstriplet_t * triplet);
qsptr qstriplet_ref_third (const qstriplet_t * triplet);
qserr qstriplet_setq_first (qstriplet_t * triplet, qsptr val);
qserr qstriplet_setq_second (qstriplet_t * triplet, qsptr val);
qserr qstriplet_setq_third (qstriplet_t * triplet, qsptr val);


/* Prototype 2: Cross-bounds pointer-content (PointerVector). */
typedef struct qspvec_s {
    qsword mgmt;
    qsptr length;
    qsptr gcback;
    qsptr gciter;
    qsptr elt[0]; /* variable length. */
} qspvec_t;

qspvec_t * qspvec_init (qspvec_t * pvec, int allocscale, qsptr len);
qsptr qspvec_ref_length (const qspvec_t * pvec);
qsptr qspvec_ref (const qspvec_t * pvec, qsword k);
qserr qspvec_setq_length (qspvec_t * pvec, qsptr len);
qserr qspvec_setq (qspvec_t * pvec, qsword k, qsptr val);


/* Prototype 3: Single-bounds octet content (WideWord). */
typedef struct qswideword_s {
    qsword mgmt;
    qsptr subtype;
    union qswidepayload_u {
	double d;
	int64_t l;
	void * ptr;
	struct qsrational_s {
	    int32_t numer;
	    int32_t denom;
	} q;
	struct qscomplex_s {
	    float a;
	    float b;
	} z;
	FILE * cfile;
    } payload;
} qswideword_t;

qswideword_t * qswideword_init (qswideword_t * wideword, qsptr subtype, union qswidepayload_u * initsrc);
qsptr qswideword_ref_subtype (const qswideword_t * wideword);
const union qswidepayload_u * qswideword_at_const (const qswideword_t * wideword);
int qswideword_fetch_payload (const qswideword_t * wideword, union qswidepayload_u * buf);
qserr qswideword_setq_subtype (qswideword_t * wideword, qsptr subtype);
union qswidepayload_u * qswideword_at (qswideword_t * wideword);
int qswideword_put_payload (qswideword_t * wideword, const union qswidepayload_u * buf);


/* Prototype 4: Cross-bounds octet content (OctetVector). */
typedef struct qsovec_s {
    qsword mgmt;
    qsptr length;
    qsword refcount;
    qsword reflock;
    qsbyte elt[0];  /* variable length. */
} qsovec_t;

qsovec_t * qsovec_init (qsovec_t * ovec, int allocscale, qsptr len);
qsptr qsovec_ref_length (const qsovec_t * ovec);
qsword qsovec_get_refcount (const qsovec_t * ovec);
qsbyte qsovec_ref (const qsovec_t * ovec, qsword k);
qserr qsovec_setq_length (qsovec_t * ovec, qsptr len);
qserr qsovec_setq (qsovec_t * ovec, qsword k, qsbyte val);
qserr qsovec_incr_refcount (qsovec_t * ovec);
qserr qsovec_decr_refcount (qsovec_t * ovec);


#endif /* QSOBJ_H_ */
