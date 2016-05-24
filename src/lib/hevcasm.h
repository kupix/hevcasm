// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.


#ifndef INCLUDED_hevcasm_h
#define INCLUDED_hevcasm_h

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>


/* macro used to ringfence code derived from the f265 project */
#ifndef USE_F265_DERIVED
#define USE_F265_DERIVED 1
#endif

/* macro used to ringfence code derived from the WebM project */
#ifndef USE_WEBM_DERIVED
#define USE_WEBM_DERIVED 1
#endif

/* macro used to ringfence code derived from the HM codec */
#ifndef USE_HM_DERIVED
#define USE_HM_DERIVED 1
#endif



/* compiler-specific inline timestamp function and alignment macro */
#ifdef _MSC_VER

#include <intrin.h>

#if _WIN64
#define HEVCASM_X64
#endif

typedef int64_t hevcasm_timestamp;


static hevcasm_timestamp hevcasm_get_timestamp()
{
	return __rdtsc();
}

#define HEVCASM_ALIGN(n, T, v) \
	__declspec(align(n)) T v

#endif

/* compiler-specific inline timestamp function and alignment macro */
#ifdef __GNUC__

#if __x86_64__
#define HEVCASM_X64
#endif

typedef uint64_t hevcasm_timestamp;

#if defined(__i386__)

static hevcasm_timestamp hevcasm_get_timestamp(void)
{
	uint64_t timestamp;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (timestamp));
	return timestamp;
}

#elif defined(__x86_64__)

static hevcasm_timestamp hevcasm_get_timestamp(void)
{
	unsigned h, l;
	__asm__ __volatile__ ("rdtsc" : "=a"(l), "=d"(h));
	return (hevcasm_timestamp)l | ((hevcasm_timestamp)h << 32);
}

#endif

#define HEVCASM_ALIGN(n, T, v) \
	T v __attribute__((aligned(n)))

#endif


#ifdef __cplusplus
extern "C" {
#endif


#define HEVCASM_INSTRUCTION_SET_XMACRO \
	X(0, C_REF, "C - reference; may be slow") \
	X(1, C_OPT, "C - optimised") \
	X(2, SSE2, "SSE2") \
	X(3, SSE3, "SSE3") \
	X(4, SSSE3, "Supplementary SSE3") \
	X(5, SSE41, "SSE4.1") \
	X(6, SSE42, "SSE4.2") \
	X(7, AVX, "AVX") \
	X(8, AVX2, "AVX2")


/* bitmask type for describing instruction sets */
typedef enum 
{
	HEVCASM_NONE = 0,
#define X(value, name, description) HEVCASM_ ## name = 1 << value,
	HEVCASM_INSTRUCTION_SET_XMACRO
#undef X
} hevcasm_instruction_set;


/* queries processor via cpuid instruction and returns a bitmask representing supported instruction sets */
hevcasm_instruction_set hevcasm_instruction_set_support();


void hevcasm_print_instruction_set_support(FILE *f, hevcasm_instruction_set mask);


typedef struct
{
	void *implementation;
} hevcasm_code;


/* create a new buffer for JIT assembler emitted object code */
hevcasm_code hevcasm_new_code(hevcasm_instruction_set mask, int size);


/* must be called to free resources used by a code buffer created by hevcasm_new_code() */
void hevcasm_delete_code(hevcasm_code);


/* library self-test entry point */
int hevcasm_main(int argc, const char *argv[]);


#define HEVCASM_RECT(width, height) (((width) << 8) | (height))


typedef void hevcasm_test_function(int *error_count, hevcasm_instruction_set mask);


#ifdef __cplusplus
}
#endif

#endif
