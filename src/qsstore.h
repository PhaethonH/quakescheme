#ifndef QSSTORE_H_
#define QSSTORE_H_

#include "qsptr.h"
#include "qsobj.h"

#define SMEM_SIZE   (1 << 16)

/* TODO: base address mapping should be invisible to the segment (i.e. move to composing object).  */

/* memory segment.*/
typedef struct qssegment_s {
    qsword baseaddr;   /* Base address for address mapping. */
    qsword cap;	  /* Total bytes available in space. */
    qsaddr_t freelist;   /* Start of freelist. */
    qsword reserved3;  /* reserved words to align 'space' on 128b boundary. */
    qsbyte space[SMEM_SIZE];  /* variable-length array. */
} __attribute__((aligned(16))) qssegment_t;

extern qssegment_t * qssegment_init (qssegment_t *, qsword baseaddr, qsword cap);
extern qssegment_t * qssegment_destroy (qssegment_t *);
extern qssegment_t * qssegment_clear (qssegment_t *);
#ifdef DEBUG_QSSEGMENT
/* expose for only debug and unit-testing. */
extern int _qssegment_unfree (qssegment_t *, qsaddr_t local_addr);
extern qsaddr_t _qssegment_split (qssegment_t *, qsaddr_t local_addr, qsword nth_boundary);
extern qsaddr_t _qssegment_fit (qssegment_t *, qsword spanbounds);
#endif /* DEBUG_QSSEGMENT */


/* Scheme Store. */
typedef struct qsstore_s {
    struct qssegment_s smem;  /* Start Memory, statically allocated. */
    struct qssegment_s * wmem;  /* Working Memory, dynamically allocated. */
    const struct qssegment_s * rmem;  /* Read (Only) Memory. */
} qsstore_t;

extern qsstore_t * qsstore_init (qsstore_t *);
extern qsstore_t * qsstore_destroy (qsstore_t *);
/* Accessor, get content byte as return value. */
extern qsbyte qsstore_get_byte (const qsstore_t *, qsaddr_t addr);
/* Accessor, get content word as return value. */
extern qsword qsstore_get_word (const qsstore_t *, qsaddr_t addr);
/* Accessor, copy contents to buffer, return successful copy count. */
extern qsword qsstore_fetch_words (const qsstore_t *, qsaddr_t addr, qsword * dest, qsword count);
/* Accessor, pointer into memory region. */
extern const qsword * qsstore_word_at_const (const qsstore_t *, qsaddr_t addr);

/* Mutator, attach working memory. */
extern qserr_t qsstore_attach_wmem (qsstore_t *, qssegment_t * wmem, qsaddr_t baseaddr);
/* Mutator, attach readonly memory. */
extern qserr_t qsstore_attach_rmem (qsstore_t *, const qssegment_t * rmem, qsaddr_t baseaddr);

/* Mutator, blindly set content byte. */
extern qserr_t qsstore_set_byte (qsstore_t *, qsaddr_t addr, qsbyte val);
/* Mutator, blindly set content word. */
extern qserr_t qsstore_set_word (qsstore_t *, qsaddr_t addr, qsword val);
/* Mutator, copy into memory, return successful copy count. */
extern qsword qsstore_put_words (qsstore_t *, qsaddr_t addr, qsword * src, qsword count);
/* Mutable, pointer into memory region. */
extern qsword * qsstore_word_at (qsstore_t *, qsaddr_t addr);


/* Memory allocation. */
extern qserr_t qsstore_alloc (qsstore_t *, qsword allocscale, qsaddr_t * out_addr);
extern qserr_t qsstore_alloc_nbounds (qsstore_t *, qsword nbounds, qsaddr_t * out_addr);
extern qserr_t qsstore_alloc_nwords (qsstore_t *, qsword nwords, qsaddr_t * out_addr);
extern qserr_t qsstore_alloc_nbytes (qsstore_t *, qsword nbytes, qsaddr_t * out_addr);


/* Memory allocation. */
extern qserr_t qsstore_trace (qsstore_t *, qsaddr_t root, int mark);
extern qserr_t qsstore_sweep (qsstore_t *);


#endif /* QSSTORE_H_ */
