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


#ifndef INCLUDED_sad_h
#define INCLUDED_sad_h

#include "hevcasm.h"


#ifdef __cplusplus
extern "C"
{
#endif


/* Rectangular SAD (Sum of Absolute Differences) with single reference */
typedef int hevcasm_sad(const uint8_t *src, ptrdiff_t stride_src, const uint8_t *ref, ptrdiff_t stride_ref, uint32_t rect);

typedef struct
{
	hevcasm_sad *sad64x64;
	hevcasm_sad *sad64x32;
	hevcasm_sad *sad32x64;
	hevcasm_sad *sad32x32;
	hevcasm_sad *sad32x16;
	hevcasm_sad *sad16x32;
	hevcasm_sad *sad16x16;
	hevcasm_sad *sad16x8;
	hevcasm_sad *sad8x16;
	hevcasm_sad *sad8x8;
	hevcasm_sad *sad8x4;
	hevcasm_sad *sadGeneric;
}
hevcasm_table_sad;

static hevcasm_sad** hevcasm_get_sad(hevcasm_table_sad *table, int width, int height)
{
	switch (HEVCASM_RECT(width, height))
	{
	case HEVCASM_RECT(64, 64): return &table->sad64x64;
	case HEVCASM_RECT(64, 32): return &table->sad64x32;
	case HEVCASM_RECT(32, 64): return &table->sad32x64;
	case HEVCASM_RECT(32, 32): return &table->sad32x32;
	case HEVCASM_RECT(32, 16): return &table->sad32x16;
	case HEVCASM_RECT(16, 32): return &table->sad16x32;
	case HEVCASM_RECT(16, 16): return &table->sad16x16;
	case HEVCASM_RECT(16, 8): return &table->sad16x8;
	case HEVCASM_RECT(8, 16): return &table->sad8x16;
	case HEVCASM_RECT(8, 8): return &table->sad8x8;
	case HEVCASM_RECT(8, 4): return &table->sad8x4;
	default:;
	}
	return &table->sadGeneric;
}

void HEVCASM_API hevcasm_populate_sad(hevcasm_table_sad *table, hevcasm_instruction_set mask);

hevcasm_test_function hevcasm_test_sad;


/* Rectangular SAD (Sum of Absolute Differences) with multiple references */
typedef void hevcasm_sad_multiref(const uint8_t *src, ptrdiff_t stride_src, const uint8_t *ref[], ptrdiff_t stride_ref, int sad[], uint32_t rect);

typedef struct
{
	hevcasm_sad_multiref *sad64x64_4;
	hevcasm_sad_multiref *sad64x32_4;
	hevcasm_sad_multiref *sad32x64_4;
	hevcasm_sad_multiref *sad32x32_4;
	hevcasm_sad_multiref *sad32x16_4;
	hevcasm_sad_multiref *sad16x32_4;
	hevcasm_sad_multiref *sad16x16_4;
	hevcasm_sad_multiref *sad16x8_4;
	hevcasm_sad_multiref *sad8x16_4;
	hevcasm_sad_multiref *sad8x8_4;
	hevcasm_sad_multiref *sad8x4_4;
	hevcasm_sad_multiref *sadGeneric_4;
}
hevcasm_table_sad_multiref;

static hevcasm_sad_multiref** hevcasm_get_sad_multiref(hevcasm_table_sad_multiref *table, int ways, int width, int height)
{
	if (ways != 4) return 0;

	switch (HEVCASM_RECT(width, height))
	{
	case HEVCASM_RECT(64, 64): return &table->sad64x64_4;
	case HEVCASM_RECT(64, 32): return &table->sad64x32_4;
	case HEVCASM_RECT(32, 64): return &table->sad32x64_4;
	case HEVCASM_RECT(32, 32): return &table->sad32x32_4;
	case HEVCASM_RECT(32, 16): return &table->sad32x16_4;
	case HEVCASM_RECT(16, 32): return &table->sad16x32_4;
	case HEVCASM_RECT(16, 16): return &table->sad16x16_4;
	case HEVCASM_RECT(16, 8): return &table->sad16x8_4;
	case HEVCASM_RECT(8, 16): return &table->sad8x16_4;
	case HEVCASM_RECT(8, 8): return &table->sad8x8_4;
	case HEVCASM_RECT(8, 4): return &table->sad8x4_4;
	default:;
	}
	return &table->sadGeneric_4;
}

void HEVCASM_API hevcasm_populate_sad_multiref(hevcasm_table_sad_multiref *table, hevcasm_instruction_set mask);

hevcasm_test_function hevcasm_test_sad_multiref;


#ifdef __cplusplus
}
#endif


#endif
