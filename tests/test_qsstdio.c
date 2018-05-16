#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check_qs.h"
#include "qsptr.h"
#include "qsstore.h"
#include "qsobj.h"
#include "qsmach.h"

/* Unit test  standard (built-in) input/output */

#define SPACELEN 20000

uint8_t _heap1[sizeof(qsstore_t) + SPACELEN*sizeof(qsobj_t)];
qsstore_t *heap1 = (qsstore_t*)&_heap1;

qs_t _scheme1, *scheme1 = &_scheme1;

char buf[131072];


void init ()
{
  qsstore_init(heap1, SPACELEN);
  qs_init(scheme1, heap1);

  //heap_dump(heap1, 0);
}


START_TEST(test_io1)
{
  init();

  qsptr_t in1 = qsSTDIO_make(heap1, QSIO_IN);
  ck_assert_int_eq(in1, QSIO_IN);
  qsptr_t out1 = qsSTDIO_make(heap1, QSIO_OUT);
  ck_assert_int_eq(out1, QSIO_OUT);

}
END_TEST

START_TEST(test_out1)
{
  init();

  /* capture stdout to "test_qsstdout.txt" */
  qsptr_t out1 = qsSTDIO_make(heap1, QSIO_OUT);
  FILE * capture = fopen("test_qsstdout.txt", "w");
  ck_assert(capture);
  FILE * real_stdout = stdout;
  stdout = capture;

  /* write string, character by character. */
  qsSTDIO_write_u8(heap1, out1, 'h');
  qsSTDIO_write_u8(heap1, out1, 'e');
  qsSTDIO_write_u8(heap1, out1, 'l');
  qsSTDIO_write_u8(heap1, out1, 'l');
  qsSTDIO_write_u8(heap1, out1, 'o');
  qsSTDIO_write_u8(heap1, out1, ',');
  qsSTDIO_write_u8(heap1, out1, ' ');
  qsSTDIO_write_u8(heap1, out1, 'w');
  qsSTDIO_write_u8(heap1, out1, 'o');
  qsSTDIO_write_u8(heap1, out1, 'r');
  qsSTDIO_write_u8(heap1, out1, 'l');
  qsSTDIO_write_u8(heap1, out1, 'd');
  qsSTDIO_write_u8(heap1, out1, '.');
  qsSTDIO_write_u8(heap1, out1, '\n');

  /* restore stdio. */
  stdout = real_stdout;
  fclose(capture);

  /* check contents of (capture) file. */
  char temp[100];
  memset(temp, sizeof(temp), 0);
  int res = 0;
  capture = fopen("test_qsstdout.txt", "r");
  ck_assert(capture);
  res = fread(temp, 1, sizeof(temp), capture);
  fclose(capture);
  ck_assert_int_eq(res, 14);
  temp[res] = 0;
  ck_assert_str_eq(temp, "hello, world.\n");
}
END_TEST

START_TEST(test_err1)
{
  init();

  /* capture stderr to "test_qsstderr.txt" */
  qsptr_t err1 = qsSTDIO_make(heap1, QSIO_ERR);
  FILE * capture = fopen("test_qsstderr.txt", "w");
  ck_assert(capture);
  FILE * real_stderr = stderr;
  stderr = capture;

  /* write string, character by character. */
  qsSTDIO_write_u8(heap1, err1, 'O');
  qsSTDIO_write_u8(heap1, err1, 'k');
  qsSTDIO_write_u8(heap1, err1, '.');
  qsSTDIO_write_u8(heap1, err1, '\n');

  /* restore stdio. */
  stderr = real_stderr;
  fclose(capture);

  /* check contents of (capture) file. */
  char temp[100];
  memset(temp, sizeof(temp), 0);
  int res = 0;
  capture = fopen("test_qsstderr.txt", "r");
  ck_assert(capture);
  res = fread(temp, 1, 4, capture);
  fclose(capture);
  ck_assert_int_eq(res, 4);
  temp[res] = 0;
  ck_assert_str_eq(temp, "Ok.\n");
}
END_TEST

START_TEST(test_in1)
{
  init();

  qsptr_t in1 = qsSTDIO_make(heap1, QSIO_IN);

  /* prepare capture file "test_qsstdin.txt" */
  char temp[100];
  memset(temp, sizeof(temp), 0);
  int res = 0;
  FILE * capture = fopen("test_qsstdin.txt", "w");
  ck_assert(capture);
  res = fwrite("hello-world\n", 1, 12, capture);
  ck_assert_int_eq(res, 12);
  fclose(capture);

  /* Set up capture file as stdin. */
  capture = fopen("test_qsstdin.txt", "r");
  FILE * real_stdin = stdin;
  stdin = capture;

  qsptr_t build_root = QSNIL;
  qsptr_t build_end = QSNIL;

  /* build list of qschar. */
  qsptr_t inbyte = qsSTDIO_read_u8(heap1, in1);
  while (inbyte != QSEOF)
    {
      qsptr_t build_byte = QSCHAR(CINT30(inbyte));
      if (ISNIL(build_root))
	{
	  build_end = build_root = qspair_make(heap1, build_byte, QSNIL);
	}
      else
	{
	  qsptr_t next = qspair_make(heap1, build_byte, QSNIL);
	  qspair_setq_d(heap1, build_end, next);
	  build_end = next;
	}
      inbyte = qsSTDIO_read_u8(heap1, in1);
    }

  /* restore stdio. */
  stdin = real_stdin;
  fclose(capture);

  /* Check contents loaded from file. */
  res = qsstr_extract(heap1, build_root, temp, sizeof(temp));
  ck_assert_int_eq(res, 12);
  ck_assert_str_eq(temp, "hello-world\n");
}
END_TEST


TESTCASE(stdio1,
  TFUNC(test_io1)
  TFUNC(test_out1)
  TFUNC(test_err1)
  TFUNC(test_in1)
  )

TESTSUITE(suite1,
  TCASE(stdio1)
  )

int main ()
{
  RUNSUITE(suite1);
}

