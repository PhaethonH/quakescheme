#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "qssexpr_private.h"


#define PARSER_VARIANT_ID 1


/*
Basic S-Expression reader.

* NULL
* list
* atom:
  * symbol (from identifier)
  * string (delimiters ('"') stripped)
  * number (float31, int30, long, double, nan, inf)
  * other (typically '#...")
*/


enum reader_op_e {
    /* Reader states. Order is significant. */
    READER_INIT = 0,
    READER_ATOM,
    READER_STRING,
    READER_STRING_ESCAPE,
    READER_STRING_END,
    READER_END,

    READER_MAX,

    PARSER_DISCARD,
    PARSER_CONSUME,
    PARSER_FORCE_END_STRING,
    PARSER_APPEND_ATOM,
    PARSER_BEGIN_LIST,
    PARSER_END_LIST,
    PARSER_HALT,
};

/* table of names for debugging purposes */
static
const char * reader_op_str[] = {
    /* Order matters, match enum reader_op_e */
    "READER_INIT",
    "READER_ATOM",
    "READER_STRING",
    "READER_STRING_ESCAPE",
    "READER_STRING_END",
    "READER_END",

    "READER_MAX",

    "PARSER_DISCARD",
    "PARSER_CONSUME",
    "PARSER_FORCE_END_STRING",
    "PARSER_APPEND_ATOM",
    "PARSER_BEGIN_LIST",
    "PARSER_END_LIST",
    "PARSER_HALT",
};

struct matchrule_s {
    parser_predicate_f predicate;
    enum reader_op_e next_state;
    enum reader_op_e output;
};

static
struct matchrule_s matchrule0_INIT[] = {
      { is_end, READER_END, PARSER_DISCARD },
      { is_whitespace, READER_INIT, PARSER_DISCARD },
      { is_op, READER_INIT, PARSER_BEGIN_LIST },
      { is_cl, READER_INIT, PARSER_END_LIST },
      { is_string_delimit, READER_STRING, PARSER_CONSUME },
      { is_any, READER_ATOM, PARSER_CONSUME },
      { NULL, 0, 0 },
};

static
struct matchrule_s matchrule0_ATOM[] = {
      { is_end, READER_END, PARSER_APPEND_ATOM },
      { is_whitespace, READER_INIT, PARSER_APPEND_ATOM },
      { is_op, READER_INIT, PARSER_APPEND_ATOM },
      { is_cl, READER_INIT, PARSER_APPEND_ATOM },
      { is_string_delimit, READER_STRING, PARSER_APPEND_ATOM },
      { is_any, READER_ATOM, PARSER_CONSUME },
      { NULL, 0, 0 },
};

static
struct matchrule_s matchrule0_STRING[] = {
      { is_end, READER_END, PARSER_FORCE_END_STRING },
      { is_string_escape, READER_STRING_ESCAPE, PARSER_CONSUME },
      { is_string_delimit, READER_STRING_END, PARSER_CONSUME },
      { is_any, READER_STRING, PARSER_CONSUME },
      { NULL, 0, 0 },
};

static
struct matchrule_s matchrule0_STRING_ESCAPE[] = {
      { is_end, READER_END, PARSER_FORCE_END_STRING },
      { is_any, READER_STRING, PARSER_CONSUME },
      { NULL, 0, 0 },
};

static
struct matchrule_s matchrule0_STRING_END[] = {
      { is_end, READER_END, PARSER_APPEND_ATOM },
      { is_any, READER_INIT, PARSER_APPEND_ATOM },
      { NULL, 0, 0 },
};

static
struct matchrule_s matchrule0_END[] = {
      { is_any, READER_END, PARSER_HALT },
      { NULL, 0, 0 },
};

static
struct matchrule_s *matchruleset0[READER_MAX] = {
    /* Order matters; match order in enum reader_op_e */
    matchrule0_INIT,
    matchrule0_ATOM,
    matchrule0_STRING,
    matchrule0_STRING_ESCAPE,
    matchrule0_STRING_END,
    matchrule0_END,
};


/* convert reversed list-of-character to an atom. */
static
qsptr_t sexpr_revlist_to_atom (qsmachine_t * mach, qsptr_t revlist)
{
  qsword lislen = qslist_length(mach, revlist);
  qsptr_t lexeme = qsutf8_make(mach, lislen, 0);
  qsutf8_hold(mach, lexeme); /* increment reference count. */
  for (int i = 0; i < lislen; i++)
    {
      qsptr_t node = qslist_ref(mach, revlist, lislen-i-1);
      int ch = CCHAR24(node);
      qsutf8_setq(mach, lexeme, i, ch);
    }
  qsptr_t retval = to_atom(mach, lexeme);
  qssexpr_log("atom [%s] is %08x\n", qsutf8_get(mach, lexeme, NULL), retval);
  qsutf8_release(mach, lexeme); /* decrement reference count. */
  return retval;
}


/*
   Feed S-Expression parser one character at a time.

   Returns completion state of parser,
     0 = no object ready
     1 = completed objected stored in *out
         if out==NULL, object can still be retrieved from private field ->root
*/
static
int sxparse_feed (qssxparse_t * parser, int ch, qsptr_t * out)
{
  enum reader_op_e nextstate = READER_END;
  enum reader_op_e output = 0;
  qsptr_t atomval = QSNIL;
  int pending = 1;
  qsmachine_t * mach = parser->mach;

  while (pending)
    {
      if (parser->prevch)
	{
	  /* Resumed with a pushed character. */
	  parser->ch = ch;
	  ch = parser->prevch;
	  parser->prevch = 0;
	}

      const struct matchrule_s *matchrules = matchruleset0[parser->state];
      const struct matchrule_s *entry = NULL;
      nextstate = READER_END;
      output = PARSER_DISCARD;
      /* find rule to match. */
      for (entry = matchrules; entry->predicate; entry++)
	{
	  if (entry->predicate(ch))
	    {
	      output = entry->output;
	      nextstate = entry->next_state;
	      break;
	    }
	}

      qssexpr_log("(ch='%c', state=%s, next=%s, output=%s)\n", ch, reader_op_str[parser->state], reader_op_str[nextstate], reader_op_str[output]);
      qssexpr_log(" (root=%08x, parent=%08x, prevnode=%08x, nextnode=%08x\n",
	     parser->root, parser->parent, parser->prevnode, parser->nextnode);

      switch (output)
	{
	case PARSER_DISCARD:
	  break;
	case PARSER_CONSUME:
	  /* concatenate current character onto pending atom. */
	  parser->lexeme = qspair_make(mach, QSCHAR(ch), parser->lexeme);
	  break;
	case PARSER_FORCE_END_STRING:
	  /* append string delimiter ("), then append as atom. */
	  parser->lexeme = qspair_make(mach, QSCHAR('"'), parser->lexeme);
	  /* fallthrough */
	case PARSER_APPEND_ATOM:
	  /* atom construction complete, append to result. */
	  atomval = sexpr_revlist_to_atom(mach, parser->lexeme);

	  if (parser->root == QSBLACKHOLE)
	    { /* case 4: toplevel atom. */
	      qssexpr_log("atom4");
	      parser->root = atomval;
	      parser->complete = 1;
	    }
	  else if (qspair_p(mach, parser->root) || ISNIL(parser->root))
	    { /* else, add to list. */
	      if (ISNIL(parser->prevnode))
		{
		  /* case 2: create (and link) list node, cdr=parent. */
		  qssexpr_log("atom2");
		  parser->nextnode = qspair_make(mach, atomval, parser->parent);
		  qspair_setq_head(mach, parser->parent, parser->nextnode);
		  if (ISNIL(parser->root))
		    {
		      /* case 3: start nesting of lists. */
		      qssexpr_log("atom3");
		      parser->root = parser->nextnode;
		    }
		}
	      else
		{
		  /* case 1: middle of a list; preserve cdr=parent. */
		  qssexpr_log("atom1");
		  qsptr_t temp = qspair_ref_tail(mach, parser->prevnode);
		  parser->nextnode = qspair_make(mach, atomval, temp);
		  qspair_setq_tail(mach, parser->prevnode, parser->nextnode);
		}
	      parser->prevnode = parser->nextnode;
	    }

	  parser->lexeme = QSNIL;
	  parser->prevch = ch; /* push one character. */
	  break;
	case PARSER_BEGIN_LIST:
	  /* start of a new list; on atom read, modify nextnode->al. */

	  if (ISNIL(parser->prevnode))
	    { /* start of list-of-list. */
	      if (parser->root == QSBLACKHOLE)
		{ /* case 1: toplevel list; changes retval on atom add. */
		  qssexpr_log("list1");
		  parser->root = QSNIL;
		  parser->parent = QSNIL;
		}
	      else if (ISNIL(parser->root))
		{ /* case 2: start list nesting; atom-add=>setcar(parent,) */
		  qssexpr_log("list2");
		  parser->root = parser->nextnode = qspair_make(mach, QSNIL, QSBLACKHOLE);
		  parser->parent = parser->nextnode;
		}
	      else
		{ /* case 3: new list is nested. */
		  qssexpr_log("list3");
		  parser->nextnode = qspair_make(mach, QSNIL, parser->parent);
		  qspair_setq_head(mach, parser->parent, parser->nextnode);
		  parser->parent = parser->nextnode;
		}
	    }
	  else
	    {
	      /* case 4: list nested in middle of list. */
	      qssexpr_log("list4");
	      qsptr_t temp = qspair_ref_tail(mach, parser->prevnode);
	      parser->nextnode = qspair_make(mach, QSNIL, temp);
	      qspair_setq_tail(mach, parser->prevnode, parser->nextnode);
	      parser->parent = parser->nextnode;
	    }

	  parser->prevnode = QSNIL;
	  break;
	case PARSER_END_LIST:
	  /* Finish list building, resume building on parent. */
	  if (!ISNIL(parser->prevnode))
	    { /* Salient list; resume parent building by chasing cdr. */
	      parser->nextnode = qspair_ref_tail(mach, parser->prevnode);
	      qspair_setq_tail(mach, parser->prevnode, QSNIL);
	    }
	  else
	    { /* No list node built, skip back to parent. */
	      parser->nextnode = parser->parent;
	    }
	  parser->prevnode = parser->nextnode;
	  if ((parser->prevnode == QSBLACKHOLE) || ISNIL(parser->prevnode))
	    parser->complete = 1; /* error or end of toplevel list. */
	  break;
	case PARSER_HALT:
	  parser->complete = 1;
	  parser->prevch = ch;
	  break;
	default:
	  break;
	}

      parser->state = nextstate;

      /* next char */
      if (parser->prevch)
	{ /* pull a pushed character. */
	  parser->ch = ch = parser->prevch;
	  parser->prevch = 0;
	  pending = 1;
	}
      else
	{ /* get next character. */
	  parser->ch = ch;
	  pending = 0;
	}
    }

  if (parser->complete)
    {
      while (qspair_p(mach, parser->prevnode))
	{ /* unwind: list-ends are cdr=parent, change to null and chase */
	  parser->nextnode = qspair_ref_tail(mach, parser->prevnode);
	  qspair_setq_tail(mach, parser->prevnode, QSNIL);
	  parser->prevnode = parser->nextnode;
	}
      if (out)
	*out = parser->root;
    }

  return parser->complete;
}


static
qssxparse_t * sxparse_init (qssxparse_t * parser, qsmachine_t * mach)
{
  return parser;
}

static
qssxparse_t * sxparse_destroy (qssxparse_t * parser)
{
  return parser;
}



#define FEED qssxparse_v##PARSER_VARIANT_ID##_feed
//int (*FEED)(qssxparse_t *, int, qsptr_t *) = sxparse_feed;
int qssxparse_v1_feed (qssxparse_t * parser, int ch, qsptr_t * out)
{
  return sxparse_feed(parser, ch, out);
}

struct qssxparser_ops_s sxparser_ops = {
    sxparse_init,
    sxparse_destroy,
    sxparse_feed,
};

struct qssxparser_ops_s * qssxparser_ops_v1 = &sxparser_ops;
