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

static int hevcasm_vp9_sad_16xH_sse2(const uint8_t *src, ptrdiff_t stride_src, const uint8_t *ref, ptrdiff_t stride_ref, uint32_t rect)
{
	const int width = rect >> 8;
	const int height = rect & 0xff;
	assert(width == 16);
	switch (height)
	{
	case 8: return vp9_sad16x8_sse2(src, (int)stride_src, ref, (int)stride_ref);
	case 16: return vp9_sad16x16_sse2(src, (int)stride_src, ref, (int)stride_ref);
	default:;
	}
	return hevcasm_sad_c(src, stride_src, ref, stride_ref, rect);
}

static int hevcasm_vp9_sad_8xH_sse2(const uint8_t *src, ptrdiff_t stride_src, const uint8_t *ref, ptrdiff_t stride_ref, uint32_t rect)
{
	const int width = rect >> 8;
	const int height = rect & 0xff;
	assert(width == 8);
	switch (height)
	{
	case 4: return vp9_sad8x4_sse2(src, (int)stride_src, ref, (int)stride_ref);
	case 8: return vp9_sad8x8_sse2(src, (int)stride_src, ref, (int)stride_ref);
	case 16: return vp9_sad8x16_sse2(src, (int)stride_src, ref, (int)stride_ref);
	default:;
	}
	return hevcasm_sad_c(src, stride_src, ref, stride_ref, rect);
}

hevcasm_sad* hevcasm_get_sad(int width, hevcasm_instruction_set mask)
{
	if (mask & HEVCASM_SSE2)
	{
		switch (width)
		{
		case 16: return &hevcasm_vp9_sad_16xH_sse2;
		case 8: return &hevcasm_vp9_sad_8xH_sse2;
		}
	}

	if (mask & HEVCASM_C)
	{
		return &hevcasm_sad_c;
	}

	return 0;
}


typedef struct
{
	hevcasm_sad *f;
	uint32_t rect;
	HEVCASM_ALIGN(32, uint8_t, src[64 * 64]);
	HEVCASM_ALIGN(32, uint8_t, ref[64 * 64]);
	int sad;
} bound_sad;

void call_sad(void *p, int n)
{
	bound_sad *s = p;
	while (n--)
	{
		s->sad = s->f(s->src, 64, s->ref, 64, s->rect);
	}
}

static const int partitions[][2] = { 
	{ 16, 16 }, { 16, 8 }, { 8, 16 },
	{ 8, 8 }, { 8, 4 }, { 4, 8 },
	{ 0, 0 } };


int hevcasm_test_sad(hevcasm_instruction_set mask)
{
	int error_count = 0;
	printf("sad\n");

	bound_sad bound;

	for (int x = 0; x < 64 * 64; x++) bound.src[x] = rand();
	for (int x = 0; x < 64 * 64; x++) bound.ref[x] = rand();

	for (int i = 0; partitions[i][0]; ++i)
	{
		const int width = partitions[i][0];
		const int height = partitions[i][1];

		printf("\t%dx%d : ", width, height);

		bound.rect = hevcasm_rect(width, height);
		bound.f = hevcasm_get_sad(width, HEVCASM_C);
		call_sad(&bound, 1);
		const int sad_c = bound.sad;
		double first_result = 0.0;

		for (hevcasm_instruction_set_idx_t i = 0; i < HEVCASM_INSTRUCTION_SET_COUNT; ++i)
		{
			bound.f = hevcasm_get_sad(width, 1 << i);

			if (bound.f)
			{
				hevcasm_count_average_cycles(call_sad, &bound, &first_result, i, 100000);

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
