#AM_STRICTFLAGS=-std=c99 -D_FORTIFY_SOURCE=2 -fasynchronous-unwind-tables -fexceptions -fpie -Wl,-pie -fpic -shared -fplugin=annobin -fstack-clash-protection -fstack-protector -fstack-protector-strong -grecord-gcc-switches -mcet -fcf-protection -O2 -pipe -Wall -Werror=format-security -Werror=implicit-function-declaration -Wl,-z,defs -Wl,-z,now -Wl,-z,relro 
AM_STRICTFLAGS=-Wall -Werror -Wextra -Wunreachable-code -Wformat=2 -Wshadow -Wswitch-default -O2 -Werror=implicit-function-declaration -Wl,-z,defs -pedantic
#AM_STRICTFLAGS=-Wall -Werror -Wextra -Wunreachable-code -Wformat=2 -Wshadow -Wswitch-default -O2 -Werror=implicit-function-declaration -Wl,-z,defs -DALLOW_CAST_TO_UNION


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

libquakescheme_a_CFLAGS=$(AM_STRICTFLAGS)

#l_%.c: %.l
## Escape backslash, dquote, EOL.
#	echo 'const char * qslib_$* = "\' > $@
#	cat $< | sed 's/\\/\\\\/g' | sed 's/"/\\"/g' | sed 's/$$/\\n\\/' >> $@
#	echo '";' >> $@

