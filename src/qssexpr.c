#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "qssexpr.h"


/*
Simplistic S-Expression reader.

* NULL
* list
* atom:
  * identifier
  * string ('"' preserved)
  * other (typically '#...")

*/



int is_end (int ch) { return ch == 0; }
int is_whitespace (int ch) { return isspace(ch); }
int is_op (int ch) { return (ch == '('); }
int is_cl (int ch) { return (ch == ')'); }
int is_string_delimit (int ch) { return (ch == '"'); }
int is_string_escape (int ch) { return (ch == '\\'); }
int is_any (int ch) { return 1; }

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
    int (*predicate)(int);
    enum reader_op_e next_state;
    enum reader_op_e output;
};

struct matchrule_s matchrule0_INIT[] = {
      { is_end, READER_END, PARSER_DISCARD },
      { is_whitespace, READER_INIT, PARSER_DISCARD },
      { is_op, READER_INIT, PARSER_BEGIN_LIST },
      { is_cl, READER_INIT, PARSER_END_LIST },
      { is_string_delimit, READER_STRING, PARSER_CONSUME },
      { is_any, READER_ATOM, PARSER_CONSUME },
      { NULL, 0, 0 },
};

struct matchrule_s matchrule0_ATOM[] = {
      { is_end, READER_END, PARSER_APPEND_ATOM },
      { is_whitespace, READER_INIT, PARSER_APPEND_ATOM },
      { is_op, READER_INIT, PARSER_APPEND_ATOM },
      { is_cl, READER_INIT, PARSER_APPEND_ATOM },
      { is_string_delimit, READER_STRING, PARSER_APPEND_ATOM },
      { is_any, READER_ATOM, PARSER_CONSUME },
      { NULL, 0, 0 },
};

struct matchrule_s matchrule0_STRING[] = {
      { is_end, READER_END, PARSER_FORCE_END_STRING },
      { is_string_escape, READER_STRING_ESCAPE, PARSER_CONSUME },
      { is_string_delimit, READER_STRING_END, PARSER_CONSUME },
      { is_any, READER_STRING, PARSER_CONSUME },
      { NULL, 0, 0 },
};

struct matchrule_s matchrule0_STRING_ESCAPE[] = {
      { is_end, READER_END, PARSER_FORCE_END_STRING },
      { is_any, READER_STRING, PARSER_CONSUME },
      { NULL, 0, 0 },
};

struct matchrule_s matchrule0_STRING_END[] = {
      { is_end, READER_END, PARSER_APPEND_ATOM },
      { is_any, READER_INIT, PARSER_APPEND_ATOM },
      { NULL, 0, 0 },
};

struct matchrule_s matchrule0_END[] = {
      { is_any, READER_END, PARSER_HALT },
      { NULL, 0, 0 },
};

struct matchrule_s *matchruleset0[READER_MAX] = {
    /* Order matters; match order in enum reader_op_e */
    matchrule0_INIT,
    matchrule0_ATOM,
    matchrule0_STRING,
    matchrule0_STRING_ESCAPE,
    matchrule0_STRING_END,
    matchrule0_END,
};


/* convert reversed list-of-character to qsutf8 (C-style string) */
static
qsptr_t qssexpr_revlist_to_qsutf8 (qsheap_t * mem, qsptr_t revlist)
{
  qsword lislen = qslist_length(mem, revlist);
  qsptr_t name = qsutf8_make(mem, lislen);
  char * payload = qsobj_ref_data(mem, name, NULL);
  for (int i = 0; i < lislen; i++)
    {
      qsptr_t node = qslist_ref(mem, revlist, lislen-i-1);
      int ch = CCHAR24(node);
      payload[i] = ch;
    }
  return qssymbol_make(mem, name);
}

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
  printf("atom [%s] is %08x\n", payload, retval);
  return retval;
}

static
qsptr_t qssexpr_parse0_cstr (qsheap_t * mem, const char * srcstr, const char ** endptr)
{
  qsptr_t retval = QSBLACKHOLE;
  qsptr_t parent = QSNIL, prevnode = QSNIL, nextnode = QSNIL;
  int ch = 0;
  int prevch = 0;
  int scan = 0;
  int halt = 0;
  enum reader_op_e state = READER_INIT, nextstate = READER_END;
  enum reader_op_e output = 0;
  qsptr_t lexeme = QSNIL;
  qsptr_t atomval = QSNIL;

  ch = srcstr[scan];
  while (!halt)
    {
      const struct matchrule_s *matchrules = matchruleset0[state];
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

      switch (output)
	{
	case PARSER_DISCARD:
	  break;
	case PARSER_CONSUME:
	  /* concatenate current character onto pending atom. */
	  lexeme = qspair_make(mem, QSCHAR(ch), lexeme);
	  break;
	case PARSER_FORCE_END_STRING:
	  /* append string delimiter ("), then append as atom. */
	  lexeme = qspair_make(mem, QSCHAR('"'), lexeme);
	  /* fallthrough */
	case PARSER_APPEND_ATOM:
	  /* atom construction complete, append to result. */
	  //atomval = qssexpr_revlist_to_qsutf8(mem, lexeme);
	  atomval = qssexpr_revlist_to_atom(mem, lexeme);

	  if (retval == QSBLACKHOLE)
	    { /* case 4: toplevel atom. */
	      retval = atomval;
	      halt = 1;
	    }
	  else if (qspair_p(mem, retval) || ISNIL(retval))
	    { /* else, add to list. */
	      if (ISNIL(prevnode))
		{
		  /* case 2: create (and link) list node, cdr=parent. */
		  nextnode = qspair_make(mem, atomval, parent);
		  qspair_setq_a(mem, parent, nextnode);
		  if (ISNIL(retval))
		    {
		      /* case 2b: start nesting of lists. */
		      retval = nextnode;
		    }
		}
	      else
		{
		  /* case 1: middle of a list; preserve cdr=parent. */
		  qsptr_t temp = qspair_ref_d(mem, prevnode);
		  nextnode = qspair_make(mem, atomval, temp);
		  qspair_setq_d(mem, prevnode, nextnode);
		}
	      prevnode = nextnode;
	    }

	  lexeme = QSNIL;
	  prevch = ch; /* push one character. */
	  break;
	case PARSER_BEGIN_LIST:
	  /* start of a new list; on atom read, modify nextnode->al. */

	  if (ISNIL(prevnode))
	    { /* start of list-of-list. */
	      if (retval == QSBLACKHOLE)
		{ /* case 1: toplevel list; changes retval on atom add. */
		  retval = QSNIL;
		  parent = QSNIL;
		}
	      else if (ISNIL(retval))
		{ /* case 2: start list nesting; atom-add=>setcar(parent,) */
		  retval = nextnode = qspair_make(mem, QSNIL, QSBLACKHOLE);
		  parent = nextnode;
		}
	      else
		{ /* case 3: new list is nested. */
		  parent = qspair_make(mem, QSNIL, parent);
		}
	    }
	  else
	    {
	      /* case 4: list nested in middle of list. */
	      qsptr_t temp = qspair_ref_d(mem, prevnode);
	      nextnode = qspair_make(mem, QSNIL, temp);
	      qspair_setq_d(mem, prevnode, nextnode);
	      parent = nextnode;
	    }

	  prevnode = QSNIL;
	  break;
	case PARSER_END_LIST:
	  /* Finish list building, resume building on parent. */
	  if (!ISNIL(prevnode))
	    { /* Salient list; resume parent building by chasing cdr. */
	      nextnode = qspair_ref_d(mem, prevnode);
	      qspair_setq_d(mem, prevnode, QSNIL);
	    }
	  else
	    { /* No list node built, skip back to parent. */
	      nextnode = parent;
	    }
	  prevnode = nextnode;
	  if ((prevnode == QSBLACKHOLE) || ISNIL(prevnode))
	    halt = 1;  /* error or end of toplevel list. */
	  break;
	case PARSER_HALT:
	  halt = 1;
	  prevch = ch;
	  break;
	}

      state = nextstate;

      if (prevch)
	{ /* pull a pushed character. */
	  ch = prevch;
	  prevch = 0;
	}
      else if (srcstr[scan])
	{ /* get next character. */
	  scan++;
	  ch = srcstr[scan];
	}
      else
	{ /* end of stream, yield \0 */
	  ch = 0;
	}
    }

  while (qspair_p(mem, prevnode))
    { /* unwind: list-ends are cdr=parent, change to null and chase */
      nextnode = qspair_ref_d(mem, prevnode);
      qspair_setq_d(mem, prevnode, QSNIL);
      prevnode = nextnode;
    }

  if (endptr)
    {
      *endptr = srcstr+scan;
    }

  return retval;
}

qsptr_t qssexpr_parse_cstr (qsheap_t * mem, int version, const char * srcstr, const char ** endptr)
{
  switch (version)
    {
    case 0:
      return qssexpr_parse0_cstr(mem, srcstr, endptr);
    }
}

