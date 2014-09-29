if DEBUG
  AM_CFLAGS =-I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/libvpx -I$(top_srcdir)/src/lib/libvpx/config/gcc -Wall -Wno-unused-function -g -O3
else
  AM_CFLAGS =-I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/libvpx -I$(top_srcdir)/src/lib/libvpx/config/gcc -Wall -Wno-unused-function
endif

SUFFIXES: .asm

.asm.o: x86inc.asm
	yasm -D ARCH_X86_64=1 -i $(top_srcdir)/src/lib/libvpx/vp9 -i $(top_srcdir)/src/lib/libvpx/config/gcc -D HAVE_ALIGNED_STACK=1 -f elf64 -o $@ $<

# The libraries to build
noinst_LIBRARIES = libhevcasm.a

# The files to add to the library and to the source distribution
libhevcasm_a_SOURCES = \
	$(libhevcasm_a_HEADERS) \
	hevcasm.c \
	residual_decode.c \
	residual_decode_a.asm \
	pred_inter.c \
	pred_inter_a.asm \
	sad.c \
	libvpx/vp9/encoder/x86/vp9_sad_sse2.asm \
	libvpx/vp9/encoder/x86/vp9_sad4d_sse2.asm

