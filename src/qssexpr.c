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


#if 0
qsptr_t qssexpr_parse0_str (qsheap_t * mem, const char * srcstr, const char ** endptr)
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

      printf("(state=%s, next_state=%s, output=%s, ch@%d='%c')\n", reader_op_str[state], reader_op_str[nextstate], reader_op_str[output], scan, ch);
      printf(" (retval=%08x, parent=%08x, prevnode=%08x\n", retval, parent, prevnode);

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
	      /* convert list-of-charater to utf8 C string */
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
	  if (qspair_p(mem, retval) || ISNIL(retval))
	    { /* building a list. */
	      puts("atom1");
	      if (!ISNIL(prevnode))
		{ /* continue nested list - (back)pointer to parent in cdr. */
		  puts("atom1a");
/*
   -- before --
   [a|P]
   ^ prevnode

   -- after --
   [a|>]*[b|P]
         ^ prevnode
*/
		  qsptr_t temp = qspair_ref_d(mem, prevnode);
		  nextnode = qspair_make(mem, atomval, temp);
		  qspair_setq_d(mem, prevnode, nextnode);
		}
	      else
		{ /* first of list - maintain pointer to parent in cdr. */
		  puts("atom1b");
/*
   -- before --
       v parent
   ... [a|P]			parent=nil
       ^ prevnode

   -- after --
             v parent
   ... [a|>]*[x|P]
                ^                  parent
	    *[b|P]             *[b|P]
	     ^ prevnode         ^ prevnode
*/
//		  nextnode = qspair_make(mem, atomval, parent);
//		  if (parent) qspair_setq_a(mem, parent, nextnode);
		  if (!ISNIL(parent))
		    {
		  puts("atom1c");
		      // extend parent.
		      nextnode = qspair_make(mem, QSNIL, qspair_ref_d(mem, parent));
		      qspair_setq_d(mem, parent, nextnode);
		      parent = nextnode;
		      // begin nested list.
		    }
		  if (ISNIL(retval))
		    retval = parent;
		  nextnode = qspair_make(mem, atomval, parent);
		}
	      /* then advance for next iteration. */
	      prevnode = nextnode;
	    }
	  else if (retval == QSBLACKHOLE)
	    { /* toplevel atom. */
	      puts("atom2");
	      retval = atomval;
	      halt = 1;
	    }
	  else
	    {
	      abort();
	    }
	  lexeme = QSNIL;
	  prevch = ch; /* push one character. */
	  break;
	case PARSER_BEGIN_LIST:
	  /* start of a new list; on atom read, modify nextnode->al. */
	  if (!ISNIL(prevnode))
	    { /* attach to list in progress; cdr is pointer to parent. */
	      puts("list1");
/*
            prevnode v  parent^
		 ... [a|P]

	    -- after --
                          v parent
		... [a|>]*[/|P]
		          ^
	prevnode=nil	  future nextnode's parent

	    -- on attach atom ---	  -- on attach new list (below) ---
	                  v parent
		... [a|>] [x|P]		  ... [a|>] [x|P]
		           |			     |
			 *[a|P]			   *[/|P]
		 prevnode ^	       prevnode=nil ^ parent

		 */
	      qsptr_t temp = qspair_ref_d(mem, prevnode);
	      parent = nextnode = qspair_make(mem, QSNIL, temp);
	      qspair_setq_d(mem, prevnode, nextnode);
	    }
	  else
	    { /* no list in progress. */
	      puts("list2");
/*	    -- before --
	        v parent
            ... [x|P]			  parent=nil

	    -- after --
	    ... [x|P]			 *[x|P=/]
	         | ^
	       *[y|P]
   prevnode=nil ^ parent
*/

	      nextnode = qspair_make(mem, QSNIL, parent);
	      /* advance parent pointer. */
	      parent = nextnode;
	    }
	  if (retval == QSBLACKHOLE)
	    {
	      puts("list4");
	    retval = QSNIL;
	    }
	  else if (ISNIL(retval))
	    {
	      puts("list4");
	    retval = nextnode;
	    }
	  else
	    {
	      abort();
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
//	  if ((prevnode == retval) || (!qspair_p(mem, prevnode)))
	  if (!qspair_p(mem, prevnode))
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
#endif //0
#if 1
qsptr_t qssexpr_parse0_str (qsheap_t * mem, const char * srcstr, const char ** endptr)
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
//      printf(" (retval=%08x, parent=%08x, prevnode=%08x\n", retval, parent, prevnode);

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
	      /* convert list-of-charater to utf8 C string */
	      qsword lexlen = qslist_length(mem, lexeme);
	      qsptr_t name = qsutf8_make(mem, lexlen);
	      char * payload = qsobj_ref_data(mem, name, NULL);
	      for (int i = 0; i < lexlen; i++)
		{
		  qsptr_t node = qslist_ref(mem, lexeme, lexlen-i-1);
		  int ch = CCHAR24(node);
//		  printf("%08x/ch %x\n", node, ch);
		  payload[i] = ch;
		}
//	      printf("payload/%d = %s\n", lexlen, payload);
	      atomval = qssymbol_make(mem, name);
	    }

//	  printf(" atom/%d = %s\n", atom.fill, atom.s);

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
		  retval = nextnode = qspair_make(mem, QSNIL, QSNIL);
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
	  if (!qspair_p(mem, prevnode))
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
//      printf("endptr +%d\n", scan);
      *endptr = srcstr+scan;
    }

  return retval;
}
#endif //0


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
