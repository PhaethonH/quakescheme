#ifndef _QSSEXPR_PRIVATE_H_
#define _QSSEXPR_PRIVATE_H_

/* Private structures for S-Expression parser variants. */

#include "qssexpr.h"


extern qsptr_t to_atom (qsmachine_t *, qsptr_t lexeme);

/* S-Expression parsing rules are expressed in terms of a finite state machine
   that changes state based on character classes, with output indicating an
   accumulator effect. */
typedef bool (*parser_predicate_f)(int);
struct matchrule_generic_s {
    parser_predicate_f predicate;
    int next_state;
    int output;  /* private to each variant. */
};


/* Character-class predicate functions for S-Expression parsing. */
extern bool is_end (int ch); /* end of input indicator. */
extern bool is_whitespace (int ch);
/** '(': list opening. */
extern bool is_op (int ch);
/** ')': list closing. */
extern bool is_cl (int ch);
/** '"': string delimiter */
extern bool is_string_delimit (int ch);
/** '\\': string escape character */
extern bool is_string_escape (int ch);
/* always true -- included for pointer-to-function fields. */
extern bool is_any (int ch);
/* always false -- included for pointer-to-function fields. */
extern bool is_none (int ch);

/** [0-9]: decimal digit */
extern bool is_decdigit (int ch);
/** [0-9a-fA-F]: hexdecimal digit. */
extern bool is_hexdigit (int ch);
/** [0-7]: octal digit */
extern bool is_octdigit (int ch);
/** [01]: binary digit */
extern bool is_bindigit (int ch);
extern int to_numeric (int ch, int base);
/** Unit imaginary number.  'i' for pure maths, 'j' in some contexts. */
extern bool is_unitimag (int ch);
/** "\x": (up to) two hexdigit codepoint value */
extern bool is_string_escape_x2 (int ch);
/** "\u": (up to) four hexdigit codepoint value */
extern bool is_string_escape_u4 (int ch);
/** "\U": (up to) eight hexdigit codepoint value */
extern bool is_string_escape_U8 (int ch);

/** single-line comment. */
extern bool is_comment (int ch);
/** end of line indicator, for command line prompt. */
extern bool is_eol (int ch); 
extern bool is_identifier_start (int ch);
extern bool is_identifier (int ch);
/** could indicate start of number or start of symbol: +,- */
extern bool is_pseudonumeric (int ch);
extern bool is_numeric (int ch);
/** decimal separator is locale-specific. */
extern bool is_decimal_separator (int ch);
/** "e" */
extern bool is_exponent_marker (int ch);
/** might be improper list marker ("."), symbol (".bond"), flonum (".007") */
extern bool is_dot (int ch);
/** as in QUOTE form, not "'". */
extern bool is_quote (int ch);
extern bool is_quasiquote (int ch);
extern bool is_unquote (int ch);
extern bool is_unquote_splice (int ch);

/** '#' */
extern bool is_extend (int ch);
/** "#(", until ')' */
extern bool is_extend_vector (int ch);
/** "#u", followed by "8" (i.e. "#u8" */
extern bool is_extend_bits (int ch);
extern bool is_extend_bits_8 (int ch);
/** "#b" */
extern bool is_extend_radix2 (int ch);
/** "#o" */
extern bool is_extend_radix8 (int ch);
/** "#d" */
extern bool is_extend_radix10 (int ch);
/** "#x" */
extern bool is_extend_radix16 (int ch);
/** "#e" */
extern bool is_extend_exact (int ch);
/** "#i" */
extern bool is_extend_inexact (int ch);
/** "#:" */
extern bool is_extend_keyword (int ch);
/** "#!" */
extern bool is_extend_directive (int ch);
/** "#|" - symbol with unusual identifier characters; until "|#" */
extern bool is_extend_symbol (int ch);
/** "#;" - block comment, until ";#" */
extern bool is_extend_comment (int ch);

extern bool is_console_xlist (int ch);
extern bool is_console_sxp (int ch);



/* Declarations for the different parser versions.
   Separation into files to effect encapsulation for version-specific constants
   (what would be namespace in C++).
*/
//int qssxparse_v0_feed (qsheap_t * mem, qssxparse_t * parser, int ch, qsptr_t * out);
//int qssxparse_v1_feed (qsheap_t * mem, qssxparse_t * parser, int ch, qsptr_t * out);
//int qssxparse_v2_feed (qsheap_t * mem, qssxparse_t * parser, int ch, qsptr_t * out);

#endif // _QSSEXPR_PRIVATE_H_
