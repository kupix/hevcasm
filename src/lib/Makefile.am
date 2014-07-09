if DEBUG
  AM_CFLAGS =-I$(top_srcdir)/src/lib -Wall -g -O3
else
  AM_CFLAGS =-I$(top_srcdir)/src/lib -Wall
endif

SUFFIXES: .asm

.asm.o: x86inc.asm
	yasm -D ARCH_X86_64=1 -D HAVE_ALIGNED_STACK=1 -f elf64 $<

# The libraries to build
noinst_LIBRARIES = libhevcasm.a

# The files to add to the library and to the source distribution
libhevcasm_a_SOURCES = \
										     $(libhevcasm_a_HEADERS) \
										     hevcasm.c \
										     residual_decode.c \
											 instrset.asm \
											 residual_decode_a.asm