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

#include "sad_a.h"
#include "sad.h"
#include "hevcasm_test.h"
#include "vp9_rtcd.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>



static int hevcasm_sad_c_ref(const uint8_t *src, ptrdiff_t stride_src, const uint8_t *ref, ptrdiff_t stride_ref, uint32_t rect)
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


hevcasm_sad* get_sad(int width, int height, hevcasm_instruction_set mask)
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

	if (mask & (HEVCASM_C_REF | HEVCASM_C_OPT))
	{
		return (hevcasm_sad*)&hevcasm_sad_c_ref;
	}

	return 0;
}


void HEVCASM_API hevcasm_populate_sad(hevcasm_table_sad *table, hevcasm_instruction_set mask)
{
	for (int height = 4; height <= 64; height += 4)
	{
		for (int width = 4; width <= 64; width += 4)
		{
			*hevcasm_get_sad(table, width, height) = get_sad(width, height, mask);
		}
	}
}


static void hevcasm_sad_multiref_4_c_ref(const uint8_t *src, ptrdiff_t stride_src, const uint8_t *ref[], ptrdiff_t stride_ref, int sad[], uint32_t rect)
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

static void wrap(const uint8_t *src, ptrdiff_t stride_src, const uint8_t *ref[], ptrdiff_t stride_ref, int sad[], uint32_t rect)
{
	hevcasm_sad_multiref_4_12xh_avx2(src, stride_src, ref, stride_ref, sad, rect);
}

hevcasm_sad_multiref* get_sad_multiref(int ways, int width, int height, hevcasm_instruction_set mask)
{
	hevcasm_sad_multiref* f = 0;

	if (ways != 4) return 0;

	if (mask & (HEVCASM_C_REF | HEVCASM_C_OPT))
	{
		f = &hevcasm_sad_multiref_4_c_ref;
	}

	if (mask & HEVCASM_SSE2) switch (HEVCASM_RECT(width, height))
	{
	case HEVCASM_RECT(64, 64): f = (hevcasm_sad_multiref*)&vp9_sad64x64x4d_sse2; break;
	case HEVCASM_RECT(64, 32): f = (hevcasm_sad_multiref*)&vp9_sad64x32x4d_sse2; break;
	case HEVCASM_RECT(32, 64): f = (hevcasm_sad_multiref*)&vp9_sad32x64x4d_sse2; break;
	case HEVCASM_RECT(32, 32): f = (hevcasm_sad_multiref*)&vp9_sad32x32x4d_sse2; break;
	case HEVCASM_RECT(32, 16): f = (hevcasm_sad_multiref*)&vp9_sad32x16x4d_sse2; break;
	case HEVCASM_RECT(16, 32): f = (hevcasm_sad_multiref*)&vp9_sad16x32x4d_sse2; break;
	case HEVCASM_RECT(16, 16): f = (hevcasm_sad_multiref*)&vp9_sad16x16x4d_sse2; break;
	case HEVCASM_RECT(16, 8): f = (hevcasm_sad_multiref*)&vp9_sad16x8x4d_sse2; break;
	case HEVCASM_RECT(8, 16): f = (hevcasm_sad_multiref*)&vp9_sad8x16x4d_sse2; break;
	case HEVCASM_RECT(8, 8): f = (hevcasm_sad_multiref*)&vp9_sad8x8x4d_sse2; break;
	case HEVCASM_RECT(8, 4): f = (hevcasm_sad_multiref*)&vp9_sad8x4x4d_sse2; break;
	}

	if (mask & HEVCASM_AVX2) switch (width)
	{
	case 64: f = hevcasm_sad_multiref_4_64xh_avx2; break;
	case 48: f = hevcasm_sad_multiref_4_48xh_avx2; break;
	case 32: f = hevcasm_sad_multiref_4_32xh_avx2; break;
	case 24: f = hevcasm_sad_multiref_4_24xh_avx2; break;
	case 16: f = hevcasm_sad_multiref_4_16xh_avx2; break;
	case 12: f = hevcasm_sad_multiref_4_12xh_avx2; break;
	case 8: if (!f) f = hevcasm_sad_multiref_4_8xh_avx2; break;
	case 4: f = hevcasm_sad_multiref_4_4xh_avx2; break;
	}

	return f;
}


void HEVCASM_API hevcasm_populate_sad_multiref(hevcasm_table_sad_multiref *table, hevcasm_instruction_set mask)
{
	for (int height = 4; height <= 64; height += 4)
	{
		for (int width = 4; width <= 64; width += 4)
		{
			*hevcasm_get_sad_multiref(table, 4, width, height) = get_sad_multiref(4, width, height, mask);
		}
	}

}



typedef struct
{
	HEVCASM_ALIGN(32, uint8_t, src[128 * 128]);
	HEVCASM_ALIGN(32, uint8_t, ref[128 * 128]);
	hevcasm_sad *f;
	int width;
	int height;
	int sad;
} 
bound_sad;


int init_sad(void *p, hevcasm_instruction_set mask)
{
	bound_sad *s = p;

	hevcasm_table_sad table;
	hevcasm_populate_sad(&table, mask);

	s->f = *hevcasm_get_sad(&table, s->width, s->height);

	if (mask == HEVCASM_C_REF) printf("\t%dx%d:", s->width, s->height);

	return !!s->f;
}


void invoke_sad(void *p, int n)
{
	bound_sad *s = p;
	const uint8_t *unaligned_ref = &s->ref[1 + 1 * 128];
	while (n--)
	{
		s->sad = s->f(s->src, 64, unaligned_ref, 64, HEVCASM_RECT(s->width, s->height));
	}
}


int mismatch_sad(void *boundRef, void *boundTest)
{
	bound_sad *ref = boundRef;
	bound_sad *test = boundTest;

	return  ref->sad != test->sad;
}


static const int partitions[][2] = {
	{ 64, 64 },{ 64, 48 },{ 64, 32 },{ 64, 16 },
	{ 48, 64 },
	{ 32, 64 },{ 32, 32 },{ 32, 24 },{ 32, 16 }, {32, 8},
	{ 24, 32 },
	{ 16, 64 },{ 16, 32 },{ 16, 16 },{ 16, 12 },{ 16, 8 },{16, 4},
	{ 12, 16 },
	{ 8, 32 },{ 8, 16 },{ 8, 8 }, { 8, 4 },
	{ 4, 8 },
	{ 0, 0 } };


void HEVCASM_API hevcasm_test_sad(int *error_count, hevcasm_instruction_set mask)
{
	printf("\nhevcasm_sad - Sum of Absolute Differences\n");

	bound_sad b[2];

	for (int x = 0; x < 128 * 128; x++) b[0].src[x] = rand();
	for (int x = 0; x < 128 * 128; x++) b[0].ref[x] = rand();

	for (int i = 0; partitions[i][0]; ++i)
	{
		b[0].width = partitions[i][0];
		b[0].height = partitions[i][1];
		b[1] = b[0];
		*error_count += hevcasm_test(&b[0], &b[1], init_sad, invoke_sad, mismatch_sad, mask, 100000);
	}
}


typedef struct
{
	hevcasm_sad_multiref *f;
	int ways;
	int width;
	int height;
	HEVCASM_ALIGN(32, uint8_t, src[128 * 128]);
	HEVCASM_ALIGN(32, uint8_t, ref[128 * 128]);
	const uint8_t *ref_array[4];
	int sad[4];
} 
bound_sad_multiref;


int init_sad_multiref(void *p, hevcasm_instruction_set mask)
{
	bound_sad_multiref *s = p;

	hevcasm_table_sad_multiref table;
	hevcasm_populate_sad_multiref(&table, mask);
	s->f = *hevcasm_get_sad_multiref(&table, s->ways, s->width, s->height);

	if (s->f && mask == HEVCASM_C_REF)
	{
		printf("\t%d-way %dx%d : ", s->ways, s->width, s->height);
	}

	return !!s->f;
}


void invoke_sad_multiref(void *p, int n)
{
	bound_sad_multiref *s = p;
	while (n--)
	{
		s->f(s->src, 64, s->ref_array, 64, s->sad, HEVCASM_RECT(s->width, s->height));
	}
}


int mismatch_sad_multiref(void *boundRef, void *boundTest)
{
	bound_sad_multiref *ref = boundRef;
	bound_sad_multiref *test = boundTest;

	for (int i = 0; i < ref->ways; ++i)
	{
		if (ref->sad[i] != test->sad[i]) return 1;
	}

	return 0;
}


void HEVCASM_API hevcasm_test_sad_multiref(int *error_count, hevcasm_instruction_set mask)
{
	bound_sad_multiref b[2];

	b[0].ways = 4;

	printf("\nhevcasm_sad_multiref - Sum Of Absolute Differences with multiple references (%d candidate references)\n", b[0].ways);

	for (int x = 0; x < 128 * 128; x++) b[0].src[x] = rand();
	for (int x = 0; x < 128 * 128; x++) b[0].ref[x] = rand();

	b[0].ref_array[0] = &b[0].ref[1 + 2 * 128];
	b[0].ref_array[1] = &b[0].ref[2 + 1 * 128];
	b[0].ref_array[2] = &b[0].ref[3 + 2 * 128];
	b[0].ref_array[3] = &b[0].ref[2 + 3 * 128];

	for (int i = 0; partitions[i][0]; ++i)
	{
		b[0].width = partitions[i][0];
		b[0].height = partitions[i][1];
		b[1] = b[0];
		*error_count += hevcasm_test(&b[0], &b[1], init_sad_multiref, invoke_sad_multiref, mismatch_sad_multiref, mask, 1);
	}
}
