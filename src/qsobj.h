#ifndef QSOBJ_H_
#define QSOBJ_H_

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

#define MGMT_MASK_USED	      (1U << MGMT_SHIFT_USED)
#define MGMT_MASK_MARK	      (1U << MGMT_SHIFT_MARK)
#define MGMT_MASK_GREY	      (1U << MGMT_SHIFT_GREY)
#define MGMT_MASK_OCT         (1U << MGMT_SHIFT_OCT)
#define MGMT_MASK_REVERS      (3U << MGMT_SHIFT_REVERS)
#define MGMT_MASK_SCORE       ( ((1U<<8)-1) << MGMT_SHIFT_SCORE )
#define MGMT_MASK_ALLOC       ( ((1U<<5)-1) << MGMT_SHIFT_ALLOC )

#define MGMT_IS_USED(x)	      (((x) & MGMT_MASK_USED) == MGMT_MASK_USED)
#define MGMT_IS_MARK(x)	      (((x) & MGMT_MASK_MARK) == MGMT_MASK_MARK)
#define MGMT_IS_GREY(x)	      (((x) & MGMT_MASK_GREY) == MGMT_MASK_GREY)
#define MGMT_IS_OCT(x)        (((x) & MGMT_MASK_OCT) == MGMT_MASK_OCT)
#define MGMT_GET_REVERS(x)    (((x) & MGMT_MASK_REVERS) >> MGMT_SHIFT_REVERS)
#define MGMT_GET_SCORE(x)     (((x) & MGMT_MASK_SCORE) >> MGMT_SHIFT_SCORE)
#define MGMT_GET_ALLOC(x)     (((x) & MGMT_MASK_ALLOC) >> MGMT_SHIFT_ALLOC)

#define MGMT_SET_USED(x)      ((x) |= MGMT_MASK_USED)
#define MGMT_CLR_USED(x)      ((x) &= ~MGMT_MASK_USED)
#define MGMT_SET_MARK(x)      ((x) |= MGMT_MASK_MARK)
#define MGMT_CLR_MARK(x)      ((x) &= ~MGMT_MASK_MARK)
#define MGMT_SET_GREY(x)      ((x) |= MGMT_MASK_GREY)
#define MGMT_CLR_GREY(x)      ((x) &= ~MGMT_MASK_GREY)
#define MGMT_SET_OCT(x)	      ((x) |= MGMT_MASK_OCT)
#define MGMT_CLR_OCT(x)	      ((x) &= ~MGMT_MASK_OCT)
#define MGMT_SET_REVERS(x,v)  ((x) = (x & ~MGMT_MASK_REVERS) | ((v << MGMT_SHIFT_REVERS) & MGMT_MASK_REVERS))
#define MGMT_SET_SCORE(x,v)   ((x) = (x & ~MGMT_MASK_SCORE) | ((v << MGMT_SHIFT_SCORE) & MGMT_MASK_SCORE))
#define MGMT_SET_ALLOC(x,v)   ((x) = (x & ~MGMT_MASK_ALLOC) | ((v << MGMT_SHIFT_ALLOC) & MGMT_MASK_ALLOC))


/* Generic object manipulation. */

typedef struct qsobj_s {
    qsword mgmt;
    qsword fields[3];
} qsobj_t;

extern qsobj_t * qsobj_init (qsobj_t * obj, int allocscale, bool octetate);
extern bool qsobj_is_used (const qsobj_t * obj);
extern bool qsobj_is_marked (const qsobj_t * obj);
extern bool qsobj_is_grey (const qsobj_t * obj);
extern bool qsobj_is_octetate (const qsobj_t * obj);
extern int qsobj_get_reversal (const qsobj_t * obj);
extern int qsobj_get_score (const qsobj_t * obj);
extern int qsobj_get_allocscale (const qsobj_t * obj);

extern void qsobj_set_used (qsobj_t * obj, bool val);
extern void qsobj_set_marked (qsobj_t * obj, bool val);
extern void qsobj_set_grey (qsobj_t * obj, bool val);
extern void qsobj_set_octetate (qsobj_t * obj, bool val);
extern void qsobj_set_reversal (qsobj_t * obj, int val);
extern void qsobj_set_score (qsobj_t * obj, int val);
extern void qsobj_set_allocscale (qsobj_t * obj, int val);



/* Prototype 0: Free list. */
#define QSFREE_SENTINEL (0x3fffffff)
typedef struct qsfreelist_s {
    qsword mgmt;
    qsword length;
    /* double-linked list. */
    qsaddr_t prev;
    qsaddr_t next;
} qsfreelist_t;

extern qsfreelist_t * qsfreelist_init (qsfreelist_t *, qsword length, qsaddr_t prev, qsaddr_t next);
extern qsfreelist_t * qsfreelist_destroy (qsfreelist_t *);
extern qsword qsfreelist_get_length (qsfreelist_t *);
extern qsaddr_t qsfreelist_get_prev (qsfreelist_t *);
extern qsaddr_t qsfreelist_get_next (qsfreelist_t *);


/* Prototype 1: Single-bounds pointer-content (Triplet). */
typedef struct qstriplet_s {
    qsword mgmt;
    qsptr_t first;
    qsptr_t second;
    qsptr_t third;
} qstriplet_t;

extern qstriplet_t * qstriplet_init (qstriplet_t * triplet, qsptr_t first, qsptr_t second, qsptr_t third);
extern qsptr_t qstriplet_ref_first (const qstriplet_t * triplet);
extern qsptr_t qstriplet_ref_second (const qstriplet_t * triplet);
extern qsptr_t qstriplet_ref_third (const qstriplet_t * triplet);
extern qserr_t qstriplet_setq_first (qstriplet_t * triplet, qsptr_t val);
extern qserr_t qstriplet_setq_second (qstriplet_t * triplet, qsptr_t val);
extern qserr_t qstriplet_setq_third (qstriplet_t * triplet, qsptr_t val);


/* Prototype 2: Cross-bounds pointer-content (PointerVector). */
typedef struct qspvec_s {
    qsword mgmt;
    qsptr_t length;
    qsptr_t gcback;
    qsptr_t gciter;
    qsptr_t elt[]; /* variable length. */
} qspvec_t;

extern qspvec_t * qspvec_init (qspvec_t * pvec, int allocscale, qsptr_t len);
extern qsptr_t qspvec_ref_length (const qspvec_t * pvec);
extern qsptr_t qspvec_ref (const qspvec_t * pvec, qsword k);
extern qserr_t qspvec_setq_length (qspvec_t * pvec, qsptr_t len);
extern qserr_t qspvec_setq (qspvec_t * pvec, qsword k, qsptr_t val);


/* Prototype 3: Single-bounds octet content (WideWord). */
typedef struct qswideword_s {
    qsword mgmt;
    qsptr_t subtype;
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

extern qswideword_t * qswideword_init (qswideword_t * wideword, qsptr_t subtype, union qswidepayload_u * initsrc);
extern qsptr_t qswideword_ref_subtype (const qswideword_t * wideword);
extern const union qswidepayload_u * qswideword_at_const (const qswideword_t * wideword);
extern int qswideword_fetch_payload (const qswideword_t * wideword, union qswidepayload_u * buf);
extern qserr_t qswideword_setq_subtype (qswideword_t * wideword, qsptr_t subtype);
extern union qswidepayload_u * qswideword_at (qswideword_t * wideword);
extern int qswideword_put_payload (qswideword_t * wideword, const union qswidepayload_u * buf);


/* Prototype 4: Cross-bounds octet content (OctetVector). */
typedef struct qsovec_s {
    qsword mgmt;
    qsptr_t length;
    qsword refcount;
    qsword reflock;
    qsbyte elt[];  /* variable length. */
} qsovec_t;

extern qsovec_t * qsovec_init (qsovec_t * ovec, int allocscale, qsptr_t len);
extern qsptr_t qsovec_ref_length (const qsovec_t * ovec);
extern qsword qsovec_get_refcount (const qsovec_t * ovec);
extern qsbyte qsovec_ref (const qsovec_t * ovec, qsword k);
extern qserr_t qsovec_setq_length (qsovec_t * ovec, qsptr_t len);
extern qserr_t qsovec_setq (qsovec_t * ovec, qsword k, qsbyte val);
extern qserr_t qsovec_incr_refcount (qsovec_t * ovec);
extern qserr_t qsovec_decr_refcount (qsovec_t * ovec);


#endif /* QSOBJ_H_ */
