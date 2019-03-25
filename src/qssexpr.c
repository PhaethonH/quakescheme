/* S-Expression parser in C.
   Intended to bootstrap Scheme reader.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>

#include "qssexpr.h"


/*
S-Expression reader.
Multiple strategies (versions).
*/

struct qssxparser_ops_s ** sxparsers[NUM_PARSER_VARIANTS] = {
    &qssxparser_ops_v0,
    &qssxparser_ops_v1,
    &qssxparser_ops_v2,
    &qssxparser_ops_v3,
};

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


bool is_console_xlist (int ch) { return ch == '/'; }
bool is_console_sxp (int ch) { return ch == '!'; }


qssxparse_t * qssxparse_init (qssxparse_t * parser, int version, qsmachine_t * mach)
{
  qssexpr_log("parser init");
  parser->version = version;
  parser->mach = mach;
  qssxparse_reset(parser);
  return parser;
}

qssxparse_t * qssxparse_destroy (qssxparse_t * parser)
{
  parser->version = 0;
  parser->mach = NULL;
  return parser;
}

qssxparse_t * qssxparse_reset (qssxparse_t * parser)
{
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

int qssxparse_feed (qssxparse_t * parser, int ch, qsptr_t * out)
{
  int (*feed)(qssxparse_t *, int, qsptr_t*) = NULL;
  switch (parser->version)
    {
#if 0
    case 0:
      return qssxparse_v0_feed(parser, ch, out);
      break;
#endif //0
    case 1:
      //return qssxparse_v1_feed(parser, ch, out);
      //return ((*(sxparsers[1]))->feed)(parser, ch, out);
      feed = (*(sxparsers[1]))->feed;
      //return (qssxparser_ops_v1->feed)(parser,ch,out);
      break;
#if 0
    case 2:
      return qssxparse_v2_feed(parser, ch, out);
      break;
#endif //0
    default:
      break;
    }
  if (feed)
    return feed(parser, ch, out);
  if (out)
    *out = QSBLACKHOLE;
  return 1;
}

/* Convert lexeme to atom.
   As this is the minimalist reader to bootstrap the Scheme reader,
   it does the minimal work in conversion.

Recognizes:
  integer (int30 and int64)
  float (32b)
  double (64b)
  string (UTF-8)
  boolean (#t, #f)
  symbol

Notably, does not recognize:
  vector (use list->vector)
  character (use integer->character)
  quoting
  keyword
 */
qsptr_t to_atom (qsmachine_t * mach, qsptr_t lexeme)
{
  if (! qsutf8_p(mach, lexeme)) return QSNIL;
  qsword slen = 0;
  const char * cstr = qsutf8_get(mach, lexeme, &slen);

  if (! cstr) return QSNIL;  /* no string. */
  if (! *cstr) return QSNIL;  /* empty string. */
  if (! slen) slen = strlen(cstr);  /* implicit string length. */

  /* try integer. */
  char * endptr = NULL;
  int64_t ival = strtol(cstr, &endptr, 10);
  if (endptr && (*endptr == 0))  /* fully consumed as integer. */
    {
      if ((MIN_INT30 <= ival) && (ival <= MAX_INT30))
	{
	  /* direct encoding. */
	  return QSINT((int)ival);
	}
      else
	{
	  /* widenum. */
	  return qslong_make(mach, ival);
	}
    }

  /* try float. */
  errno = 0;
  float fval = strtof(cstr, &endptr);
  if ((0 == errno) && (! *endptr))
    {
      return QSFLOAT(fval);
    }

  /* try double (maybe out of range for strtof) */
  errno = 0;
  double dval = strtod(cstr, &endptr);
  if ((0 == errno) && (! *endptr))
    {
      return qsdouble_make(mach, dval);
    }

  /* try string */
  if (('"' == cstr[0]) && ('"' == cstr[slen-1]))
    {
      return qsutf8_inject_charp(mach, cstr+1, slen-2);
    }

  /* try special. */
  if ('#' == cstr[0])
    {
      if ((0 == strcmp(cstr+1, "t")) || (0 == strcmp(cstr+1, "true")))
	{
	  return QSTRUE;
	}
      if ((0 == strcmp(cstr+1, "f")) || (0 == strcmp(cstr+1, "false")))
	{
	  return QSFALSE;
	}
    }
  if (0 == strcmp(cstr, "lambda"))
    return QSLAMBDA;
  if (0 == strcmp(cstr, "λ"))
    return QSLAMBDA;

  /* fallback to symbol. */
  return qssymbol_intern_c(mach, cstr, slen);
}




#if 0
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

  while (! halt)
    {
      if (srcstr[scan])
	{
#if 1
	  wideres = mbrtowc(&widechar, srcstr + scan, srclen - scan + 1, &ps);
	  /* TODO: error: invalid multibyte sequence. */
	  if (wideres < 0) return QSERROR_INVALID;
	  scan += wideres;
	  ch = widechar;
#else
	  ch = srcstr[scan];
	  scan++;
#endif //0
	}
      else
	{ /* end of stream, yield nul */
	  ch = 0;
	}

      halt = qssxparse_feed(mem, parser, ch, &retval);
    }
  if (endptr)
    {
      *endptr = srcstr + scan;
    }
  return retval;
}
#endif //0


