SUFFIXES: .asm

.asm.o: x86inc.asm
	nasm -DARCH_X86_64 -DARCH_AMD64 -f elf64 -o $@ $<

# The libraries to build
noinst_LIBRARIES = libhevcasmf265.a

# The files to add to the library and to the source distribution
libhevcasmf265_a_SOURCES = \
	dct.asm \
	encode.asm \
	intra.asm \
	pixel.asm
