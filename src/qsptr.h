#ifndef QSPTR_H_
#define QSPTR_H_

#include <stdint.h>


/* Bit-tagging schema

 3          2          1
10987654 32109876 54321098 76543210
                                  0 float31
                                 01 int30
                                011 sync29
                               0111 iter28 - traverse vector as if list.
                             ..1111 (ptr26)
                             001111 sym26
                             011111 heap26
                             101111  -
                             111111 (extend26)
                           ..111111 (ptr24)
                           00111111 char24
                           01111111  -
                           10111111  -
                           11111111 (extend24)
                  01111110 11111111 error16
                  01111111 11111111 portFD16
                  11111111 11111111 const16
                            
*/


/* inline list:

{
  SYNC29(ptr,allocscale=31),
  SYM_INLINE_LIST,
  reserved,
  reserved,

  EL0,
  EL1,
  EL2,
  EL3,

  EL4,
  EL5,
  EL6,
  EL7,

  ...
  SYM_END_OF_LIST,
}

*/


typedef uint32_t qsword;
typedef qsword qsptr_t;

union qsbits_u {
  float f;
  int32_t i;
  uint32_t u;
  qsptr_t p;
};

typedef union qsbits_u qsbits_t;





#define SHIFT_PTR31 1
#define SHIFT_PTR30 2
#define SHIFT_PTR29 3
#define SHIFT_PTR28 4
#define SHIFT_PTR26 6
#define SHIFT_PTR24 8
#define SHIFT_PTR16 16

#define SHIFT_FLOAT31 SHIFT_PTR31
#define SHIFT_INT30 SHIFT_PTR30
#define SHIFT_SYNC29 SHIFT_PTR29
#define SHIFT_ITER28 SHIFT_PTR28
#define SHIFT_SYM26 SHIFT_PTR26
#define SHIFT_HEAP26 SHIFT_PTR26
#define SHIFT_CHAR24 SHIFT_PTR24
#define SHIFT_ERROR16 SHIFT_PTR16
#define SHIFT_CONST16 SHIFT_PTR16

#define TAGMASK_FLOAT31 ((1 << SHIFT_PTR31) - 1)
#define TAGMASK_INT30 ((1 << SHIFT_PTR30) - 1)
#define TAGMASK_ITER28 ((1 << SHIFT_PTR28) - 1)
#define TAGMASK_SYNC29 ((1 << SHIFT_PTR29) - 1)
#define TAGMASK_HEAP26 ((1 << SHIFT_PTR26) - 1)
#define TAGMASK_SYM26 ((1 << SHIFT_PTR26) - 1)
#define TAGMASK_CHAR24 ((1 << SHIFT_PTR24) - 1)
#define TAGMASK_ERROR16 ((1 << SHIFT_PTR16) - 1)
#define TAGMASK_CONST16 ((1 << SHIFT_PTR16) - 1)

#define TAG_FLOAT31 0x0000
#define TAG_INT30   0x0001
#define TAG_SYNC29  0x0003
#define TAG_ITER28  0x0007
#define TAG_HEAP26  0x001f
#define TAG_SYM26   0x000f
#define TAG_CHAR24  0x003f
#define TAG_ERROR16 0x7eff
#define TAG_CONST16 0xffff


#define QSFLOAT(x) ((((qsbits_t)((float)(x))).u & ~(TAGMASK_FLOAT31)) | TAG_FLOAT31)
#define QSINT(x) ((qsword)(((x) << SHIFT_INT30) | TAG_INT30))
#define QSITER(x) (((qsword)((x) << SHIFT_ITER28) | TAG_ITER28))
#define QSOBJ(x) ((qsword)(((x) << SHIFT_HEAP26) | TAG_HEAP26))
#define QSSYM(x) ((qsword)(((x) << SHIFT_SYM26) | TAG_SYM26))
#define QSCHAR(x) ((qsword)(((x) << SHIFT_CHAR24) | TAG_CHAR24))
#define QSERROR(x) ((qsword)(((x) << SHIFT_ERROR16) | TAG_ERROR16))
#define QSCONST(x) ((qsword)(((x) << SHIFT_CONST16) | TAG_CONST16))

#define MAX_INT30 ((int)(~((qsword)0) >> (SHIFT_PTR30 + 1)))
#define MIN_INT30 ((int)(-MAX_INT30 - 1))

#define ISNIL(x) (x == QSNIL)
#define ISFLOAT31(x) ((((qsbits_t)(x)).u & TAGMASK_FLOAT31) == TAG_FLOAT31)
#define ISINT30(x) ((((qsbits_t)(x)).u & TAGMASK_INT30) == TAG_INT30)
#define ISSYNC29(x) ((((qsbits_t)(x)).u & TAGMASK_SYNC29) == TAG_SYNC29)
#define ISITER28(x) ((((qsbits_t)(x)).u & TAGMASK_ITER28) == TAG_ITER28)
#define ISHEAP26(x) ((((qsbits_t)(x)).u & TAGMASK_HEAP26) == TAG_HEAP26)
#define ISOBJ26(x) ((((qsbits_t)(x)).u & TAGMASK_HEAP26) == TAG_HEAP26)
#define ISSYM26(x) ((((qsbits_t)(x)).u & TAGMASK_SYM26) == TAG_SYM26)
#define ISCHAR24(x) ((((qsbits_t)(x)).u & TAGMASK_CHAR24) == TAG_CHAR24)
#define ISERROR16(x) ((((qsbits_t)(x)).u & TAGMASK_ERROR16) == TAG_ERROR16)
#define ISCONST16(x) ((((qsbits_t)(x)).u & TAGMASK_CONST16) == TAG_CONST16)

#define CFLOAT31(p) (((qsbits_t)(p)).f)
#define CINT30(p) ((((qsbits_t)(p)).i & ~TAGMASK_INT30) / 4)  /* sign-extend */
#define CHEAP26(p) ((((qsbits_t)(p)).u & ~TAGMASK_HEAP26) >> SHIFT_HEAP26)
#define CITER28(p) ((((qsbits_t)(p)).u & ~TAGMASK_ITER28) >> SHIFT_ITER28)
#define COBJ26(p) ((((qsbits_t)(p)).u & ~TAGMASK_HEAP26) >> SHIFT_HEAP26)
#define CSYM26(p) ((((qsbits_t)(p)).u & ~TAGMASK_SYM26) >> SHIFT_SYM26)
#define CCHAR24(p) ((((qsbits_t)(p)).u & ~TAGMASK_CHAR24) >> SHIFT_CHAR24)
#define CERROR16(p) ((((qsbits_t)(p)).u & ~TAGMASK_ERROR16) >> SHIFT_ERROR16)
#define CCONST16(p) ((((qsbits_t)(p)).u & ~TAGMASK_CONST16) >> SHIFT_CONST16)


/* Constants. These values can be considered system-level symbols. */
#define QSNIL	QSCONST(0)
#define QSTRUE	QSCONST(1)
#define QSFALSE	QSCONST(2)
#define QSBOL	QSCONST(3)  // beginning-of-list, nested immlist.
#define QSEOL	QSCONST(4)  // end-of-list, for immlist.
#define QSBLACKHOLE QSCONST(8)  // 'unassigned' value.
/* cons-tagging constants. */
#define QST_RBTREE    QSCONST(0x60)
#define QST_SYMSTORE  QSCONST(0x61)
#define QST_SYMBOL    QSCONST(0x62)
#define QST_ENV	      QSCONST(0x63)
#define QST_LAMBDA    QSCONST(0x64)
#define QST_CLOSURE   QSCONST(0x65)
#define QST_KONT      QSCONST(0x70)
#define QSKONT_HALT    QSCONST(0x71)
#define QSKONT_LETK    QSCONST(0x72)
#define QSKONT_RETURN  QSCONST(0x73)
#define QSKONT_BRANCH  QSCONST(0x74)
// 0x40..0x4f reserved for numeric types.
/* Numeric tower type enumeration. */
#define QSNUMTYPE_NAN		QSCONST(0x40)
#define QSNUMTYPE_INT		QSCONST(0x41)
#define QSNUMTYPE_FLOAT		QSCONST(0x42)
#define QSNUMTYPE_LONG		QSCONST(0x43)
#define QSNUMTYPE_DOUBLE	QSCONST(0x44)
#define QSNUMTYPE_INT2		QSCONST(0x45)  /* integer pair, rational. */
#define QSNUMTYPE_FLOAT2	QSCONST(0x46)  /* float pair, complex. */
#define QSNUMTYPE_FLOAT4	QSCONST(0x48)  /* math vec4_t */
#define QSNUMTYPE_FLOAT16CM	QSCONST(0x4a)  /* math vec16_t (matrix_4x4_column_major) */
#define QSNUMTYPE_INF		QSCONST(0x4f)
typedef qsptr_t qsnumtype_t;

#define QSBOOL(x)	(x ? QSTRUE : QSFALSE)
#define CBOOL(x)	(x != QSFALSE)

/* Standard (built-in) I/O ports. */
#define QSIO_IN		QSCONST(0x80)
#define QSIO_OUT	QSCONST(0x81)
#define QSIO_ERR	QSCONST(0x82)
#define QSEOF		QSCONST(0x8f)


/* low-level error codes. */
typedef qsptr_t qserror_t;
#define QSERROR_NONE	QSERROR(0) // no error.
#define QSERROR_OK	QSERROR_NONE
#define QSERROR_NOIMPL	QSERROR(1) // Not Implemented.
#define QSERROR_NOMEM	QSERROR(2) // Out Of Memory.
#define QSERROR_INVALID	QSERROR(3) // invalid|inaccessible memory address.
#define QSERROR_RANGE	QSERROR(4) // parameter out of range.
#define QSERROR_TYPE	QSERROR(5) // invalid type provided.


#endif  // QSPTR_H_
