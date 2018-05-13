#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "qssexpr.h"


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


#include <stdarg.h>
static
int qssexpr_logv (const char * fmt, va_list vp)
{
  int retval;
  if (0)
    {
     retval = vprintf(fmt, vp);
     puts("");
    }
  return retval;
}
static
int qssexpr_log (const char * fmt, ...)
{
  va_list vp;
  va_start(vp, fmt);
  return qssexpr_logv(fmt, vp);
}



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
qsptr_t qssexpr_revlist_to_atom (qsheap_t * mem, qsptr_t revlist)
{
  qsword lislen = qslist_length(mem, revlist);
  qsptr_t lexeme = qsutf8_make(mem, lislen);
  char * payload = qsobj_ref_data(mem, lexeme, NULL);
  for (int i = 0; i < lislen; i++)
    {
      qsptr_t node = qslist_ref(mem, revlist, lislen-i-1);
      int ch = CCHAR24(node);
      payload[i] = ch;
    }
  size_t slen;
  qsptr_t retval = qsatom_parse_cstr(mem, payload, lislen);
  qssexpr_log("atom [%s] is %08x\n", payload, retval);
  return retval;
}


/*
   Feed S-Expression parser one character at a time.

   Returns completion state of parser,
     0 = no object ready
     1 = completed objected stored in *out
         if out==NULL, object can still be retrieved from private field ->root
*/
int qssxparse_v1_feed (qsheap_t * mem, struct qssxparse_s * parser, int ch, qsptr_t * out)
{
  enum reader_op_e nextstate = READER_END;
  enum reader_op_e output = 0;
  qsptr_t atomval = QSNIL;
  int pending = 1;

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
	  parser->lexeme = qspair_make(mem, QSCHAR(ch), parser->lexeme);
	  break;
	case PARSER_FORCE_END_STRING:
	  /* append string delimiter ("), then append as atom. */
	  parser->lexeme = qspair_make(mem, QSCHAR('"'), parser->lexeme);
	  /* fallthrough */
	case PARSER_APPEND_ATOM:
	  /* atom construction complete, append to result. */
	  atomval = qssexpr_revlist_to_atom(mem, parser->lexeme);

	  if (parser->root == QSBLACKHOLE)
	    { /* case 4: toplevel atom. */
	      qssexpr_log("atom4");
	      parser->root = atomval;
	      parser->complete = 1;
	    }
	  else if (qspair_p(mem, parser->root) || ISNIL(parser->root))
	    { /* else, add to list. */
	      if (ISNIL(parser->prevnode))
		{
		  /* case 2: create (and link) list node, cdr=parent. */
		  qssexpr_log("atom2");
		  parser->nextnode = qspair_make(mem, atomval, parser->parent);
		  qspair_setq_a(mem, parser->parent, parser->nextnode);
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
		  qsptr_t temp = qspair_ref_d(mem, parser->prevnode);
		  parser->nextnode = qspair_make(mem, atomval, temp);
		  qspair_setq_d(mem, parser->prevnode, parser->nextnode);
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
		  parser->root = parser->nextnode = qspair_make(mem, QSNIL, QSBLACKHOLE);
		  parser->parent = parser->nextnode;
		}
	      else
		{ /* case 3: new list is nested. */
		  qssexpr_log("list3");
		  parser->nextnode = qspair_make(mem, QSNIL, parser->parent);
		  qspair_setq_a(mem, parser->parent, parser->nextnode);
		  parser->parent = parser->nextnode;
		}
	    }
	  else
	    {
	      /* case 4: list nested in middle of list. */
	      qssexpr_log("list4");
	      qsptr_t temp = qspair_ref_d(mem, parser->prevnode);
	      parser->nextnode = qspair_make(mem, QSNIL, temp);
	      qspair_setq_d(mem, parser->prevnode, parser->nextnode);
	      parser->parent = parser->nextnode;
	    }

	  parser->prevnode = QSNIL;
	  break;
	case PARSER_END_LIST:
	  /* Finish list building, resume building on parent. */
	  if (!ISNIL(parser->prevnode))
	    { /* Salient list; resume parent building by chasing cdr. */
	      parser->nextnode = qspair_ref_d(mem, parser->prevnode);
	      qspair_setq_d(mem, parser->prevnode, QSNIL);
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
      while (qspair_p(mem, parser->prevnode))
	{ /* unwind: list-ends are cdr=parent, change to null and chase */
	  parser->nextnode = qspair_ref_d(mem, parser->prevnode);
	  qspair_setq_d(mem, parser->prevnode, QSNIL);
	  parser->prevnode = parser->nextnode;
	}
      if (out)
	*out = parser->root;
    }

  return parser->complete;
}

