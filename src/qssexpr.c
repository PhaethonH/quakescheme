#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "qssexpr.h"


/*
S-Expression reader.
Multiple strategies (versions).
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



bool is_end (int ch) { return ch == 0; }
bool is_whitespace (int ch) { return isspace(ch); }
bool is_op (int ch) { return (ch == '('); }
bool is_cl (int ch) { return (ch == ')'); }
bool is_string_delimit (int ch) { return (ch == '"'); }
bool is_string_escape (int ch) { return (ch == '\\'); }
bool is_any (int ch) { return 1; }


struct qssxparse_s * qssxparse_init (qsheap_t * mem, struct qssxparse_s * parser, int version)
{
  qssexpr_log("parser init");
  parser->version = version;
  parser->root = QSBLACKHOLE;
  parser->parent = QSNIL;
  parser->prevnode = QSNIL;
  parser->nextnode = QSNIL;
  parser->ch = 0;
  parser->prevch = 0;
  parser->state = 0;
  parser->lexeme = QSNIL;
  parser->complete = 0;

  return parser;
}

#if 0
static
qsptr_t qssexpr_parse0_cstr (qsheap_t * mem, const char * srcstr, const char ** endptr)
{
  qsptr_t retval = QSBLACKHOLE;
  int ch = 0;
  int scan = 0;
  struct qssxparse_s _parser = { 0, }, *parser = &_parser;

  qssxparse_init(mem, parser, 0);
  ch = srcstr[scan];
  while (! parser->complete)
    {
      qssxparse_v0_feed(mem, parser, ch, &retval);

      if (srcstr[scan])
	{
	  scan++;
	  ch = srcstr[scan];
	}
      else
	{ /* end of stream, yield \0 */
	  ch = 0;
	}
    }
  if (endptr)
    {
      *endptr = srcstr + scan;
    }
  return retval;
}
#endif //0

int qssxparse_feed (qsheap_t * mem, struct qssxparse_s * parser, int ch, qsptr_t * out)
{
  switch (parser->version)
    {
    case 0:
      return qssxparse_v0_feed(mem, parser, ch, out);
      break;
    case 1:
      return qssxparse_v1_feed(mem, parser, ch, out);
      break;
    default:
      break;
    }
  if (out)
    *out = QSBLACKHOLE;
  return 1;
}




qsptr_t qssexpr_parse_cstr (qsheap_t * mem, int version, const char * srcstr, const char ** endptr)
{
  qsptr_t retval = QSBLACKHOLE;
  int ch = 0;
  int scan = 0;
  int halt = 0;
  struct qssxparse_s _parser = { 0, }, *parser = &_parser;

  qssxparse_init(mem, parser, version);
  ch = srcstr[scan];
  while (! halt)
    {
      halt = qssxparse_feed(mem, parser, ch, &retval);

      if (srcstr[scan])
	{
	  scan++;
	  ch = srcstr[scan];
	}
      else
	{ /* end of stream, yield \0 */
	  ch = 0;
	}
    }
  if (endptr)
    {
      *endptr = srcstr + scan;
    }
  return retval;
}

