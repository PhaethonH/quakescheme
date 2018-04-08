lib_LIBRARIES = libquakescheme.a

libquakescheme_a_SOURCES = \
  qsptr.h \
  qsheap.h \
  qsheap.c \
  qsobj.h \
  qsobj.c

#l_%.c: %.l
## Escape backslash, dquote, EOL.
#	echo 'const char * qslib_$* = "\' > $@
#	cat $< | sed 's/\\/\\\\/g' | sed 's/"/\\"/g' | sed 's/$$/\\n\\/' >> $@
#	echo '";' >> $@

