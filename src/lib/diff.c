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

#include "diff_a.h"
#include "diff.h"
#include "hevcasm_test.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>


static int hevcasm_ssd_c_ref(const uint8_t *p1, const uint8_t *p2, int n)
{
	int sum = 0;
	for (int i = 0; i<n; ++i)
	{
		const int diff = p1[i] - p2[i];
		sum += diff * diff;
	}
	return sum;
}


hevcasm_ssd * HEVCASM_API hevcasm_get_ssd(int size, hevcasm_instruction_set mask)
{
	hevcasm_ssd *f = 0;
	
	if (mask & (HEVCASM_C_REF | HEVCASM_C_OPT)) f = hevcasm_ssd_c_ref;

	if (mask & HEVCASM_AVX) f = hevcasm_ssd_avx;

	return f;
}


#define BLOCK_SIZE 0x200


typedef struct
{
	HEVCASM_ALIGN(32, uint8_t, data[2][BLOCK_SIZE]);
	hevcasm_ssd *f;
	int ssd;
}
bound_ssd;


int get_ssd(void *p, hevcasm_instruction_set mask)
{
	bound_ssd *s = p;

	s->f = hevcasm_get_ssd(BLOCK_SIZE, mask);

	if (mask == HEVCASM_C_REF) printf("\t%d:", BLOCK_SIZE);

	return !!s->f;
}


void invoke_ssd(void *p, int n)
{
	bound_ssd *s = p;
	while (n--)
	{
		s->ssd = s->f(s->data[0], s->data[1], BLOCK_SIZE);
	}
}


int mismatch_ssd(void *boundRef, void *boundTest)
{
	bound_ssd *ref = boundRef;
	bound_ssd *test = boundTest;

	return  ref->ssd != test->ssd;
}


void HEVCASM_API hevcasm_test_ssd(int *error_count, hevcasm_instruction_set mask)
{
	printf("\nhevcasm_ssd - Sum of Square Differences\n");

	bound_ssd b[2];

	for (int n = 0; n < 2; ++n)
	{
		for (int x = 0; x < BLOCK_SIZE; ++x)
		{
			b[0].data[n][x] = rand();
		}
	}

	b[1] = b[0];

	*error_count += hevcasm_test(&b[0], &b[1], get_ssd, invoke_ssd, mismatch_ssd, mask, 100000);
}
