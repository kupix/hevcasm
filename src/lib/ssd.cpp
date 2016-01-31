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
#include "Jit.h"
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

#define ORDER(a, b, c, d) ((a << 6) | (b << 4) | (c << 2) | d)


struct Ssd
	:
	Jit::Function
{
	Ssd(Jit::Buffer *buffer, int width, int height)
		:
		Jit::Function(buffer, Jit::CountArguments<hevcasm_ssd>::value),
		width(width),
		height(height)
	{
		this->build();
	}

	int width, height;

	void assemble() override
	{
		auto &r0 = arg64(0);
		auto &r1 = arg64(1);
		auto &r2 = arg64(2);
		auto &r3 = arg64(3);
		auto &r4 = arg64(4);
		auto &r5 = arg64(5);

		if (width % 16 == 0)
		{
			auto &m0 = regXmm(0);
			auto &m1 = regXmm(1);
			auto &m2 = regXmm(2);
			auto &m3 = regXmm(3);
			auto &m4 = regXmm(4);
			auto &m5 = regXmm(5);

			vpxor(m0, m0);
			vpxor(m5, m5);

			L("loop");
			{
				for (int x = 0; x < width; x += 16)
				{
					vmovdqa(m1, ptr[r0 + x]);
					vmovdqa(m2, ptr[r2 + x]);
					vpunpckhbw(m3, m1, m5);
					vpunpckhbw(m4, m2, m5);
					vpunpcklbw(m1, m5);
					vpunpcklbw(m2, m5);
					vpsubw(m1, m2);
					vpsubw(m3, m4);
					vpmaddwd(m1, m1);
					vpmaddwd(m3, m3);
					vpaddd(m0, m1);
					vpaddd(m0, m3);
				}
				lea(r0, ptr[r0 + r1]);
				lea(r2, ptr[r2 + r3]);
			}
			dec(Xbyak::Reg32(r5.getIdx()));
			jg("loop");

			vpshufd(m1, m0, ORDER(3, 2, 3, 2));
			vpaddd(m0, m1); // Review: phaddd might be better here
			vpshufd(m1, m0, ORDER(3, 2, 0, 1));
			vpaddd(m0, m1);
			vmovd(eax, m0);
		}
	}
};



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

	static Jit::Buffer jitBuffer(100000);

	if (mask & HEVCASM_AVX)
	{
		{
			static Ssd ssd(&jitBuffer, 16, 16);
			*hevcasm_get_ssd(table, 4) = ssd;
		}
		{
			static Ssd ssd(&jitBuffer, 32, 32);
			*hevcasm_get_ssd(table, 5) = ssd;
		}
		{
			static Ssd ssd(&jitBuffer, 64, 64);
			*hevcasm_get_ssd(table, 6) = ssd;
		}
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
	bound_ssd *s = (bound_ssd *)p;

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
	bound_ssd *s = (bound_ssd *)p;
	const int nCbS = 1 << s->log2TrafoSize;
	while (n--)
	{
		s->ssd = s->f(s->srcA, 2*nCbS, s->srcB, 2*nCbS, nCbS, nCbS);
	}
}


int mismatch_ssd(void *boundRef, void *boundTest)
{
	bound_ssd *ref = (bound_ssd *)boundRef;
	bound_ssd *test = (bound_ssd *)boundTest;

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
