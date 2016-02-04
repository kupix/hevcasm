// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.


#ifndef INCLUDED_hevcasm_h
#define INCLUDED_hevcasm_h

#include <stddef.h>
#include <inttypes.h>
#include <stdio.h>


// Macro used to ringfence code derived from the f265 project
#ifndef USE_F265_DERIVED
#define USE_F265_DERIVED 1
#endif

// Macro used to ringfence code derived from the WebM project
#ifndef USE_WEBM_DERIVED
#define USE_WEBM_DERIVED 1
#endif


#ifdef WIN32

#include <intrin.h>

#if _WIN64
#define HEVCASM_X64
#endif

typedef int64_t hevcasm_timestamp;


static hevcasm_timestamp hevcasm_get_timestamp()
{
	return __rdtsc();
}


#ifdef HEVCASM_DLL_EXPORTS
#define HEVCASM_API __declspec( dllexport ) 
#else
#ifdef HEVCASM_DLL_IMPORTS
#define HEVCASM_API __declspec( dllimport ) 
#else
#define HEVCASM_API
#endif
#endif

#define HEVCASM_ALIGN(n, T, v) \
	__declspec(align(n)) T v

#endif

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

#define HEVCASM_API

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

#define HEVCASM_INSTRUCTION_SET_COUNT 9


#ifdef __cplusplus
extern "C"
{
#endif

typedef enum 
{
	HEVCASM_NONE = 0,
#define X(value, name, description) HEVCASM_ ## name = 1 << value,
	HEVCASM_INSTRUCTION_SET_XMACRO
#undef X
} hevcasm_instruction_set;



/*
Queries processor via cpuid instruction and returns a bitmask representing supported instruction sets.
*/
hevcasm_instruction_set HEVCASM_API hevcasm_instruction_set_support();


void HEVCASM_API hevcasm_print_instruction_set_support(FILE *f, hevcasm_instruction_set mask);


/*
Library self-test entry point.
*/
int HEVCASM_API hevcasm_main(int argc, const char *argv[]);


#define HEVCASM_RECT(width, height) (((width) << 8) | (height))


typedef void HEVCASM_API hevcasm_test_function(int *error_count, hevcasm_instruction_set mask);



#ifdef __cplusplus
}
#endif


#endif
