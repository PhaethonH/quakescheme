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
                             001111  -
                             011111 heap26
                             101111  -
                             111111 (extend26)
                           ..111111 (ptr24)
                           00111111 char24
                           01111111  -
                           10111111  -
                           11111111 (extend24)
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
};

typedef union qsbits_u qsbits_t;





#define SHIFT_PTR31 1
#define SHIFT_PTR30 2
#define SHIFT_PTR29 3
#define SHIFT_PTR26 6
#define SHIFT_PTR24 8
#define SHIFT_PTR16 16

#define SHIFT_FLOAT31 SHIFT_PTR31
#define SHIFT_INT30 SHIFT_PTR30
#define SHIFT_SYNC29 SHIFT_PTR29
#define SHIFT_ITER28 SHIFT_PTR28
#define SHIFT_HEAP26 SHIFT_PTR26
#define SHIFT_CHAR24 SHIFT_PTR24
#define SHIFT_CONST16 SHIFT_PTR16

#define TAGMASK_FLOAT31 ((1 << SHIFT_PTR31) - 1)
#define TAGMASK_INT30 ((1 << SHIFT_PTR30) - 1)
#define TAGMASK_SYNC29 ((1 << SHIFT_PTR29) - 1)
#define TAGMASK_HEAP26 ((1 << SHIFT_PTR26) - 1)
#define TAGMASK_CHAR24 ((1 << SHIFT_PTR24) - 1)
#define TAGMASK_CONST16 ((1 << SHIFT_PTR16) - 1)

#define TAG_FLOAT31 0x0001
#define TAG_INT30   0x0003
#define TAG_SYNC29  0x0007
#define TAG_HEAP26  0x001f
#define TAG_CHAR24  0x003f
#define TAG_CONST16 0xffff


#define QSFLOAT(x) (((qsbits_t)(x)).u & ~TAGMASK_FLOAT31)
#define QSINT(x) (((x) << SHIFT_INT30) | TAGMASK_INT30)
#define QSITER(x) (((x) << SHIFT_ITER28) | TAGMASK_ITER28)
#define QSOBJ(x) (((x) << SHIFT_HEAP26) | TAGMASK_HEAP26)
#define QSCHAR(x) (((x) << SHIFT_CHAR24) | TAGMASK_CHAR24)
#define QSCONST(x) (((x) << SHIFT_CONST16) | TAGMASK_CONST16)

#define ISFLOAT31(x) (((x) & TAGMASK_FLOAT30) == TAG_FLOAT31)
#define ISINT30(x) (((x) & TAGMASK_INT30) == TAG_INT30)
#define ISSYNC29(x) (((x) & TAGMASK_SYNC29) == TAG_SYNC29)
#define ISHEAP26(x) (((x) & TAGMASK_HEAP26) == TAG_HEAP26)
#define ISOBJ26(x) (((x) & TAGMASK_HEAP26) == TAG_HEAP26)
#define ISCHAR24(x) (((x) & TAGMASK_CHAR24) == TAG_CHAR24)
#define ISCONST16(x) (((x) & TAGMASK_CONST16) == TAG_CONST16)

#define CFLOAT31(p) (((qsbits_t)(p)).f)
#define CINT30(p) (((p) & ~TAGMASK_INT30) >> SHIFT_PTR30)
#define CHEAP26(p) (((p) & ~TAGMASK_HEAP26) >> SHIFT_HEAP26)
#define COBJ26(p) (((p) & ~TAGMASK_HEAP26) >> SHIFT_HEAP26)
#define CCHAR24(p) (((p) & ~TAGMASK_CHAR24) >> SHIFT_CHAR24)


#define QSNULL QSCONST(0)
#define QSTRUE QSCONST(1)
#define QSBOL QSCONST(3)  // beginning-of-list, nested immlist.
#define QSEOL QSCONST(4)  // end-of-list, for immlist.


#endif  // QSPTR_H_
