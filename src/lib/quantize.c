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

#include "quantize_a.h"
#include "quantize.h"
#include "hevcasm_test.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>


static int Clip3(int min, int max, int value)
{
	if (value < min) return min;
	if (value > max) return max;
	return value;
}


static void hevcasm_quantize_inverse_c_ref(int16_t *dst, const int16_t *src, int scale, int shift, int n)
{
	while (n--)
	{
		*dst++ = (int16_t)Clip3(
			-32768,
			32767,
			((*src++ * scale) + (1 << (shift - 1))) >> shift);
	}
}


static hevcasm_quantize_inverse * get_quantize_inverse(hevcasm_instruction_set mask)
{
	hevcasm_quantize_inverse *f = 0;
	
	if (mask & (HEVCASM_C_REF | HEVCASM_C_OPT)) f = hevcasm_quantize_inverse_c_ref;

	if (mask & HEVCASM_SSE41) f = hevcasm_quantize_inverse_sse4;

	return f;
}

void hevcasm_populate_quantize_inverse(hevcasm_table_quantize_inverse *table, hevcasm_instruction_set mask)
{
	table->p = get_quantize_inverse(mask);
}



typedef struct
{
	int16_t *src;
	HEVCASM_ALIGN(32, int16_t, dst[32*32]);
	hevcasm_quantize_inverse *f;
	int scale;
	int shift;
	int log2TrafoSize;
}
hevcasm_bound_quantize_inverse;


int init_quantize_inverse(void *p, hevcasm_instruction_set mask)
{
	hevcasm_bound_quantize_inverse *s = p;

	hevcasm_table_quantize_inverse table;

	hevcasm_populate_quantize_inverse(&table, mask);

	s->f = *hevcasm_get_quantize_inverse(&table);
	
	assert(s->f == get_quantize_inverse(mask));

	if (s->f && mask == HEVCASM_C_REF)
	{
		const int nCbS = 1 << s->log2TrafoSize;
		printf("\t%dx%d : ", nCbS, nCbS);
	}

	return !!s->f;
}


void invoke_quantize_inverse(void *p, int count)
{
	hevcasm_bound_quantize_inverse *s = (hevcasm_bound_quantize_inverse *)p;
	while (count--)
	{
		const int n = 1 << (2 * s->log2TrafoSize);
		s->f(s->dst, s->src, s->scale, s->shift, n);
	}
}


int mismatch_quantize_inverse(void *boundRef, void *boundTest)
{
	hevcasm_bound_quantize_inverse *ref = boundRef;
	hevcasm_bound_quantize_inverse *test = boundTest;

	const int nCbS = 1 << ref->log2TrafoSize;

	return memcmp(ref->dst, test->dst, nCbS * nCbS * sizeof(int16_t));
}


void HEVCASM_API hevcasm_test_quantize_inverse(int *error_count, hevcasm_instruction_set mask)
{
	printf("\nhevcasm_quantize_inverse - Inverse Quantization (\"scaling\")\n");

	HEVCASM_ALIGN(32, int16_t, src[32 * 32]);

	for (int x = 0; x < 32 * 32; ++x) src[x] = (rand() & 0xff) - 0x100;

	hevcasm_bound_quantize_inverse b[2];
	b[0].src = src;
	b[0].scale = 51;
	b[0].shift = 14;

	for (b[0].log2TrafoSize = 2; b[0].log2TrafoSize <= 5; ++b[0].log2TrafoSize)
	{
		b[1] = b[0];
		*error_count += hevcasm_test(&b[0], &b[1], init_quantize_inverse, invoke_quantize_inverse, mismatch_quantize_inverse, mask, 100000);
	}
}


static int hevcasm_quantize_c_ref(int16_t *dst, const int16_t *src, int scale, int shift, int offset, int n)
{
	assert(scale < 0x8000);
	assert(offset < 0x8000);
	assert(shift >= 16);
	assert(shift <= 27);

	offset <<= (shift - 16);
	assert(offset < 0x4000000);

	int cbf = 0;
	while (n--)
	{
		int x = *src++;
		int sign = x < 0 ? -1 : 1;

		x = abs(x);
		x = ((x * scale) + offset) >> shift;
		x *= sign;
		x = Clip3(-32768, 32767, x);

		cbf |= x;

		*dst++ = x;
	}
	return cbf;
}


static hevcasm_quantize * get_quantize(hevcasm_instruction_set mask)
{
	hevcasm_quantize *f = 0;

	if (mask & (HEVCASM_C_REF | HEVCASM_C_OPT)) f = hevcasm_quantize_c_ref;

	if (mask & HEVCASM_SSE41) f = hevcasm_quantize_sse4;

	return f;
}


void hevcasm_populate_quantize(hevcasm_table_quantize *table, hevcasm_instruction_set mask)
{
	table->p = get_quantize(mask);
}




typedef struct
{
	int16_t *src;
	HEVCASM_ALIGN(32, int16_t, dst[32 * 32]);
	hevcasm_quantize *f;
	int scale;
	int shift;
	int offset;
	int log2TrafoSize;
	int cbf;
}
hevcasm_bound_quantize;


int init_quantize(void *p, hevcasm_instruction_set mask)
{
	hevcasm_bound_quantize *s = p;
	hevcasm_table_quantize table;
	hevcasm_populate_quantize(&table, mask);
	s->f = *hevcasm_get_quantize(&table);
	assert(s->f == get_quantize(mask));
	if (mask == HEVCASM_C_REF)
	{
		const int nCbS = 1 << s->log2TrafoSize;
		printf("\t%dx%d : ", nCbS, nCbS);
	}
	return !!s->f;
}


void invoke_quantize(void *p, int iterations)
{
	hevcasm_bound_quantize *s = (hevcasm_bound_quantize *)p;
	while (iterations--)
	{
		const int n = 1 << (2 * s->log2TrafoSize);
		s->cbf = s->f(s->dst, s->src, s->scale, s->shift, s->offset, n);
	}
}


int mismatch_quantize(void *boundRef, void *boundTest)
{
	hevcasm_bound_quantize *ref = boundRef;
	hevcasm_bound_quantize *test = boundTest;

	const int n = 1 << (2 * ref->log2TrafoSize);

	if (!!ref->cbf != !!test->cbf) return 1;

	return memcmp(ref->dst, test->dst, n * sizeof(int16_t));
}


void HEVCASM_API hevcasm_test_quantize(int *error_count, hevcasm_instruction_set mask)
{
	printf("\nhevcasm_quantize - Quantization\n");

	HEVCASM_ALIGN(32, int16_t, src[32 * 32]);

	for (int x = 0; x < 32 * 32; ++x)
	{
		src[x] = rand() - rand();
	}

	hevcasm_bound_quantize b[2];

	b[0].src = src;
	b[0].scale = 51;
	b[0].shift = 20;
	b[0].offset = 14;

	for (b[0].log2TrafoSize = 2; b[0].log2TrafoSize <= 5; ++b[0].log2TrafoSize)
	{
		b[1] = b[0];
		*error_count += hevcasm_test(&b[0], &b[1], init_quantize, invoke_quantize, mismatch_quantize, mask, 100000);
	}
}





static void hevcasm_quantize_reconstruct_c_ref(uint8_t *rec, ptrdiff_t stride_rec, const uint8_t *predSamples, ptrdiff_t stride_pred, const int16_t *resSamples, int n)
{
	for (int y = 0; y < n; ++y)
	{
		for (int x = 0; x < n; ++x)
		{
			rec[x + y * stride_rec] = (uint8_t)Clip3(0, 255, predSamples[x + y * stride_pred] + resSamples[x + y * n]);
		}
	}

}


hevcasm_quantize_reconstruct * HEVCASM_API get_quantize_reconstruct(int log2TrafoSize, hevcasm_instruction_set mask)
{
	hevcasm_quantize_reconstruct *f = 0;
		
	if (mask & (HEVCASM_C_REF | HEVCASM_C_OPT)) f = hevcasm_quantize_reconstruct_c_ref;

	if (mask & HEVCASM_SSE41)
	{
		const int nCbS = 1 << log2TrafoSize;
		if (nCbS == 4) f = hevcasm_quantize_reconstruct_4x4_sse4;
		if (nCbS == 8) f = hevcasm_quantize_reconstruct_8x8_sse4;
		if (nCbS == 16) f = hevcasm_quantize_reconstruct_16x16_sse4;
		if (nCbS == 32) f = hevcasm_quantize_reconstruct_32x32_sse4;
	}

	return f;
}


void HEVCASM_API hevcasm_populate_quantize_reconstruct(hevcasm_table_quantize_reconstruct *table, hevcasm_instruction_set mask)
{
	for (int log2TrafoSize = 2; log2TrafoSize < 6; ++log2TrafoSize)
	{
		*hevcasm_get_quantize_reconstruct(table, log2TrafoSize) = get_quantize_reconstruct(log2TrafoSize, mask);
	}
}


typedef struct
{
	HEVCASM_ALIGN(32, uint8_t, rec[32 * 32]);
	ptrdiff_t stride_rec;
	const uint8_t *pred;
	ptrdiff_t stride_pred;
	const int16_t *res;
	int log2TrafoSize;
	hevcasm_quantize_reconstruct *f;
}
bound_quantize_reconstruct;


int init_quantize_reconstruct(void *p, hevcasm_instruction_set mask)
{
	bound_quantize_reconstruct *s = p;

	hevcasm_table_quantize_reconstruct table;

	hevcasm_populate_quantize_reconstruct(&table, mask);

	s->f = *hevcasm_get_quantize_reconstruct(&table, s->log2TrafoSize);

	assert(s->f == get_quantize_reconstruct(s->log2TrafoSize, mask));

	if (mask == HEVCASM_C_REF)
	{
		const int nCbS = 1 << s->log2TrafoSize;
		printf("\t%dx%d : ", nCbS, nCbS);
	}

	return !!s->f;
}


void invoke_quantize_reconstruct(void *p, int n)
{
	bound_quantize_reconstruct *s = p;
	while (n--)
	{
		const int nCbS = 1 << s->log2TrafoSize;
		s->f(s->rec, s->stride_rec, s->pred, s->stride_pred, s->res, nCbS);
	}
}

int mismatch_quantize_reconstruct(void *boundRef, void *boundTest)
{
	bound_quantize_reconstruct *ref = boundRef;
	bound_quantize_reconstruct *test = boundTest;

	const int nCbS = 1 << ref->log2TrafoSize;

	int mismatch = 0;
	for (int y = 0; y < nCbS; ++y)
	{
		mismatch |= memcmp(
			&ref->rec[y * ref->stride_rec],
			&test->rec[y * test->stride_rec],
			nCbS);
	}

	return mismatch;
}



void HEVCASM_API hevcasm_test_quantize_reconstruct(int *error_count, hevcasm_instruction_set mask)
{
	printf("\nhevcasm_quantize_reconstruct - Reconstruction\n");

	HEVCASM_ALIGN(32, uint8_t, pred[32 * 32]);
	HEVCASM_ALIGN(32, int16_t, res[32 * 32]);

	for (int x = 0; x < 32 * 32; ++x)
	{
		pred[x] = rand() & 0xff;
		res[x] = (rand() & 0x1ff) - 0x100;
	}

	bound_quantize_reconstruct b[2];

	b[0].stride_rec = 32;
	b[0].pred = pred;
	b[0].stride_pred = 32;
	b[0].res = res;

	for (b[0].log2TrafoSize = 2; b[0].log2TrafoSize <= 5; ++b[0].log2TrafoSize)
	{
		b[1] = b[0];
		*error_count += hevcasm_test(&b[0], &b[1], init_quantize_reconstruct, invoke_quantize_reconstruct, mismatch_quantize_reconstruct, mask, 100000);
	}
}
