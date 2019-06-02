#include "qsobj.h"

/* Generic object manipulation. */

qsobj_t * qsobj_init (qsobj_t * obj, int allocscale, bool octetate)
{
  qsword mgmt = TAG_SYNC29;
  mgmt |= (1 << MGMT_SHIFT_USED);
  mgmt |= (allocscale << MGMT_SHIFT_ALLOC) & MGMT_MASK_ALLOC;
  mgmt |= (octetate ? (1 << MGMT_SHIFT_OCT) : 0);
  obj->mgmt = mgmt;
  obj->fields[0] = QSNIL;
  obj->fields[1] = QSNIL;
  obj->fields[2] = QSNIL;
  return NULL;
}

bool qsobj_is_used (const qsobj_t * obj)
{
  if (obj->mgmt & MGMT_MASK_USED)
    return true;
  return false;
}

bool qsobj_is_marked (const qsobj_t * obj)
{
  if (obj->mgmt & MGMT_MASK_MARK)
    return true;
  return false;
}

bool qsobj_is_grey (const qsobj_t * obj)
{
  if (obj->mgmt & MGMT_MASK_GREY)
    return true;
  return false;
}

bool qsobj_is_octetate (const qsobj_t * obj)
{
  if (obj->mgmt & MGMT_MASK_OCT)
    return true;
  return false;
}

int qsobj_get_reversal (const qsobj_t * obj)
{
  return (obj->mgmt & MGMT_MASK_REVERS) >> MGMT_SHIFT_REVERS;
}

int qsobj_get_score (const qsobj_t * obj)
{
  return (obj->mgmt & MGMT_MASK_SCORE) >> MGMT_SHIFT_SCORE;
}

int qsobj_get_allocscale (const qsobj_t * obj)
{
  return (obj->mgmt & MGMT_MASK_ALLOC) >> MGMT_SHIFT_ALLOC;
}


void qsobj_set_used (qsobj_t * obj, bool val)
{
  if (val)
    obj->mgmt |= (1 << MGMT_SHIFT_USED);
  else
    obj->mgmt &= ~(1 << MGMT_SHIFT_USED);
}

void qsobj_set_marked (qsobj_t * obj, bool val)
{
  if (val)
    obj->mgmt |= (1 << MGMT_SHIFT_MARK);
  else
    obj->mgmt &= ~(1 << MGMT_SHIFT_MARK);
}

void qsobj_set_grey (qsobj_t * obj, bool val)
{
  if (val)
    obj->mgmt |= (1 << MGMT_SHIFT_GREY);
  else
    obj->mgmt &= ~(1 << MGMT_SHIFT_GREY);
}

void qsobj_set_octetate (qsobj_t * obj, bool val)
{
  if (val)
    obj->mgmt |= (1 << MGMT_SHIFT_OCT);
  else
    obj->mgmt &= ~(1 << MGMT_SHIFT_OCT);
}

void qsobj_set_reversal (qsobj_t * obj, int val)
{
  obj->mgmt &= ~MGMT_MASK_REVERS;
  obj->mgmt |= (val << MGMT_SHIFT_REVERS) & MGMT_MASK_REVERS;
}

void qsobj_set_score (qsobj_t * obj, int val)
{
  obj->mgmt &= ~MGMT_MASK_SCORE;
  obj->mgmt |= (val << MGMT_SHIFT_SCORE) & MGMT_MASK_SCORE;
}

void qsobj_set_allocscale (qsobj_t * obj, int val)
{
  obj->mgmt &= ~MGMT_MASK_ALLOC;
  obj->mgmt |= (val << MGMT_SHIFT_ALLOC) & MGMT_MASK_ALLOC;
}




/* Prototype 1: Single-bounds pointer-content (Triplet). */
qstriplet_t * qstriplet_init (qstriplet_t * triplet, qsptr_t first, qsptr_t second, qsptr_t third)
{
  qsobj_init((qsobj_t*)triplet, 0, false);
  triplet->first = first;
  triplet->second = second;
  triplet->third = third;
  return triplet;
}

qsptr_t qstriplet_ref_first (const qstriplet_t * triplet)
{
  return triplet->first;
}

qsptr_t qstriplet_ref_second (const qstriplet_t * triplet)
{
  return triplet->second;
}

qsptr_t qstriplet_ref_third (const qstriplet_t * triplet)
{
  return triplet->third;
}

qserr_t qstriplet_setq_first (qstriplet_t * triplet, qsptr_t val)
{
  triplet->first = val;
  return QSERR_OK;
}

qserr_t qstriplet_setq_second (qstriplet_t * triplet, qsptr_t val)
{
  triplet->second = val;
  return QSERR_OK;
}

qserr_t qstriplet_setq_third (qstriplet_t * triplet, qsptr_t val)
{
  triplet->third = val;
  return QSERR_OK;
}


/* Prototype 2: Cross-bounds pointer-content (PointerVector). */
qspvec_t * qspvec_init (qspvec_t * pvec, int allocscale, qsptr_t len)
{
  qsobj_init((qsobj_t*)pvec, allocscale, false);
  pvec->length = len;
  return pvec;
}

qsptr_t qspvec_ref_length (const qspvec_t * pvec)
{
  return pvec->length;
}

qsptr_t qspvec_ref (const qspvec_t * pvec, qsword k)
{
  qsword lim = (1 << qsobj_get_allocscale((qsobj_t*)pvec));
  if (k < lim)
    return pvec->elt[k];
  else
    return QSNIL;  /* TODO: out of bounds exception. */
}

qserr_t qspvec_setq_length (qspvec_t * pvec, qsptr_t len)
{
  pvec->length = len;
  return QSERR_OK;
}

qsptr_t qspvec_setq (qspvec_t * pvec, qsword k, qsptr_t val)
{
  qsword lim = (1 << qsobj_get_allocscale((qsobj_t*)pvec));
  if (k < lim)
    pvec->elt[k] = val;
  else
    return QSERR_FAULT;  /* Out Of Bounds. */
  return QSERR_OK;
}



/* Prototype 3: Single-bounds octet content (WideWord). */
qswideword_t * qswideword_init (qswideword_t * wideword, qsptr_t subtype, union qswidepayload_u * initsrc)
{
  qsobj_init((qsobj_t*)wideword, 0, true);
  wideword->subtype = subtype;
  if (initsrc)
    {
      wideword->payload = *initsrc;
    }
  return wideword;
}

qsptr_t qswideword_ref_subtype (const qswideword_t * wideword)
{
  return wideword->subtype;
}

const union qswidepayload_u * qswideword_at_const (const qswideword_t * wideword)
{
  return &(wideword->payload);
}

int qswideword_fetch_payload (const qswideword_t * wideword, union qswidepayload_u * buf)
{
  *buf = wideword->payload;
  return 1;
}

qserr_t qswideword_setq_subtype (qswideword_t * wideword, qsptr_t subtype)
{
  wideword->subtype = subtype;
  return QSERR_OK;
}

union qswidepayload_u * qswideword_at (qswideword_t * wideword)
{
  return &(wideword->payload);
}

int qswideword_put_payload (qswideword_t * wideword, const union qswidepayload_u * buf)
{
  wideword->payload = *buf;
  return 1;
}



/* Prototype 4: Cross-bounds octet content (OctetVector). */
qsovec_t * qsovec_init (qsovec_t * ovec, int allocscale, qsptr_t length)
{
  qsobj_init((qsobj_t*)ovec, allocscale, true);
  ovec->length = length;
  ovec->refcount = 0;
  ovec->reflock = 0;
  return ovec;
}

qsptr_t qsovec_ref_length (const qsovec_t * ovec)
{
  return ovec->length;
}

qsword qsovec_get_refcount (const qsovec_t * ovec)
{
  return ovec->refcount;
}

qsbyte qsovec_ref (const qsovec_t * ovec, qsword k)
{
  qsword lim = (1 << qsobj_get_allocscale((qsobj_t*)ovec));
  if (k < lim)
    {
      return ovec->elt[k];
    }
  return 0;
}

qserr_t qsovec_setq_length (qsovec_t * ovec, qsptr_t len)
{
  ovec->length = len;
  return QSERR_OK;
}

qserr_t qsovec_setq (qsovec_t * ovec, qsword k, qsbyte val)
{
  qsword lim = (1 << qsobj_get_allocscale((qsobj_t*)ovec));
  if (k < lim)
    {
      ovec->elt[k] = val;
      return QSERR_OK;
    }
  else
    {
      return QSERR_FAULT;   /* Out Of Bounds. */
    }
}

qserr_t qsovec_incr_refcount (qsovec_t * ovec)
{
  /* TODO: implement. */
  (void)ovec;
  return QSERR_OK;
}

qserr_t qsovec_decr_refcount (qsovec_t * ovec)
{
  /* TODO: implement. */
  (void)ovec;
  return QSERR_OK;
}



qsfreelist_t * qsfreelist_init (qsfreelist_t * freelist, qsword length, qsaddr_t prev, qsaddr_t next)
{
  freelist->mgmt = (TAG_SYNC29);  /* not-Used. */
  freelist->length = length;
  freelist->prev = prev;
  freelist->next = next;
  return freelist;
}

qsfreelist_t * qsfreelist_destroy (qsfreelist_t * freelist)
{
  return freelist;
}

qsword qsfreelist_length (qsfreelist_t * freelist)
{
  return freelist->length;
}

qsaddr_t qsfreelist_prev (qsfreelist_t * freelist)
{
  return freelist->prev;
}

qsaddr_t qsfreelist_next (qsfreelist_t * freelist)
{
  return freelist->next;
}

