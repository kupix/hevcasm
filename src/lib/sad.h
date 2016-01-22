/*
The copyright in this software is being made available under the BSD
License, included below. This software may be subject to other third party
and contributor rights, including patent rights, and no such rights are
granted under this license.


Copyright(c) 2011 - 2016, Parabola Research Limited
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
#include <functional>


#ifdef __cplusplus
extern "C"
{
#endif

#define X_HEVC_PU_SIZES \
	X(64, 64); \
	X(64, 48); \
	X(64, 32); \
	X(64, 16); \
	X(48, 64); \
	X(32, 64); \
	X(32, 32); \
	X(32, 24); \
	X(32, 16); \
	X(32, 8); \
	X(24, 32); \
	X(16, 64); \
	X(16, 32); \
	X(16, 16); \
	X(16, 12); \
	X(16, 8); \
	X(16, 4); \
	X(12, 16); \
	X(8, 32); \
	X(8, 16); \
	X(8, 8); \
	X(8, 4); \
	X(4, 8); \


//typedef std::function<int(const uint8_t * /*src*/, ptrdiff_t /*stride_src*/, const uint8_t * /*ref*/, ptrdiff_t /*stride_ref*/, uint32_t /*rect*/)> hevcasm_sad;

/* Rectangular SAD (Sum of Absolute Differences) with single reference */
typedef int hevcasm_sad(const uint8_t *src, ptrdiff_t stride_src, const uint8_t *ref, ptrdiff_t stride_ref, uint32_t rect);





typedef struct
{
#define X(w, h) \
	hevcasm_sad *sad ## w ## x ## h;
	
	X_HEVC_PU_SIZES
#undef X

	hevcasm_sad *sadGeneric;
}
hevcasm_table_sad;

static hevcasm_sad** hevcasm_get_sad(hevcasm_table_sad *table, int width, int height)
{
	switch (HEVCASM_RECT(width, height))
	{
#define X(w, h) \
	case HEVCASM_RECT(w, h): return &table->sad ## w ## x ## h;

	X_HEVC_PU_SIZES
#undef X
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
	hevcasm_sad_multiref *lookup[16][16];
	hevcasm_sad_multiref *sadGeneric_4;
}
hevcasm_table_sad_multiref;

static hevcasm_sad_multiref** hevcasm_get_sad_multiref(hevcasm_table_sad_multiref *table, int ways, int width, int height)
{
	if (ways != 4) return 0;
	return &table->lookup[(width>>2)-1][(height>>2)-1];
}

void HEVCASM_API hevcasm_populate_sad_multiref(hevcasm_table_sad_multiref *table, hevcasm_instruction_set mask);

hevcasm_test_function hevcasm_test_sad_multiref;


#ifdef __cplusplus
}
#endif


#endif
