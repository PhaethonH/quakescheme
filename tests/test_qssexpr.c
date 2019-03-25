#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsstore.h"
#include "qsobj.h"
#include "qsmach.h"
#include "qssexpr.h"

/* Unit test  S-Expression parsing. */

qsmachine_t _machine, *machine=&_machine;
char buf[4096];

qssxparse_t _parse, *parse=&_parse;


void init ()
{
  qsmachine_init(machine);
  qssxparse_init(parse, 1, machine);
}


qsptr_t parse_and_print (const char * s)
{
  const char * t;
  int res = 0;
  qsptr_t out;
  qssxparse_reset(parse);
  for (t = s; *t; ++t)
    {
      if ((res = qssxparse_feed(parse, *t, &out)))
	break;
    }
  if (! res)
    res = qssxparse_feed(parse, 0, &out);
  if (! res)
    return QSBLACKHOLE;
  qsptr_crepr(machine, out, buf, sizeof(buf));
  return out;
}

START_TEST(test_read_print)
{
  init();

  qsptr_t out;

  /* test integer. */
  out = parse_and_print("123");
  ck_assert_int_ne(out, QSBLACKHOLE);
  ck_assert(qsint_p(machine, out));
  ck_assert_int_eq(CINT30(out), 123);
  ck_assert_str_eq(buf, "123");

  /* test string. */
  out = parse_and_print("   \"hello, world.\"");
  ck_assert_int_ne(out, QSBLACKHOLE);
  ck_assert(qsutf8_p(machine, out));
  ck_assert_str_eq(buf, "\"hello, world.\"");

  /* test simple list. */
  out = parse_and_print("(1  2      3)");
  ck_assert_int_ne(out, QSBLACKHOLE);
  ck_assert(qspair_p(machine, out));
  ck_assert_str_eq(buf, "(1 2 3)");

  /* test float. */
  out = parse_and_print("3.14159265");
  ck_assert_int_ne(out, QSBLACKHOLE);
  ck_assert(qsfloat_p(machine, out));
  ck_assert_str_eq(buf, "3.14159");

  /* test float implied by decimal separator. */
  out = parse_and_print("1.");
  ck_assert_int_ne(out, QSBLACKHOLE);
  ck_assert(qsfloat_p(machine, out));
  ck_assert_str_eq(buf, "1");

  /* test symbol. */
  out = parse_and_print("\n foo ");
  ck_assert_int_ne(out, QSBLACKHOLE);
  ck_assert(qssymbol_p(machine, out));
  ck_assert_str_eq(buf, "foo");

  /* test boolean. */
  out = parse_and_print("#t");
  ck_assert_int_ne(out, QSBLACKHOLE);
  ck_assert(qsbool_p(machine, out));
  ck_assert_str_eq(buf, "#t");
  out = parse_and_print("#f");
  ck_assert_int_ne(out, QSBLACKHOLE);
  ck_assert(qsbool_p(machine, out));
  ck_assert_str_eq(buf, "#f");

  /* test nil. */
  out = parse_and_print("()");
  ck_assert_int_ne(out, QSBLACKHOLE);
  ck_assert_int_eq(out, QSNIL);
  ck_assert_str_eq(buf, "()");

  /* test 'lambda. */
  out = parse_and_print("lambda");
  ck_assert_int_ne(out, QSBLACKHOLE);
  ck_assert_int_eq(out, QSLAMBDA);
  ck_assert_str_eq(buf, "#<const 6>");

  /* test nested list. */
  out = parse_and_print("(10 11 (foo bar) 12  )");
  ck_assert_int_ne(out, QSBLACKHOLE);
  ck_assert(qspair_p(machine, out));
  ck_assert_str_eq(buf, "(10 11 (foo bar) 12)");
}
END_TEST

START_TEST(test_test2)
{
  init();
}
END_TEST


TESTCASE(case1,
  TFUNC(test_read_print)
  TFUNC(test_test2)
  )

TESTSUITE(suite1,
  TCASE(case1)
  )

int main ()
{
  RUNSUITE(suite1);
}

