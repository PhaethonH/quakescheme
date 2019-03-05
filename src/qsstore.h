#ifndef QSSTORE_H_
#define QSSTORE_H_

#include "qsptr.h"
#include "qsobj.h"

#define SMEM_SIZE   (1 << 16)

/* memory segment. */
typedef struct qssegment_s {
    qsword baseaddr;   /* Base address for address mapping. */
    qsword cap;	  /* Total bytes available in space. */
    qsaddr freelist;   /* Start of freelist. */
    qsword reserved3;  /* reserved words to align 'space' on 128b boundary. */
    qsbyte space[SMEM_SIZE];  /* variable-length array. */
} qssegment_t;

qssegment_t * qssegment_init (qssegment_t *, qsword baseaddr, qsword cap);
qssegment_t * qssegment_destroy (qssegment_t *);
qssegment_t * qssegment_clear (qssegment_t *);
#ifdef DEBUG_QSSEGMENT
/* only expose for debug and unit-testing. */
int _qssegment_unfree (qssegment_t *, qsaddr local_addr);
qsaddr _qssegment_split (qssegment_t *, qsaddr local_addr, qsword nth_boundary);
qsaddr _qssegment_fit (qssegment_t *, qsword spanbounds);
#endif /* DEBUG_QSSEGMENT */


/* Scheme Store. */
typedef struct qsstore_s {
    struct qssegment_s smem;  /* Start Memory, statically allocated. */
    struct qssegment_s * wmem;  /* Working Memory, dynamically allocated. */
    const struct qssegment_s * rmem;  /* Read (Only) Memory. */
} qsstore_t;

qsstore_t * qsstore_init (qsstore_t *);
qsstore_t * qsstore_destroy (qsstore_t *);
/* Accessor, get content byte as return value. */
qsbyte qsstore_get_byte (const qsstore_t *, qsaddr addr);
/* Accessor, get content word as return value. */
qsword qsstore_get_word (const qsstore_t *, qsaddr addr);
/* Accessor, copy contents to buffer, return successful copy count. */
qsword qsstore_fetch_words (const qsstore_t *, qsaddr addr, qsword * dest, qsword count);
/* Accessor, pointer into memory region. */
const qsword * qsstore_word_at_const (const qsstore_t *, qsaddr addr);

/* Mutator, attach working memory. */
qserr qsstore_attach_wmem (qsstore_t *, qssegment_t * wmem, qsaddr baseaddr);
/* Mutator, attach readonly memory. */
qserr qsstore_attach_rmem (qsstore_t *, const qssegment_t * rmem, qsaddr baseaddr);

/* Mutator, blindly set content byte. */
qserr qsstore_set_byte (qsstore_t *, qsaddr addr, qsbyte val);
/* Mutator, blindly set content word. */
qserr qsstore_set_word (qsstore_t *, qsaddr addr, qsword val);
/* Mutator, copy into memory, return successful copy count. */
qsword qsstore_put_words (qsstore_t *, qsaddr addr, qsword * src, qsword count);
/* Mutable, pointer into memory region. */
qsword * qsstore_word_at (qsstore_t *, qsaddr addr);


/* Memory allocation. */
qserr qsstore_alloc (qsstore_t *, qsword allocscale, qsaddr * out_addr);
qserr qsstore_alloc_nbounds (qsstore_t *, qsword nbounds, qsaddr * out_addr);
qserr qsstore_alloc_nwords (qsstore_t *, qsword nwords, qsaddr * out_addr);
qserr qsstore_alloc_nbytes (qsstore_t *, qsword nbytes, qsaddr * out_addr);


#endif /* QSSTORE_H_ */
