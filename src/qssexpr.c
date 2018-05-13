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



static
bool one_of (int * accept, int ch)
{
  /* modeled after strchr */
  int * iter;
  for (iter = accept; *iter; iter++)
    {
      if (*iter == ch) return 1;
    }
  return 0;
}

bool is_end (int ch) { return ch == 0; }
bool is_whitespace (int ch) { return isspace(ch); }
bool is_op (int ch) { return (ch == '('); }
bool is_cl (int ch) { return (ch == ')'); }
bool is_string_delimit (int ch) { return (ch == '"'); }
bool is_string_escape (int ch) { return (ch == '\\'); }
bool is_any (int ch) { return 1; }

bool is_decdigit (int ch) { return isdigit(ch); }
bool is_hexdigit (int ch) { return isxdigit(ch); }
bool is_octdigit (int ch)
{
  int accept[] = {
      '0', '1', '2', '3', '4', '5', '6', '7',
      0
  };
  return one_of(accept, ch);
}
bool is_bindigit (int ch) { return (ch == '0') || (ch == '1'); }
bool is_unitimage (int ch)
{
  int accept[] = {
      'i',
      'j',
      0x2148, /* U+2148 DOUBLE-STRUCK SMALL ITALIC I */
      0
  };
  return one_of(accept, ch);
}
bool is_string_escape_x2 (int ch) { return (ch == 'x'); }
bool is_string_escape_u4 (int ch) { return (ch == 'u'); }
bool is_string_escape_U8 (int ch) { return (ch == 'U'); }

bool is_comment (int ch) { return ch == ';'; }
bool is_eol (int ch)
{
  int accept[] = {
      '\n',
      '\r',
      0
  };
  return one_of(accept, ch);
}
bool is_pseudonumeric (int ch)
{
  int accept[] = {
      '+',
      '-',
      0
  };
  return one_of(accept, ch);
}
bool is_decimal_separator (int ch)
{
  int accept[] = {
      '.',
      ',',
      0
  };
  return one_of(accept, ch);
}
bool is_exponent_marker (int ch) { return ch == 'e' || ch == 'E'; }
bool is_dot (int ch) { return ch == '.'; }
bool is_quote (int ch) { return ch == '\''; }
bool is_quasiquote (int ch) { return ch == '`'; }
bool is_unquote (int ch) { return ch == ','; }
bool is_unquote_splice (int ch) { return ch == '@'; }

bool is_extend (int ch) { return ch == '#'; }
bool is_extend_vector (int ch) { return ch == ')'; }
bool is_extend_bits (int ch) { return ch == 'u'; }
bool is_extend_bits_8 (int ch) { return ch == '8'; }
bool is_extend_radix2 (int ch) { return ch == 'b'; }
bool is_extend_radix8 (int ch) { return ch == 'o'; }
bool is_extend_radix10 (int ch) { return ch == 'd'; }
bool is_extend_radix16 (int ch) { return ch == 'x'; }
bool is_extend_exact (int ch) { return ch == 'e'; }
bool is_extend_inexact (int ch) { return ch == 'i'; }
bool is_extend_keyword (int ch) { return ch == ':'; }
bool is_extend_directive (int ch) { return ch == '!'; }
bool is_extend_symbol (int ch) { return (ch == '|' || ch == 0xa6); }
bool is_extend_comment (int ch) { return ch == ';'; }

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
  int srclen = 0;
  struct qssxparse_s _parser = { 0, }, *parser = &_parser;
#if 1
  mbstate_t ps = { 0, };
  wchar_t widechar;
  int wideres = 0;
#endif //0

  qssxparse_init(mem, parser, version);
  srclen = strlen(srcstr);

#if 1
  wideres = mbrtowc(&widechar, srcstr + scan, srclen - scan + 1, &ps);
  if (wideres < 0) return QSERROR_INVALID; /* TODO: error: invalid multibyte sequence. */
  ch = widechar;
#else
  ch = srcstr[scan];
#endif //wchar

  while (! halt)
    {
      halt = qssxparse_feed(mem, parser, ch, &retval);

      if (srcstr[scan])
	{
#if 1
	  scan += wideres;
	  wideres = mbrtowc(&widechar, srcstr + scan, srclen - scan + 1, &ps);
	  if (wideres < 0) return QSERROR_INVALID; /* TODO: error: invalid multibyte sequence. */
	  ch = widechar;
#else
	  scan++;
	  ch = srcstr[scan];
#endif //wchar
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

