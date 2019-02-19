#ifndef QSPTR_H_
#define QSPTR_H_

#include <stdint.h>
#include <string.h>

/* Declare word-pointer types. */

/* Adopt the primitive integer type. */
typedef uint32_t qsword;
typedef uint8_t qsbyte;
/* Alias to indicate word content is tagged pointer. */
typedef qsword qsptr;
/* Alias to indicate return value is one of Err20 (mainly object mutators). */
typedef qsword qserr;

/* C union to manipulate bit patterns across types (type-punning). */
union qsbits_u {
    qsword w;
    uint32_t u;
    int32_t i;
    float f;
    char c;
    char s[4];
};
/*
Word-Pointer expanding-tag encoding

3322222 222221111 111111          0
1098765 432109876 54321098 76543210 space
                                  0 float31
                                 01 int30
                                011 sync29
                               0111 iter28
                             ..1111 (expand26)
                             00     obj26, object-id
                             01     sym26, object blessed as symbol
                             10      (reserved)
                           ..111111 (expand28)
                           00       char24
                           01        (reserved)
                           10        (reserved)
                      0... 11111111 (expand20)
                       000          const20, compile-time symbols
                       001          err20, primal/no-heap Exceptions
                       010          prim20, primitive operation
                       011          fd20, POSIX file descriptor Port

*/


/* tag checks. */
#define SHIFT_TAG31 (1)
#define SHIFT_TAG30 (2)
#define SHIFT_TAG29 (3)
#define SHIFT_TAG28 (4)
#define SHIFT_TAG26 (6)
#define SHIFT_TAG24 (8)
#define SHIFT_TAG20 (12)

#define MASK_TAG31 ((1 << SHIFT_TAG31) - 1)
#define MASK_TAG30 ((1 << SHIFT_TAG30) - 1)
#define MASK_TAG29 ((1 << SHIFT_TAG29) - 1)
#define MASK_TAG28 ((1 << SHIFT_TAG28) - 1)
#define MASK_TAG26 ((1 << SHIFT_TAG26) - 1)
#define MASK_TAG24 ((1 << SHIFT_TAG24) - 1)
#define MASK_TAG20 ((1 << SHIFT_TAG20) - 1)

#define TAG_FLOAT31     (0x000)
#define TAG_INT30       (0x001)
#define TAG_SYNC29      (0x003)
#define TAG_ITER28      (0x007)
#define TAG_OBJ26       (0x00f)
#define TAG_SYM26       (0x01f)
#define TAG_CHAR24      (0x03f)
#define TAG_CONST20     (0x0ff)
#define TAG_ERR20       (0x1ff)
#define TAG_PRIM20      (0x2ff)
#define TAG_FD20        (0x3ff)


/* space/type predicates. */
#define ISFLOAT31(x)    ((x & MASK_TAG31) == TAG_FLOAT31)
#define ISINT30(x)      ((x & MASK_TAG30) == TAG_INT30)
#define ISSYNC29(x)     ((x & MASK_TAG29) == TAG_SYNC29)
#define ISITER28(x)     ((x & MASK_TAG28) == TAG_ITER28)
#define ISOBJ26(x)      ((x & MASK_TAG26) == TAG_OBJ26)
#define ISSYM26(x)      ((x & MASK_TAG26) == TAG_SYM26)
#define ISCHAR24(x)     ((x & MASK_TAG24) == TAG_CHAR24)
#define ISCONST20(x)    ((x & MASK_TAG20) == TAG_CONST20)
#define ISPRIM20(x)     ((x & MASK_TAG20) == TAG_PRIM20)
#define ISERR20(x)      ((x & MASK_TAG20) == TAG_ERR20)
#define ISFD20(x)       ((x & MASK_TAG20) == TAG_FD20)


/* Conversions from word-pointers. */
#define CFLOAT31(x)     ((union qsbits_u)(x)).f
#define CINT30(x)       (((union qsbits_u)(x & ~(MASK_TAG30))).i / (1 << SHIFT_TAG30))
#define CITER28(x)      (((union qsbits_u)(x)).w >> SHIFT_TAG28)
#define COBJ26(x)       (((union qsbits_u)(x)).w >> SHIFT_TAG26)
#define CSYM26(x)       (((union qsbits_u)(x)).w >> SHIFT_TAG26)
#define CCHAR24(x)      (((union qsbits_u)(x)).w >> SHIFT_TAG24)
#define CCONST20(x)     (((union qsbits_u)(x)).w >> SHIFT_TAG20)
#define CPRIM20(x)      (((union qsbits_u)(x)).w >> SHIFT_TAG20)
#define CERR20(x)       (((union qsbits_u)(x)).w >> SHIFT_TAG20)
#define CFD20(x)        (((union qsbits_u)(x)).w >> SHIFT_TAG20)


/* Conversions to word-pointers. */
#define QSFLOAT(f)      (((union qsbits_u)((float)(f))).w & ~MASK_TAG31)
#define QSINT(i)        (((union qsbits_u)(i * (1 << SHIFT_TAG30))).w | TAG_INT30)
#define QSITER(a)       (((union qsbits_u)(a << SHIFT_TAG28)).w | TAG_ITER28)
#define QSOBJ(a)        (((union qsbits_u)(a << SHIFT_TAG26)).w | TAG_OBJ26)
#define QSSYM(a)        (((union qsbits_u)(a << SHIFT_TAG26)).w | TAG_SYM26)
#define QSCHAR(c)       (((union qsbits_u)(c << SHIFT_TAG24)).w | TAG_CHAR24)
#define QSCONST(k)      (((union qsbits_u)(k << SHIFT_TAG20)).w | TAG_CONST20)
#define QSPRIM(p)       (((union qsbits_u)(p << SHIFT_TAG20)).w | TAG_PRIM20)
#define QSERR(e)        (((union qsbits_u)(e << SHIFT_TAG20)).w | TAG_ERR20)
#define QSFD(n)         (((union qsbits_u)(n << SHIFT_TAG20)).w | TAG_FD20)


/* Enumeration for space Const20 */
/*  Single-valued space Nil. */
#define QSNIL           QSCONST(0)
/*  Two-valued space Boolean. */
#define QSTRUE          QSCONST(1)
#define QSFALSE         QSCONST(2)
/*  other values in space Const. */
/*  Blackhole - value to indicate variable not yet bound. */
#define QSBLACKHOLE     QSCONST(3)

/*  Infinities, positive and negative. */
#define QSINF           QSCONST(10)
#define QSNINF          QSCONST(11)
/*  Not-A-Number.  Ignore difference between signaling and non-signaling. */
#define QSNAN           QSCONST(12)
#define QSNNAN          QSCONST(13)

/*  Numeric tower types, for type coercion. */
#define QSNUM_INT       QSCONST(1000)   /* word, int30 */
#define QSNUM_FLOAT     QSCONST(1001)   /* word, float31 */
#define QSNUM_LONG      QSCONST(1002)   /* Wideword, int64 */
#define QSNUM_DOUBLE    QSCONST(1003)   /* Wideword, double64 */
#define QSNUM_RATIONAL  QSCONST(1004)   /* Wideword, rational, pair of int32 */
#define QSNUM_COMPLEX   QSCONST(1005)   /* Wideword, complex, pair of float32 */
#define QSNUM_BIGNUM    QSCONST(1006)   /* Octetvector, bignum */
/*  other enums for wideword */
#define QSWIDE_CPTR     QSCONST(1010)   /* C pointer (void*) */
#define QSWIDE_FILE     QSCONST(1010)   /* C File Pointer (FILE). */


  
/* Enumeration for space Err20 */
#define QSERR_OK        QSERR(0)
#define QSERR_NOMEM     QSERR(1)	/* Memory exhausted. */
#define QSERR_UNBOUND   QSERR(2)	/* Unbound free variable. */
#define QSERR_FAULT     QSERR(3)	/* Memory access fault. */



#endif /* QSPTR_H_ */
