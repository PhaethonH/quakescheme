#include <assert.h>

#include <math.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>

#include "qsobj.h"


/* printf-able, tag pointer with type. */
const char * __p (qsptr_t p)
{
  static char buf[32] = { 0, };
  if (ISFLOAT31(p))
    {
      snprintf(buf, sizeof(buf), "#f:%08x", p);
    }
  else if (ISINT30(p))
    {
      snprintf(buf, sizeof(buf), "#i:%08x", p);
    }
  else if (ISSYNC29(p))
    {
      snprintf(buf, sizeof(buf), "#s:%08x", p);
    }
  else if (ISITER28(p))
    {
      snprintf(buf, sizeof(buf), "#ι:%08x", p);
    }
  else if (ISOBJ26(p))
    {
      snprintf(buf, sizeof(buf), "#o:%08x", p);
    }
  else if (ISCHAR24(p))
    {
      snprintf(buf, sizeof(buf), "#c:%08x", p);
    }
  else if (ISERROR16(p))
    {
      snprintf(buf, sizeof(buf), "#E:%08x", p);
    }
  else if (ISCONST16(p))
    {
      snprintf(buf, sizeof(buf), "#k:%08x", p);
    }
  else
    {
      snprintf(buf, sizeof(buf), "#?:%08x", p);
    }
  return buf;
}




/* Attempt to cast to object (else freelist or data) */
qsobj_t * qsobj (qsmem_t * mem, qsptr_t p, qsmemaddr_t * out_addr)
{
  if (!ISHEAP26(p)) return NULL;
  qsmemaddr_t addr = COBJ26(p);
  qsobj_t * retval = (qsobj_t*)qsheap_ref(mem, addr);
  if (!retval) return NULL;
  if (! qsheapcell_is_synced((qsheapcell_t*)retval)) return NULL;
  if (! qsheapcell_is_used((qsheapcell_t*)retval)) return NULL;
  if (out_addr)
    *out_addr = addr;
  return retval;
}

int qsobj_used_p (qsmem_t * mem, qsptr_t p)
{
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  if (!obj) return 0;
  return qsheapcell_is_used((qsheapcell_t*)obj);
}

int qsobj_marked_p (qsmem_t * mem, qsptr_t p)
{
  qsobj_t * obj = qsobj(mem, p, NULL);
  if (!obj) return 0;
  return qsheapcell_is_marked((qsheapcell_t*)obj);
}

int qsobj_ref_parent (qsmem_t * mem, qsptr_t p)
{
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  if (!obj) return 0;
  return qsheapcell_get_parent((qsheapcell_t*)obj);
}

int qsobj_ref_score (qsmem_t * mem, qsptr_t p)
{
  qsobj_t * obj = qsobj(mem, p, NULL);
  if (!obj) return 0;
  return qsheapcell_get_score((qsheapcell_t*)obj);
}

qsword qsobj_ref_allocsize (qsmem_t * mem, qsptr_t p)
{
  qsobj_t * obj = qsobj(mem, p, NULL);
  if (!obj) return 0;
  int allocscale = qsheapcell_get_allocscale((qsheapcell_t*)obj);
  return (1 << allocscale);
}

qsptr_t qsobj_get (qsmem_t * mem, qsptr_t p, qsword field_idx)
{
  qsobj_t * obj = qsobj(mem, p, NULL);
  if (!obj) return QSNIL;
  switch (field_idx)
    {
    case 0: return obj->_0; break;
    case 1: return obj->_1; break;
    case 2: return obj->_2; break;
    default: return QSNIL; break;
    }
  return QSNIL;
}

qsptr_t qsobj_setq_marked (qsmem_t * mem, qsptr_t p, qsword val)
{
  qsobj_t * obj = qsobj(mem, p, NULL);
  if (!obj) return 0;
  qsheapcell_set_marked((qsheapcell_t*)obj, val);
  return p;
}

qsptr_t qsobj_setq_score (qsmem_t * mem, qsptr_t p, qsword val)
{
  qsobj_t * obj = qsobj(mem, p, NULL);
  if (!obj) return 0;
  qsheapcell_set_score((qsheapcell_t*)obj, val);
  return p;
}

qsptr_t qsobj_setq_parent (qsmem_t * mem, qsptr_t p, qsword val)
{
  qsobj_t * obj = qsobj(mem, p, NULL);
  if (!obj) return 0;
  qsheapcell_set_parent((qsheapcell_t*)obj, val);
  return p;
}

/* Object as single-celled pointer contents. */
qsobj_t * qsobj_unicellular (qsmem_t * mem, qsptr_t p, qsmemaddr_t * out_addr)
{
  qsobj_t * obj = qsobj(mem, p, out_addr);
  if (!obj) return NULL;
  qsheapcell_t * heapcell = (qsheapcell_t*)obj;
  if (qsheapcell_is_octet(heapcell)) return NULL;
  if (qsheapcell_get_allocscale(heapcell) > 0) return NULL;
  return obj;
}

/* Object as multi-celled pointer contents. */
qsobj_t * qsobj_multicellular (qsmem_t * mem, qsptr_t p, qsmemaddr_t * out_addr)
{
  qsobj_t * obj = qsobj(mem, p, out_addr);
  if (!obj) return NULL;
  qsheapcell_t * heapcell = (qsheapcell_t*)obj;
  if (qsheapcell_is_octet(heapcell)) return NULL;
  if (qsheapcell_get_allocscale(heapcell) == 0) return NULL;
  return obj;
}

/* Object is single-celled octet contents (uni- octet -ate). */
qsobj_t * qsobj_unioctetate (qsmem_t * mem, qsptr_t p, qsmemaddr_t * out_addr)
{
  qsobj_t * obj = qsobj(mem, p, out_addr);
  if (!obj) return NULL;
  qsheapcell_t * heapcell = (qsheapcell_t*)obj;
  if (! qsheapcell_is_octet(heapcell)) return NULL;
  if (qsheapcell_get_allocscale(heapcell) > 0) return NULL;
  return obj;
}

/* Object is multi-celled octet contents (multi- octet -ate). */
qsobj_t * qsobj_multioctetate (qsmem_t * mem, qsptr_t p, qsmemaddr_t * out_addr)
{
  qsobj_t * obj = qsobj(mem, p, out_addr);
  if (!obj) return NULL;
  qsheapcell_t * heapcell = (qsheapcell_t*)obj;
  if (! qsheapcell_is_octet(heapcell)) return NULL;
  if (qsheapcell_get_allocscale(heapcell) == 0) return NULL;
  return obj;
}

/*
   Conglomerated allocation logic:

   [in]k - number of cells (non-octetate) or number of bytes (octetate).
   [in]octetate - contents are bytes, else contents are pointers.
   [out]out_addr - linear address in heap space.

Returns: pointer value of newly allocation object, or error code (QSERROR_*).
*/
qsptr_t qsobj_make (qsmem_t * mem, qsword k, int octetate, qsmemaddr_t * out_addr)
{
  qsptr_t retval = QSERROR_NOMEM;
  qsmemaddr_t addr = 0;

  if (octetate)
    {
      retval = qsheap_alloc_with_nbytes(mem, (k>0)?k:0, &addr);
    }
  else
    {
      retval = qsheap_alloc_ncells(mem, (k>1)?k:1, &addr);
    }

  if (retval == QSERROR_OK)
    {
      retval = QSOBJ(addr);
      if (out_addr)
	*out_addr = addr;
    }
  // else retval already holds error code.
  return retval;
}

qserror_t qsobj_kmark (qsmem_t * mem, qsptr_t p)
{
  qsmemaddr_t addr = 0;
  qsobj_t * obj = qsobj(mem, p, &addr);
  if (!obj) return QSERROR_OK;
  qsptr_t currptr = p;
  qsptr_t backptr = QSNIL;
  qsptr_t next_visit = QSNIL;

  backptr = QSERROR_INVALID;
  while (currptr != QSERROR_INVALID)
    {
      if (qstree(mem, currptr))
	{
	  qstree_kmark(mem, currptr, backptr, &next_visit);
	  backptr = currptr;
	}
      else if (qsvector(mem, currptr, NULL))
	{
	  qsvector_kmark(mem, currptr, backptr, &next_visit);
	  backptr = currptr;
	}
      else
	{
	  next_visit = backptr;
	  backptr = currptr;
	}
      currptr = next_visit;
    }
  return QSERROR_OK;
}




qstree_t * qstree (qsmem_t * mem, qsptr_t t)
{
  qsobj_t * obj = qsobj_unicellular(mem, t, NULL);
  if (!obj) return NULL;
  return (qstree_t *)obj;
}

qsptr_t qstree_ref_left (qsmem_t * mem, qsptr_t t)
{
  qstree_t * tree = qstree(mem, t);
  if (!tree)
    {
      // TODO: exception.
      return QSNIL;
    }
  return tree->left;
}

qsptr_t qstree_ref_data (qsmem_t * mem, qsptr_t t)
{
  qstree_t * tree = qstree(mem, t);
  if (!tree)
    {
      // TODO: exception.
      return QSNIL;
    }
  return tree->data;
}

qsptr_t qstree_ref_right (qsmem_t * mem, qsptr_t t)
{
  qstree_t * tree = qstree(mem, t);
  if (!tree)
    {
      // TODO: exception.
      return QSNIL;
    }
  return tree->right;
}

qsptr_t qstree_setq_left (qsmem_t * mem, qsptr_t t, qsptr_t val)
{
  qstree_t * tree = qstree(mem, t);
  if (!tree)
    {
      // TODO: exception.
      return QSNIL;
    }
  tree->left = val;
  return t;
}

qsptr_t qstree_setq_data (qsmem_t * mem, qsptr_t t, qsptr_t val)
{
  qstree_t * tree = qstree(mem, t);
  if (!tree)
    {
      // TODO: exception.
      return QSNIL;
    }
  tree->data = val;
  return t;
}

qsptr_t qstree_setq_right (qsmem_t * mem, qsptr_t t, qsptr_t val)
{
  qstree_t * tree = qstree(mem, t);
  if (!tree)
    {
      // TODO: exception.
      return QSNIL;
    }
  tree->right = val;
  return t;
}

qsptr_t qstree_make (qsmem_t * mem, qsptr_t left, qsptr_t data, qsptr_t right)
{
  qsptr_t retval = QSNIL;
  qsmemaddr_t addr = 0;
  if (!ISOBJ26((retval = qsobj_make(mem, 1, 0, &addr)))) return retval;

  qstree_t * tree = (qstree_t*)qsobj(mem, retval, NULL);
  tree->left = left;
  tree->data = data;
  tree->right = right;
  return retval;
}

qserror_t qstree_kmark (qsmem_t * mem, qsptr_t p, qsptr_t backptr, qsptr_t * nextptr)
{
  *nextptr = QSNIL;
  qsptr_t currptr = p;
  qsptr_t tempptr = QSNIL;
  qstree_t * tree = qstree(mem, p);  // try to get as tree.
  qsheapcell_t * heapcell = (qsheapcell_t*)tree;  // manipulate heap mgmt.
  if (!heapcell || qsheapcell_is_marked(heapcell))
    {
      /* invalid tree or already marked; nothing to recurse. */
      *nextptr = backptr;
      return QSERROR_OK;
    }
  int parent = 0;

  parent = qsheapcell_get_parent(heapcell);
  switch (parent)
    {
    case 0:
      /* chase left. */
      // store parent into "left" while chasing "left".
      tempptr = heapcell->fields[0];
      heapcell->fields[0] = backptr;
      // chase "left".
      currptr = tempptr;
      qsheapcell_set_parent(heapcell, 1);
      break;
    case 1:
      /* chase center. */
      // swap "left=parent" with "center" while chasing "center".
      tempptr = heapcell->fields[1];
      heapcell->fields[1] = heapcell->fields[0];
      heapcell->fields[0] = backptr;
      // chase "center".
      currptr = tempptr;
      qsheapcell_set_parent(heapcell, 2);
      break;
    case 2:
      /* chase right. */
      // swap "center=parent" with "right" while chasing "right".
      tempptr = heapcell->fields[2];
      heapcell->fields[2] = heapcell->fields[1];
      heapcell->fields[1] = backptr;
      // chase "center".
      currptr = tempptr;
      qsheapcell_set_parent(heapcell, 3);
      break;
    case 3:
      /* backtrack to parent. */
      // restore "right=parent" and while chasing "parent".
      tempptr = heapcell->fields[2];
      heapcell->fields[2] = backptr;
      // chase "right=parent".
      currptr = tempptr;
      qsheapcell_set_parent(heapcell, 0);
      // children are all chased, set Marked.
      qsheapcell_set_marked(heapcell, 1);
      break;
    default:
      break;
    }

  *nextptr = currptr;

  return QSERROR_OK;
}

int qstree_crepr (qsmem_t * mem, qsptr_t t, char * buf, int buflen)
{
  int n = 0;
  return n;
}




/* Helper functions for R-B tree. */

/* Rotate the (sub)tree left, returns new root. */
qsptr_t qsrbnode_rotate_left (qsmem_t * mem, qsptr_t pivot)
{
  if (ISNIL(pivot)) return pivot;  // no tree.
  qsptr_t rchild = qstree_ref_right(mem, pivot);
  if (ISNIL(rchild)) return pivot;  // no child to bring up.
  qsptr_t temp = qstree_ref_left(mem, rchild);
  qstree_setq_left(mem, rchild, pivot);
  qstree_setq_right(mem, pivot, temp);
  return rchild;
}

/* Rotate the (sub)tree right, returns new root. */
qsptr_t qsrbnode_rotate_right (qsmem_t * mem, qsptr_t pivot)
{
  if (ISNIL(pivot)) return pivot;  // no tree.
  qsptr_t lchild = qstree_ref_left(mem, pivot);
  if (ISNIL(lchild)) return pivot;  // no child to bring up.
  qsptr_t temp = qstree_ref_right(mem, lchild);
  qstree_setq_right(mem, lchild, pivot);
  qstree_setq_left(mem, pivot, temp);
  return lchild;
}

int qsrbnode_red_p (qsmem_t * mem, qsptr_t node)
{
  if (ISNIL(node)) return 0;
  return (1 == qsobj_ref_score(mem, node));
}

int qsrbnode_black_p (qsmem_t * mem, qsptr_t node)
{
  if (ISNIL(node)) return 1;
  return !qsrbnode_red_p(mem, node);
}

qsptr_t qsrbnode_paint (qsmem_t * mem, qsptr_t node, int val)
{
  return qsobj_setq_score(mem, node, val ? 1 : 0);
}

qsptr_t qsrbnode_paint_red (qsmem_t * mem, qsptr_t node)
{
  return qsrbnode_paint(mem, node, 1);
}

qsptr_t qsrbnode_paint_black (qsmem_t * mem, qsptr_t node)
{
  return qsrbnode_paint(mem, node, 0);
}




qsrbtree_t * qsrbtree (qsmem_t * mem, qsptr_t t)
{
  qsobj_t * obj = qsobj_multicellular(mem, t, NULL);
  if (!obj) return NULL;
  if (qsobj_ref_allocsize(mem, t) != 2) return NULL;
  qsrbtree_t * tree = (qsrbtree_t*)obj;
  if (tree->variant != QSRBTREE) return NULL;
  return tree;
}

qsptr_t qsrbtree_make (qsmem_t * mem, qsptr_t top_node, qsptr_t cmp)
{
  qsptr_t retval = QSNIL;
  qsmemaddr_t addr = 0;
  if (!ISOBJ26((retval = qsobj_make(mem, 2, 0, &addr)))) return retval;

  qsrbtree_t * rbtree = (qsrbtree_t*)qsobj(mem, retval, NULL);
  rbtree->variant = QSRBTREE;
  rbtree->top = top_node;
  rbtree->cmp = cmp;
  rbtree->mutex = QSINT(0);
  return retval;
}

qsptr_t qsrbtree_ref_top (qsmem_t * mem, qsptr_t root)
{
  qsrbtree_t * rbtree = qsrbtree(mem, root);
  if (!rbtree) return QSNIL;
  return rbtree->top;
}

qsptr_t qsrbtree_ref_cmp (qsmem_t * mem, qsptr_t root)
{
  qsrbtree_t * rbtree = qsrbtree(mem, root);
  if (!rbtree) return QSNIL;
  return rbtree->cmp;
}

qsptr_t qsrbtree_ref_up (qsmem_t * mem, qsptr_t root)
{
  qsrbtree_t * rbtree = qsrbtree(mem, root);
  if (!rbtree) return QSNIL;
  return rbtree->up;
}

qsptr_t qsrbtree_ref_down (qsmem_t * mem, qsptr_t root)
{
  qsrbtree_t * rbtree = qsrbtree(mem, root);
  if (!rbtree) return QSNIL;
  return rbtree->down;
}

qsptr_t qsrbtree_setq_top (qsmem_t * mem, qsptr_t root, qsptr_t val)
{
  qsrbtree_t * rbtree = qsrbtree(mem, root);
  if (!rbtree) return QSNIL;
  rbtree->top = val;
  return root;
}

qsptr_t qsrbtree_setq_cmp (qsmem_t * mem, qsptr_t root, qsptr_t val)
{
  qsrbtree_t * rbtree = qsrbtree(mem, root);
  if (!rbtree) return QSNIL;
  rbtree->cmp = val;
  return root;
}

qsptr_t qsrbtree_setq_up (qsmem_t * mem, qsptr_t root, qsptr_t val)
{
  qsrbtree_t * rbtree = qsrbtree(mem, root);
  if (!rbtree) return QSNIL;
  rbtree->up = val;
  return root;
}

qsptr_t qsrbtree_setq_down (qsmem_t * mem, qsptr_t root, qsptr_t val)
{
  qsrbtree_t * rbtree = qsrbtree(mem, root);
  if (!rbtree) return QSNIL;
  rbtree->down = val;
  return root;
}


/* Lock tree for update, indicates tree may be in inconsistent state. */
qsptr_t qsrbtree_lock (qsmem_t * mem, qsptr_t root)
{
  return QSTRUE;
}

/* Unlock tree after updating. */
qsptr_t qsrbtree_unlock (qsmem_t * mem, qsptr_t root)
{
  return QSTRUE;
}

/* Split a tree to set up reverse pointers.
   Updates internal "splits" state to track the current split point.
   Returns pointer to root of newly detached subtree.
 */
qsptr_t qsrbtree_split (qsmem_t * mem, int leftward, qsptr_t rootptr)
{
  qsptr_t next = QSNIL;
  qsptr_t parent = QSNIL;
  qsrbtree_t * rbtree = qsrbtree(mem, rootptr);
  assert(rbtree);
  qsptr_t currptr = qsrbtree_ref_down(mem, rootptr);
  if (leftward)
    {
      next = qstree_ref_left(mem, currptr);
      qsptr_t parent = qsrbtree_ref_up(mem, rootptr);
      /* indicate ->left is actually parent. */
      currptr = qstree_setq_left(mem, currptr, parent);
      currptr = qsobj_setq_parent(mem, currptr, 1); 
    }
  else
    {
      next = qstree_ref_right(mem, currptr);
      qsptr_t parent = qsrbtree_ref_up(mem, rootptr);
      /* indicate ->right is actually parent. */
      currptr = qstree_setq_right(mem, currptr, parent);
      currptr = qsobj_setq_parent(mem, currptr, 2); 
    }
  qsrbtree_setq_up(mem, rootptr, currptr); // parent of subtree.
  qsrbtree_setq_down(mem, rootptr, next); // subtree
  return rootptr;
}

qsptr_t qsrbtree_split_left (qsmem_t * mem, qsptr_t rootptr)
{
  return qsrbtree_split(mem, 1, rootptr);
}

qsptr_t qsrbtree_split_right (qsmem_t * mem, qsptr_t rootptr)
{
  return qsrbtree_split(mem, 0, rootptr);
}

/* Mend a tree to restore from reverse pointers.
   Attaches 'currptr' back into 'root' based on internal "splits" state.
   Returns pointer to newly mended parent node (parent of subtree).
 */
qsptr_t qsrbtree_mend (qsmem_t * mem, qsptr_t rootptr)
{
  qsptr_t up = QSNIL;
  assert(rootptr);

  qsptr_t parent = qsrbtree_ref_up(mem, rootptr);
  if (ISNIL(parent))
    {
      /* Done. */
      rootptr = qsrbtree_setq_down(mem, rootptr, QSNIL);
      return rootptr;
    }
  qsptr_t currptr = qsrbtree_ref_down(mem, rootptr);

  int ancestry = qsobj_ref_parent(mem, parent);
  if (ancestry == 1)
    {
      /* restore left. */
      up = qstree_ref_left(mem, parent);
      currptr = qstree_setq_left(mem, parent, currptr);
    }
  else if (ancestry == 2)
    {
      /* restore right. */
      up = qstree_ref_right(mem, parent);
      currptr = qstree_setq_right(mem, parent, currptr);
    }
  else
    {
      /* inconsistent state. */
      abort();
    }
  currptr = qsobj_setq_parent(mem, parent, 0);  /* clear reversal. */

  /* move up. */
  rootptr = qsrbtree_setq_up(mem, rootptr, up);
  rootptr = qsrbtree_setq_down(mem, rootptr, currptr);

  return rootptr;
}

qsptr_t qsrbtree_ref_uncle (qsmem_t * mem, qsptr_t rootptr)
{
  qsptr_t parent = qsrbtree_ref_up(mem, rootptr);
  if (ISNIL(parent)) return QSNIL;
  int ancestry = qsobj_ref_parent(mem, parent);
  qsptr_t grandparent = QSNIL;
  if (ancestry == 1)
    {
      /* (grand)parent is stored in "left". */
      grandparent = qstree_ref_left(mem, parent);
    }
  else if (ancestry == 2)
    {
      /* (grand)parent is stored in "right". */
      grandparent = qstree_ref_right(mem, parent);
    }
  else
    {
      /* inconsistent state. */
      abort();
    }

  if (ISNIL(grandparent)) return QSNIL;

  ancestry = qsobj_ref_parent(mem, grandparent);
  if (ancestry == 1)
    {
      /* greatgrandparent in "left", sibling is right. */
      return qstree_ref_right(mem, grandparent);
    }
  else if (ancestry == 2)
    {
      /* greatgrandparent in "right", sibling is left. */
      return qstree_ref_left(mem, grandparent);
    }
  else
    {
      /* inconsistent state. */
      abort();
    }

  return QSNIL;
}

qsptr_t qsrbtree_ref_grandparent (qsmem_t * mem, qsptr_t rootptr)
{
  qsptr_t currptr = qsrbtree_ref_down(mem, rootptr);
  qsptr_t parent = qsrbtree_ref_up(mem, rootptr);
  if (ISNIL(currptr)) return QSNIL;
  if (ISNIL(parent)) return QSNIL;
  int ancestry = qsobj_ref_parent(mem, parent);
  if (ancestry == 1)
    {
      /* (grand)parent is stored in "left". */
      return qstree_ref_left(mem, parent);
    }
  else if (ancestry == 2)
    {
      /* (grand)parent is stored in "right". */
      return qstree_ref_right(mem, parent);
    }
  else
    {
      /* inconsistent state. */
      abort();
    }
  return QSNIL;
}


/* Insert association pair, rebalancing as needed.
   Merging whole trees not supported.
 */
qsptr_t qsrbtree_insert (qsmem_t * mem, qsptr_t root, qsptr_t apair)
{
  /* the "New" tree node; starts off black. */
  qsptr_t N = qstree_make(mem, QSNIL, apair, QSNIL);

  if (ISNIL(root))
    {
      /* create tree. */
      root = qsrbtree_make(mem, N, QSNIL);
      return root;
    }
  if (ISNIL(qsrbtree_ref_top(mem, root)))
    {
      /* empty tree. */
      root = qsrbtree_setq_top(mem, root, N);
      return root;
    }

  qsrbtree_lock(mem, root);
  root = qsrbtree_setq_up(mem, root, QSNIL);
  root = qsrbtree_setq_down(mem, root, qsrbtree_ref_top(mem, root));

  qsptr_t d = QSNIL;
  qsptr_t newkey = qspair_ref_a(mem, apair);
  int libra = 0;
  qsword lim = 258;  /* avoid infinite loop; max used depth around  31. */

  /* traverse to bottom of tree. */
  while (!ISNIL(qsrbtree_ref_down(mem, root)) && (--lim > 0))
    {
      qsptr_t currnode = qsrbtree_ref_down(mem, root);
      d = qstree_ref_data(mem, currnode);
      // TODO: custom comparator
      if (qstree(mem, d))  // also covers pair (apair).
	{
	  //qsptr_t currkey = qspair_ref_a(mem, d);
	  qsptr_t currkey = qstree_ref_data(mem, d);
	  libra = qsstr_cmp(mem, newkey, currkey);
	}
      else
	{
	  /* No compare; bias to right (as would a list). */
	  libra = 1;
	}
      if (libra == 0)
	break; /* match. */
      else if (libra < 0)
	{
	  /* go left. */
	  root = qsrbtree_split_left(mem, root);
	  assert(!ISNIL(root));
	}
      else if (libra > 0)
	{
	  /* go right. */
	  root = qsrbtree_split_right(mem, root);
	  assert(!ISNIL(root));
	}
    }

  /* insertion. */
  /* mend() naturally stitches new node into correct place. */
  root = qsrbtree_setq_down(mem, root, N);

  /* rebalancing. */
  int case3recursion = 0;  /* Case 3 requires recursion of rebalancing. */

  do
    {
      qsptr_t P = qsrbtree_ref_up(mem, root);
      if (!qsrbnode_red_p(mem, P))
	{
	  /* case 2: parent is black, N (should be) red. */
	  qsrbnode_paint_red(mem, N);
	  goto unwind;
	}
      /* Parent is red. */
      N = qsrbnode_paint_red(mem, N);
      qsptr_t U = qsrbtree_ref_uncle(mem, root);
      qsptr_t G = qsrbtree_ref_grandparent(mem, root);
      if (qsrbnode_red_p(mem, U))
	{
	  /* case 3: red P, red U
	     => blacken P, blacken U, redden G, recurse on G.
	     */
	  P = qsrbnode_paint_black(mem, P);
	  U = qsrbnode_paint_black(mem, U);
	  G = qsrbnode_paint_red(mem, G);

	  /* recurse on G. */
	  root = qsrbtree_mend(mem, root);  /* curr = P */
	  assert(!ISNIL(root));
	  root = qsrbtree_mend(mem, root);  /* curr = G */
	  assert(!ISNIL(root));

	  P = qsrbtree_ref_up(mem, root);
	  if (ISNIL(P) || (N == qsrbtree_ref_top(mem, root)))
	    {
	      /* ended up at root: make it black, done. */
	      qsptr_t rootnode = qsrbtree_ref_top(mem, root);
	      rootnode = qsrbnode_paint_black(mem, rootnode);
	      root = qsrbtree_setq_top(mem, root, rootnode);
	      goto unwind;
	    }

	  case3recursion = 1;
	  N = G;
	  continue;  /* repeat loop: recurse on G (N' = G). */
	}

      /* case 4 or 5: red P, U black.
	 => find out if inner-rotation or outer-rotation.
	 */
      /* Path Grandparent-to-Parent; left=-1, right=1 */
      int gp = qsobj_ref_parent(mem, G) == 1 ? -1 : 1;
      /* Path Parent-to-Node; left=-1, right=1 */
      int pn = qsobj_ref_parent(mem, P) == 1 ? -1 : 1;

      /* Move up to grandparent to track rotates on G. */
      root = qsrbtree_mend(mem, root);
      assert(!ISNIL(root));
      root = qsrbtree_mend(mem, root);
      assert(!ISNIL(root));

      assert(qsobj_ref_parent(mem, P) == 0);
      assert(qsobj_ref_parent(mem, U) == 0);
      assert(qsobj_ref_parent(mem, G) == 0);
      assert(qsobj_ref_parent(mem, N) == 0);

      if (gp != pn)
	{
	  /* case 4: G.left,P.right || G.right,P.left  (inner rotation)
	     => Convert to case 5.
	     */
	  if (pn == -1)
	    {
	      /* G.right, P.left  becomes  G.right, P.right (case 5) */
	      P = qsrbnode_rotate_right(mem, P);
	      G = qstree_setq_right(mem, G, P);
	      pn = 1;
	    }
	  else
	    {
	      /* G.left, P.right  becomes  G.left, P.left (case 5) */
	      P = qsrbnode_rotate_left(mem, P);
	      G = qstree_setq_left(mem, G, P);
	      pn = -1;
	    }
	}

      if (gp == pn)
	{
	  /* case 5: G.left,P.left || G.right,P.right  (outer rotation) */
	  qsptr_t S = QSNIL;  /* sibling to N after rotation. */
	  if (pn == -1)
	    {
	      G = qsrbnode_rotate_right(mem, G);
	      S = qstree_ref_right(mem, G);
	    }
	  else
	    {
	      G = qsrbnode_rotate_left(mem, G);
	      S = qstree_ref_left(mem, G);
	    }
	  /* paint S red, paint G' black, N is red. */
	  S = qsrbnode_paint_red(mem, S);
	  G = qsrbnode_paint_black(mem, G);
	  /* rearrange splits. */
	  root = qsrbtree_setq_down(mem, root, G);

	  goto unwind;
	}
      else
	{
	  /* inconsistent state. */
	  abort();
	}

    }
  while (case3recursion);

unwind:
  /* recursive mend */
  while (!ISNIL(qsrbtree_ref_up(mem, root)))
    {
      root = qsrbtree_mend(mem, root);
      assert(!ISNIL(root));
    }

  /* Top node might have changed. */
  qsptr_t final_mend = qsrbtree_ref_down(mem, root);
  if (final_mend != qsrbtree_ref_top(mem, root))
    {
      root = qsrbtree_setq_top(mem, root, final_mend);
    }
  assert(ISNIL(qsrbtree_ref_up(mem, root)));
  root = qsrbtree_setq_down(mem, root, QSNIL);
  qsrbtree_unlock(mem, root);

  return root;
}

/* Return best tree node fulfilling critera 'key'.
   nil if not found.
 */
qsptr_t qstree_find (qsmem_t * mem, qsptr_t t, qsptr_t key, qsptr_t * nearest)
{
  qsptr_t probe = t;
  qsptr_t prev = QSNIL;
  qsptr_t d;
  int libra = 0;
  qsptr_t probekey = QSNIL;
  while (!ISNIL(probe))
    {
      prev = probe;
      d = qstree_ref_data(mem, probe);
      probekey = QSNIL;
      libra = 0;
      // TODO: custom comparator
      if (qstree(mem, d))
	{
	  //probekey = qspair_ref_a(mem, d);
	  probekey = qstree_ref_data(mem, d);
	  libra = qsstr_cmp(mem, key, probekey);
	}
      else
	{
	  /* uncomparable; just go right. */
	  libra = 1;
	}
      if (libra == 0) return probe;  // match.
      else if (libra < 0) probe = qstree_ref_left(mem, probe);
      else if (libra > 0) probe = qstree_ref_right(mem, probe);
    }
  if (nearest)
    *nearest = prev;  // best match (ready for insert).
  return QSNIL;
}

/* Returns association-pair matching criteria 'key'.
   nil if not found.
   */
qsptr_t qstree_assoc (qsmem_t * mem, qsptr_t t, qsptr_t key)
{
  qsptr_t probe = qstree_find(mem, t, key, NULL);
  if (ISNIL(probe)) return QSNIL;
  qsptr_t d = qstree_ref_data(mem, probe);
  return d;
}

qsptr_t qsrbtree_find (qsmem_t * mem, qsptr_t root, qsptr_t key, qsptr_t * nearest)
{
  qsptr_t raw_tree = qsrbtree_ref_top(mem, root);
  return qstree_find(mem, raw_tree, key, nearest);
}

qsptr_t qsrbtree_assoc (qsmem_t * mem, qsptr_t root, qsptr_t key)
{
  qsptr_t probe = qsrbtree_find(mem, root, key, NULL);
  if (ISNIL(probe)) return QSNIL;
  qsptr_t d = qstree_ref_data(mem, probe);
  return d;
}




qsibtree_t * qsibtree (qsmem_t * mem, qsptr_t t)
{
  qstree_t * tree = qstree(mem, t);
  return (qsibtree_t*)tree;
}

qsptr_t qsibtree_make (qsmem_t * mem)
{
  qsptr_t retval = qstree_make(mem, QSINT(0), QSNIL, QSNIL);
  return retval;
}

qsword qsibtree_ref_filled (qsmem_t * mem, qsptr_t t)
{
  qsibtree_t * ibtree = qsibtree(mem, t);
  if (!ibtree) return 0;
  return CINT30(ibtree->filled);
}

qsptr_t qsibtree_ref_idx0 (qsmem_t * mem, qsptr_t t)
{
  qsibtree_t * ibtree = qsibtree(mem, t);
  if (!ibtree) return QSNIL;
  return ibtree->idx0;
}

qsptr_t qsibtree_ref_ones (qsmem_t * mem, qsptr_t t)
{
  qsibtree_t * ibtree = qsibtree(mem, t);
  if (!ibtree) return QSNIL;
  return ibtree->ones;
}

qsptr_t qsibtree_setq_filled (qsmem_t * mem, qsptr_t t, qsword count)
{
  qsibtree_t * ibtree = qsibtree(mem, t);
  if (!ibtree) return t;
  ibtree->filled = QSINT(count);
  return t;
}

qsptr_t qsibtree_setq_idx0 (qsmem_t * mem, qsptr_t t, qsptr_t val)
{
  qsibtree_t * ibtree = qsibtree(mem, t);
  if (!ibtree) return t;
  ibtree->idx0 = val;
  return t;
}

qsptr_t qsibtree_setq_ones (qsmem_t * mem, qsptr_t t, qsptr_t val)
{
  qsibtree_t * ibtree = qsibtree(mem, t);
  if (!ibtree) return t;
  ibtree->ones = val;
  return t;
}

qsptr_t qsibnode_find (qsmem_t * mem, qsptr_t t, qsword path)
{
  qsptr_t curr = t;
  qsword remainder = path;
  while ((remainder > 1) && !ISNIL(curr))
    {
      if (remainder & 0x01)
	{
	  /* go right. */
	  curr = qstree_ref_right(mem, curr);
	}
      else
	{
	  /* go left. */
	  curr = qstree_ref_left(mem, curr);
	}
      remainder >>= 1;
    }
  return curr;
}

qsptr_t qsibnode_pave (qsmem_t * mem, qsptr_t t, qsword path)
{
  qsptr_t curr = t;
  qsword remainder = path;
  while (remainder > 1)
    {
      if (remainder & 0x01)
	{
	  /* rightwards. */
	  if (!ISNIL(qstree_ref_right(mem, curr)))
	    {
	      /* go right. */
	      curr = qstree_ref_right(mem, curr);
	    }
	  else
	    {
	      /* make right. */
	      qsptr_t next = qstree_make(mem, QSNIL, QSNIL, QSNIL);
	      qstree_setq_right(mem, curr, next);
	      curr = next;
	    }
	}
      else
	{
	  /* leftwards. */
	  if (!ISNIL(qstree_ref_left(mem, curr)))
	    {
	      /* go left. */
	      curr = qstree_ref_left(mem, curr);
	    }
	  else
	    {
	      /* make left. */
	      qsptr_t next = qstree_make(mem, QSNIL, QSNIL, QSNIL);
	      qstree_setq_left(mem, curr, next);
	      curr = next;
	    }
	}
      remainder >>= 1;
    }
  return curr;
}

qsword qsibtree_length (qsmem_t * mem, qsptr_t t)
{
  return qsibtree_ref_filled(mem, t);
}

qsptr_t qsibtree_ref (qsmem_t * mem, qsptr_t t, qsword path)
{
  if (path == 0)
    {
      return qsibtree_ref_idx0(mem, t);
    }
  qsptr_t ones = qsibtree_ref_ones(mem, t);
  qsptr_t target = qsibnode_find(mem, ones, path);
  if (ISNIL(target)) return QSNIL;
  return qstree_ref_data(mem, target);
}

qsptr_t qsibtree_setq (qsmem_t * mem, qsptr_t t, qsword path, qsptr_t entry)
{
  if (path == 0)
    {
      qsibtree_setq_idx0(mem, t, entry);
    }
  else if (path == 1)
    {
      qsptr_t ones = qsibtree_ref_ones(mem, t);
      if (ISNIL(ones))
	{
	  ones = qstree_make(mem, QSNIL, entry, QSNIL);
	  qsibtree_setq_ones(mem, t, ones);
	}
      else
	{
	  qstree_setq_data(mem, ones, entry);
	}
    }
  else
    {
      qsptr_t target = qsibnode_pave(mem, qsibtree_ref_ones(mem, t), path);
      qstree_setq_data(mem, target, entry);
    }

  if (path > CINT30(qsibtree_ref_filled(mem, t)))
    {
      qsibtree_setq_filled(mem, t, path);
    }

  return t;
}




/* Pair is a degenerate tree where left==QSNIL */
qspair_t * qspair (qsmem_t * mem, qsptr_t p)
{
  qstree_t * tree = qstree(mem, p);
  if (!tree) return NULL;
  if (tree->left != QSNIL) return NULL;
  return (qspair_t *)tree;
}

qsptr_t qspair_ref_a (qsmem_t * mem, qsptr_t p)
{
  qspair_t * pair = qspair(mem, p);
  if (!pair)
    {
      /* TODO: exception. */
      return QSNIL;
    }
  return pair->a;
}

qsptr_t qspair_ref_d (qsmem_t * mem, qsptr_t p)
{
  qspair_t * pair = qspair(mem, p);
  if (!pair)
    {
      /* TODO: exception. */
      return QSNIL;
    }
  return pair->d;
}

qsptr_t qspair_setq_a (qsmem_t * mem, qsptr_t p, qsptr_t val)
{
  qspair_t * pair = qspair(mem, p);
  if (!pair)
    {
      /* TODO: exception. */
      return QSNIL;
    }
  pair->a = val;
  return p;
}

qsptr_t qspair_setq_d (qsmem_t * mem, qsptr_t p, qsptr_t val)
{
  qspair_t * pair = qspair(mem, p);
  if (!pair)
    {
      /* TODO: exception. */
      return QSNIL;
    }
  pair->d = val;
  return p;
}

qserror_t qspair_alloc (qsmem_t * mem, qsptr_t * out_ptr, qsmemaddr_t * out_addr)
{
  return QSERROR_NOIMPL;
}

qsptr_t qspair_make (qsmem_t * mem, qsptr_t a, qsptr_t b)
{
  qsptr_t retval;
  return qstree_make(mem, QSNIL, a, b);
}

int qspair_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  return n;
}




/* Construct list from QSBOL terminated arguments. */
qsptr_t qslist_make (qsmem_t * mem, ...)
{
}

qsword qslist_length (qsmem_t * mem, qsptr_t p)
{
  qsword retval = 0;
  qsptr_t curr = p;
  while (curr != QSNIL)
    {
      retval++;
      qsptr_t next = qspair_ref_d(mem, curr);
      if (qspair(mem, next))
	{
	  curr = next;
	}
      else
	{
	  curr = QSNIL;
	}
    }
  return retval;
}

qsptr_t qslist_tail (qsmem_t * mem, qsptr_t p, qsword nth)
{
  qsptr_t curr = p;
  qsword ofs = 0;
  while ((ofs < nth) && (curr != QSNIL))
    {
      ofs++;
      qsptr_t next = qspair_ref_d(mem, curr);
      if (qspair(mem, next))
	{
	  curr = next;
	}
      else
	{
	  curr = QSNIL;
	}
    }
  if (curr)
    {
      return curr;
    }
  return QSNIL;
}

qsptr_t qslist_ref (qsmem_t * mem, qsptr_t p, qsword k)
{
  qsptr_t curr = qslist_tail(mem, p, k);
  if (curr != QSNIL)
    {
      return qspair_ref_a(mem, p);
    }
  return QSERROR_INVALID;
}




qsvector_t * qsvector (qsmem_t * mem, qsptr_t v, qsword * out_lim)
{
  qsobj_t * obj = qsobj_multicellular(mem, v, NULL);
  if (!obj) return NULL;
  if (! ISINT30(qsobj_get(mem, v, 0))) return NULL;
  if (out_lim)
    {
      *out_lim = CINT30(qsobj_get(mem, v, 0));
    }
  return (qsvector_t*)obj;
}

qsword qsvector_length (qsmem_t * mem, qsptr_t v)
{
  qsword len = 0;
  qsvector_t * vec = qsvector(mem, v, &len);
  if (!vec) return 0;  // TODO: exception
  return len;
}

qsptr_t qsvector_ref (qsmem_t * mem, qsptr_t v, qsword ofs)
{
  qsword lim = 0;
  qsvector_t * vec = qsvector(mem, v, &lim);
  if (!vec) return QSERROR_RANGE;
  if ((ofs < 0) || (ofs >= lim))
    {
      // TODO: exception.
      return QSERROR_RANGE;
    }
  return vec->_d[ofs];
}

qsptr_t qsvector_setq (qsmem_t * mem, qsptr_t v, qsword ofs, qsptr_t val)
{
  qsword lim = 0;
  qsvector_t * vec = qsvector(mem, v, &lim);
  if (!vec) return QSNIL;
  if ((ofs < 0) || (ofs >= lim))
    {
      // TODO: exception.
      return QSNIL;
    }
  vec->_d[ofs] = val;
  return v;
}

qserror_t qsvector_alloc (qsmem_t * mem, qsptr_t * out_ptr, qsmemaddr_t * out_addr, qsword cap)
{
  return QSERROR_NOIMPL;
}

qsptr_t qsvector_make (qsmem_t * mem, qsword k, qsptr_t fill)
{
  qsptr_t retval = QSNIL;
  qsmemaddr_t addr = 0;
  qsword ncells = 1 + (k / 4)+1;  // always terminate with QSEOL.
  if (!ISOBJ26((retval = qsobj_make(mem, ncells, 0, &addr)))) return retval;

  qsvector_t * vector = (qsvector_t*)qsheap_ref(mem, addr);
  vector->len = QSINT(k);
  vector->gc_backtrack = QSNIL;
  vector->gc_iter = QSNIL;
  qsword i;
  for (i = 0; i < k; i++)
    {
      vector->_d[i] = fill;
    }
  retval = QSOBJ(addr);
  vector->_d[k] = QSEOL; // terminate with QSEOL for qsiter implementation.
  return retval;
}

qserror_t qsvector_mark (qsmem_t * mem, qsptr_t v)
{
  qsword max = 0;
  qsvector_t * vec = qsvector(mem, v, &max);
  if (!vec) return QSERROR_INVALID;
  qsheapcell_t * cell = (qsheapcell_t*)vec;

  while (! qsobj_marked_p(mem, v))
    {
      /* main loop */
      if (qsheapcell_get_parent(cell) == 0)
	{
	  vec->gc_iter = QSINT(0);
	  qsheapcell_set_parent(cell, 1);
	}
      else if (qsheapcell_get_parent(cell) == 1)
	{
	  qsmemaddr_t idx = CINT30(qsheapcell_get_field(cell, 2));
	  while (idx < max)
	    {
	      qsptr_t elt = qsvector_ref(mem, v, idx);
	      /* TODO: recurse mark on elt. */
	      idx += 1;
	      qsheapcell_set_field(cell, 2, QSINT(idx));
	    }
	  qsheapcell_set_field(cell, 2, QSNIL);
	  qsheapcell_set_parent(cell, 0);
	  qsheapcell_set_marked(cell, 1);
	}
    }
  return QSERROR_OK;
}

qserror_t qsvector_kmark (qsmem_t * mem, qsptr_t p, qsptr_t backptr, qsptr_t * nextptr)
{
  qsword lim = 0;
  qsvector_t * vec = qsvector(mem, p, &lim);  // try to get as vector.
  qsheapcell_t * heapcell = (qsheapcell_t*)vec; // heap manipulation.
  if (!heapcell || qsheapcell_is_marked(heapcell))
    {
      *nextptr = backptr;
      return 0;
    }

  int parent = 0;
  qsptr_t elt = QSNIL;

  parent = qsheapcell_get_parent(heapcell);
  if (parent == 0)
    {
      qsheapcell_set_parent(heapcell, 1);
      vec->gc_backtrack = backptr;
      vec->gc_iter = QSINT(0);
    }
  else if (parent == 1)
    {
      qsword idx = CINT30(vec->gc_iter);
      if (idx >= lim)
	{
	  qsheapcell_set_marked(heapcell, 1);
	  qsheapcell_set_parent(heapcell, 0);
	  backptr = vec->gc_backtrack;
	  vec->gc_backtrack = QSNIL;
	  vec->gc_iter = QSNIL;
	  *nextptr = backptr;
	  return 0;
	}
      else
	{
	  elt = qsvector_ref(mem, p, idx);
	  vec->gc_iter = QSINT(idx+1);
	}
    }
  else
    {
      return QSERROR_INVALID;
    }

  *nextptr = elt;
  return QSERROR_OK;
}

int qsvector_crepr (qsmem_t * mem, qsptr_t v, char * buf, int buflen)
{
  int n = 0;
  return n;
}

/* Copy a qsvector from C memory into Scheme memory. */
qsptr_t qsvector_inject (qsmem_t * mem, qsword nelts, qsptr_t * carray)
{
  qsptr_t v;
  v = qsvector_make(mem, nelts, QSNIL);
  if (! ISOBJ26(v)) return 0;
  qsword i;
  for (i = 0; i < nelts; i++)
    {
      qsvector_setq(mem, v, i, carray[i]);
    }
  return v;
}

qsptr_t * qsvector_cptr (qsmem_t * mem, qsptr_t v, qsword * out_len)
{
  qsvector_t * cvec = qsvector(mem, v, out_len);
  if (!cvec) return NULL;
  return cvec->_d;
}








/**************/
/* Bytevector */
/**************/

qsbytevec_t * qsbytevec (qsmem_t * mem, qsptr_t bv, qsword * out_lim)
{
  qsobj_t * obj = qsobj_multioctetate(mem, bv, NULL);
  if (!obj) return NULL;
  if (! ISINT30(qsobj_get(mem, bv, 0))) return NULL;
  if (out_lim)
    {
      *out_lim = CINT30(qsobj_get(mem, bv, 0));
    }
  return (qsbytevec_t *)obj;
}

qserror_t qsbytevec_lock (qsmem_t * mem, qsptr_t bv)
{
  return QSERROR_NOIMPL;
}

qserror_t qsbytevec_unlock (qsmem_t * mem, qsptr_t bv)
{
  return QSERROR_NOIMPL;
}

qsword qsbytevec_refcount (qsmem_t * mem, qsptr_t bv)
{
  qsbytevec_t * bytevec = qsbytevec(mem, bv, NULL);
  if (!bytevec) return 0;
  return CINT30(bytevec->refcount);
}

// TODO: recount_increment and refcount_decrement

qsword qsbytevec_length (qsmem_t * mem, qsptr_t bv)
{
  qsbytevec_t * bytevec = qsbytevec(mem, bv, NULL);
  if (!bytevec) return 0;
  return CINT30(bytevec->len);
}

qsword qsbytevec_ref (qsmem_t * mem, qsptr_t bv, qsword ofs)
{
  qsbytevec_t * bytevec = qsbytevec(mem, bv, NULL);
  if (!bytevec) return 0;
  qsword max = qsbytevec_length(mem, bv);
  if ((ofs < 0) || (ofs >= max))
    return 0;
  return bytevec->_d[ofs];
}

qsptr_t qsbytevec_setq (qsmem_t * mem, qsptr_t bv, qsword ofs, qsword octet)
{
  qsbytevec_t * bytevec = qsbytevec(mem, bv, NULL);
  if (!bytevec) return 0;
  qsword max = qsbytevec_length(mem, bv);
  if ((ofs < 0) || (ofs >= max))
    return 0;
  bytevec->_d[ofs] = octet;
  return bv;
}

qsptr_t qsbytevec_make (qsmem_t * mem, qsword k, qsword fill)
{
  qsptr_t retval = QSNIL;
  qsmemaddr_t addr = 0;
  if (!ISOBJ26((retval = qsobj_make(mem, k, 1, &addr)))) return retval;

  qsbytevec_t * bytevec = (qsbytevec_t*)qsheap_ref(mem, addr);
  bytevec->len = QSINT(k);
  qsword i;
  for (i = 0; i < k; i++)
    {
      bytevec->_d[i] = (uint8_t)fill;
    }
  return retval;
}

int qsbytevec_crepr (qsmem_t * mem, qsptr_t bv, char * buf, int buflen)
{
  int n = 0;
  return n;
}

uint8_t * qsbytevec_cptr (qsmem_t * mem, qsword bv, qsword * out_len)
{
  qsbytevec_t * bytevec = qsbytevec(mem, bv, out_len);
  if (!bytevec) return NULL;
  return bytevec->_d;
}

qsptr_t qsbytevec_inject (qsmem_t * mem, qsword nbytes, uint8_t * carray)
{
  qsptr_t retval;
  retval = qsbytevec_make(mem, nbytes, 0);
  if (! ISOBJ26(retval)) return retval;
  uint8_t * target = qsbytevec_cptr(mem, retval, NULL);
  qsword i = 0;
  for (i = 0; i < nbytes; i++)
    {
      target[i] = carray[i];
    }
  return retval;
}








/****************/
/* Wide numbers */
/****************/

qswidenum_t * qswidenum (qsmem_t * mem, qsptr_t n, qsnumtype_t * out_numtype)
{
  if (!ISHEAP26(n)) return NULL;
  qsobj_t * obj = qsobj_unioctetate(mem, n, NULL);
  qsptr_t discriminator = obj->_0;
  switch (discriminator)
    {
    case QSNUMTYPE_NAN:
    case QSNUMTYPE_LONG:
    case QSNUMTYPE_DOUBLE:
    case QSNUMTYPE_INT2:
    case QSNUMTYPE_FLOAT2:
    case QSNUMTYPE_FLOAT4:
    case QSNUMTYPE_FLOAT16CM:
      if (out_numtype)
	*out_numtype = discriminator;
      return (qswidenum_t*)obj;
      break;
    default:
      return NULL;
      break;
    }
}

qsnumtype_t qswidenum_variant (qsmem_t * mem, qsptr_t n)
{
  qsnumtype_t variant = QSNUMTYPE_NAN;
  qswidenum_t * wn = qswidenum(mem, n, &variant);
  if (!wn) return QSNUMTYPE_NAN;
  return variant;
}

qswidenum_t * qswidenum_premake (qsmem_t * mem, qsnumtype_t variant, qsptr_t * out_ptr)
{
  qswidenum_t * retval = NULL;
  qsptr_t p = QSNIL;
  qsmemaddr_t addr = 0;
  qserror_t err = QSERROR_OK;
  switch (variant)
    {
    case QSNUMTYPE_LONG:
    case QSNUMTYPE_DOUBLE:
    case QSNUMTYPE_INT2:
    case QSNUMTYPE_FLOAT2:
    case QSNUMTYPE_FLOAT4:
    case QSNUMTYPE_FLOAT16CM:
      if (!ISOBJ26((p = qsobj_make(mem, 0, 1, &addr)))) return NULL;
      retval = (qswidenum_t*)qsobj(mem, p, NULL);
      retval->variant = variant;
      break;
    default:
      retval->variant = QSNUMTYPE_NAN;
      break;
    }
  if (out_ptr)
    *out_ptr = QSOBJ(addr);
  return retval;
}




qswidenum_t * qslong (qsmem_t * mem, qsptr_t l)
{
  qsnumtype_t variant = QSNUMTYPE_NAN;
  qswidenum_t * wn = qswidenum(mem, l, &variant);
  if (!wn) return NULL;
  if (variant != QSNUMTYPE_LONG) return NULL;
  return wn;
}

qserror_t qslong_fetch (qsmem_t * mem, qsptr_t l, long * out_long)
{
  qswidenum_t * wn = qslong(mem, l);
  if (!wn) return QSERROR_INVALID;
  if (out_long)
    *out_long = wn->payload.l;
  return QSERROR_OK;
}

long qslong_get (qsmem_t * mem, qsptr_t l)
{
  long retval = 0;
  qserror_t err = qslong_fetch(mem, l, &retval);
  if (err != QSERROR_OK)
    return 0;
  return retval;
}

qsptr_t qslong_make (qsmem_t * mem, long val)
{
  qswidenum_t * wn = NULL;
  qsptr_t retval = QSNIL;
  wn = qswidenum_premake(mem, QSNUMTYPE_LONG, &retval);
  if (wn)
    {
      wn->payload.l = val;
      return retval;
    }
  return QSERROR_NOMEM;
}

qsptr_t qslong_make2 (qsmem_t * mem, int32_t high, uint32_t low)
{
  return QSERROR_NOIMPL;
}

int qslong_crepr (qsmem_t * mem, qsptr_t l, char * buf, int buflen)
{
  int n = 0;
  return n;
}




qswidenum_t * qsdouble (qsmem_t * mem, qsptr_t d)
{
  qsnumtype_t variant = QSNUMTYPE_NAN;
  qswidenum_t * wn = qswidenum(mem, d, &variant);
  if (!wn) return NULL;
  if (variant != QSNUMTYPE_LONG) return NULL;
  return wn;
}

qserror_t qsdouble_fetch (qsmem_t * mem, qsptr_t d, double * out_double)
{
  qswidenum_t * wn = qsdouble(mem, d);
  if (!wn) return QSERROR_INVALID;
  if (out_double)
    *out_double = wn->payload.d;
  return QSERROR_OK;
}

double qsdouble_get (qsmem_t * mem, qsptr_t d)
{
  double retval = 0;
  qserror_t err = qsdouble_fetch(mem, d, &retval);
  if (err)
    return 0;
  return retval;
}

qsptr_t qsdouble_make (qsmem_t * mem, double val)
{
  qswidenum_t * wn = NULL;
  qsptr_t retval = QSNIL;
  wn = qswidenum_premake(mem, QSNUMTYPE_LONG, &retval);
  if (wn)
    {
      wn->payload.d = val;
      return retval;
    }
  return QSERROR_NOMEM;
}

int qsdouble_crepr (qsmem_t * mem, qsptr_t d, char * buf, int buflen)
{
  int n = 0;
  return n;
}




int qswidenum_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  qsnumtype_t variant = 0;
  qswidenum_t * wn = qswidenum(mem, n, &variant);
  switch (variant)
    {
    case QSNUMTYPE_LONG:
      n += qslong_crepr(mem, p, buf+n, buflen-n);
      break;
    case QSNUMTYPE_DOUBLE:
      n += qsdouble_crepr(mem, p, buf+n, buflen-n);
      break;
    default:
      n += snprintf(buf+n, buflen-n, "#<WIDENUM:%d:%08x>", variant, p);
      break;
    }
  return n;
}








/**********************/
/* Word-sized objects */
/**********************/


/* Iterator pointer, for iterating immlist and vectors-as-list. */
qsptr_t qsiter (qsmem_t * mem, qsptr_t it)
{
  if (ISITER28(it)) return it;
  return QSNIL;
}

qsptr_t qsiter_make (qsmem_t * meme, qsmemaddr_t addr)
{
  // TODO: bounds check.
  return QSITER(addr);
}

qsword qsiter_get (qsmem_t * mem, qsptr_t it)
{
  if (ISITER28(it))
    {
      return CITER28(it);
    }
  // TODO: exception.
  return 0;
}

int qsiter_on_pair (qsmem_t * mem, qsptr_t it, qsptr_t * out_pairptr)
{
  if (ISITER28(it))
    {
      qsword ofs = qsiter_get(mem, it);
      qsmemaddr_t addr = (ofs >> 2);
      qsptr_t pairptr = QSOBJ(addr);
      qspair_t * pair = qspair(mem, pairptr);
      if (pair)
	{
	  if (out_pairptr)
	    *out_pairptr = pairptr;
	  return 1;
	}
    }
  return 0;
}

qsptr_t qsiter_item (qsmem_t * mem, qsptr_t it)
{
  qsptr_t pairptr = QSNIL;
  if (qsiter_on_pair(mem, it, &pairptr))
    {
      /* iter is pointing to pair cell. */
      return qspair_ref_a(mem, pairptr);
    }
  else if (ISITER28(it))
    {
      qsword ofs = qsiter_get(mem, it);
      qsptr_t ref = 0;
      qserror_t err = qsheap_word(mem, ofs, &ref);
      if (err != QSERROR_OK)
	return QSNIL;
      if (ref == QSBOL)
	{
	  // nested list; nested iterator.
	  qsptr_t iter2 = QSITER( CITER28(it)+1 );
	  return iter2;
	}
      return ref;
    }
  // TODO: exception.
  return QSNIL;
}

qsptr_t qsiter_next (qsmem_t * mem, qsptr_t it)
{
  qsptr_t pairptr = QSNIL;
  if (qsiter_on_pair(mem, it, &pairptr))
    {
      /* iter is pointing to pair cell. */
      qsptr_t next = qspair_ref_d(mem, pairptr);
      if (ISNIL(next))
	return QSNIL;
      qsmemaddr_t next_addr = COBJ26(next);
      qsmemaddr_t iter_addr = next_addr << 2;
      return qsiter_make(mem, iter_addr);
    }
  else if (ISITER28(it))
    {
      qsptr_t peek = 0;
      qserror_t err = 0;
      int depth = 0;
      qsword startofs = qsiter_get(mem, it);
      err = qsheap_word(mem, startofs, &peek);
      if (err != QSERROR_OK) return QSNIL;
      if (peek == QSBOL) depth++; // goal: skip to end of nested list.

      qsword nextofs = startofs+1;
      err = qsheap_word(mem, nextofs, &peek);
      if (err != QSERROR_OK) return QSNIL;

      while (depth > 0)
	{
	  err = qsheap_word(mem, nextofs, &peek);
	  if (err != QSERROR_OK)
	    {
	      // invalid address, treat as end of iterator.
	      depth = 0;
	      peek = QSEOL;
	      break;
	    }
	  else if (peek == QSEOL)
	    {
	      depth--;
	      // might be end of nested list, keep moving.
	    }
	  else if (peek == QSBOL)
	    {
	      depth++;
	      // beginning of list, seek end of nested list.
	    }
	  else if (ISSYNC29(peek))
	    {
	      // impinging on next object.
	      depth = 0;
	      peek = QSEOL;
	      break;
	    }
	  nextofs++;
	}
      if (peek != QSEOL)  // did not end on end-of-list marker.
	return qsiter_make(mem, nextofs);
    }
  // invalid iterator, or iterator ended on end-of-list.
  return QSNIL;
}

qsptr_t qsiter_crepr (qsmem_t * mem, qsptr_t it, char * buf, int buflen)
{
  int n = 0;
  return n;
}




qsptr_t qsint (qsmem_t * mem, qsptr_t q)
{
  if (ISINT30(q)) return q;
  else if (ISFLOAT31(q))
    {
      int casted_int = (int)CFLOAT31(q);
      return QSINT(casted_int);
    }
  else
    {
      // TODO: wide numbers.
      // TODO: exception.
      return QSNIL;
    }
  return QSNIL;
}

int32_t qsint_get (qsmem_t * mem, qsptr_t q)
{
  if (ISINT30(q)) return CINT30(q);
  return 0;
}

qsptr_t qsint_make (qsmem_t * mem, int32_t val)
{
  if ((val < -0x400000000) || (val > 0x3fffffff))
    {
      return QSERROR_RANGE;
    }
  return QSINT(val);
}

int qsint_crepr (qsmem_t * mem, qsptr_t i, char * buf, int buflen)
{
  int n = 0;
  int val = CINT30(i);
  n = snprintf(buf, buflen, "%d", val);
  return n;
}




qsptr_t qsfloat (qsmem_t * mem, qsptr_t q)
{
  if (ISFLOAT31(q)) return q;
  if (ISINT30(q))
    {
      float casted_float = (float)CINT30(q);
      return QSFLOAT(casted_float);
    }
  else
    {
      // TODO: wide numbers.
      // TODO: exception.
      return QSNIL;
    }
  return QSNIL;
}

float qsfloat_get (qsmem_t * mem, qsptr_t q)
{
  if (ISFLOAT31(q)) return CFLOAT31(q);
  return NAN;
}

qsptr_t qsfloat_make (qsmem_t * mem, float val)
{
  return QSFLOAT(val);
}

int qsfloat_crepr (qsmem_t * mem, qsptr_t f, char * buf, int buflen)
{
  int n = 0;
  float val = 0;
  if (qsfloat(mem,f)) val = CFLOAT31(f);
  n = snprintf(buf, buflen, "%f", val);
  return n;
}




qsptr_t qschar (qsmem_t * mem, qsptr_t q)
{
  if (ISCHAR24(q)) return q;
  return QSNIL;
}

int qschar_get (qsmem_t * mem, qsptr_t q)
{
  if (ISCHAR24(q)) return CCHAR24(q);
  return 0;
}

qsptr_t qschar_make (qsmem_t * mem, int val)
{
  if ((val < 0) || (val >= (1 << 24)))
    {
      return QSERROR_RANGE;
    }
  return QSCHAR(val);
}

int qschar_crepr (qsmem_t * mem, qsptr_t c, char * buf, int buflen)
{
  int n = 0;
  int codepoint = 0;
  if (qschar(mem,c)) codepoint = CCHAR24(c);
  switch (codepoint)
    {
    case 7: /* bel */
      n += snprintf(buf+n, buflen-n, "#\\bel");
      break;
    case 8: /* bs */
      n += snprintf(buf+n, buflen-n, "#\\bs");
      break;
    case 9: /* tab */
      n += snprintf(buf+n, buflen-n, "#\\tab");
      break;
    case 10: /* lf */
      n += snprintf(buf+n, buflen-n, "#\\newline");
      break;
    case 13: /* cr */
      n += snprintf(buf+n, buflen-n, "#\\cr");
      break;
    case 27: /* esc */
      n += snprintf(buf+n, buflen-n, "#\\esc");
      break;
    case 32: /* space */
      n += snprintf(buf+n, buflen-n, "#\\space");
      break;
    default:
      if (codepoint <= 255)
	{
	  n += snprintf(buf+n, buflen-n, "#\\%c", codepoint);
	}
      else if (codepoint <= 0xffff)
	{
	  n += snprintf(buf+n, buflen-n, "#\\u%04x", codepoint);
	}
      else
	{
	  n += snprintf(buf+n, buflen-n, "#\\U%08x", codepoint);
	}
    }
  return n;
}




qsptr_t qserr (qsmem_t * mem, qsptr_t e)
{
  if (ISERROR16(e)) return e;
  return QSNIL;
}

int qserr_get (qsmem_t * mem, qsptr_t e)
{
  if (ISERROR16(e)) return CERROR16(e);
  return 0;
}

qsptr_t qserr_make (qsmem_t * mem, int errcode)
{
  return QSERROR(errcode);
}

int qserr_crepr (qsmem_t * mem, qsptr_t c, char * buf, int buflen)
{
  int n = 0;
  switch (c)
    {
    case QSERROR_OK:
      n += snprintf(buf+n, buflen-n, "#<ERROR:%s>", "OK");
      break;
    case QSERROR_INVALID:
      n += snprintf(buf+n, buflen-n, "#<ERROR:%s>", "INVALID");
      break;
    case QSERROR_NOMEM:
      n += snprintf(buf+n, buflen-n, "#<ERROR:%s>", "NOMEM");
      break;
    case QSERROR_NOIMPL:
      n += snprintf(buf+n, buflen-n, "#<ERROR:%s>", "NOIMPL");
      break;
    case QSERROR_RANGE:
      n += snprintf(buf+n, buflen-n, "#<ERROR:%s>", "RANGE");
      break;
    case QSERROR_TYPE:
      n += snprintf(buf+n, buflen-n, "#<ERROR:%s>", "TYPE");
      break;
    default:
      n += snprintf(buf+n, buflen-n, "#<ERROR:%08x>", c);
      break;
    }
  return n;
}




qsptr_t qsconst (qsmem_t * mem, qsptr_t n)
{
  if (ISCONST16(n)) return n;
  return QSNIL;
}

int qsconst_get (qsmem_t * mem, qsptr_t n)
{
  if (ISCONST16(n)) return CCONST16(n);
  return 0;
}

qsptr_t qsconst_make (qsmem_t * mem, int constcode)
{
  return QSCONST(constcode);
}

int qsconst_crepr (qsmem_t * mem, qsptr_t c, char * buf, int buflen)
{
  int n = 0;
  switch (c)
    {
    case QSNIL:
      n += snprintf(buf+n, buflen-n, "'()");
      break;
    case QSTRUE:
      n += snprintf(buf+n, buflen-n, "#t");
      break;
    case QSBOL:
      n += snprintf(buf+n, buflen-n, "#bol");
      break;
    case QSEOL:
      n += snprintf(buf+n, buflen-n, "#eol");
      break;
    case QSBLACKHOLE:
      break;
    default:
      n += snprintf(buf+n, buflen-n, "#<const:%08x>", c);
      break;
    }
  return n;
}




/*******************************/
/* Variable-substrate objects. */
/*******************************/

/* List:
   1. linked pair cells
   2. vector with BOL termination
 */
// expanded qsiter_*() to accommodate this object.

/* String:
   1. conschar24: pair cells of char24
   2. vecchar24: vector of char24 with BOL termination
   3. cstr8: bytevector (UTF-8)
 */


/*
   qsutf8 = using qsbytevector space as C string.
   Canonical string format (due to interchange with string.h functions).
*/
qsutf8_t * qsutf8 (qsmem_t * mem, qsptr_t s)
{
  qsobj_t * obj = qsobj_multioctetate(mem, s, NULL);
  if (!obj) return NULL;
  if (! ISNIL(qsobj_get(mem, s, 0))) return NULL;
  return (qsutf8_t*)obj;
}

static
qsword _qsutf8_hardlimit (qsmem_t * mem, qsptr_t s)
{
  qsword ncells = qsobj_ref_allocsize(mem, s);
  qsword retval = (ncells - 1) * (sizeof(qsheapcell_t) / sizeof(qsptr_t));
  return retval;
}

qsword qsutf8_length (qsmem_t * mem, qsptr_t s)
{
  qsword retval = 0;
  qsutf8_t * utf8str = qsutf8(mem, s);
  if (!utf8str) return retval;
  retval = strlen(utf8str->_d);
  return retval;
}

int qsutf8_ref (qsmem_t * mem, qsptr_t s, qsword k)
{
  qsword retval = 0;
  qsutf8_t * utf8str = qsutf8(mem, s);
  if (!utf8str) return -1;
  qsword max = _qsutf8_hardlimit(mem, s);
  if ((k < 0) || (k >= max))
    {
      return 0;
    }
  return utf8str->_d[k];
}

qsptr_t qsutf8_setq (qsmem_t * mem, qsptr_t s, qsword k, qsword val)
{
  qsutf8_t * utf8str = qsutf8(mem, s);
  if (!utf8str) return -1;
  qsword max = _qsutf8_hardlimit(mem, s);
  if ((k < 0) || (k >= max))
    {
      return 0;
    }
  utf8str->_d[k] = (uint8_t)(val & 0xff);
  return s;
}

qsptr_t qsutf8_make (qsmem_t * mem, qsword slen)
{
  qsptr_t retval = QSNIL;
  qsmemaddr_t addr = 0;
  qsword k = slen + 1;
  if (!ISOBJ26((retval = qsobj_make(mem, k, 1, &addr)))) return retval;

  qsutf8_t * utf8str = (qsutf8_t*)qsheap_ref(mem, addr);
  utf8str->variant = QSNIL;
  qsword i;
  for (i = 0; i < k; i++)
    {
      utf8str->_d[i] = (uint8_t)0;
    }
  return retval;
}

int qsutf8_crepr (qsmem_t * mem, qsptr_t s, char * buf, int buflen)
{
  return 0;
}

uint8_t * qsutf8_cptr (qsmem_t * mem, qsptr_t s)
{
  qsutf8_t * utf8str = qsutf8(mem, s);
  if (!utf8str) return NULL;
  return utf8str->_d;
}

qsptr_t qsutf8_inject (qsmem_t * mem, const char * cstr, qsword slen)
{
  qsptr_t retval;
  qsword k = slen;
  if (k < 1)
    {
      k = strlen(cstr) + 1;
    }
  retval = qsutf8_make(mem, k);
  if (! ISOBJ26(retval)) return retval;
  uint8_t * target = qsutf8_cptr(mem, retval);
  qsword i = 0;
  for (i = 0; i < k; i++)
    {
      target[i] = cstr[i];
    }
  target[i] = 0;
  i++;
  return retval;
}

int qsutf8_extract (qsmem_t * mem, qsptr_t s, char * cstr, qsword slen)
{
  qsword lim = _qsutf8_hardlimit(mem, s);
  const char * _d = qsutf8_cptr(mem, s);
  qsword i;
  for (i = 0; (i < lim) && (i+1 < slen) && (cstr[i] != 0); i++)
    {
      cstr[i] = _d[i];
    }
  cstr[i] = 0;
  i++;
  return i;
}




qsptr_t qsstr (qsmem_t * mem, qsptr_t s)
{
  if (qsutf8(mem,s))
    {
      return s;
    }
  else if (qspair(mem,s))
    {
      // naive check: first element is character.
      if (! ISCHAR24(qspair_ref_a(mem,s))) return QSNIL;
    }
  else if (qsvector(mem,s,NULL))
    {
      // naive check: first element is character.
      if (! ISCHAR24(qsvector_ref(mem,s,0))) return QSNIL;
    }
  else if (qsbytevec(mem,s,NULL))
    {
      // always counts as string.
    }
  return s;
}

static qsword qsstr_length_cons (qsmem_t * mem, qsptr_t s)
{
  qsword n = 0;
  qsptr_t curr = s;
  while (qspair(mem,curr))
    {
      n++;
      curr = qspair_ref_d(mem,curr);
    }
  return n;
}

static qsword qsstr_length_vec (qsmem_t * mem, qsptr_t s)
{
  return qsvector_length(mem, s);
}

static qsword qsstr_length_bytevec (qsmem_t * mem, qsptr_t s)
{
  return qsbytevec_length(mem, s);
}

qsword qsstr_length (qsmem_t * mem, qsptr_t s)
{
  qsword lim = 0;
  if (qsutf8(mem,s))
    {
      return qsutf8_length(mem, s);
    }
  else if (qspair(mem,s))
    {
      lim = qsstr_length_cons(mem,s);
      return qsstr_length_cons(mem,s);
    }
  else if (qsvector(mem, s, &lim))
    {
      // lim already assigned.
    }
  else if (qsbytevec(mem, s, &lim))
    {
      // lim already assigned.
    }
  return lim;
}

static qsptr_t qsstr_ref_cons (qsmem_t * mem, qsptr_t s, qsword nth)
{
  qsword ofs = 0;
  qsptr_t curr = s;
  while ((ofs < nth) && qspair(mem,curr))
    {
      ofs++;
    }
  if (!qspair(mem,curr))
    {
      return QSERROR_RANGE;
    }
  qsptr_t elt = qspair_ref_a(mem, curr);
  if (ISCHAR24(elt)) return elt;
  else return QSCHAR(0);
}

static qsptr_t qsstr_ref_vec (qsmem_t * mem, qsptr_t s, qsword nth, qsword hard_limit)
{
  qsword lim = 0;
  qsvector_t * vec = qsvector(mem, s, &lim);
  if ((nth >= hard_limit) || (nth < 0) || (nth >= lim))
    return QSERROR_RANGE;
  qsptr_t elt = qsvector_ref(mem, s, nth);
  if (ISCHAR24(elt)) return elt;
  else return QSCHAR(0);
}

static qsptr_t qsstr_ref_bytevec (qsmem_t * mem, qsptr_t s, qsword nth, qsword hard_limit)
{
  qsword lim = 0;
  qsbytevec_t * bv = qsbytevec(mem, s, &lim);
  if ((nth >= hard_limit) || (nth < 0) || (nth >= lim))
    return QSERROR_RANGE;
  qsword ch = qsbytevec_ref(mem, s, nth);
  return QSCHAR(ch);
}

qsptr_t qsstr_ref (qsmem_t * mem, qsptr_t s, qsword nth)
{
  qsword lim;
  if (qsutf8(mem, s))
    {
      return qsutf8_ref(mem, s, nth);
    }
  else if (qspair(mem, s))
    {
      return qsstr_ref_cons(mem, s, nth);
    }
  else if (qsvector(mem, s, &lim))
    {
      return qsstr_ref_vec(mem, s, nth, lim);
    }
  else if (qsbytevec(mem, s, &lim))
    {
      return qsstr_ref_bytevec(mem, s, nth, lim);
    }
  return QSCHAR(0);
}

qsptr_t qsstr_setq (qsmem_t * mem, qsptr_t s, qsword nth, qsword codepoint)
{
  qsword lim = 0;
  if (qsutf8(mem, s))
    {
      return qsutf8_setq(mem, s, nth, codepoint);
    }
  else if (qsvector(mem, s, &lim))
    {
      if (nth >= lim) return QSERROR_RANGE;
      qsvector_setq(mem, s, nth, QSCHAR(codepoint));
      return s;
    }
  else if (qsbytevec(mem, s, &lim))
    {
      int filtered = codepoint & 0xff;
      if (nth >= lim) return QSERROR_RANGE;
      qsbytevec_setq(mem, s, nth, filtered);
      return s;
    }
  /* cannot be modified in place. */
  return QSERROR_INVALID;
}

qsptr_t qsstr_make (qsmem_t * mem, qsword k, qsword codepoint_fill)
{
  qsptr_t retval = QSNIL;
  /* default is qsutf8 */
  retval = qsutf8_make(mem, k);
  return retval;
}

/* inject C string as utf-8 bytevector */
qsptr_t qsstr_inject (qsmem_t * mem, const char * cstr, qsword slen)
{
  return qsutf8_inject(mem, cstr, slen);
}

/* inject C wide-char string as utf-32 vector */
qsptr_t qsstr_inject_wchar (qsmem_t * mem, const wchar_t * ws, qsword wslen)
{
  qsptr_t retval = QSNIL;
  retval = qsvector_make(mem, wslen, QSCHAR(0));
  if (! ISOBJ26(retval)) return retval;
  qsword i = 0;
  for (i = 0; i < wslen; i++)
    {
      qsvector_setq(mem, retval, i, QSCHAR(ws[i]));
    }
  return retval;
}

/* copy out as C string; list/vector of char to char, or copy bytevector. */
qsword qsstr_extract (qsmem_t * mem, qsptr_t s, char * cstr, qsword slen)
{
  qsword retval = 0;
  qsword lim = 0;
  qsword idx = 0;
  if (qsutf8(mem, s))
    {
      return qsutf8_extract(mem, s, cstr, slen);
    }
  else if (qspair(mem, s))
    {
      qsptr_t curr = s;
      mbstate_t ps = { 0, };
      while (qspair(mem, curr) && (idx+1 < slen-MB_CUR_MAX))
	{
	  qsptr_t elt = qspair_ref_a(mem, curr);
	  wchar_t ch = ISCHAR24(elt) ? CCHAR24(elt) : 0;
	  int span = wcrtomb(cstr + idx, ch, &ps);
	  if (span > 0)
	    {
	      idx += span;
	    }
	  else
	    {
	      abort();
	    }

	  curr = qspair_ref_d(mem, curr);
	}
      cstr[idx] = 0;
      retval = idx;
    }
  else if (qsvector(mem, s, &lim))
    {
      qsword i;
      mbstate_t ps = { 0, };
      for (i = 0; (i+1 < slen) && (i < lim); i++)
	{
	  qsptr_t elt = qsvector_ref(mem, s, i);
	  wchar_t ch = ISCHAR24(elt) ? CCHAR24(elt) : 0;
	  int span = wcrtomb(cstr + idx, ch, &ps);
	  if (span > 0)
	    {
	      idx += span;
	    }
	  else
	    {
	      abort();
	    }
	}
      cstr[idx] = 0;
      retval = idx;
    }
  else if (qsbytevec(mem, s, &lim))
    {
      const char * bvstr = qsbytevec_cptr(mem, s, &lim);
      qsword i;
      for (i = 0; (i < lim) && (i+1 < slen); i++)
	{
	  cstr[i] = bvstr[i];
	}
      cstr[i] = 0;
      retval = i;
    }
  return retval;
}

/* copy out as C wide-string; list/vector of char to wchar, or bytevector to wchar */
qsword qsstr_extract_wchar (qsmem_t * mem, qsptr_t s, wchar_t * ws, qsword wlen)
{
  qsword retval = 0;
  qsword lim = 0;
  qsword idx = 0;
  if (qspair(mem, s))
    {
      qsptr_t curr = s;
      while (qspair(mem, curr) && (idx+1 < wlen))
	{
	  qsptr_t elt = qspair_ref_a(mem, curr);
	  if (! ISCHAR24(elt)) elt = QSCHAR(0);
	  ws[idx] = CCHAR24(elt);
	  idx++;
	  curr = qspair_ref_d(mem, curr);
	}
      ws[idx] = 0;
      retval = idx;
    }
  else if (qsvector(mem, s, &lim))
    {
      qsword i;
      for (i = 0; (i+1 < wlen) && (i < lim); i++)
	{
	  qsptr_t elt = qsvector_ref(mem, s, i);
	  if (! ISCHAR24(elt)) elt = QSCHAR(0);
	  ws[idx] = qsvector_ref(mem, s, idx);
	  idx++;
	}
      ws[idx] = 0;
      retval = idx;
    }
  else if (qsbytevec(mem, s, &lim))
    {
      const char * cstr = qsbytevec_cptr(mem, s, &lim);
      retval = mbstowcs(ws, cstr, wlen);
    }
  return retval;
}

int qsstr_cmp (qsmem_t * mem, qsptr_t a, qsptr_t b)
{
  if (a == b) return 0;
  qsword k = 0;
  qsword lim_a = qsstr_length(mem, a);
  qsword lim_b = qsstr_length(mem, b);
  qsword lim = (lim_a < lim_b) ? lim_a : lim_b;
  for (k = 0; k < lim; k++)
    {
      int ch_a = k < lim_a ? qsstr_ref(mem, a, k) : 0;
      int ch_b = k < lim_b ? qsstr_ref(mem, b, k) : 0;
      if (ch_a < ch_b) return -1;
      if (ch_a > ch_b) return 1;
      if (ch_a <= 0) return -1;
      if (ch_b <= 0) return 1;
      // else ch_a == ch_b  =>  continue
    }
  if (lim_a < lim_b) return 1;
  else if (lim_a > lim_b) return -1;
  return 0;
}




/* Symbols. */
/* qssym - interned symbols. */
qsptr_t qssym (qsmem_t * mem, qsptr_t y)
{
  if (ISSYM26(y)) return y;
  else return QSNIL;
}

qsword qssym_get (qsmem_t * mem, qsptr_t y)
{
  if (ISNIL(qssym(mem, y))) return 0;
  return CSYM26(y);
}

qsptr_t qssym_make (qsmem_t * mem, qsptr_t symbol_id)
{
  return QSSYM(symbol_id);
}

int qssym_crepr (qsmem_t * mem, qsptr_t y, char * buf, int buflen)
{
  int n = 0;
  return n;
}



/* qssymbol - tie name and id. */
qssymbol_t * qssymbol (qsmem_t * mem, qsptr_t yy)
{
  qsmemaddr_t out_addr = 0;
  qsobj_t * obj = qsobj(mem, yy, &out_addr);
  if (!obj) return NULL;
  qssymbol_t * symbol = (qssymbol_t*)obj;
//  if (symbol->indicator != yy) return NULL;
  if (!qsstr(mem, symbol->name)) return NULL;
  //if (!qssym(mem, symbol->id)) return NULL;
  if (CINT30(qstree_ref_right(mem, yy)) != COBJ26(yy)) return NULL;
  /* right field should be a symbol id that points back to object itself. */
  //if (out_addr != qssym_get(mem, symbol->id)) return NULL;
  return symbol;
}

qsptr_t qssymbol_ref_name (qsmem_t * mem, qsptr_t yy)
{
  qssymbol_t * symbol = qssymbol(mem, yy);
  if (!symbol) return QSNIL;
  return symbol->name;
}

qsptr_t qssymbol_ref_id (qsmem_t * mem, qsptr_t yy)
{
  qssymbol_t * symbol = qssymbol(mem, yy);
  if (!symbol) return QSNIL;
  //return symbol->id;
  return COBJ26(yy);
}

qsptr_t qssymbol_make (qsmem_t * mem, qsptr_t name)
{
  // TODO: copy name?
#if 0
  qsptr_t retval = qstree_make(mem, QSNIL, name, QSNIL);
  if (!ISOBJ26(retval)) return retval;
//  /* left field points back to object. */
//  qstree_setq_left(mem, retval, retval);
  /* right field ends up pointing to self. */
  qsmemaddr_t addr = COBJ26(retval);
  qsptr_t y = qssym_make(mem, addr);
  qstree_setq_right(mem, retval, y);
#else
  qsptr_t retval = qstree_make(mem, QSSYMBOL, name, QSNIL);
  if (!ISOBJ26(retval)) return retval;
  /* right field value to address value. */
  qstree_setq_right(mem, retval, QSINT(COBJ26(retval)));
#endif //0

  return retval;
}



/* symstore - composed of two structures:
   1. symtable - indexed by integer, resolves symbol_id to symbol_name.
   2. symtree - indexed by name, resolves symbol_name to symbol_id.
*/

qssymstore_t * qssymstore (qsmem_t * mem, qsptr_t o)
{
  qsobj_t * obj = qsobj(mem, o, NULL);
  if (!obj) return NULL;
  qssymstore_t * ystore = (qssymstore_t*)obj;
  if (ystore->tag != QSSYMSTORE) return NULL;
  return ystore;
}

qsptr_t qssymstore_make (qsmem_t * mem)
{
  qsptr_t retval = QSNIL;
  qsmemaddr_t addr = 0;
  if (!ISOBJ26((retval = qsobj_make(mem, 1, 0, &addr)))) return retval;

  qssymstore_t * symstore = (qssymstore_t*)qsobj(mem, retval, NULL);
  symstore->tag = QSSYMSTORE;
  symstore->table = qsibtree_make(mem);
  symstore->tree = qsrbtree_make(mem, QSNIL, QSNIL);
  return retval;
}

qsptr_t qssymstore_ref_table (qsmem_t * mem, qsptr_t o)
{
  qssymstore_t * ystore = qssymstore(mem, o);
  if (! ystore) return QSNIL;
  return ystore->table;
}
qsptr_t qssymstore_ref_tree (qsmem_t * mem, qsptr_t o)
{
  qssymstore_t * ystore = qssymstore(mem, o);
  if (! ystore) return QSNIL;
  return ystore->tree;
}

qsptr_t qssymstore_setq_table (qsmem_t * mem, qsptr_t o, qsptr_t val)
{
  qssymstore_t * ystore = qssymstore(mem, o);
  if (! ystore) return QSNIL;
  return ystore->table;
}
qsptr_t qssymstore_setq_tree (qsmem_t * mem, qsptr_t o, qsptr_t val)
{
  qssymstore_t * ystore = qssymstore(mem, o);
  if (! ystore) return QSNIL;
  return ystore->tree;
}

/* symbol objects are not interned by default. */
qsptr_t qssymstore_intern (qsmem_t * mem, qsptr_t o, qsptr_t symbol_object)
{
  /* insert linearly into table. */
#if 0
  qsptr_t ibtree = qssymstore_ref_table(mem, o);
  qsword symid = qsibtree_ref_filled(mem, ibtree);
  ibtree = qsibtree_setq(mem, ibtree, symid, symbol_object);
  qssymstore_setq_table(mem, o, ibtree);
#endif //0
  /* insert lookup into tree. */
  qsptr_t rbtree = qssymstore_ref_tree(mem, o);
  rbtree = qsrbtree_insert(mem, rbtree, symbol_object);
  qssymstore_setq_tree(mem, o, rbtree);
  
  return o;
}

/* Obtain symbol object by id:int. */
qsptr_t qssymstore_ref (qsmem_t * mem, qsptr_t o, qsword sym_id)
{
  qsptr_t ibtree = qssymstore_ref_table(mem, o);
  qsptr_t retval = qsibtree_ref(mem, ibtree, sym_id);
  return retval;
}

/* Obtain symbol object by name:str. */
qsptr_t qssymstore_assoc (qsmem_t * mem, qsptr_t o, qsptr_t key)
{
  qsptr_t rbtree = qssymstore_ref_tree(mem, o);
  qsptr_t retval = qsrbtree_assoc(mem, rbtree, key);
  return retval;
}

int qssymstore_crepr (qsmem_t * mem, qsptr_t o)
{
  int n = 0;
  return n;
}




int qsobj_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  if (qspair(mem, p))
    {
      n += qspair_crepr(mem, p, buf+n, buflen-n);
    }
  else if (qstree(mem, p))
    {
      n += qstree_crepr(mem, p, buf+n, buflen-n);
    }
  else if (qsvector(mem, p, NULL))
    {
      n += qsvector_crepr(mem, p, buf+n, buflen-n);
    }
  else if (qswidenum(mem, p, NULL))
    {
      n += qswidenum_crepr(mem, p, buf+n, buflen-n);
    }
  else if (qsbytevec(mem, p, NULL))
    {
      n += qsbytevec_crepr(mem, p, buf+n, buflen-n);
    }
  return n;
}

int qsunknown_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  n += snprintf(buf+n, buflen-n, "#<UNKNWON:%08x>", p);
  return n;
}

int qsptr_crepr (qsmem_t * mem, qsptr_t p, char * buf, int buflen)
{
  int n = 0;
  if (ISFLOAT31(p))
    {
      n += qsfloat_crepr(mem, p, buf+n, buflen-n);
    }
  else if (ISINT30(p))
    {
      n += qsint_crepr(mem, p, buf+n, buflen-n);
    }
  else if (ISITER28(p))
    {
      n += qsiter_crepr(mem, p, buf+n, buflen-n);
    }
  else if (ISOBJ26(p))
    {
      n += qsobj_crepr(mem, p, buf+n, buflen-n);
    }
  else if (ISCHAR24(p))
    {
      n += qschar_crepr(mem, p, buf+n, buflen-n);
    }
  else if (ISERROR16(p))
    {
      n += qserr_crepr(mem, p, buf+n, buflen-n);
    }
  else if (ISCONST16(p))
    {
      n += qsconst_crepr(mem, p, buf+n, buflen-n);
    }
  else
    {
      n += qsunknown_crepr(mem, p, buf+n, buflen-n);
    }
  return n;
}
