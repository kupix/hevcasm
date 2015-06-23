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

#include "hadamard.h"
#include "hevcasm_test.h"

#include <stdlib.h>
#include <assert.h>




static void hadamard_iteration(int m, int n, int *dst, int *src, ptrdiff_t stride)
{
	for (int i = 0; i < m; i += 2 * n)
	{
		for (int j = 0; j < n; ++j)
		{
			int a = src[(i + j)*stride];
			int b = src[(i + n + j)*stride];
			dst[i + j] = a + b;
			dst[i + n + j] = a - b;
		}
	}
}

static void hadamard_transform(int m, int n, int *dst, int *src, ptrdiff_t stride)
{
	if (n == 1)
	{
		hadamard_iteration(m, n, dst, src, stride);
	}
	else
	{
		assert(m <= 8);
		int temp[8];
		hadamard_iteration(m, n, temp, src, stride);
		hadamard_transform(m, n / 2, dst, temp, 1);
	}
}


static int compute_satd(int n, const uint8_t *pA, ptrdiff_t strideA, const uint8_t *pB, ptrdiff_t strideB)
{
	assert(n <= 8);

	int intermediate[8][8];

	// subtraction and horizontal transform
	for (int y = 0; y < n; ++y)
	{
		int diff[8];
		for (int x = 0; x < n; ++x)
		{
			diff[x] = pA[x] - pB[x];
		}

		hadamard_transform(n, n / 2, intermediate[y], diff, 1);

		pA += strideA;
		pB += strideB;
	}
	
	// vertical transform and sum of absolutes
	const int roundingOffset = n / 4;
	int sad = roundingOffset;
	for (int x = 0; x < n; ++x)
	{
		int transformed[8];

		hadamard_transform(n, n / 2, transformed, &intermediate[0][x], 8);

		for (int y = 0; y < n; ++y)
		{
			sad += abs(transformed[y]);
		}
	}
	return sad / (n / 2);
}


static int compute_satd_2x2(const uint8_t *pA, ptrdiff_t strideA, const uint8_t *pB, ptrdiff_t strideB)
{
	return compute_satd(2, pA, strideA, pB, strideB);
}


static int compute_satd_4x4(const uint8_t *pA, ptrdiff_t strideA, const uint8_t *pB, ptrdiff_t strideB)
{
	return compute_satd(4, pA, strideA, pB, strideB);
}


static int compute_satd_8x8(const uint8_t *pA, ptrdiff_t strideA, const uint8_t *pB, ptrdiff_t strideB)
{
	return compute_satd(8, pA, strideA, pB, strideB);
}


#ifdef HEVCASM_X64
hevcasm_hadamard_satd hevcasm_hadamard_satd_4x4_sse2;
hevcasm_hadamard_satd hevcasm_hadamard_satd_8x8_avx2;
#endif

void HEVCASM_API hevcasm_populate_hadamard_satd(hevcasm_table_hadamard_satd *table, hevcasm_instruction_set mask)
{
	*hevcasm_get_hadamard_satd(table, 1) = 0;
	*hevcasm_get_hadamard_satd(table, 2) = 0;
	*hevcasm_get_hadamard_satd(table, 3) = 0;

	if (mask & (HEVCASM_C_REF | HEVCASM_C_OPT))
	{
		*hevcasm_get_hadamard_satd(table, 1) = compute_satd_2x2;
		*hevcasm_get_hadamard_satd(table, 2) = compute_satd_4x4;
		*hevcasm_get_hadamard_satd(table, 3) = compute_satd_8x8;
	}

#ifdef HEVCASM_X64
	if (mask & HEVCASM_SSE2)
	{
		*hevcasm_get_hadamard_satd(table, 2) = hevcasm_hadamard_satd_4x4_sse2;
	}

	if (mask & HEVCASM_AVX2)
	{
		*hevcasm_get_hadamard_satd(table, 3) = hevcasm_hadamard_satd_8x8_avx2;
	}
#endif
}


typedef struct
{
	uint8_t *srcA, *srcB;
	int log2TrafoSize;
	int satd;
	hevcasm_hadamard_satd *f;
}
bound_hadamard_satd;


int init_hadamard_satd(void *p, hevcasm_instruction_set mask)
{
	bound_hadamard_satd *s = p;

	hevcasm_table_hadamard_satd table;

	hevcasm_populate_hadamard_satd(&table, mask);

	s->f = *hevcasm_get_hadamard_satd(&table, s->log2TrafoSize);

	if (mask == HEVCASM_C_REF)
	{
		const int nCbS = 1 << s->log2TrafoSize;
		printf("\t%dx%d : ", nCbS, nCbS);
	}

	return !!s->f;
}


void invoke_hadamard_satd(void *p, int n)
{
	bound_hadamard_satd *s = p;
	const int nCbS = 1 << s->log2TrafoSize;
	while (n--)
	{
		s->satd = s->f(s->srcA, 2*nCbS, s->srcB, 2*nCbS);
	}
}


int mismatch_hadamard_satd(void *boundRef, void *boundTest)
{
	bound_hadamard_satd *ref = boundRef;
	bound_hadamard_satd *test = boundTest;

	return ref->satd != test->satd;
}


void HEVCASM_API hevcasm_test_hadamard_satd(int *error_count, hevcasm_instruction_set mask)
{
	printf("\nhevcasm_hadamard_satd - Hadamard Sum of Absolute Transformed Differences\n");

	uint8_t  srcA[16 * 8];
	uint8_t  srcB[16 * 8];

	for (int i = 0; i < 16 * 8; ++i)
	{
		srcA[i] = rand() & 0xff;
		srcB[i] = rand() & 0xff;
	}

	bound_hadamard_satd b[2];

	b[0].srcA = srcA;
	b[0].srcB = srcB;

	for (b[0].log2TrafoSize = 3; b[0].log2TrafoSize >= 1; --b[0].log2TrafoSize)
	{
		b[1] = b[0];
		*error_count += hevcasm_test(&b[0], &b[1], init_hadamard_satd, invoke_hadamard_satd, mismatch_hadamard_satd, mask, 100000);
	}
}
