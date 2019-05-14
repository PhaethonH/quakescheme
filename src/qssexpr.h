#ifndef _QSSEXPR_H_
#define _QSSEXPR_H_

#include <stdarg.h>
#include "qsobj.h"
#include "qsval.h"
#include "qsmach.h"


#define NUM_PARSER_VARIANTS 4

enum qssxparser_variant_e {
    PARSER_INTEGER_SYMBOL_STRING = 0,
    PARSER_NUMBER_SYMBOL_STRING = 1,
    PARSER_CONSOLE = 2,
};

/* logging support. */
extern int qssexpr_logv (const char * fmt, va_list vp);
extern int qssexpr_log (const char * fmt, ...);

/* Main entry point: read one S-expression term from string. */
qsptr_t qssexpr_parse_cstr (qsmachine_t * mach, int version, const char * cstr, const char ** endptr);


/* Parser state information and ruleset handle. */
struct qssxparse_s {
    unsigned int version;

    int complete;   /* 1=expression in root is complete; 0=incomplete. */

    qsmachine_t * mach;
    qsptr_t root;
    qsptr_t parent;
    qsptr_t prevnode;
    qsptr_t nextnode;

    int ch;         /* most recently processed character. */
    int prevch;     /* pushed (unget) character to be processed next cycle. */
    int state;      /* current parser state */
    qsptr_t lexeme; /* lexeme accumulated so far. */

    /* Rulesets are private, referenced by 'version'. */
};

typedef struct qssxparse_s qssxparse_t;

extern qssxparse_t * qssxparse_init (qssxparse_t *, int variant, qsmachine_t * mach);
extern qssxparse_t * qssxparse_destroy (qssxparse_t *);
extern qssxparse_t * qssxparse_reset (qssxparse_t *);
/* Externally drive the parser, one character at time.
   While inefficient, this arrangement allows the parser to be agnostic about
   the bytestream source: ROM string, string port, file port, etc.
*/
extern int qssxparse_feed (qssxparse_t *, int ch, qsptr_t * out);


/* Parser operations table.
   Helps support having multiple variants of parsers to accommodate varying
   levels of features.
 */
extern struct qssxparser_ops_s {
/*
    int (*logv)(const char * fmt, va_list vp);
    int (*log)(const char * fmt, ...);
*/

    qssxparse_t * (*init)(qssxparse_t *, qsmachine_t *);
    qssxparse_t * (*destroy)(qssxparse_t *);
    int (*feed)(qssxparse_t *, int ch, qsptr_t * out);
} ** sxparsers[NUM_PARSER_VARIANTS];


/* N.B. may need to be made extern then declared/defined in respective .c files. */
extern struct qssxparser_ops_s * qssxparser_ops_v0;
extern struct qssxparser_ops_s * qssxparser_ops_v1;
extern struct qssxparser_ops_s * qssxparser_ops_v2;
extern struct qssxparser_ops_s * qssxparser_ops_v3;
/*
extern int qssxparser_v0_feed (qssxparse_t * parser, int ch, qsptr_t * out);
extern int qssxparser_v1_feed (qssxparse_t * parser, int ch, qsptr_t * out);
extern int qssxparser_v2_feed (qssxparse_t * parser, int ch, qsptr_t * out);
extern int qssxparser_v3_feed (qssxparse_t * parser, int ch, qsptr_t * out);
*/


#endif // _QSSEXPR_H_
