#include <stdlib.h>

#include "qsheap.h"


qsheap_t * qsheap_init (qsheap_t * heap, uint32_t len)
{
  heap->wlock = 0;
  heap->cap = len;
  return heap;
}

qsheap_t * qsheap_destroy (qsheap_t * heap)
{
  return heap;
}

qsheapaddr_t qsheap_alloc (qsheap_t * heap, int allocscale)
{
  return 0;
}

qsheapaddr_t qsheap_alloc_ncells (qsheap_t * heap, qsword ncells)
{
/* Take log2 of number of cells (2*n to accomodate cells)
 0 => 0
 1 => 0
 2 => 1
 3 => 2
 4 => 2
 5 => 3
..8 => 3
 9 => 4
..16 => 4
*/
  int nbits = 0;
  if (ncells > 1) ncells--;
  while (ncells > 0)
    {
      nbits++;
      ncells >>= 1;
    }
  return qsheap_alloc(heap, nbits);
}

int qsheap_word (qsheap_t * heap, qsheapaddr_t word_addr, qsword * out_word)
{
  if ((word_addr < 0) || (word_addr >= heap->max))
    return 0;
  if (out_word)
    *out_word = heap->space[word_addr];
  return 1;
}

qsobj_t * qsheap_ref (qsheap_t * heap, qsheapaddr_t cell_addr)
{
  qsheapaddr_t word_addr = 4 * cell_addr;
  if (0 == qsheap_get(heap, word_addr, NULL))
    {
      return (qsobj_t*)(heap->space + word_addr);
    }
  return NULL;
}




qsobj_t * qsobj_init (qsobj_t * obj, int is_octet)
{
  obj->_0 = QSNIL;
  obj->_1 = QSNIL;
  obj->_2 = QSNIL;
  return obj;
}

qsobj_t * qsobj_destroy (qsobj_t * obj)
{
  return obj;
}

qsptr_t qsobj_get_mgmt (qsobj_t * obj)
{
  return obj->mgmt;
}

int qsobj_is_used (qsobj_t * obj)
{
  return !!(MGMT_IS_USED(obj->mgmt));
}

int qsobj_is_marked (qsobj_t * obj)
{
  return !!(MGMT_IS_MARKED(obj->mgmt));
}

int qsobj_is_octet (qsobj_t * obj)
{
  return !!(MGMT_IS_OCTET(obj->mgmt));
}

qsptr_t qsobj_get (qsobj_t * obj, int fld)
{
  qsptr_t retval = QSNIL;

  switch (fld)
    {
    case 0:
      retval = obj->_0;
      break;
    case 1:
      retval = obj->_1;
      break;
    case 2:
      retval = obj->_2;
      break;
    default:
      /* TODO: exception */
      break;
    }
  return retval;
}

void qsobj_set (qsobj_t * obj, int fld, qsptr_t val)
{
  switch (fld)
    {
    case 0:
      obj->_0 = val;
      break;
    case 1:
      obj->_1 = val;
      break;
    case 2:
      obj->_2 = val;
      break;
    default:
      /* TODO: exception */
      break;
    }
}

// increment reference count, for byte-objects.
void qsobj_up (qsobj_t * obj)
{
}

// decrement reference count, for byte-objects.
void qsobj_down (qsobj_t * obj)
{
}

