/*
The copyright in this software is being made available under the BSD
License, included below. This software may be subject to other third party
and contributor rights, including patent rights, and no such rights are
granted under this license.


Copyright(c) 2011 - 2015, Parabola Research Limited
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

#include "ssd.h"
#include "hevcasm_test.h"

#include <stdlib.h>
#include <assert.h>


static int hevcasm_ssd_c_ref(const uint8_t *pA, ptrdiff_t strideA, const uint8_t *pB, ptrdiff_t strideB, int w, int h)
{
	int ssd = 0;
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			const int diff = pA[x + y * strideA] - pB[x + y * strideB];
			ssd += diff * diff;
		}
	}
	return ssd;
}


hevcasm_ssd hevcasm_ssd_16x16_avx;
hevcasm_ssd hevcasm_ssd_32x32_avx;
hevcasm_ssd hevcasm_ssd_64x64_avx;


void HEVCASM_API hevcasm_populate_ssd(hevcasm_table_ssd *table, hevcasm_instruction_set mask)
{
	*hevcasm_get_ssd(table, 2) = 0;
	*hevcasm_get_ssd(table, 3) = 0;
	*hevcasm_get_ssd(table, 4) = 0;
	*hevcasm_get_ssd(table, 5) = 0;
	*hevcasm_get_ssd(table, 6) = 0;

	if (mask & (HEVCASM_C_REF | HEVCASM_C_OPT))
	{
		*hevcasm_get_ssd(table, 2) = hevcasm_ssd_c_ref;
		*hevcasm_get_ssd(table, 3) = hevcasm_ssd_c_ref;
		*hevcasm_get_ssd(table, 4) = hevcasm_ssd_c_ref;
		*hevcasm_get_ssd(table, 5) = hevcasm_ssd_c_ref;
		*hevcasm_get_ssd(table, 6) = hevcasm_ssd_c_ref;
	}

	if (mask & HEVCASM_AVX)
	{
		*hevcasm_get_ssd(table, 4) = hevcasm_ssd_16x16_avx;
		*hevcasm_get_ssd(table, 5) = hevcasm_ssd_32x32_avx;
		*hevcasm_get_ssd(table, 6) = hevcasm_ssd_64x64_avx;
	}
}


typedef struct
{
	uint8_t *srcA, *srcB;
	int log2TrafoSize;
	int ssd;
	hevcasm_ssd *f;
}
bound_ssd;


int init_ssd(void *p, hevcasm_instruction_set mask)
{
	bound_ssd *s = p;

	hevcasm_table_ssd table;

	hevcasm_populate_ssd(&table, mask);

	s->f = *hevcasm_get_ssd(&table, s->log2TrafoSize);

	if (mask == HEVCASM_C_REF)
	{
		const int nCbS = 1 << s->log2TrafoSize;
		printf("\t%dx%d : ", nCbS, nCbS);
	}

	return !!s->f;
}


void invoke_ssd(void *p, int n)
{
	bound_ssd *s = p;
	const int nCbS = 1 << s->log2TrafoSize;
	while (n--)
	{
		s->ssd = s->f(s->srcA, 2*nCbS, s->srcB, 2*nCbS, nCbS, nCbS);
	}
}


int mismatch_ssd(void *boundRef, void *boundTest)
{
	bound_ssd *ref = boundRef;
	bound_ssd *test = boundTest;

	return ref->ssd != test->ssd;
}


void HEVCASM_API hevcasm_test_ssd(int *error_count, hevcasm_instruction_set mask)
{
	printf("\nhevcasm_ssd - Sum of Square Differences\n");

	uint8_t  srcA[64 * 96];
	uint8_t  srcB[64 * 96];

	for (int i = 0; i < 64 * 96; ++i)
	{
		srcA[i] = rand() & 0xff;
		srcB[i] = rand() & 0xff;
	}

	bound_ssd b[2];

	b[0].srcA = srcA;
	b[0].srcB = srcB;

	for (b[0].log2TrafoSize = 2; b[0].log2TrafoSize <= 6; ++b[0].log2TrafoSize)
	{
		b[1] = b[0];
		*error_count += hevcasm_test(&b[0], &b[1], init_ssd, invoke_ssd, mismatch_ssd, mask, 100000);
	}
}
