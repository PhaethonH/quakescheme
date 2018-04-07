# Tests to run.
TESTS = test_qsptr \
  test_qsheap



AM_CFLAGS = @CHECK_CFLAGS@ -I$(top_builddir)/src
AM_LDFLAGS = $(top_builddir)/src/libquakescheme.a @CHECK_LIBS@

# Tests to build.  Simplified by naming the files after their test names.
check_PROGRAMS = $(TESTS)

bin_PROGRAMS = qs_repl


# make check
