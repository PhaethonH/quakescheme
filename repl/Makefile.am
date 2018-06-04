bin_PROGRAMS = qsrepl

qsrepl_SOURCES = \
  qsrepl.c

qsrepl_LDADD = $(top_builddir)/src/libquakescheme.a -lm

qsrepl_CFLAGS = -I$(top_srcdir)/src $(AM_CFLAGS)

#l_%.c: %.l
## Escape backslash, dquote, EOL.
#	echo 'const char * qslib_$* = "\' > $@
#	cat $< | sed 's/\\/\\\\/g' | sed 's/"/\\"/g' | sed 's/$$/\\n\\/' >> $@
#	echo '";' >> $@

