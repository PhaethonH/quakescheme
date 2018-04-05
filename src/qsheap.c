#include "qsheap.h"



qsobj_t * qsobj_init (qsobj_t * obj, int is_octet)
{
  obj->_0 = QSNULL;
  obj->_1 = QSNULL;
  obj->_2 = QSNULL;
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
  qsptr_t retval = QSNULL;

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

