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

#include "sad.h"
#include "hevcasm_test.h"
#include "vp9_rtcd.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>


/* declaration for assembly functions in xxxx.asm */



static int hevcasm_sad_c(const uint8_t *src, ptrdiff_t stride_src, const uint8_t *ref, ptrdiff_t stride_ref, uint32_t rect)
{
	const int width = rect >> 8;
	const int height = rect & 0xff;
	int sad = 0;
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			sad += abs((int)src[x + y * stride_src] - (int)ref[x + y * stride_ref]);
		}
	}
	return sad;
}

hevcasm_sad* hevcasm_get_sad(int width, int height, hevcasm_instruction_set mask)
{
	if (mask & HEVCASM_SSE2) switch (HEVCASM_RECT(width, height))
	{
	case HEVCASM_RECT(64, 64): return (hevcasm_sad*)&vp9_sad64x64_sse2;
	case HEVCASM_RECT(64, 32): return (hevcasm_sad*)&vp9_sad64x32_sse2;
	case HEVCASM_RECT(32, 64): return (hevcasm_sad*)&vp9_sad32x64_sse2;
	case HEVCASM_RECT(32, 32): return (hevcasm_sad*)&vp9_sad32x32_sse2;
	case HEVCASM_RECT(32, 16): return (hevcasm_sad*)&vp9_sad32x16_sse2;
	case HEVCASM_RECT(16, 32): return (hevcasm_sad*)&vp9_sad16x32_sse2;
	case HEVCASM_RECT(16, 16): return (hevcasm_sad*)&vp9_sad16x16_sse2;
	case HEVCASM_RECT(16, 8): return (hevcasm_sad*)&vp9_sad16x8_sse2;
	case HEVCASM_RECT(8, 16): return (hevcasm_sad*)&vp9_sad8x16_sse2;
	case HEVCASM_RECT(8, 8): return (hevcasm_sad*)&vp9_sad8x8_sse2;
	case HEVCASM_RECT(8, 4): return (hevcasm_sad*)&vp9_sad8x4_sse2;
	}

	if (mask & HEVCASM_C)
	{
		return &hevcasm_sad_c;
	}

	return 0;
}

static void hevcasm_sad_multiref_4_c(const uint8_t *src, ptrdiff_t stride_src, const uint8_t *ref[], ptrdiff_t stride_ref, int sad[], uint32_t rect)
{
	const int width = rect >> 8;
	const int height = rect & 0xff;

	sad[0] = 0;
	sad[1] = 0;
	sad[2] = 0;
	sad[3] = 0;

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			for (int way = 0; way < 4; ++way)
			{
				sad[way] += abs((int)src[x + y * stride_src] - (int)ref[way][x + y * stride_ref]);
			}
		}
	}
}

hevcasm_sad_multiref* hevcasm_get_sad_multiref(int ways, int width, int height, hevcasm_instruction_set mask)
{
	if (ways != 4) return 0;

	if (mask & HEVCASM_SSE2) switch (HEVCASM_RECT(width, height))
	{
	case HEVCASM_RECT(64, 64): return (hevcasm_sad_multiref*)&vp9_sad64x64x4d_sse2;
	case HEVCASM_RECT(64, 32): return (hevcasm_sad_multiref*)&vp9_sad64x32x4d_sse2;
	case HEVCASM_RECT(32, 64): return (hevcasm_sad_multiref*)&vp9_sad32x64x4d_sse2;
	case HEVCASM_RECT(32, 32): return (hevcasm_sad_multiref*)&vp9_sad32x32x4d_sse2;
	case HEVCASM_RECT(32, 16): return (hevcasm_sad_multiref*)&vp9_sad32x16x4d_sse2;
	case HEVCASM_RECT(16, 32): return (hevcasm_sad_multiref*)&vp9_sad16x32x4d_sse2;
	case HEVCASM_RECT(16, 16): return (hevcasm_sad_multiref*)&vp9_sad16x16x4d_sse2;
	case HEVCASM_RECT(16, 8): return (hevcasm_sad_multiref*)&vp9_sad16x8x4d_sse2;
	case HEVCASM_RECT(8, 16): return (hevcasm_sad_multiref*)&vp9_sad8x16x4d_sse2;
	case HEVCASM_RECT(8, 8): return (hevcasm_sad_multiref*)&vp9_sad8x8x4d_sse2;
	case HEVCASM_RECT(8, 4): return (hevcasm_sad_multiref*)&vp9_sad8x4x4d_sse2;
	}

	if (mask & HEVCASM_C)
	{
		return &hevcasm_sad_multiref_4_c;
	}

	return 0;
}


typedef struct
{
	hevcasm_sad *f;
	uint32_t rect;
	HEVCASM_ALIGN(32, uint8_t, src[128 * 128]);
	HEVCASM_ALIGN(32, uint8_t, ref[128 * 128]);
	int sad;
} bound_sad;

void call_sad(void *p, int n)
{
	bound_sad *s = p;
	const uint8_t *unaligned_ref = &s->ref[1 + 1 * 128];
	while (n--)
	{

		s->sad = s->f(s->src, 64, unaligned_ref, 64, s->rect);
	}
}

static const int partitions[][2] = {
	{ 64, 64 }, { 64, 32 },
	{ 32, 64 }, { 32, 32 }, { 32, 16 },
	{ 16, 32 }, { 16, 16 }, { 16, 8 },
	{ 8, 16 }, { 8, 8 }, { 8, 4 }, 
	{ 4, 8 },
	{ 0, 0 } };

int hevcasm_test_sad(hevcasm_instruction_set mask)
{
	int error_count = 0;
	printf("sad\n");

	bound_sad bound;

	for (int x = 0; x < 128 * 128; x++) bound.src[x] = rand();
	for (int x = 0; x < 128 * 128; x++) bound.ref[x] = rand();

	for (int i = 0; partitions[i][0]; ++i)
	{
		const int width = partitions[i][0];
		const int height = partitions[i][1];

		printf("\t%dx%d : ", width, height);

		bound.rect = HEVCASM_RECT(width, height);
		bound.f = hevcasm_get_sad(width, height, HEVCASM_C);
		call_sad(&bound, 1);
		const int sad_c = bound.sad;
		double first_result = 0.0;

		for (hevcasm_instruction_set_idx_t i = 0; i < HEVCASM_INSTRUCTION_SET_COUNT; ++i)
		{
			bound.f = hevcasm_get_sad(width, height, 1 << i);

			if (bound.f)
			{
				hevcasm_count_average_cycles(call_sad, &bound, &first_result, i, 25600000 / (width * height));

				const int mismatch = bound.sad != sad_c;
				if (mismatch)
				{
					printf("-MISMATCH ");
					++error_count;
				}
			}
		}
		printf("\n");
	}
	printf("\n");
	return error_count;
}

typedef struct
{
	hevcasm_sad_multiref *f;
	uint32_t rect;
	HEVCASM_ALIGN(32, uint8_t, src[128 * 128]);
	HEVCASM_ALIGN(32, uint8_t, ref[128 * 128]);
	uint8_t *ref_array[4];
	int sad[4];
} bound_sad_multiref;

void call_sad_multiref(void *p, int n)
{
	bound_sad_multiref *s = p;
	while (n--)
	{
		s->f(s->src, 64, s->ref_array, 64, s->sad, s->rect);
	}
}

int hevcasm_test_sad_multiref(hevcasm_instruction_set mask)
{
	int error_count = 0;

	const int ways = 4;
	printf("%d-way sad\n", ways);

	bound_sad_multiref bound;

	for (int x = 0; x < 128 * 128; x++) bound.src[x] = rand();
	for (int x = 0; x < 128 * 128; x++) bound.ref[x] = rand();

	bound.ref_array[0] = &bound.ref[1 + 2 * 128];
	bound.ref_array[1] = &bound.ref[2 + 1 * 128];
	bound.ref_array[2] = &bound.ref[3 + 2 * 128];
	bound.ref_array[3] = &bound.ref[2 + 3 * 128];

	for (int i = 0; partitions[i][0]; ++i)
	{
		const int width = partitions[i][0];
		const int height = partitions[i][1];

		printf("\t%dx%d : ", width, height);

		bound.rect = HEVCASM_RECT(width, height);
		bound.f = hevcasm_get_sad_multiref(ways, width, height, HEVCASM_C);
		call_sad_multiref(&bound, 1);

		int sad_c[4];
		for (int way = 0; way < ways; ++way) sad_c[way] = bound.sad[way];

		double first_result = 0.0;

		for (hevcasm_instruction_set_idx_t i = 0; i < HEVCASM_INSTRUCTION_SET_COUNT; ++i)
		{
			bound.f = hevcasm_get_sad_multiref(ways, width, height, 1 << i);

			if (bound.f)
			{
				hevcasm_count_average_cycles(call_sad_multiref, &bound, &first_result, i, 25600000 / (width * height));

				const int mismatch =
					bound.sad[0] != sad_c[0] ||
					bound.sad[1] != sad_c[1] ||
					bound.sad[2] != sad_c[2] ||
					bound.sad[3] != sad_c[3];

				if (mismatch)
				{
					printf("-MISMATCH ");
					++error_count;
				}
			}
		}
		printf("\n");
	}
	printf("\n");
	return error_count;
}
