if DEBUG
  AM_CFLAGS =-I$(top_srcdir)/src/bin -I$(top_srcdir)/src/lib -Wall -Wno-unused-functions -g
else
  AM_CFLAGS =-I$(top_srcdir)/src/bin -I$(top_srcdir)/src/lib -Wall -Wno-unused-functions -O3
endif

bin_PROGRAMS = hevcasm
hevcasm_SOURCES = main.c

hevcasm_LDADD = $(top_builddir)/src/lib/libhevcasm.a $(AM_LDFLAGS)
