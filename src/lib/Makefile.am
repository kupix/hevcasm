SUBDIRS = f265

if DEBUG
  AM_CFLAGS =-I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/libvpx -I$(top_srcdir)/src/lib/libvpx/config/gcc -Wall -Wno-unused-function -g
else
  AM_CFLAGS =-I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/libvpx -I$(top_srcdir)/src/lib/libvpx/config/gcc -Wall -Wno-unused-function -O3
endif

SUFFIXES: .asm

.asm.o: x86inc.asm
	yasm -D ARCH_X86_64=1 -i $(top_srcdir)/src/lib/x264 -i $(top_srcdir)/src/lib/libvpx/vp9 -i $(top_srcdir)/src/lib/libvpx/config/gcc -D HAVE_ALIGNED_STACK=1 -f elf64 -o $@ $<

# The libraries to build
noinst_LIBRARIES = libhevcasm.a

# The files to add to the library and to the source distribution
libhevcasm_a_SOURCES = \
	$(libhevcasm_a_HEADERS) \
	diff.c \
	hadamard.c \
	hevcasm.c \
	hevcasm_test.c \
	pred_intra.c \
	pred_inter.c \
	ssd.c \
	ssd_a.asm \
	sad_a.asm \
	quantize.c \
	residual_decode.c \
	sad.c \
	diff_a.asm \
	hadamard_a.asm \
	pred_inter_a.asm \
	quantize_a.asm \
	residual_decode_a.asm \
	libvpx/vp9/encoder/x86/vp9_sad_sse2.asm \
	libvpx/vp9/encoder/x86/vp9_sad4d_sse2.asm

libhevcasm_a_LIBADD = \
	f265/dct.o \
	f265/intra.o

