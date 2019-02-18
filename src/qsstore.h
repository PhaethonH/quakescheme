#ifndef QSSTORE_H_
#define QSSTORE_H_

#include "qsptr.h"

#define SMEM_SIZE   (1 << 16)

/* memory segment. */
typedef struct qsmem_s {
    qsword baseaddr;   /* Base address for address mapping. */
    qsword cap;	  /* Total bytes available in space. */
    qsword reserved2;  /* reserved words to align 'space' on 128b boundary. */
    qsword reserved3;
    qsbyte space[SMEM_SIZE];  /* variable-length array. */
} qsmem_t;

qsmem_t * qsmem_init (qsmem_t *);
qsmem_t * qsmem_destroy (qsmem_t *);


/* Scheme Store. */
typedef struct qsstore_s {
    struct qsmem_s smem;  /* Start Memory, statically allocated. */
    struct qsmem_s * wmem;  /* Working Memory, dynamically allocated. */
    const struct qsmem_s * rmem;  /* Read (Only) Memory. */
} qsstore_t;

qsstore_t * qsstore_init (qsstore_t *);
qsstore_t * qsstore_destroy (qsstore_t *);
/* Accessor, get content byte as return value. */
qsbyte qsstore_get_byte (const qsstore_t *, qsword addr);
/* Accessor, get content word as return value. */
qsword qsstore_get_word (const qsstore_t *, qsword addr);
/* Accessor, copy contents to buffer, return successful copy count. */
qsword qsstore_fetch_words (const qsstore_t *, qsword addr, qsword * dest, qsword count);

/* Mutator, blindly set content byte. */
void qsstore_set_byte (qsstore_t *, qsword addr, qsbyte val);
/* Mutator, blindly set content word. */
void qsstore_set_word (qsstore_t *, qsword addr, qsword val);
/* Mutator, copy into memory, return successful copy count. */
qsword qsstore_put_words (qsstore_t *, qsword addr, qsword * src, qsword count);


#endif /* QSSTORE_H_ */
