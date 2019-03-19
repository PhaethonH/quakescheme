# Tests to run.
TESTS = test_qsptr \
 test_qsstore \
 test_qsobj \
 test_qsval \
 test_qsmach \
 test_gc



AM_CFLAGS = @CHECK_CFLAGS@ -I$(top_srcdir)/src
AM_LDFLAGS = $(top_builddir)/src/libquakescheme.a @CHECK_LIBS@

# Tests to build.  Simplified by naming the files after their test names.
check_PROGRAMS = $(TESTS)


# make check
