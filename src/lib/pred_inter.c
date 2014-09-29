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

#include "pred_inter.h"
#include "hevcasm_test.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>


/* declaration for assembly functions in pred_inter_a.asm */
// todo: move to pred_inter_a.h

void hevcasm_pred_uni_copy_8to8_16xh_sse2(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_copy_8to8_32xh_sse2(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_copy_8to8_48xh_sse2(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_copy_8to8_64xh_sse2(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);

void hevcasm_pred_uni_8tap_8to8_h_16xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_8tap_8to8_h_32xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_8tap_8to8_h_48xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_8tap_8to8_h_64xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_4tap_8to8_h_16xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_4tap_8to8_h_32xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);

void hevcasm_pred_uni_8tap_8to16_h_16xh_sse4(int16_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_8tap_8to16_h_32xh_sse4(int16_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_8tap_8to16_h_48xh_sse4(int16_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_8tap_8to16_h_64xh_sse4(int16_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_4tap_8to16_h_16xh_sse4(int16_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_4tap_8to16_h_32xh_sse4(int16_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);

void hevcasm_pred_uni_8tap_8to8_v_8xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_8tap_8to8_v_16xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_8tap_8to8_v_24xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_8tap_8to8_v_32xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_8tap_8to8_v_48xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_8tap_8to8_v_64xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);

void hevcasm_pred_uni_8tap_16to8_v_16xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_8tap_16to8_v_32xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_8tap_16to8_v_48xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_8tap_16to8_v_64xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);

void hevcasm_pred_bi_v_8tap_16to16_16xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *refAtop, const int16_t *refBtop, ptrdiff_t stride_ref, int nPbW, int nPbH, int yFracA, int yFracB);
void hevcasm_pred_bi_v_8tap_16to16_32xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *refAtop, const int16_t *refBtop, ptrdiff_t stride_ref, int nPbW, int nPbH, int yFracA, int yFracB);
void hevcasm_pred_bi_v_8tap_16to16_48xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *refAtop, const int16_t *refBtop, ptrdiff_t stride_ref, int nPbW, int nPbH, int yFracA, int yFracB);
void hevcasm_pred_bi_v_8tap_16to16_64xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *refAtop, const int16_t *refBtop, ptrdiff_t stride_ref, int nPbW, int nPbH, int yFracA, int yFracB);

void hevcasm_pred_uni_4tap_8to8_v_8xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_4tap_8to8_v_16xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_4tap_8to8_v_24xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_4tap_8to8_v_32xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);

void hevcasm_pred_uni_4tap_16to8_v_16xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
void hevcasm_pred_uni_4tap_16to8_v_32xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);

void hevcasm_pred_bi_v_4tap_16to16_8xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *refAtop, const int16_t *refBtop, ptrdiff_t stride_ref, int nPbW, int nPbH, int yFracA, int yFracB);
void hevcasm_pred_bi_v_4tap_16to16_16xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *refAtop, const int16_t *refBtop, ptrdiff_t stride_ref, int nPbW, int nPbH, int yFracA, int yFracB);
void hevcasm_pred_bi_v_4tap_16to16_32xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *refAtop, const int16_t *refBtop, ptrdiff_t stride_ref, int nPbW, int nPbH, int yFracA, int yFracB);

void hevcasm_pred_bi_8to8_copy_16xh_sse2(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref0, const uint8_t *ref1, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac0, int yFrac0, int xFrac1, int yFrac1);
void hevcasm_pred_bi_8to8_copy_32xh_sse2(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref0, const uint8_t *ref1, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac0, int yFrac0, int xFrac1, int yFrac1);
void hevcasm_pred_bi_8to8_copy_48xh_sse2(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref0, const uint8_t *ref1, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac0, int yFrac0, int xFrac1, int yFrac1);
void hevcasm_pred_bi_8to8_copy_64xh_sse2(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref0, const uint8_t *ref1, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac0, int yFrac0, int xFrac1, int yFrac1);



static int Clip3(int min, int max, int value)
{
	if (value < min) return min;
	if (value > max) return max;
	return value;
}


int hevcasm_pred_coefficient(int n, int fractionalPosition, int k)
{
	if (n == 8)
	{
		static const int kernel[4][8] =
		{
			{ 0, 0, 0, 64, 0, 0, 0, 0 },
			{ -1, 4, -10, 58, 17, -5, 1, 0 },
			{ -1, 4, -11, 40, 40, -11, 4, -1 },
			{ 0, 1, -5, 17, 58, -10, 4, -1 }
		};

		return kernel[fractionalPosition][k];
	}
	else
	{
		static const int kernel[8][4] =
		{
			{ 0, 64, 0, 0 },
			{ -2, 58, 10, -2 },
			{ -4, 54, 16, -2 },
			{ -6, 46, 28, -4 },
			{ -4, 36, 36, -4 },
			{ -4, 28, 46, -6 },
			{ -2, 16, 54, -4 },
			{ -2, 10, 58, -2 },
		};

		return kernel[fractionalPosition][k];
	}
};


/* 
Generic function to handle all luma, chroma, 8 and 16-bit interpolation. 
Consider this a reference implementation: it will not run fast.
*/
static void hevcasm_pred_uni_filter_generic(
	void *dst, int sizeof_dst, ptrdiff_t stride_dst, 
	const void *src, int sizeof_src, ptrdiff_t stride_src, 
	int w, int h, 
	ptrdiff_t stride_tap, 
	int n, int fractionalPosition, int shift, int add)
{
	int16_t *dst16 = dst;
	uint8_t *dst8 = dst;

	const int16_t *src16 = src;
	const uint8_t *src8 = src;

	while (h--)
	{
		for (int x = 0; x<w; ++x)
		{
			int a = add << shift >> 1;

			for (int k = 0; k<n; ++k)
			{
				const ptrdiff_t src_offset = x + (k - n / 2 + 1) * stride_tap;

				const int srcSample = sizeof_src == 2 
					? src16[src_offset]
					: src8[src_offset];

				a += hevcasm_pred_coefficient(n, fractionalPosition, k) * srcSample;
			}

			a >>= shift;

			if (sizeof_dst == 2)
			{
				dst16[x] = a;
			}
			else
			{
				dst8[x] = Clip3(0, 255, a);
			}
		}

		dst16 += stride_dst;
		dst8 += stride_dst;

		src16 += stride_src;
		src8 += stride_src;
	}
};


void hevcasm_pred_uni_copy_block(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int w, int h, int xFrac, int yFrac)
{
	assert(!xFrac);
	assert(!yFrac);
	while (h--)
	{
		memcpy(dst, ref, w);
		dst += stride_dst;
		ref += stride_ref;
	}
}

void hevcasm_pred_uni_8tap_filter_8to8_h(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int w, int h, int xFrac, int yFrac)
{
	assert(xFrac);
	assert(!yFrac);
	hevcasm_pred_uni_filter_generic(dst, 1, stride_dst, ref, 1, stride_ref, w, h, 1, 8, xFrac, 6, 1);
}


void hevcasm_pred_uni_8tap_filter_8to8_v(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int w, int h, int xFrac, int yFrac)
{
	assert(!xFrac);
	assert(yFrac);
	hevcasm_pred_uni_filter_generic(dst, 1, stride_dst, ref, 1, stride_ref, w, h, stride_ref, 8, yFrac, 6, 1);
}


void hevcasm_pred_uni_8tap_filter_8to8_hv(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int w, int h, int xFrac, int yFrac)
{
	int16_t intermediate[(64 + 7) * 64];

	/* Horizontal filter */
	hevcasm_pred_uni_filter_generic(intermediate, 2, 64, ref - 3 * stride_ref, 1, stride_ref, w, h + 7, 1, 8, xFrac, 0, 0);

	/* Vertical filter */
	hevcasm_pred_uni_filter_generic(dst, 1, stride_dst, intermediate + 3 * 64, 2, 64, w, h, 64, 8, yFrac, 12, 1);
}


#define MAKE_hevcasm_pred_uni_xtap_8to8_hv(taps, suffix) \
 \
 static void hevcasm_pred_uni_ ## taps ## tap_8to8_hv ## suffix(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac) \
{ \
	HEVCASM_ALIGN(32, int16_t, intermediate[(64 + taps - 1) * 64]); \
	 \
	/* Horizontal filter */ \
	hevcasm_pred_uni_ ## taps ## tap_8to16_h ## suffix(intermediate, 64, ref - (taps/2-1) * stride_ref, stride_ref, nPbW, nPbH + taps - 1, xFrac, 0);	\
	 \
	/* Vertical filter */	 \
	hevcasm_pred_uni_ ## taps ## tap_16to8_v ## suffix(dst, stride_dst, intermediate + (taps/2-1) * 64, 64, nPbW, nPbH, 0, yFrac); \
} \

MAKE_hevcasm_pred_uni_xtap_8to8_hv(8, _16xh_sse4)
MAKE_hevcasm_pred_uni_xtap_8to8_hv(8, _32xh_sse4)
MAKE_hevcasm_pred_uni_xtap_8to8_hv(8, _48xh_sse4)
MAKE_hevcasm_pred_uni_xtap_8to8_hv(8, _64xh_sse4)
MAKE_hevcasm_pred_uni_xtap_8to8_hv(4, _16xh_sse4)
MAKE_hevcasm_pred_uni_xtap_8to8_hv(4, _32xh_sse4)

void hevcasm_pred_uni_4tap_filter_8to8_h(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int w, int h, int xFrac, int yFrac)
{
	assert(xFrac);
	assert(!yFrac);
	hevcasm_pred_uni_filter_generic(dst, 1, stride_dst, ref, 1, stride_ref, w, h, 1, 4, xFrac, 6, 1);
}


void hevcasm_pred_uni_4tap_filter_8to8_v(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int w, int h, int xFrac, int yFrac)
{
	assert(!xFrac);
	assert(yFrac);
	hevcasm_pred_uni_filter_generic(dst, 1, stride_dst, ref, 1, stride_ref, w, h, stride_ref, 4, yFrac, 6, 1);
}


void hevcasm_pred_uni_4tap_filter_8to8_hv(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int w, int h, int xFrac, int yFrac)
{
	int16_t intermediate[(32 + 3) * 32];

	/* Horizontal filter */
	hevcasm_pred_uni_filter_generic(intermediate, 2, 32, ref - stride_ref, 1, stride_ref, w, h + 3, 1, 4, xFrac, 0, 0);

	/* Vertical filter */
	hevcasm_pred_uni_filter_generic(dst, 1, stride_dst, intermediate + 32, 2, 32, w, h, 32, 4, yFrac, 12, 1);
}


hevcasm_pred_uni_filter_8to8* HEVCASM_API hevcasm_get_pred_uni_filter_8to8(int taps, int w, int h, int xFrac, int yFrac, hevcasm_instruction_set mask)
{
	hevcasm_pred_uni_filter_8to8 *f = 0;
	if (mask & HEVCASM_C_OPT)
	{
		if (taps == 8)
		{
			/* luma 8-tap filter */

			if (xFrac)
			{
				if (yFrac)
				{
					f = hevcasm_pred_uni_8tap_filter_8to8_hv;
				}
				else
				{
					f = hevcasm_pred_uni_8tap_filter_8to8_h;
				}
			}
			else
			{
				if (yFrac)
				{
					f = hevcasm_pred_uni_8tap_filter_8to8_v;
				}
				else
				{
					f = hevcasm_pred_uni_copy_block;
				}
			}
		}
		else
		{
			assert(taps == 4);
			/* chroma 4-tap filter */

			if (xFrac)
			{
				if (yFrac)
				{
					f = hevcasm_pred_uni_4tap_filter_8to8_hv;
				}
				else
				{
					f = hevcasm_pred_uni_4tap_filter_8to8_h;
				}
			}
			else
			{
				if (yFrac)
				{
					f = hevcasm_pred_uni_4tap_filter_8to8_v;
				}
				else
				{
					f = hevcasm_pred_uni_copy_block;
				}
			}
		}
	}
	
	if (mask & HEVCASM_SSE2)
	{
		if (!xFrac && !yFrac)
		{
			if (w <= 64) f = hevcasm_pred_uni_copy_8to8_64xh_sse2;
			if (w <= 48) f = hevcasm_pred_uni_copy_8to8_48xh_sse2;
			if (w <= 32) f = hevcasm_pred_uni_copy_8to8_32xh_sse2;
			if (w <= 16) f = hevcasm_pred_uni_copy_8to8_16xh_sse2;
		}
	}

	if (mask & HEVCASM_SSE41)
	{
		if (taps == 8 && xFrac && !yFrac)
		{
			if (w <= 64) f = hevcasm_pred_uni_8tap_8to8_h_64xh_sse4;
			if (w <= 48) f = hevcasm_pred_uni_8tap_8to8_h_48xh_sse4;
			if (w <= 32) f = hevcasm_pred_uni_8tap_8to8_h_32xh_sse4;
			if (w <= 16) f = hevcasm_pred_uni_8tap_8to8_h_16xh_sse4;
		}
		if (taps == 8 && !xFrac && yFrac)
		{
			if (w <= 64) f = hevcasm_pred_uni_8tap_8to8_v_64xh_sse4;
			if (w <= 48) f = hevcasm_pred_uni_8tap_8to8_v_48xh_sse4;
			if (w <= 32) f = hevcasm_pred_uni_8tap_8to8_v_32xh_sse4;
			if (w <= 24) f = hevcasm_pred_uni_8tap_8to8_v_24xh_sse4;
			if (w <= 16) f = hevcasm_pred_uni_8tap_8to8_v_16xh_sse4;
			if (w <= 8) f = hevcasm_pred_uni_8tap_8to8_v_8xh_sse4;
		}
		if (taps == 8 && xFrac && yFrac)
		{
			if (w <= 64) f = hevcasm_pred_uni_8tap_8to8_hv_64xh_sse4;
			if (w <= 48) f = hevcasm_pred_uni_8tap_8to8_hv_48xh_sse4;
			if (w <= 32) f = hevcasm_pred_uni_8tap_8to8_hv_32xh_sse4;
			if (w <= 16) f = hevcasm_pred_uni_8tap_8to8_hv_16xh_sse4;
		}
		if (taps == 4 && xFrac && !yFrac)
		{
			if (w <= 16) f = hevcasm_pred_uni_4tap_8to8_h_16xh_sse4;
			if (w <= 32) f = hevcasm_pred_uni_4tap_8to8_h_32xh_sse4;
		}
		if (taps == 4 && !xFrac && yFrac)
		{
			if (w <= 32) f = hevcasm_pred_uni_4tap_8to8_v_32xh_sse4;
			if (w <= 24) f = hevcasm_pred_uni_4tap_8to8_v_24xh_sse4;
			if (w <= 16) f = hevcasm_pred_uni_4tap_8to8_v_16xh_sse4;
			if (w <= 8) f = hevcasm_pred_uni_4tap_8to8_v_8xh_sse4;
		}
		if (taps == 4 && xFrac && yFrac)
		{
			if (w <= 32) f = hevcasm_pred_uni_4tap_8to8_hv_32xh_sse4;
			if (w <= 16) f = hevcasm_pred_uni_4tap_8to8_hv_16xh_sse4;
		}
	}
	return f;
}


typedef struct 
{
	hevcasm_pred_uni_filter_8to8 *f;
	uint8_t *dst;
	ptrdiff_t stride_dst;
	const uint8_t *ref;
	ptrdiff_t stride_ref;
	int w;
	int h;
	int xFrac;
	int yFrac;
}
bind_pred_inter;


void call_pred_inter(void *p, int n)
{
	bind_pred_inter *s = p;
	while (n--)
	{
		s->f(s->dst, s->stride_dst, s->ref, s->stride_ref, s->w, s->h, s->xFrac, s->yFrac);
	}
}


static int test_partitions(int taps, uint8_t *dst0, uint8_t *dst1, bind_pred_inter *bound, hevcasm_instruction_set mask)
{
	int error_count = 0;

	const int partitions[24][2] =
	{
		{ 8, 4 }, { 8, 8 }, { 4, 8 },
		{ 16, 4 }, { 16, 8 }, { 16, 12 }, { 16, 16 }, { 12, 16 }, { 8, 16 }, { 4, 16 },
		{ 32, 8 }, { 32, 16 }, { 32, 24 }, { 32, 32 }, { 24, 32 }, { 16, 32 }, { 8, 32 },
		{ 64, 16 }, { 64, 32 }, { 64, 48 }, { 64, 64 }, { 48, 64 }, { 32, 64 }, { 16, 64 },
	};

	for (int k = 0; k < 24; ++k)
	{
		const int nPbW = partitions[k][0];
		const int nPbH = partitions[k][1];

		bound->w = nPbW * taps / 8;
		bound->h = nPbH * taps / 8;

		printf("\t%dx%d %d-tap %s%s : ", bound->w, bound->h, taps, bound->xFrac ? "H" : "", bound->yFrac ? "V" : "");

		bound->f = hevcasm_get_pred_uni_filter_8to8(taps, bound->w, bound->h, bound->xFrac, bound->yFrac, HEVCASM_C_OPT);

		bound->dst = dst0;
		memset(bound->dst, 0, 64 * bound->stride_dst);
		call_pred_inter(bound, 1);
		bound->dst = dst1;

		double first_result = 0.0;

		for (hevcasm_instruction_set_idx_t i = 0; i < HEVCASM_INSTRUCTION_SET_COUNT; ++i)
		{
			if (!((1 << i) & mask)) continue;

			bound->f = hevcasm_get_pred_uni_filter_8to8(taps, bound->w, bound->h, bound->xFrac, bound->yFrac, 1 << i);

			if (!bound->f) continue;

			memset(bound->dst, 0, 64 * bound->stride_dst);

			hevcasm_count_average_cycles(call_pred_inter, bound, &first_result, i, 100);

			for (int y = 0; y < bound->h; ++y)
			{
				const int mismatch = memcmp(&dst0[y*bound->stride_dst], &dst1[y*bound->stride_dst], bound->w);
				if (mismatch)
				{
					printf("-MISMATCH");
					++error_count;
					break;
				}
			}
		}

		printf("\n");
	}

	return error_count;
}

int hevcasm_test_pred_uni(hevcasm_instruction_set mask)
{
	printf("pred_inter\n");

	int error_count = 0;

	bind_pred_inter bound;

#define STRIDE_DST 192
#define STRIDE_REF 192
	HEVCASM_ALIGN(32, uint8_t, dst[2][64 * STRIDE_DST]);
	HEVCASM_ALIGN(32, uint8_t, ref[80 * STRIDE_REF]);
	bound.stride_dst = STRIDE_DST;
	bound.stride_ref = STRIDE_REF;
#undef STRIDE_DST
#undef STRIDE_REF

	for (int x = 0; x < 80 * bound.stride_ref; x++) ref[x] = rand() & 0xff;
	bound.ref = ref + 8 * bound.stride_ref;

	for (int taps = 8; taps >= 4; taps -= 4)
	{
		for (bound.yFrac = 0; bound.yFrac < 2; ++bound.yFrac)
		{
			for (bound.xFrac = 0; bound.xFrac < 2; ++bound.xFrac)
			{
				error_count += test_partitions(taps, dst[0], dst[1], &bound, mask);
			}
		}
	}

	return error_count;
}


void hevcasm_pred_bi_mean_16and16to8_c(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *ref0, const int16_t *ref1, ptrdiff_t stride_ref, int w, int h)
{
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			int v = ((int)ref0[x + y * stride_ref] + (int)ref1[x + y * stride_ref] + (1 << 6)) >> 7;
			v = Clip3(0, 255, v);
			dst[x + y * stride_dst] = v;
		}
	}
}


#define MAKE_hevcasm_pred_bi_xtap_8to8(taps) \
 \
void hevcasm_pred_bi_ ## taps ## tap_8to8_c(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref0, const uint8_t *ref1, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac0, int yFrac0, int xFrac1, int yFrac1) \
{ \
	int16_t intermediate[4][(64 + taps - 1) * 64]; \
	 \
	const int w = nPbW; \
	const int h = nPbH; \
	 \
	/* Horizontal filter */ \
	hevcasm_pred_uni_filter_generic(intermediate[2], 2, 64, ref0 - (taps/2-1) * stride_ref, 1, stride_ref, w, h + taps - 1, 1, taps, xFrac0, 0, 0); \
	 \
	/* Vertical filter */ \
	hevcasm_pred_uni_filter_generic(intermediate[0], 2, 64, intermediate[2] + (taps / 2 - 1) * 64, 2, 64, w, h, 64, taps, yFrac0, 6, 0); \
	 \
	/* Horizontal filter */ \
	hevcasm_pred_uni_filter_generic(intermediate[3], 2, 64, ref1 - (taps / 2 - 1) * stride_ref, 1, stride_ref, w, h + taps - 1, 1, taps, xFrac1, 0, 0); \
	 \
	/* Vertical filter */ \
	hevcasm_pred_uni_filter_generic(intermediate[1], 2, 64, intermediate[3] + (taps / 2 - 1) * 64, 2, 64, w, h, 64, taps, yFrac1, 6, 0); \
	 \
	/* Combine two references for bi pred */ \
	hevcasm_pred_bi_mean_16and16to8_c(dst, stride_dst, intermediate[0], intermediate[1], 64, nPbW, nPbH); \
} \

MAKE_hevcasm_pred_bi_xtap_8to8(8)
MAKE_hevcasm_pred_bi_xtap_8to8(4)


#define MAKE_hevcasm_pred_bi_xtap_8to8_sse4(taps, width) \
	\
	void hevcasm_pred_bi_ ## taps ## tap_8to8_ ## width ## xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref0, const uint8_t *ref1, ptrdiff_t stride_ref, int w, int h, int xFrac0, int yFrac0, int xFrac1, int yFrac1) \
{ \
	HEVCASM_ALIGN(32, int16_t, intermediate[2][(64 + taps - 1) * 64]); /* todo: can reduce stack usage here by making array size function of width */ \
	\
	/* Horizontal filter */ \
	hevcasm_pred_uni_ ## taps ## tap_8to16_h_ ## width ## xh_sse4(intermediate[0], 64, ref0 - (taps/2-1) * stride_ref, stride_ref, w, h + taps - 1, xFrac0, 0); \
	 \
	/* Horizontal filter */ \
	hevcasm_pred_uni_ ## taps ## tap_8to16_h_ ## width ## xh_sse4(intermediate[1], 64, ref1 - (taps/2-1) * stride_ref, stride_ref, w, h + taps - 1, xFrac1, 0); \
	\
	/* Two vertical filters and combine their output for bi pred */ \
	hevcasm_pred_bi_v_ ## taps ## tap_16to16_ ## width ## xh_sse4(dst, stride_dst, intermediate[0], intermediate[1], 64, w, h, yFrac0, yFrac1); \
} \

MAKE_hevcasm_pred_bi_xtap_8to8_sse4(8, 16)
MAKE_hevcasm_pred_bi_xtap_8to8_sse4(8, 32)
MAKE_hevcasm_pred_bi_xtap_8to8_sse4(8, 48)
MAKE_hevcasm_pred_bi_xtap_8to8_sse4(8, 64)

MAKE_hevcasm_pred_bi_xtap_8to8_sse4(4, 16)
MAKE_hevcasm_pred_bi_xtap_8to8_sse4(4, 32)


hevcasm_pred_bi_8to8* HEVCASM_API hevcasm_get_pred_bi_8to8(int taps, int w, int h, int xFracA, int yFracA, int xFracB, int yFracB, hevcasm_instruction_set mask)
{
	hevcasm_pred_bi_8to8 *f = 0;
	
	if (mask & HEVCASM_C_REF)
	{
		if (taps == 8) f = hevcasm_pred_bi_8tap_8to8_c;
		if (taps == 4) f = hevcasm_pred_bi_4tap_8to8_c;
	}

	if (mask & HEVCASM_SSE2)
	{
		if (!xFracA && !yFracA && !xFracB && !yFracB)
		{
			if (w <= 64) f = hevcasm_pred_bi_8to8_copy_64xh_sse2;
			if (w <= 48) f = hevcasm_pred_bi_8to8_copy_48xh_sse2;
			if (w <= 32) f = hevcasm_pred_bi_8to8_copy_32xh_sse2;
			if (w <= 16) f = hevcasm_pred_bi_8to8_copy_16xh_sse2;
		}
	}

	if (mask & HEVCASM_SSE41)
	{
		if (taps == 8 && (xFracA || yFracA || xFracB || yFracB))
		{
			if (w <= 64) f = hevcasm_pred_bi_8tap_8to8_64xh_sse4;
			if (w <= 48) f = hevcasm_pred_bi_8tap_8to8_48xh_sse4;
			if (w <= 32) f = hevcasm_pred_bi_8tap_8to8_32xh_sse4;
			if (w <= 16) f = hevcasm_pred_bi_8tap_8to8_16xh_sse4;
		}
		if (taps == 4 && (xFracA || yFracA || xFracB || yFracB))
		{
			if (w <= 32) f = hevcasm_pred_bi_4tap_8to8_32xh_sse4;
			if (w <= 16) f = hevcasm_pred_bi_4tap_8to8_16xh_sse4;
		}
	}

	return f;
}


typedef struct
{
	hevcasm_pred_bi_8to8 *f;
	uint8_t *dst;
	ptrdiff_t stride_dst;
	const uint8_t *refA;
	const uint8_t *refB;
	ptrdiff_t stride_ref;
	int w;
	int h;
	int xFracA;
	int yFracA;
	int xFracB;
	int yFracB;
}
bind_pred_bi;

// todo ref0 -> refA etc.
// todo nPbW -> w

void call_pred_bi(void *p, int n)
{
	bind_pred_bi *s = p;
	while (n--)
	{
		s->f(s->dst, s->stride_dst, s->refA, s->refB, s->stride_ref, s->w, s->h, s->xFracA, s->yFracA, s->xFracB, s->yFracB);
	}
}

static int test_partitions_bi(int taps, uint8_t *dst0, uint8_t *dst1, bind_pred_bi *bound, hevcasm_instruction_set mask)
{
	int error_count = 0;

	const int partitions[24][2] =
	{
		{ 8, 8 },
		{ 16, 4 }, { 16, 8 }, { 16, 12 }, { 16, 16 }, { 12, 16 }, { 8, 16 }, { 4, 16 },
		{ 32, 8 }, { 32, 16 }, { 32, 24 }, { 32, 32 }, { 24, 32 }, { 16, 32 }, { 8, 32 },
		{ 64, 16 }, { 64, 32 }, { 64, 48 }, { 64, 64 }, { 48, 64 }, { 32, 64 }, { 16, 64 },
	};

	for (int k = 0; k < 22; ++k)
	{
		const int nPbW = partitions[k][0];
		const int nPbH = partitions[k][1];

		bound->w = nPbW * taps / 8;
		bound->h = nPbH * taps / 8;

		printf("\t%dx%d %d-tap A-%s%s B-%s%s : ", bound->w, bound->h, taps, bound->xFracA ? "H" : "", bound->yFracA ? "V" : "", bound->xFracB ? "H" : "", bound->yFracB ? "V" : "");

		bound->f = hevcasm_get_pred_bi_8to8(taps, bound->w, bound->h, bound->xFracA, bound->yFracA, bound->xFracB, bound->yFracB, HEVCASM_C_REF);

		bound->dst = dst0;
		memset(bound->dst, 0, 64 * bound->stride_dst);
		call_pred_bi(bound, 1);
		bound->dst = dst1;

		double first_result = 0.0;

		for (hevcasm_instruction_set_idx_t i = 0; i < HEVCASM_INSTRUCTION_SET_COUNT; ++i)
		{
			if (!((1 << i) & mask)) continue;

			bound->f = hevcasm_get_pred_bi_8to8(taps, bound->w, bound->h, bound->xFracA, bound->yFracA, bound->xFracB, bound->yFracB, 1 << i);

			if (!bound->f) continue;

			memset(bound->dst, 0, 64 * bound->stride_dst);

			hevcasm_count_average_cycles(call_pred_bi, bound, &first_result, i, 100);

			for (int y = 0; y < bound->h; ++y)
			{
				const int mismatch = memcmp(&dst0[y*bound->stride_dst], &dst1[y*bound->stride_dst], bound->w);
				if (mismatch)
				{
					printf("-MISMATCH");
					++error_count;
					break;
				}
			}
		}

		printf("\n");
	}

	return error_count;
}

int hevcasm_test_pred_bi(hevcasm_instruction_set mask)
{
	printf("pred_bi\n");

	int error_count = 0;

	bind_pred_bi bound;

#define STRIDE_DST 192
#define STRIDE_REF 192
	HEVCASM_ALIGN(32, uint8_t, dst[2][64 * STRIDE_DST]);
	HEVCASM_ALIGN(32, uint8_t, ref[2][80 * STRIDE_REF]);
	bound.stride_dst = STRIDE_DST;
	bound.stride_ref = STRIDE_REF;
#undef STRIDE_DST
#undef STRIDE_REF

	for (int x = 0; x < 80 * bound.stride_ref; x++)
	{
		ref[0][x] = rand() & 0xff;
		ref[1][x] = rand() & 0xff;
	}

	bound.refA = ref[0] + 8 * bound.stride_ref;
	bound.refB = ref[1] + 8 * bound.stride_ref;

	for (int taps = 8; taps >= 4; taps -= 4)
	{
		bound.xFracA = bound.yFracA = bound.xFracB = bound.yFracB = 0;
		error_count += test_partitions_bi(taps, dst[0], dst[1], &bound, mask);
		bound.xFracA = bound.yFracA = bound.xFracB = bound.yFracB = 1;
		error_count += test_partitions_bi(taps, dst[0], dst[1], &bound, mask);

		//for (bound.yFracA = 0; bound.yFracA < 2; ++bound.yFracA)
		//{
		//	for (bound.xFracA = 0; bound.xFracA < 2; ++bound.xFracA)
		//	{
		//		for (bound.yFracB = 0; bound.yFracB < 2; ++bound.yFracB)
		//		{
		//			for (bound.xFracB = 0; bound.xFracB < 2; ++bound.xFracB)
		//			{
		//				error_count += test_partitions_bi(taps, dst[0], dst[1], &bound, mask);
		//			}
		//		}
		//	}
		//}
	}

	return error_count;
}
