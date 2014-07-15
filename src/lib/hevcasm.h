/*
The copyright in this software is being made available under the BSD
License, included below. This software may be subject to other third party
and contributor rights, including patent rights, and no such rights are
granted under this license.


Copyright(c) 2011 - 2014, Parabola Research Limited
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met :

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and / or other materials provided with the distribution.
* Neither the name of the copyright holder nor the names of its contributors may
be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef INCLUDED_hevcasm_h
#define INCLUDED_hevcasm_h

#include <stddef.h>
#include <inttypes.h>
#include <stdio.h>


#ifdef WIN32

#include <intrin.h>


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

#define HEVCASM_ALIGN(n, T, v) \
	T v __attribute__((aligned(n)))

#define HEVCASM_API

#endif



#define HEVCASM_INSTRUCTION_SET_XMACRO \
	X(0, C, "generic C code") \
	X(1, SSE2, "SSE2") \
	X(2, SSE3, "SSE3") \
	X(3, SSSE3, "Supplementary SSE3") \
	X(4, SSE41, "SSE4.1") \
	X(5, POPCNT, "POPCNT") \
	X(6, SSE42, "SSE4.2") \
	X(7, AVX, "AVX") \
	X(8, RDRAND, "RDRAND") \
	X(9, PCLMUL_AES, "PCLMUL and AES") \
	X(10, AVX2, "AVX2") 

#define HEVCASM_INSTRUCTION_SET_COUNT 11



#ifdef __cplusplus
extern "C"
{
#endif


typedef enum {
#define X(value, name, description) HEVCASM_ ## name = 1 << value,
	HEVCASM_INSTRUCTION_SET_XMACRO
#undef X
} hevcasm_instruction_set;


hevcasm_instruction_set HEVCASM_API hevcasm_instruction_set_support();

void HEVCASM_API hevcasm_print_instruction_set_support(FILE *f, hevcasm_instruction_set mask);

int HEVCASM_API hevcasm_main(int argc, const char *argv[]);

#define HEVCASM_RECT(width, height) (((width) << 8) | (height))




#ifdef __cplusplus
}
#endif


#endif
