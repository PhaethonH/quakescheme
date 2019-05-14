#AM_STRICTFLAGS=-std=c99 -Wall -Wextra  -Wstrict-aliasing -pedantic -Werror -Wunreachable-code -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Wlogical-op -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses -fdiagnostics-show-option
AM_STRICTFLAGS=-Wall -Werror -Wextra


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

