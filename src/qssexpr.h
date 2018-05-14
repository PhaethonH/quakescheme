#ifndef _QSSEXPR_H_
#define _QSSEXPR_H_

#include "qsobj.h"


/* Main entry point: read one S-expression term from string. */
qsptr_t qssexpr_parse_cstr (qsheap_t * mem, int version, const char * cstr, const char ** endptr);



/* S-Expression parser state. */
struct qssxparse_s {
    /* Allow for multiple version of S-Expression parser, for varying degrees of conversion to objects. */
    int version;
    int complete;   /* 1=expression in root is complete; 0=incomplete. */

    qsheap_t * mem;
    qsptr_t root;
    qsptr_t parent;
    qsptr_t prevnode;
    qsptr_t nextnode;

    int ch;         /* most recently processed character. */
    int prevch;     /* pushed (unget) character to be processed next cycle. */
    int state;      /* current parser state */
    qsptr_t lexeme; /* lexeme accumulated so far. */
};

typedef struct qssxparse_s qssxparse_t;


/* S-Expression parsing rules are expressed in terms of a finite state machine
   that changes state based on character classes, with output indicating an
   accumulator effect. */
typedef bool (*parser_predicate_f)(int);
struct matchrule_generic_s {
    parser_predicate_f predicate;
    int next_state;
    int output;
};


qssxparse_t * qssxparse_init (qsheap_t * mem, qssxparse_t * parser, int version);
int qssxparse_feed (qsheap_t * mem, qssxparse_t * parser, int ch, qsptr_t * out);



/* Character-class predicate functions for S-Expression parsing. */
bool is_end (int ch); /* end of input indicator. */
bool is_whitespace (int ch);
/** '(': list opening. */
bool is_op (int ch);
/** ')': list closing. */
bool is_cl (int ch);
/** '"': string delimiter */
bool is_string_delimit (int ch);
/** '\\': string escape character */
bool is_string_escape (int ch);
/* always true -- included for pointer-to-function fields. */
bool is_any (int ch);
/* always false -- included for pointer-to-function fields. */
bool is_none (int ch);

/** [0-9]: decimal digit */
bool is_decdigit (int ch);
/** [0-9a-fA-F]: hexdecimal digit. */
bool is_hexdigit (int ch);
/** [0-7]: octal digit */
bool is_octdigit (int ch);
/** [01]: binary digit */
bool is_bindigit (int ch);
int to_numeric (int ch, int base);
/** Unit imaginary number.  'i' for pure maths, 'j' in some contexts. */
bool is_unitimag (int ch);
/** "\x": (up to) two hexdigit codepoint value */
bool is_string_escape_x2 (int ch);
/** "\u": (up to) four hexdigit codepoint value */
bool is_string_escape_u4 (int ch);
/** "\U": (up to) eight hexdigit codepoint value */
bool is_string_escape_U8 (int ch);

/** single-line comment. */
bool is_comment (int ch);
/** end of line indicator, for command line prompt. */
bool is_eol (int ch); 
bool is_identifier_start (int ch);
bool is_identifier (int ch);
/** could indicate start of number or start of symbol: +,- */
bool is_pseudonumeric (int ch);
bool is_numeric (int ch);
/** decimal separator is locale-specific. */
bool is_decimal_separator (int ch);
/** "e" */
bool is_exponent_marker (int ch);
/** might be improper list marker ("."), symbol (".bond"), flonum (".007") */
bool is_dot (int ch);
/** as in QUOTE form, not "'". */
bool is_quote (int ch);
bool is_quasiquote (int ch);
bool is_unquote (int ch);
bool is_unquote_splice (int ch);

/** '#' */
bool is_extend (int ch);
/** "#(", until ')' */
bool is_extend_vector (int ch);
/** "#u", followed by "8" (i.e. "#u8" */
bool is_extend_bits (int ch);
bool is_extend_bits_8 (int ch);
/** "#b" */
bool is_extend_radix2 (int ch);
/** "#o" */
bool is_extend_radix8 (int ch);
/** "#d" */
bool is_extend_radix10 (int ch);
/** "#x" */
bool is_extend_radix16 (int ch);
/** "#e" */
bool is_extend_exact (int ch);
/** "#i" */
bool is_extend_inexact (int ch);
/** "#:" */
bool is_extend_keyword (int ch);
/** "#!" */
bool is_extend_directive (int ch);
/** "#|" - symbol with unusual identifier characters; until "|#" */
bool is_extend_symbol (int ch);
/** "#;" - block comment, until ";#" */
bool is_extend_comment (int ch);

bool is_console_xlist (int ch);
bool is_console_sxp (int ch);



/* Declarations for the different parser versions.
   Separation into files to effect encapsulation for version-specific constants
   (what would be namespace in C++).
*/
int qssxparse_v0_feed (qsheap_t * mem, qssxparse_t * parser, int ch, qsptr_t * out);
int qssxparse_v1_feed (qsheap_t * mem, qssxparse_t * parser, int ch, qsptr_t * out);
int qssxparse_v2_feed (qsheap_t * mem, qssxparse_t * parser, int ch, qsptr_t * out);

#endif // _QSSEXPR_H_
