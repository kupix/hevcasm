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


//#include "pred_intra_a.h"
#include "pred_intra.h"
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



static uint8_t p(const uint8_t *neighbours, int dx, int dy)
{
	assert(dx == -1 || dy == -1);
	assert(dx != dy);

	return neighbours[dy < 0 ? 64 + dx : 63 - dy];
}


void HEVCASM_API hevcasm_pred_intra_dc_ref(uint8_t *dst, const uint8_t *neighbours, int intraPredMode, hevcasm_pred_intra_packed packed)
{
	const int k = (packed >> 1) & 0x7f;
	const int nTbS = 1 << k;

	int dcVal = nTbS;
	for (int x1 = 0; x1 < nTbS; ++x1)
	{
		dcVal += p(neighbours, x1, -1);
	}
	for (int y1 = 0; y1 < nTbS; ++y1)
	{
		dcVal += p(neighbours, -1, y1);
	}
	dcVal >>= (k + 1);

	const int filter_edge = (packed & 1);

	int start = 0;
	if (filter_edge)
	{
		start = 1;
		dst[0 + 0 * nTbS] = (p(neighbours, -1, 0) + 2 * dcVal + p(neighbours, 0, -1) + 2) >> 2;
		for (int x = 1; x<nTbS; ++x)
		{
			dst[x + 0*nTbS] = (p(neighbours, x, -1) + 3 * dcVal + 2) >> 2;
		}
		for (int y = 1; y<nTbS; ++y)
		{
			dst[0 + y*nTbS] = (p(neighbours, -1, y) + 3 * dcVal + 2) >> 2;
		}
	}

	for (int y = start; y<nTbS; ++y)
	{
		memset(&dst[start + y*nTbS], dcVal, nTbS - start);
	}
}


#ifdef WIN32
#define FASTCALL __fastcall
#else
#define FASTCALL
#endif


void FASTCALL f265_lbd_predict_intra_dc_8_avx2(uint8_t *dst, const uint8_t *neighbours, int intraPredMode, hevcasm_pred_intra_packed packed);


void hevcasm_populate_pred_intra(hevcasm_table_pred_intra *table, hevcasm_instruction_set mask)
{
	for (int k = 2; k <= 5; ++k)
	{
		for (int intraPredModeY = 0; intraPredModeY < 35; ++intraPredModeY)
		{
			*hevcasm_get_pred_intra(table, intraPredModeY, hevcasm_pred_intra_pack(0, k)) = 0;
			*hevcasm_get_pred_intra(table, intraPredModeY, hevcasm_pred_intra_pack(1, k)) = 0;
		}

		for (int cIdx = 0; cIdx < 2; ++cIdx)
		{
			hevcasm_pred_intra **entry = hevcasm_get_pred_intra(table, 1, hevcasm_pred_intra_pack(cIdx, k));
			*entry = 0;
			if (mask & HEVCASM_C_REF) *entry = hevcasm_pred_intra_dc_ref;
			if (mask & HEVCASM_C_OPT) *entry = hevcasm_pred_intra_dc_ref;
		}
	}

	hevcasm_pred_intra **entry = hevcasm_get_pred_intra(table, 1, hevcasm_pred_intra_pack(1, 3));
#if !defined(WIN32) || defined(_M_X64)
	if (mask & HEVCASM_AVX2) *entry = f265_lbd_predict_intra_dc_8_avx2;
#endif
}


typedef struct 
{
	hevcasm_pred_intra *f;
	HEVCASM_ALIGN(32, uint8_t, dst[64 * 64]);
	const uint8_t *neighbours;
	int intraPredMode;
	int packed;
}
bound_pred_intra;


static int get_pred_intra(void *p, hevcasm_instruction_set mask)
{
	bound_pred_intra *s = p;

	const char *lookup[35] = { 0, "DC", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	const char *name = lookup[s->intraPredMode];
	if (!name) return 0;
		
	hevcasm_table_pred_intra table;

	hevcasm_populate_pred_intra(&table, mask);

	s->f = *hevcasm_get_pred_intra(&table, s->intraPredMode, s->packed);

	if (s->f && mask == HEVCASM_C_REF)
	{
		const int k = (s->packed >> 1) & 0x7f;
		const int nTbS = 1 << k;
		printf("\t%dx%d %s", nTbS, nTbS, name);
		if (s->packed & 1)
		{
			printf(" edge");
		}
	}

	memset(s->dst, 0, 64 * 64);

	return !!s->f;
}


void invoke_pred_intra(void *p, int n)
{
	bound_pred_intra *s = p;
	while (n--)
	{
		s->f(s->dst, s->neighbours, s->intraPredMode, s->packed);
	}
}


int mismatch_pred_intra(void *boundRef, void *boundTest)
{
	bound_pred_intra *ref = boundRef;
	bound_pred_intra *test = boundTest;

	const int k = (ref->packed >> 1) & 0x7f;
	const int nTbS = 1 << k;

	return memcmp(ref->dst, test->dst, nTbS*nTbS);
}


void HEVCASM_API hevcasm_test_pred_intra(int *error_count, hevcasm_instruction_set mask)
{
	printf("\nhevcasm_pred_intra - Intra Prediction\n");

	bound_pred_intra b[2];

	HEVCASM_ALIGN(32, uint8_t, neighbours[256]);
	b[0].neighbours = neighbours + 128;

	for (int x = 0; x < 256; x++) neighbours[x] = rand() & 0xff;

	for (int k = 2; k <= 5; ++k)
	{
		b[0].packed = hevcasm_pred_intra_pack(1, k);
		for (b[0].intraPredMode = 1; b[0].intraPredMode < 2; ++b[0].intraPredMode)
		{
			b[1] = b[0];
			hevcasm_test(&b[0], &b[1], get_pred_intra, invoke_pred_intra, mismatch_pred_intra, mask, 1000);
		}
	}
}
