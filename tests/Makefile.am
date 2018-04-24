# Tests to run.
TESTS = test_qsptr \
  test_qsheap \
  test_qstree \
  test_qsvector \
  test_qsbytevec \
  test_qswidenum \
  test_qsiter \
  test_qsstr \
  test_rbtree \
  test_ibtree \
  test_gc \
  test_symbols



AM_CFLAGS = @CHECK_CFLAGS@ -I$(top_srcdir)/src
AM_LDFLAGS = $(top_builddir)/src/libquakescheme.a @CHECK_LIBS@

# Tests to build.  Simplified by naming the files after their test names.
check_PROGRAMS = $(TESTS)

bin_PROGRAMS = qs_repl


# make check
