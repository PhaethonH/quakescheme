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

struct matchrule_s matchrule_INIT[] = {
      { is_end, READER_END, PARSER_DISCARD },
      { is_whitespace, READER_INIT, PARSER_DISCARD },
      { is_op, READER_INIT, PARSER_BEGIN_LIST },
      { is_cl, READER_INIT, PARSER_END_LIST },
      { is_string_delimit, READER_STRING, PARSER_CONSUME },
      { is_any, READER_ATOM, PARSER_CONSUME },
      { NULL, 0, 0 },
};

struct matchrule_s matchrule_ATOM[] = {
      { is_end, READER_END, PARSER_APPEND_ATOM },
      { is_whitespace, READER_INIT, PARSER_APPEND_ATOM },
      { is_op, READER_INIT, PARSER_APPEND_ATOM },
      { is_cl, READER_INIT, PARSER_APPEND_ATOM },
      { is_string_delimit, READER_STRING, PARSER_APPEND_ATOM },
      { is_any, READER_ATOM, PARSER_CONSUME },
      { NULL, 0, 0 },
};

struct matchrule_s matchrule_STRING[] = {
      { is_end, READER_END, PARSER_FORCE_END_STRING },
      { is_string_escape, READER_STRING_ESCAPE, PARSER_CONSUME },
      { is_string_delimit, READER_STRING_END, PARSER_CONSUME },
      { is_any, READER_STRING, PARSER_CONSUME },
      { NULL, 0, 0 },
};

struct matchrule_s matchrule_STRING_ESCAPE[] = {
      { is_end, READER_END, PARSER_FORCE_END_STRING },
      { is_any, READER_STRING, PARSER_CONSUME },
      { NULL, 0, 0 },
};

struct matchrule_s matchrule_STRING_END[] = {
      { is_end, READER_END, PARSER_APPEND_ATOM },
      { is_any, READER_INIT, PARSER_APPEND_ATOM },
      { NULL, 0, 0 },
};

struct matchrule_s matchrule_END[] = {
      { is_any, READER_END, PARSER_HALT },
      { NULL, 0, 0 },
};

struct matchrule_s *matchruleset[READER_MAX] = {
    matchrule_INIT,
    matchrule_ATOM,
    matchrule_STRING,
    matchrule_STRING_ESCAPE,
    matchrule_STRING_END,
    matchrule_END,
};


qsptr_t qssexpr_parse0_str (qsheap_t * mem, const char * srcstr, const char ** endptr)
{
  qsptr_t retval = QSNIL;
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
      const struct matchrule_s *matchrules = matchruleset[state], *entry = NULL;
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

//      printf("(state=%s, next_state=%s, output=%s, ch@%d='%c')\n", reader_op_str[state], reader_op_str[nextstate], reader_op_str[output], scan, ch);

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
	    {
	      qsword lexlen = qslist_length(mem, lexeme);
	      qsptr_t name = qsutf8_make(mem, lexlen);
	      char * payload = qsobj_ref_data(mem, name, NULL);
	      for (int i = 0; i < lexlen; i++)
		{
		  qsptr_t node = qslist_ref(mem, lexeme, lexlen-i-1);
		  int ch = CCHAR24(node);
		  printf("%08x/ch %x\n", node, ch);
		  payload[i] = ch;
		}
	      printf("payload/%d = %s\n", lexlen, payload);
	      atomval = qssymbol_make(mem, name);
	    }

//	  printf(" atom/%d = %s\n", atom.fill, atom.s);
	  if (retval)
	    { /* building a list. */
	      if (prevnode)
		{ /* continue nested list - (back)pointer to parent in cdr. */
		  qsptr_t temp = qspair_ref_d(mem, prevnode);
		  nextnode = qspair_make(mem, atomval, temp);
		  qspair_setq_d(mem, prevnode, nextnode);
		}
	      else
		{ /* first of list - maintain pointer to parent in cdr. */
		  nextnode = qspair_make(mem, atomval, parent);
		  if (parent) qspair_setq_a(mem, parent, nextnode);
		}
	      /* then advance for next iteration. */
	      prevnode = nextnode;
	    }
	  else
	    { /* toplevel atom. */
	      retval = atomval;
	      halt = 1;
	    }
	  lexeme = QSNIL;
	  prevch = ch; /* push one character. */
	  break;
	case PARSER_BEGIN_LIST:
	  /* start of a new list; on atom read, modify nextnode->al. */
	  if (prevnode)
	    { /* attach to list in progress; cdr is pointer to parent. */
	      qsptr_t temp = qspair_ref_d(mem, prevnode);
	      parent = nextnode = qspair_make(mem, QSNIL, temp);
	      qspair_setq_d(mem, prevnode, nextnode);
	    }
	  else
	    { /* no list in progress. */
	      nextnode = qspair_make(mem, QSNIL, parent);
	      parent = nextnode;
	    }
	  if (!retval) retval = nextnode;
	  prevnode = QSNIL;
	  break;
	case PARSER_END_LIST:
	  /* Finish list building, resume building on parent. */
	  if (prevnode)
	    { /* Salient list; resume parent building by chasing cdr. */
	      nextnode = qspair_ref_d(mem, prevnode);
	      qspair_setq_d(mem, prevnode, QSNIL);
	    }
	  else
	    { /* No list node built, skip back to parent. */
	      nextnode = parent;
	    }
	  prevnode = nextnode;
	  if ((prevnode == retval) || (!qspair_p(mem ,prevnode)))
	    halt = 1;
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
    {
      nextnode = qspair_ref_d(mem, prevnode);
      qspair_setq_d(mem, prevnode, QSNIL);
      prevnode = nextnode;
    }

  if (endptr)
    {
//      printf("endptr +%d\n", scan);
      *endptr = srcstr+scan;
    }

  return retval;
}


/*
int sexpr_print (struct sexpr_s * sexpr)
{
again:
  if (sexpr == NULL)
    return 0;
  if (sexpr->atom)
    {
      printf("%s", sexpr->atom);
    }
  else
    {
      printf("(");
      sexpr_print(sexpr->al);
      printf(")");
    }
  if (sexpr->d)
    {
      printf(" ");
      sexpr = sexpr->d;
      goto again;
    }
  return 0;
}
*/


#if 0

int test_run (const char * srcstr)
{
  const char * remainder = NULL;
  struct sexpr_s * sexpr = sexpr_parse_str(NULL, srcstr, &remainder);
  printf("sexpr=[");
  sexpr_print(sexpr);
  printf("]  remainder=[%s]\n", remainder);
}

static
int test1 ()
{
  test_run( "\"hi\"" );
  test_run( "\"hi" );
}

static
int test2 ()
{
  test_run( "(foo bar(foobar) baz quux ())" );
  test_run( "((foo))" );
}

static
int test3 ()
{
  test_run( "  (1 (atom #\\space \"foobar\")lorem_ipsum_dolor_sit_amet(alpha bravo charlie (d e l t a)) (#true))extra" );
}


int main ()
{
  struct sexpr_s * sexpr0 = sexpr_init(NULL, NULL, NULL, NULL);
  struct sexpr_s * sexpr0a = sexpr_init(NULL, "bar", NULL, NULL);
  struct sexpr_s * sexpr0b = sexpr_init(NULL, "foo", NULL, sexpr0a);
  sexpr0->al = sexpr0b;
  sexpr_print(sexpr0); puts("");

  test1();
  test2();
  test3();

  return 0;
}

#endif //0
