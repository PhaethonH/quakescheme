lib_LIBRARIES = libquakescheme.a

libquakescheme_a_SOURCES = \
  qsptr.h \
  qsstore.h \
  qsstore.c \
  qsobj.h \
  qsobj.c \
  qsval.h \
  qsval.c \
  qsmach.h \
  qsmach.c \
  qssexpr.h \
  qssexpr.c \
  qssexpr_1.c \
  qsprims_1.c

#l_%.c: %.l
## Escape backslash, dquote, EOL.
#	echo 'const char * qslib_$* = "\' > $@
#	cat $< | sed 's/\\/\\\\/g' | sed 's/"/\\"/g' | sed 's/$$/\\n\\/' >> $@
#	echo '";' >> $@

