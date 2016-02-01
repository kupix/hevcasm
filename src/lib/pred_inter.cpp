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

#include "pred_inter_a.h"
#include "pred_inter.h"
#include "hevcasm_test.h"
#include "Jit.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>


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
static void hevcasm_pred_uni_generic(
	void *dst, int sizeof_dst, ptrdiff_t stride_dst, 
	const void *src, int sizeof_src, ptrdiff_t stride_src, 
	int w, int h, 
	ptrdiff_t stride_tap, 
	int n, int fractionalPosition, int shift, int add)
{
	int16_t *dst16 = (int16_t *)dst;
	uint8_t *dst8 = (uint8_t *)dst;

	const int16_t *src16 = (int16_t *)src;
	const uint8_t *src8 = (uint8_t *)src;

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


void hevcasm_pred_uni_8tap_8to8_h(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int w, int h, int xFrac, int yFrac)
{
	assert(xFrac);
	assert(!yFrac);
	hevcasm_pred_uni_generic(dst, 1, stride_dst, ref, 1, stride_ref, w, h, 1, 8, xFrac, 6, 1);
}


void hevcasm_pred_uni_8tap_8to8_v(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int w, int h, int xFrac, int yFrac)
{
	assert(!xFrac);
	assert(yFrac);
	hevcasm_pred_uni_generic(dst, 1, stride_dst, ref, 1, stride_ref, w, h, stride_ref, 8, yFrac, 6, 1);
}


void hevcasm_pred_uni_8tap_8to8_hv(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int w, int h, int xFrac, int yFrac)
{
	int16_t intermediate[(64 + 7) * 64];

	/* Horizontal filter */
	hevcasm_pred_uni_generic(intermediate, 2, 64, ref - 3 * stride_ref, 1, stride_ref, w, h + 7, 1, 8, xFrac, 0, 0);

	/* Vertical filter */
	hevcasm_pred_uni_generic(dst, 1, stride_dst, intermediate + 3 * 64, 2, 64, w, h, 64, 8, yFrac, 12, 1);
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

//MAKE_hevcasm_pred_uni_xtap_8to8_hv(8, _16xh_sse4)
//MAKE_hevcasm_pred_uni_xtap_8to8_hv(8, _32xh_sse4)
//MAKE_hevcasm_pred_uni_xtap_8to8_hv(8, _48xh_sse4)
//MAKE_hevcasm_pred_uni_xtap_8to8_hv(8, _64xh_sse4)
//MAKE_hevcasm_pred_uni_xtap_8to8_hv(4, _16xh_sse4)
//MAKE_hevcasm_pred_uni_xtap_8to8_hv(4, _32xh_sse4)


void hevcasm_pred_uni_4tap_8to8_h(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int w, int h, int xFrac, int yFrac)
{
	assert(xFrac);
	assert(!yFrac);
	hevcasm_pred_uni_generic(dst, 1, stride_dst, ref, 1, stride_ref, w, h, 1, 4, xFrac, 6, 1);
}


void hevcasm_pred_uni_4tap_8to8_v(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int w, int h, int xFrac, int yFrac)
{
	assert(!xFrac);
	assert(yFrac);
	hevcasm_pred_uni_generic(dst, 1, stride_dst, ref, 1, stride_ref, w, h, stride_ref, 4, yFrac, 6, 1);
}


void hevcasm_pred_uni_4tap_8to8_hv(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int w, int h, int xFrac, int yFrac)
{
	int16_t intermediate[(32 + 3) * 32];

	/* Horizontal filter */
	hevcasm_pred_uni_generic(intermediate, 2, 32, ref - stride_ref, 1, stride_ref, w, h + 3, 1, 4, xFrac, 0, 0);

	/* Vertical filter */
	hevcasm_pred_uni_generic(dst, 1, stride_dst, intermediate + 32, 2, 32, w, h, 32, 4, yFrac, 12, 1);
}


struct PredUniCopy
	:
	Jit::Function
{
	int width;

	PredUniCopy(Jit::Buffer *buffer, int width)
		:
		Jit::Function(buffer, Jit::CountArguments<hevcasm_pred_uni_8to8>::value),
		width(width)
	{
		this->build();
	}

	void assemble() override
	{
		auto &r0 = arg64(0);
		auto &r1 = arg64(1);
		auto &r2 = arg64(2);
		auto &r3 = arg64(3);
		auto height = Xbyak::Reg32(arg64(5).getIdx());

		L("loop");
		{
			int const n = width / 16;

			Xbyak::Xmm const *m[4];
			for (int i = 0; i < n; ++i)
			{
				m[i] = &regXmm(i);
				movdqu(*m[i], ptr[r2 + 16 * i]);
			}

			for (int i = 0; i < n; ++i)
			{
				movdqu(ptr[r0 + 16 * i], *m[i]);
			}

			lea(r2, ptr[r2 + r3]);
			lea(r0, ptr[r0 + r1]);
		}
		dec(height);
		jg("loop");
	}
};


//rename
struct PredUni
	:
	Jit::Function
{
	PredUni(Jit::Buffer *buffer, hevcasm_pred_uni_8to8 *, int taps, int width, int xFrac, int yFrac, int inputBitDepth)
		:
		Jit::Function(buffer, Jit::CountArguments<hevcasm_pred_uni_8to8>::value),
		taps(taps),
		width(width),
		xFrac(xFrac),
		yFrac(yFrac),
		inputBitDepth(inputBitDepth),
		refs(1)
	{
		this->build();
	}

	PredUni(Jit::Buffer *buffer, hevcasm_pred_bi_v_16to16 *, int taps, int width, int xFrac, int yFrac, int inputBitDepth)
		:
		Jit::Function(buffer, Jit::CountArguments<hevcasm_pred_bi_v_16to16>::value),
		taps(taps),
		width(width),
		xFrac(xFrac),
		yFrac(yFrac),
		inputBitDepth(inputBitDepth),
		refs(2)
	{
		this->build();
	}

	PredUni(Jit::Buffer *buffer, hevcasm_pred_uni_8to16 *, int taps, int width, int xFrac, int yFrac, int inputBitDepth)
		:
		Jit::Function(buffer, Jit::CountArguments<hevcasm_pred_uni_8to16>::value),
		taps(taps),
		width(width),
		xFrac(xFrac),
		yFrac(yFrac),
		inputBitDepth(inputBitDepth),
		outputBitDepth(16),
		refs(1)
	{
		this->build();
	}

	int refs;
	int taps;
	int width;
	int xFrac;
	int yFrac;
	int inputBitDepth;

	Xbyak::Label coefficients;
	Xbyak::Label coefficients16;
	Xbyak::Label constant_times_4_dd_0x800;
	Xbyak::Label constant_times_8_dw_0x20;
	Xbyak::Label constant_times_8_dw_0x40;

	void data() override
	{
		align();

		L(coefficients);
		for (int frac = 0; frac < (12 - taps); ++frac)
		{
			for (int k = 0; k < taps; k += 2)
			{
				int coeff0 = hevcasm_pred_coefficient(taps, frac, k);
				int coeff1 = hevcasm_pred_coefficient(taps, frac, k + 1);

				db({ coeff0, coeff1 }, 8);
			}
		}

		L(coefficients16);
		for (int frac = 0; frac < (12 - taps); ++frac)
		{
			for (int k = 0; k < taps; k += 2)
			{
				int coeff0 = hevcasm_pred_coefficient(taps, frac, k);
				int coeff1 = hevcasm_pred_coefficient(taps, frac, k + 1);

				dw({ coeff0, coeff1 }, 4);
			}
		}

		L(constant_times_4_dd_0x800);
		dd({ 0x800 }, 4);

		L(constant_times_8_dw_0x20);
		dw({ 0x20 }, 8);

		L(constant_times_8_dw_0x40);
		dw({ 0x40 }, 8);
	}

	void assemble() override
	{
		if (this->refs == 2)
			this->assembleBi();
		else
			this->assembleUni();
	}

	void assembleBi()
	{
		this->PRED_BI_V_8NxH(taps, 16, width);
	}

	int outputBitDepth = 8;

	void assembleUni()
	{
		auto &r0 = arg64(0);
		auto &r1 = arg64(1);
		auto &r2 = arg64(2);
		auto &r3 = arg64(3);
		auto &r4 = arg64(4);
		auto &r5 = arg64(5);
		auto &r6 = arg64(6);
		auto &r7 = arg64(7);

		if (xFrac && !yFrac)
		{
			this->PRED_UNI_H_16NxH(taps, outputBitDepth, width);
			return;
		}

		if (xFrac && yFrac)
		{
			this->stackSize = 64 * (64 + taps - 1) * 2 + 16;


			push(r0);
			lea(r0, ptr[rsp + 8]);
			push(r1);
			push(r5);
			push(r0);

			mov(r1, 64);
			if (taps == 8)
			{
				sub(r2, r3);
				sub(r2, r3);
			}
			sub(r2, r3);
			add(r5, taps - 1);

			// first H then continue to V
			this->PRED_UNI_H_16NxH(taps, 16, width);

			pop(r0);
//			mov(r2, 0); mov(r2, ptr[r2]);
			lea(r2, ptr[r0 + 128 * (taps / 2 - 1)]);
			mov(r3, 64);

			pop(r5);
			pop(r1);
			pop(r0);

			inputBitDepth = 16;
		}

		Xbyak::Reg32 r5d(r5.getIdx());
		Xbyak::Reg32 r6d(r6.getIdx());

		auto &m0 = regXmm(0);
		auto &m1 = regXmm(1);
		auto &m2 = regXmm(2);
		auto &m3 = regXmm(3);
		//auto &m4 = regXmm(4);
		auto &m5 = regXmm(4); // !

		if (inputBitDepth == 16) shl(r3, 1);

		// adjust input pointer (subtract (taps/2-1) * stride)
		for (int i = 0; i < taps / 2 - 1; ++i) sub(r2, r3);

		if (inputBitDepth == 16)
		{
			lea(r4, ptr[rip + coefficients16]);
		}
		else
		{
			lea(r4, ptr[rip + coefficients]);
		}

		mov(r6, r7); // ??? avoid probls
		shl(r6d, 4 + taps / 4);
		lea(r4, ptr[r4 + r6]);

		L("loopV");
		{
			for (int i = 0; i < width / 8; ++i)
			{
				// each iteration operates on a row of 8-samples

				if (inputBitDepth == 16)
				{
					movdqa(m3, ptr[rip + constant_times_4_dd_0x800]);
					movdqa(m5, m3);
				}
				else
				{
					movdqa(m3, ptr[rip + constant_times_8_dw_0x20]);
				}

				for (int j = 0; j < taps / 2; ++j)
				{
					// each iteration of this loop performs two filter taps

					if (inputBitDepth == 16)
					{
						movdqu(m0, ptr[r2]);
						movdqu(m1, ptr[r2 + r3]);
						movdqa(m2, m0);
						punpckhwd(m2, m1);
						punpcklwd(m0, m1);
						pmaddwd(m0, ptr[r4]);
						pmaddwd(m2, ptr[r4]);

						paddd(m3, m0);
						paddd(m5, m2);
					}
					else
					{
						movq(m0, ptr[r2]);
						movq(m1, ptr[r2 + r3]);
						punpcklbw(m0, m1);
						pmaddubsw(m0, ptr[r4]);

						paddw(m3, m0);
					}

					lea(r2, ptr[r2 + r3 * 2]);

					add(r4, 16);
				}

				neg(r3);
				lea(r2, ptr[r2 + r3 * taps]);
				neg(r3);

				sub(r4, taps * 8);

				if (inputBitDepth == 16)
				{
					psrad(m3, 12);
					psrad(m5, 12);
					packssdw(m3, m5);
				}
				else
				{
					psraw(m3, 6);
				}

				packuswb(m3, m3);

				movdqu(ptr[r0], m3);

				lea(r2, ptr[r2 + inputBitDepth]);
				lea(r0, ptr[r0 + 8]);
			}

			sub(r0, 8 * width / 8);
			sub(r2, inputBitDepth * width / 8);

			add(r0, r1);
			add(r2, r3);
		}
		dec(r5d);
		jg("loopV");
	}

	// taps is number of filter taps (4 or 8)
	// outputTypeBits is size of output type (8 for uint8_t rounded, 16 for int16_t right shifted 6)
	// dx is dx (horizontal offset as integer number of samples) 
	void PRED_UNI_H_16x1(int taps, int outputTypeBits, int dx)
	{
		auto &r0 = reg64(0);
		auto &r2 = reg64(2);

		auto &m0 = xmm0;
		auto &m1 = xmm1;
		auto &m2 = xmm2;
		auto &m3 = xmm3;
		auto &m4 = xmm4;
		auto &m5 = xmm5;
		auto &m6 = xmm6;
		auto &m7 = xmm7;

		movdqu(m2, ptr[r2 + 1 - (taps / 2) + dx]);
		pmaddubsw(m2, m4);
		// m2 = eca86420 (even positions, first two taps)

		movdqu(m1, ptr[r2 + 2 - (taps / 2) + dx]);
		pmaddubsw(m1, m4);
		// m1 = fdb97531 (odd positions, first two taps)

		movdqu(m0, ptr[r2 + 3 - (taps / 2) + dx]);
		pmaddubsw(m0, m5);
		// m0 = eca86420 (even positions, two taps)

		paddw(m2, m0);
		// m2 = eca86420 (even positions)

		movdqu(m0, ptr[r2 + 4 - (taps / 2) + dx]);
		pmaddubsw(m0, m5);
		// m0 = fdb97531 (odd positions, two taps)

		paddw(m1, m0);
		// m1 = fdb97531 (odd positions)

		if (taps == 8)
		{
			// need four more taps...

			movdqu(m0, ptr[r2 + 5 - (taps / 2) + dx]);
			pmaddubsw(m0, m6);
			// m0 = eca86420(even positions, two taps)

			paddw(m2, m0);
			// m2 = eca86420(even positions)

			movdqu(m0, ptr[r2 + 6 - (taps / 2) + dx]);
			pmaddubsw(m0, m6);
			// m0 = fdb97531(odd positions, two taps)

			paddw(m1, m0);
			// m1 = fdb97531(odd positions)

			movdqu(m0, ptr[r2 + 7 - (taps / 2) + dx]);
			pmaddubsw(m0, m7);
			// m0 = eca86420(even positions, two taps)

			paddw(m2, m0);
			// m2 = eca86420(even positions)

			movdqu(m0, ptr[r2 + 8 - (taps / 2) + dx]);
			pmaddubsw(m0, m7);
			// m0 = fdb97531(odd positions, two taps)

			paddw(m1, m0);
			// m1 = fdb97531(odd positions)
		}

		movq(m0, m2);
		punpcklwd(m0, m1);
		// m0 = 76543210

		punpckhwd(m2, m1);
		// m2 = fedcba98

		if (outputTypeBits == 16)
		{
			movdqu(ptr[r0 + 2 * dx], m0);
			movdqu(ptr[r0 + 2 * dx + 16], m2);
		}
		else
		{
			paddw(m0, ptr[rip + constant_times_8_dw_0x20]);
			paddw(m2, ptr[rip + constant_times_8_dw_0x20]);
			psraw(m0, 6);
			psraw(m2, 6);
			packuswb(m0, m2);
			movdqu(ptr[r0 + dx], m0);
		}
	}


	// taps is number of filter taps (4 or 8)
	// outputTypeBits is size of output type (8 for uint8_t rounded, 16 for int16_t right shifted 6)
	// width is block width (number of samples, multiple of 16)
	void PRED_UNI_H_16NxH(int taps, int outputTypeBits, int width)
	{
		auto &r0 = reg64(0);
		auto &r1 = reg64(1);
		auto &r2 = reg64(2);
		auto &r3 = reg64(3);
		auto &r4 = reg64(4);
		auto &r5 = reg64(5);
		Xbyak::Reg32 r5d(r5.getIdx());
		auto &r6 = reg64(6);
		Xbyak::Reg32 r6d(r6.getIdx());

		auto &m0 = regXmm(0);
		auto &m1 = regXmm(1);
		auto &m2 = regXmm(2);
		auto &m3 = regXmm(3);
		auto &m4 = regXmm(4);
		auto &m5 = regXmm(5);
		auto &m6 = regXmm(6);
		auto &m7 = regXmm(7);

		if (taps == 8)
		{
			shl(r6d, 6);  // frac *= 4 * 16
		}
		else
		{
			shl(r6d, 5);  // frac *= 2 * 16
		}
		lea(r4, ptr[rip + coefficients]);

		movdqa(m4, ptr[r4 + r6]);
		movdqa(m5, ptr[r4 + r6 + 1 * 16]);
		if (taps == 8)
		{
			movdqa(m6, ptr[r4 + r6 + 2 * 16]);
			movdqa(m7, ptr[r4 + r6 + 3 * 16]);
		}

		if (outputTypeBits == 16)
		{
			// dst is int16_t * so need to double the stride
			shl(r1, 1);  // 
		}

		L("loop");
		{
			for (int i = 0; i < width / 16; ++i)
			{
				int const dx = 16 * i;
				PRED_UNI_H_16x1(taps, outputTypeBits, dx);
			}

			add(r0, r1);
			add(r2, r3);
		}
		dec(r5d);
		jg("loop");
	}

	// void hevcasm_pred_bi_v_%1tap_16to16_%3xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *refAtop, const int16_t *refBtop, ptrdiff_t stride_ref, int nPbW, int nPbH, int yFracA, int yFracB)//
	void PRED_BI_V_8NxH(int taps, int inputTypeSize, int width)
	{
		// taps is number of filter taps (4 or 8)//
		// inputTypeSize is size of input type (8 for uint8_t, 16 for int16_t right shifted 6)
		// width is block width (number of samples, multiple of 8)
		assert(inputTypeSize == 16);

		// // void hevcasm_pred_bi_v_%1tap_16to16_%3xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *refAtop, const int16_t *refBtop, ptrdiff_t stride_ref, int nPbW, int nPbH, int yFracA, int yFracB)//
		// INIT_XMM sse4
		// cglobal pred_bi_v_%1tap_16to16_%3xh, 9, 9, 8

		auto &r0 = arg64(0); // dst
		auto &r1 = arg64(1); // stride_dst
		auto &r2 = arg64(2); // refA
		auto &r3 = arg64(3); // refB
		auto &r4 = arg64(4); // stride_ref
		auto &r5 = arg64(5); // width
		auto &r6 = arg64(6); 
		auto r6d = Xbyak::Reg32(r6.getIdx()); // height
		auto &r7 = arg64(7);
		auto r7d = Xbyak::Reg32(r7.getIdx()); // yFracA
		auto &r8 = arg64(8);
		auto r8d = Xbyak::Reg32(r8.getIdx()); // yFracB

		auto &m0 = regXmm(0);
		auto &m1 = regXmm(1);
		auto &m2 = regXmm(2);
		auto &m3 = regXmm(3);
		auto &m4 = regXmm(4);
		auto &m5 = regXmm(5);
		auto &m6 = regXmm(6);
		auto &m7 = regXmm(7);

		shl(r4, 1);

#if 1 // ARCH_X86_64
		shl(r7d, 4 + taps / 4);
		shl(r8d, 4 + taps / 4);
		lea(r5, ptr[rip + coefficients16]);
		lea(r7, ptr[r5 + r7]);
		lea(r8, ptr[r5 + r8]);
#define coeffA r7
#define coeffB r8
#define stride_dst r1
#else
		mov r5, r7m
		shl r5, 4 + taps / 4
		lea r1, [pred_inter_ % 1tap_coefficient_pairs_4_dw + r5]
		mov r5, r8m
		shl r5, 4 + taps / 4
		lea r5, [pred_inter_ % 1tap_coefficient_pairs_4_dw + r5]
		% define coeffA r1
		%define coeffB r5
		%define stride_dst r1m
#endif
		L("loopBiV");
		{
			for (int i = 0; i < width / 8; ++i)
			{
				// each iteration of this loop operates on a row of 8-samples

				pxor(m3, m3);
				movdqa(m5, m3);
				movdqa(m6, m3);
				movdqa(m7, m3);

				for (int j = 0; j < taps / 2; ++j)
				{
					// each iteration of this loop performs two filter taps

					// reference picture A
					movdqu(m0, ptr[r2]);
					movdqu(m1, ptr[r2 + r4]);
					movdqa(m2, m0);
					punpckhwd(m2, m1);
					punpcklwd(m0, m1);
					pmaddwd(m0, ptr[coeffA]);
					pmaddwd(m2, ptr[coeffA]);
					paddd(m3, m0);
					paddd(m5, m2);
					lea(r2, ptr[r2 + r4 * 2]);

					// reference picture B
					movdqu(m0, ptr[r3]);
					movdqu(m1, ptr[r3 + r4]);
					movdqa(m2, m0);
					punpckhwd(m2, m1);
					punpcklwd(m0, m1);
					pmaddwd(m0, ptr[coeffB]);
					pmaddwd(m2, ptr[coeffB]);
					paddd(m6, m0);
					paddd(m7, m2);
					lea(r3, ptr[r3 + r4 * 2]);

					add(coeffA, 16);
					add(coeffB, 16);
				}

				neg(r4);
				lea(r2, ptr[r2 + r4 * taps]);
				lea(r3, ptr[r3 + r4 * taps]);
				neg(r4);

				sub(coeffA, 8 * taps);
				sub(coeffB, 8 * taps);

				psrad(m3, 6);
				psrad(m5, 6);
				psrad(m6, 6);
				psrad(m7, 6);

				packssdw(m3, m5);
				packssdw(m6, m7);

				paddsw(m3, m6);
				paddsw(m3, ptr[rip + constant_times_8_dw_0x40]);
				psraw(m3, 7);

				packuswb(m3, m3);

				movdqu(ptr[r0], m3);

				lea(r2, ptr[r2 + 16]);
				lea(r3, ptr[r3 + 16]);
				lea(r0, ptr[r0 + 8]);
			}
			sub(r2, 2 * width);
			sub(r3, 2 * width);
			sub(r0, width);

			add(r2, r4);
			add(r3, r4);
			add(r0, stride_dst);
		}
		dec(r6d);
		jg("loopBiV");
#undef coeffA
#undef coeffB
#undef stride_dst
	}
};


void wrap(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int w, int h, int xFrac, int yFrac)
{
	static Jit::Buffer buffer(100000);
	hevcasm_pred_uni_8to8 *f=0;
	static PredUni a(&buffer, f, 8, 16, xFrac, yFrac, 8);
	f = a;
	f(dst, stride_dst, ref, stride_ref, w, h, xFrac, yFrac);
}



static hevcasm_pred_uni_8to8* get_pred_uni_8to8(int taps, int w, int h, int xFrac, int yFrac, hevcasm_instruction_set mask)
{
	hevcasm_pred_uni_8to8 *f = 0;
	if (mask & (HEVCASM_C_REF | HEVCASM_C_OPT))
	{
		if (taps == 8)
		{
			/* luma 8-tap filter */

			if (xFrac)
			{
				if (yFrac)
				{
					f = hevcasm_pred_uni_8tap_8to8_hv;
				}
				else
				{
					f = hevcasm_pred_uni_8tap_8to8_h;
				}
			}
			else
			{
				if (yFrac)
				{
					f = hevcasm_pred_uni_8tap_8to8_v;
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
					f = hevcasm_pred_uni_4tap_8to8_hv;
				}
				else
				{
					f = hevcasm_pred_uni_4tap_8to8_h;
				}
			}
			else
			{
				if (yFrac)
				{
					f = hevcasm_pred_uni_4tap_8to8_v;
				}
				else
				{
					f = hevcasm_pred_uni_copy_block;
				}
			}
		}
	}
	
	static Jit::Buffer buffer(100000);

	if (mask & HEVCASM_SSE2)
	{
		if (!xFrac && !yFrac)
		{
			if (w <= 64)
			{
				static PredUniCopy a(&buffer, 64);
				f = a;
			}
			if (w <= 48)
			{
				static PredUniCopy a(&buffer, 48);
				f = a;
			}
			if (w <= 32)
			{
				static PredUniCopy a(&buffer, 32);
				f = a;
			}
			if (w <= 16)
			{
				static PredUniCopy a(&buffer, 16);
				f = a;
			}
		}
	}

	if (mask & HEVCASM_SSE41)
	{
		if (xFrac && !yFrac)
		{
			if (taps == 8)
			{
				if (w <= 64)
				{
					static PredUni a(&buffer, f, taps, 64, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 48)
				{
					static PredUni a(&buffer, f,  taps, 48, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 32)
				{
					static PredUni a(&buffer, f, taps, 32, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 16)
				{
					static PredUni a(&buffer, f, taps, 16, xFrac, yFrac, 8);
					f = a;
				}
			}
			if (taps == 4)
			{
				if (w <= 64)
				{
					static PredUni a(&buffer, f, taps, 64, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 48)
				{
					static PredUni a(&buffer, f, taps, 48, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 32)
				{
					static PredUni a(&buffer, f, taps, 32, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 16)
				{
					static PredUni a(&buffer, f, taps, 16, xFrac, yFrac, 8);
					f = a;
				}
			}
		}
		if (!xFrac && yFrac)
		{
			if (taps == 8)
			{
				if (w <= 64)
				{
					static PredUni a(&buffer, f, taps, 64, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 48)
				{
					static PredUni a(&buffer, f, taps, 48, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 32)
				{
					static PredUni a(&buffer, f, taps, 32, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 24)
				{
					static PredUni a(&buffer, f, taps, 24, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 16)
				{
					static PredUni a(&buffer, f, taps, 16, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 8)
				{
					static PredUni a(&buffer, f, taps, 8, xFrac, yFrac, 8);
					f = a;
				}
			}
			if (taps == 4)
			{
				if (w <= 64)
				{
					static PredUni a(&buffer, f, taps, 64, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 48)
				{
					static PredUni a(&buffer, f, taps, 48, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 32)
				{
					static PredUni a(&buffer, f, taps, 32, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 24)
				{
					static PredUni a(&buffer, f, taps, 24, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 16)
				{
					static PredUni a(&buffer, f, taps, 16, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 8)
				{
					static PredUni a(&buffer, f, taps, 8, xFrac, yFrac, 8);
					f = a;
				}
			}
		}
		if (xFrac && yFrac)
		{
			if (taps == 8)
			{
				if (w <= 64)
				{
					static PredUni a(&buffer, f, taps, 64, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 48)
				{
					static PredUni a(&buffer, f, taps, 48, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 32)
				{
					static PredUni a(&buffer, f, taps, 32, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 16)
				{
					f = wrap;
					//static PredUni a(&buffer, f,  taps, 16, xFrac, yFrac, 8);
					//f = a;
				}
			}
			if (taps == 4)
			{
				if (w <= 64)
				{
					static PredUni a(&buffer, f, taps, 64, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 48)
				{
					static PredUni a(&buffer, f, taps, 48, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 32)
				{
					static PredUni a(&buffer, f, taps, 32, xFrac, yFrac, 8);
					f = a;
				}
				if (w <= 16)
				{
					static PredUni a(&buffer, f, taps, 16, xFrac, yFrac, 8);
					f = a;
				}
			}
		}
	}
	return f;
}




void hevcasm_populate_pred_uni_8to8(hevcasm_table_pred_uni_8to8 *table, hevcasm_instruction_set mask)
{
	for (int taps = 4; taps <= 8; taps += 4)
	{
		for (int w = 0; w <= 8 * taps; w += taps)
		{
			for (int xFrac = 0; xFrac < 2; ++xFrac)
			{
				for (int yFrac = 0; yFrac < 2; ++yFrac)
				{
					*hevcasm_get_pred_uni_8to8(table, taps, w, 0, xFrac, yFrac)
						= get_pred_uni_8to8(taps, w, 0, xFrac, yFrac, mask);
				}
			}
		}
	}
}



#define STRIDE_DST 192

typedef struct 
{
	HEVCASM_ALIGN(32, uint8_t, dst[64 * STRIDE_DST]);
	hevcasm_pred_uni_8to8 *f;
	ptrdiff_t stride_dst;
	const uint8_t *ref;
	ptrdiff_t stride_ref;
	int w;
	int h;
	int xFrac;
	int yFrac;
	int taps;
}
bound_pred_uni;


static int get_pred_uni(void *p, hevcasm_instruction_set mask)
{
	bound_pred_uni *s = (bound_pred_uni *)p;

	hevcasm_table_pred_uni_8to8 table;

	hevcasm_populate_pred_uni_8to8(&table, mask);

	s->f = *hevcasm_get_pred_uni_8to8(&table, s->taps, s->w, s->h, s->xFrac, s->yFrac);

	assert(s->f == get_pred_uni_8to8(s->taps, s->w, s->h, s->xFrac, s->yFrac, mask));

	if (s->f && mask == HEVCASM_C_REF)
	{
		printf("\t%dx%d %d-tap %s%s : ", s->w, s->h, s->taps, s->xFrac ? "H" : "", s->yFrac ? "V" : "");
	}

	memset(s->dst, 0, 64 * s->stride_dst);

	return !!s->f;
}


void invoke_pred_uni(void *p, int n)
{
	bound_pred_uni *s = (bound_pred_uni *)p;
	while (n--)
	{
		s->f(s->dst, s->stride_dst, s->ref, s->stride_ref, s->w, s->h, s->xFrac, s->yFrac);
	}
}


int mismatch_pred_uni(void *boundRef, void *boundTest)
{
	bound_pred_uni *ref = (bound_pred_uni *)boundRef;
	bound_pred_uni *test = (bound_pred_uni *)boundTest;

	for (int y = 0; y < ref->h; ++y)
	{
		if (memcmp(&ref->dst[y*ref->stride_dst], &test->dst[y*test->stride_dst], ref->w)) return 1;
	}

	return 0;
}


static void test_partitions(int *error_count, bound_pred_uni *b, hevcasm_instruction_set mask)
{
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

		b[0].w = nPbW * b[0].taps / 8;
		b[0].h = nPbH * b[0].taps / 8;

		b[1] = b[0];

		*error_count += hevcasm_test(&b[0], &b[1], get_pred_uni, invoke_pred_uni, mismatch_pred_uni, mask, 1000);
	}
}


void HEVCASM_API hevcasm_test_pred_uni(int *error_count, hevcasm_instruction_set mask)
{
	printf("\nhevcasm_pred_uni - Unireference Inter Prediction (single-reference motion compensation)\n");

	bound_pred_uni b[2];

#define STRIDE_REF 192
	HEVCASM_ALIGN(32, uint8_t, ref[80 * STRIDE_REF]);
	b[0].stride_dst = STRIDE_DST;
	b[0].stride_ref = STRIDE_REF;
	b[0].ref = ref + 8 * b[0].stride_ref;
#undef STRIDE_DST
#undef STRIDE_REF

	for (int x = 0; x < 80 * b[0].stride_ref; x++) ref[x] = rand() & 0xff;

	for (b[0].taps = 8; b[0].taps >= 4; b[0].taps -= 4)
	{
		for (b[0].yFrac = 0; b[0].yFrac < 2; ++b[0].yFrac)
		{
			for (b[0].xFrac = 0; b[0].xFrac < 2; ++b[0].xFrac)
			{
				test_partitions(error_count, b, mask);
			}
		}
	}
}


void hevcasm_pred_bi_mean_16and16to8_c_ref(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *ref0, const int16_t *ref1, ptrdiff_t stride_ref, int w, int h)
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
void hevcasm_pred_bi_ ## taps ## tap_8to8_c_ref(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref0, const uint8_t *ref1, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac0, int yFrac0, int xFrac1, int yFrac1) \
{ \
	int16_t intermediate[4][(64 + taps - 1) * 64]; \
	 \
	const int w = nPbW; \
	const int h = nPbH; \
	 \
	/* Horizontal filter */ \
	hevcasm_pred_uni_generic(intermediate[2], 2, 64, ref0 - (taps/2-1) * stride_ref, 1, stride_ref, w, h + taps - 1, 1, taps, xFrac0, 0, 0); \
	 \
	/* Vertical filter */ \
	hevcasm_pred_uni_generic(intermediate[0], 2, 64, intermediate[2] + (taps / 2 - 1) * 64, 2, 64, w, h, 64, taps, yFrac0, 6, 0); \
	 \
	/* Horizontal filter */ \
	hevcasm_pred_uni_generic(intermediate[3], 2, 64, ref1 - (taps / 2 - 1) * stride_ref, 1, stride_ref, w, h + taps - 1, 1, taps, xFrac1, 0, 0); \
	 \
	/* Vertical filter */ \
	hevcasm_pred_uni_generic(intermediate[1], 2, 64, intermediate[3] + (taps / 2 - 1) * 64, 2, 64, w, h, 64, taps, yFrac1, 6, 0); \
	 \
	/* Combine two references for bi pred */ \
	hevcasm_pred_bi_mean_16and16to8_c_ref(dst, stride_dst, intermediate[0], intermediate[1], 64, nPbW, nPbH); \
} \

MAKE_hevcasm_pred_bi_xtap_8to8(8)
MAKE_hevcasm_pred_bi_xtap_8to8(4)




struct PredBi
	:
	Jit::Function
{
	PredBi(Jit::Buffer *buffer, int width, int taps=0)
		:
		Jit::Function(buffer, Jit::CountArguments<hevcasm_pred_bi_8to8>::value),
		width(width),
		taps(taps)
	{
		this->build();
	}

	int width, taps;

	void assembleInterp()
	{
		auto &m0 = regXmm(0);
		auto &m1 = regXmm(1);
		auto &m2 = regXmm(2);
		auto &m3 = regXmm(3);
		auto &m4 = regXmm(4);
		auto &m5 = regXmm(5);
		auto &m6 = regXmm(6);
		auto &m7 = regXmm(7);

		auto &r0 = arg64(0); // dst
		auto &r1 = arg64(1);
		auto &r2 = arg64(2); //ref0
		auto &r3 = arg64(3); //ref1
		auto &r4 = arg64(4); 
		auto &r5 = arg64(5);// w
		auto &r6 = arg64(6);// h
		auto &r7 = arg64(7);// xFrac0
		auto &r8 = arg64(8);// yFrac0
		auto &r9 = arg64(9);// xFrac1
		auto &r10 = arg64(10);// yFrac1

		this->stackSize = (64 + taps - 1) * 64 * 2 * 2 + 12 * 8; 

		// cautious __chkstk
		for (int i = this->stackSize - 8; i > 8; i -= 2048)
		{
			mov(ptr[rsp + i], r0);
		}

		// save all arguments in allocated stack memory
		for (int i = 0; i < 11; ++i)
		{
			mov(ptr[rsp + 8 * i], reg64(i));
		}

		// setup arguments to call H filter (ref0)
		lea(r0, ptr[rsp + 12 * 8]);
		mov(r1, 64);
		mov(r3, r4);
		neg(r4);
		if (taps == 8)
		{
			lea(r2, ptr[r2 + r4 * 2]);
		}
		lea(r2, ptr[r2 + r4]);
		mov(r4, r5); // w
		mov(r5, r6); // h
		add(r5, taps - 1);
		mov(r6, r7);// xFrac
		xor (r7, r7); // yFrac

		// naked call (registers preserved)
		call(labelPRED_UNI_H_16NxH);

		// restore arguments from stack memory
		for (int i = 0; i < 11; ++i)
		{
			mov(reg64(i), ptr[rsp + 8 * i] );
		}

		// setup arguments to call H filter (ref1)
		lea(r0, ptr[rsp + 12 * 8 + (64 + taps - 1) * 64 * 2]);
		mov(r1, 64);
		mov(r2, r3);
		mov(r3, r4);
		neg(r4);
		if (taps == 8)
		{
			lea(r2, ptr[r2 + r4 * 2]);
		}
		lea(r2, ptr[r2 + r4]);
		mov(r4, r5); // w
		mov(r5, r6); // h
		add(r5, taps - 1);
		mov(r6, r9);// xFrac
		xor (r7, r7); // yFrac

		// naked call (registers preserved)
		call(labelPRED_UNI_H_16NxH);

		// restore arguments from stack memory
		for (int i = 0; i < 11; ++i)
		{
			mov(reg64(i), ptr[rsp + 8 * i]);
		}


		lea(r2, ptr[rsp + 12 * 8]); // refAtop
		lea(r3, ptr[rsp + 12 * 8 + (64 + taps - 1) * 64 * 2]); // refBtop
		mov(r4, 64); // stride_ref

		// inline "function"
		// void hevcasm_pred_bi_v_%1tap_16to16_%3xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *refAtop, const int16_t *refBtop, ptrdiff_t stride_ref, int nPbW, int nPbH, int yFracA, int yFracB)//
		this->PRED_BI_V_8NxH(this->taps, 16, this->width);



		//p16(intermediate[2], 64, ref0 - (taps / 2 - 1) * stride_ref, stride_ref, w, h + taps - 1, xFrac0, 0); \

		
	}

	void assemble() override
	{
		if (taps)
		{
			assembleInterp();
			return;
		}

		auto &r0 = arg64(0);
		auto &r1 = arg64(1);
		auto &r2 = arg64(2);
		auto &r3 = arg64(3);
		auto &r4 = arg64(4);
		auto &r5 = arg64(5);
		auto r6d = Xbyak::Reg32(arg64(6).getIdx());

		auto &m0 = regXmm(0);
		auto &m1 = regXmm(1);

		L("loop");
		{
			for (int dx = 0; dx < width; dx += 16)
			{
				movdqu(m0, ptr[r2 + dx]);
				movdqu(m1, ptr[r3 + dx]);
				pavgb(m0, m1);
				movdqu(ptr[r0 + dx], m0);
			}
			lea(r2, ptr[r2 + r4]);
			lea(r3, ptr[r3 + r4]);
			lea(r0, ptr[r0 + r1]);
		}
		dec(r6d);
		jg("loop");
	}

	Xbyak::Label coefficients;
	Xbyak::Label coefficients16;
	Xbyak::Label constant_times_4_dd_0x800;
	Xbyak::Label constant_times_8_dw_0x20;
	Xbyak::Label constant_times_8_dw_0x40;
	Xbyak::Label labelPRED_UNI_H_16NxH;
	Xbyak::Label labelPRED_BI_V_8NxH;

	void data() override
	{
		align();

		L(coefficients);
		for (int frac = 0; frac < (12 - taps); ++frac)
		{
			for (int k = 0; k < taps; k += 2)
			{
				int coeff0 = hevcasm_pred_coefficient(taps, frac, k);
				int coeff1 = hevcasm_pred_coefficient(taps, frac, k + 1);

				db({ coeff0, coeff1 }, 8);

			}
		}

		L(coefficients16);
		for (int frac = 0; frac < (12 - taps); ++frac)
		{
			for (int k = 0; k < taps; k += 2)
			{
				int coeff0 = hevcasm_pred_coefficient(taps, frac, k);
				int coeff1 = hevcasm_pred_coefficient(taps, frac, k + 1);

				dw({ coeff0, coeff1 }, 4);
			}
		}

		L(constant_times_4_dd_0x800);
		dd({0x800}, 4);

		L(constant_times_8_dw_0x20);
		dw({ 0x20 }, 8);

		L(constant_times_8_dw_0x40);
		dw({ 0x40 }, 8);

		if (this->taps)
		{
			L(labelPRED_UNI_H_16NxH);
			this->PRED_UNI_H_16NxH(this->taps, 16, this->width);
			ret();
		}
	}

	// taps is number of filter taps (4 or 8)
	// outputTypeBits is size of output type (8 for uint8_t rounded, 16 for int16_t right shifted 6)
	// dx is dx (horizontal offset as integer number of samples) 
	void PRED_UNI_H_16x1(int taps, int outputTypeBits, int dx)
	{
		auto &r0 = reg64(0);
		auto &r2 = reg64(2);

		auto &m0 = xmm0;
		auto &m1 = xmm1;
		auto &m2 = xmm2;
		auto &m3 = xmm3;
		auto &m4 = xmm4;
		auto &m5 = xmm5;
		auto &m6 = xmm6;
		auto &m7 = xmm7;

		movdqu(m2, ptr[r2 + 1 - (taps / 2) + dx]);
		pmaddubsw(m2, m4);
		// m2 = eca86420 (even positions, first two taps)

		movdqu(m1, ptr[r2 + 2 - (taps / 2) + dx]);
		pmaddubsw(m1, m4);
		// m1 = fdb97531 (odd positions, first two taps)

		movdqu(m0, ptr[r2 + 3 - (taps / 2) + dx]);
		pmaddubsw(m0, m5);
		// m0 = eca86420 (even positions, two taps)

		paddw(m2, m0);
		// m2 = eca86420 (even positions)

		movdqu(m0, ptr[r2 + 4 - (taps / 2) + dx]);
		pmaddubsw(m0, m5);
		// m0 = fdb97531 (odd positions, two taps)

		paddw(m1, m0);
		// m1 = fdb97531 (odd positions)

		if (taps == 8)
		{
			// need four more taps...

			movdqu(m0, ptr[r2 + 5 - (taps / 2) + dx]);
			pmaddubsw(m0, m6);
			// m0 = eca86420(even positions, two taps)

			paddw(m2, m0);
			// m2 = eca86420(even positions)

			movdqu(m0, ptr[r2 + 6 - (taps / 2) + dx]);
			pmaddubsw(m0, m6);
			// m0 = fdb97531(odd positions, two taps)

			paddw(m1, m0);
			// m1 = fdb97531(odd positions)

			movdqu(m0, ptr[r2 + 7 - (taps / 2) + dx]);
			pmaddubsw(m0, m7);
			// m0 = eca86420(even positions, two taps)

			paddw(m2, m0);
			// m2 = eca86420(even positions)

			movdqu(m0, ptr[r2 + 8 - (taps / 2) + dx]);
			pmaddubsw(m0, m7);
			// m0 = fdb97531(odd positions, two taps)

			paddw(m1, m0);
			// m1 = fdb97531(odd positions)
		}

		movq(m0, m2);
		punpcklwd(m0, m1);
		// m0 = 76543210

		punpckhwd(m2, m1);
		// m2 = fedcba98

		if (outputTypeBits == 16)
		{
			movdqu(ptr[r0 + 2 * dx], m0);
			movdqu(ptr[r0 + 2 * dx + 16], m2);
		}
		else
		{
			paddw(m0, ptr[rip + constant_times_8_dw_0x20]);
			paddw(m2, ptr[rip + constant_times_8_dw_0x20]);
			psraw(m0, 6);
			psraw(m2, 6);
			packuswb(m0, m2);
			movdqu(ptr[r0 + dx], m0);
		}
	}


	// taps is number of filter taps (4 or 8)
	// outputTypeBits is size of output type (8 for uint8_t rounded, 16 for int16_t right shifted 6)
	// width is block width (number of samples, multiple of 16)
	void PRED_UNI_H_16NxH(int taps, int outputTypeBits, int width)
	{
		auto &r0 = reg64(0);
		auto &r1 = reg64(1);
		auto &r2 = reg64(2);
		auto &r3 = reg64(3);
		auto &r4 = reg64(4);
		auto &r5 = reg64(5);
		Xbyak::Reg32 r5d(r5.getIdx());
		auto &r6 = reg64(6);
		Xbyak::Reg32 r6d(r6.getIdx());

		auto &m0 = regXmm(0);
		auto &m1 = regXmm(1);
		auto &m2 = regXmm(2);
		auto &m3 = regXmm(3);
		auto &m4 = regXmm(4);
		auto &m5 = regXmm(5);
		auto &m6 = regXmm(6);
		auto &m7 = regXmm(7);

		if (taps == 8)
		{
			shl(r6d, 6);  // frac *= 4 * 16
		}
		else
		{
			shl(r6d, 5);  // frac *= 2 * 16
		}
		lea(r4, ptr[rip + coefficients]);

		movdqa(m4, ptr[r4 + r6]);
		movdqa(m5, ptr[r4 + r6 + 1 * 16]);
		if (taps == 8)
		{
			movdqa(m6, ptr[r4 + r6 + 2 * 16]);
			movdqa(m7, ptr[r4 + r6 + 3 * 16]);
		}

		if (outputTypeBits == 16)
		{
			// dst is int16_t * so need to double the stride
			shl(r1, 1);  // 
		}

		L("loopHH");
		{
			for (int i = 0; i < width / 16; ++i)
			{
				int const dx = 16 * i;
				PRED_UNI_H_16x1(taps, outputTypeBits, dx);
			}

			add(r0, r1);
			add(r2, r3);
		}
		dec(r5d);
		jg("loopHH");
	}

	// void hevcasm_pred_bi_v_%1tap_16to16_%3xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *refAtop, const int16_t *refBtop, ptrdiff_t stride_ref, int nPbW, int nPbH, int yFracA, int yFracB)//
	void PRED_BI_V_8NxH(int taps, int inputTypeSize, int width)
	{
		// taps is number of filter taps (4 or 8)//
		// inputTypeSize is size of input type (8 for uint8_t, 16 for int16_t right shifted 6)
		// width is block width (number of samples, multiple of 8)
		assert(inputTypeSize == 16);

		// // void hevcasm_pred_bi_v_%1tap_16to16_%3xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *refAtop, const int16_t *refBtop, ptrdiff_t stride_ref, int nPbW, int nPbH, int yFracA, int yFracB)//
		// INIT_XMM sse4
		// cglobal pred_bi_v_%1tap_16to16_%3xh, 9, 9, 8

		auto &r0 = arg64(0); // dst
		auto &r1 = arg64(1); // stride_dst
		auto &r2 = arg64(2); // refA
		auto &r3 = arg64(3); // refB
		auto &r4 = arg64(4); // stride_ref
		auto &r5 = arg64(5); // width
		auto &r6 = arg64(6);
		auto r6d = Xbyak::Reg32(r6.getIdx()); // height
		auto &r7 = arg64(7);
		auto r7d = Xbyak::Reg32(r7.getIdx()); // yFracA
		auto &r8 = arg64(8);
		auto r8d = Xbyak::Reg32(r8.getIdx()); // yFracB

		auto &m0 = regXmm(0);
		auto &m1 = regXmm(1);
		auto &m2 = regXmm(2);
		auto &m3 = regXmm(3);
		auto &m4 = regXmm(4);
		auto &m5 = regXmm(5);
		auto &m6 = regXmm(6);
		auto &m7 = regXmm(7);

		shl(r4, 1);

#if 1 // ARCH_X86_64
		shl(r7d, 4 + taps / 4);
		shl(r8d, 4 + taps / 4);
		lea(r5, ptr[rip + coefficients16]);
		lea(r7, ptr[r5 + r7]);
		lea(r8, ptr[r5 + r8]);
#define coeffA r7
#define coeffB r8
#define stride_dst r1
#else
		mov r5, r7m
			shl r5, 4 + taps / 4
			lea r1, [pred_inter_ % 1tap_coefficient_pairs_4_dw + r5]
			mov r5, r8m
			shl r5, 4 + taps / 4
			lea r5, [pred_inter_ % 1tap_coefficient_pairs_4_dw + r5]
			% define coeffA r1
			%define coeffB r5
			%define stride_dst r1m
#endif
			L("loopBiV");
		{
			for (int i = 0; i < width / 8; ++i)
			{
				// each iteration of this loop operates on a row of 8-samples

				pxor(m3, m3);
				movdqa(m5, m3);
				movdqa(m6, m3);
				movdqa(m7, m3);

				for (int j = 0; j < taps / 2; ++j)
				{
					// each iteration of this loop performs two filter taps

					// reference picture A
					movdqu(m0, ptr[r2]);
					movdqu(m1, ptr[r2 + r4]);
					movdqa(m2, m0);
					punpckhwd(m2, m1);
					punpcklwd(m0, m1);
					pmaddwd(m0, ptr[coeffA]);
					pmaddwd(m2, ptr[coeffA]);
					paddd(m3, m0);
					paddd(m5, m2);
					lea(r2, ptr[r2 + r4 * 2]);

					// reference picture B
					movdqu(m0, ptr[r3]);
					movdqu(m1, ptr[r3 + r4]);
					movdqa(m2, m0);
					punpckhwd(m2, m1);
					punpcklwd(m0, m1);
					pmaddwd(m0, ptr[coeffB]);
					pmaddwd(m2, ptr[coeffB]);
					paddd(m6, m0);
					paddd(m7, m2);
					lea(r3, ptr[r3 + r4 * 2]);

					add(coeffA, 16);
					add(coeffB, 16);
				}

				neg(r4);
				lea(r2, ptr[r2 + r4 * taps]);
				lea(r3, ptr[r3 + r4 * taps]);
				neg(r4);

				sub(coeffA, 8 * taps);
				sub(coeffB, 8 * taps);

				psrad(m3, 6);
				psrad(m5, 6);
				psrad(m6, 6);
				psrad(m7, 6);

				packssdw(m3, m5);
				packssdw(m6, m7);

				paddsw(m3, m6);
				paddsw(m3, ptr[rip + constant_times_8_dw_0x40]);
				psraw(m3, 7);

				packuswb(m3, m3);

				movdqu(ptr[r0], m3);

				lea(r2, ptr[r2 + 16]);
				lea(r3, ptr[r3 + 16]);
				lea(r0, ptr[r0 + 8]);
			}
			sub(r2, 2 * width);
			sub(r3, 2 * width);
			sub(r0, width);

			add(r2, r4);
			add(r3, r4);
			add(r0, stride_dst);
		}
		dec(r6d);
		jg("loopBiV");
#undef coeffA
#undef coeffB
#undef stride_dst
	}

};



#define MAKE_hevcasm_pred_bi_xtap_8to8_sse4(taps, width) \
	\
	void hevcasm_pred_bi_ ## taps ## tap_8to8_ ## width ## xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref0, const uint8_t *ref1, ptrdiff_t stride_ref, int w, int h, int xFrac0, int yFrac0, int xFrac1, int yFrac1) \
{ \
	HEVCASM_ALIGN(32, int16_t, intermediate[4][(64 + taps - 1) * 64]); /* todo: can reduce stack usage here by making array size function of width */ \
	\
	const int nPbW = w; \
	const int nPbH = h; \
	 \
	/* Horizontal filter */ \
	 \
	static Jit::Buffer buffer(20000); \
	hevcasm_pred_uni_8to16 *p16=0; \
	static PredUni a(&buffer, p16, taps, width, 1, 0, 8); \
	p16 = a; \
	p16(intermediate[2], 64, ref0 - (taps/2-1) * stride_ref, stride_ref, w, h + taps - 1, xFrac0, 0); \
	p16(intermediate[3], 64, ref1 - (taps/2-1) * stride_ref, stride_ref, w, h + taps - 1, xFrac1, 0); \
	\
	hevcasm_pred_bi_v_16to16 *p=0; \
	static PredUni predUni(&buffer, p, taps, width, 1, 1, 16); \
	p = predUni; \
	\
	/* Two vertical filters and combine their output for bi pred */ \
	if (0) p(dst, stride_dst, intermediate[2], intermediate[3], 64, w, h, yFrac0, yFrac1); \
	\
	hevcasm_pred_bi_8to8 *pp = 0; \
	static PredBi ab(&buffer, width, taps); \
	hevcasm_pred_bi_8to8 *pb = ab; \
	pb(dst, stride_dst, ref0, ref1, stride_ref, w, h, xFrac0, xFrac1, xFrac1, yFrac1); \
} \


MAKE_hevcasm_pred_bi_xtap_8to8_sse4(8, 16)
MAKE_hevcasm_pred_bi_xtap_8to8_sse4(8, 32)
MAKE_hevcasm_pred_bi_xtap_8to8_sse4(8, 48)
MAKE_hevcasm_pred_bi_xtap_8to8_sse4(8, 64)

MAKE_hevcasm_pred_bi_xtap_8to8_sse4(4, 16)
MAKE_hevcasm_pred_bi_xtap_8to8_sse4(4, 32)

hevcasm_pred_bi_8to8* get_pred_bi_8to8(int taps, int w, int h, int xFracA, int yFracA, int xFracB, int yFracB, hevcasm_instruction_set mask)
{
	hevcasm_pred_bi_8to8 *f = 0;
	
	if (mask & (HEVCASM_C_REF | HEVCASM_C_OPT))
	{
		if (taps == 8) f = hevcasm_pred_bi_8tap_8to8_c_ref;
		if (taps == 4) f = hevcasm_pred_bi_4tap_8to8_c_ref;
	}

	if (mask & HEVCASM_SSE2)
	{
		if (!xFracA && !yFracA && !xFracB && !yFracB)
		{
			static Jit::Buffer buffer(10000);
			if (w <= 64)
			{
				static PredBi a(&buffer, 64);
				f = a;
			}
			if (w <= 48)
			{
				static PredBi a(&buffer, 48);
				f = a;
			}
			if (w <= 32)
			{
				static PredBi a(&buffer, 32);
				f = a;
			}
			if (w <= 16)
			{
				static PredBi a(&buffer, 16);
				f = a;
			}

		}
	}

	if (mask & HEVCASM_SSE41)
	{
		static Jit::Buffer buffer(40000);
		if (taps == 8 && (xFracA || yFracA || xFracB || yFracB))
		{
			if (w <= 64)
			{
				static PredBi a(&buffer, 64, taps);
				f = a;
			}
			if (w <= 48)
			{
				static PredBi a(&buffer, 48, taps);
				f = a;
			}
			if (w <= 32)
			{
				static PredBi a(&buffer, 32, taps);
				f = a;
			}
			if (w <= 16)
			{
				static PredBi a(&buffer, 16, taps);
				f = a;
			}
		}
		if (taps == 4 && (xFracA || yFracA || xFracB || yFracB))
		{
			if (w <= 32)
			{
				static PredBi a(&buffer, 32, taps);
				f = a;
			}
			if (w <= 16)
			{
				static PredBi a(&buffer, 16, taps);
				f = a;
			}
		}
	}

	return f;
}


void hevcasm_populate_pred_bi_8to8(hevcasm_table_pred_bi_8to8 *table, hevcasm_instruction_set mask)
{
	for (int taps = 4; taps <= 8; taps += 4)
	{
		for (int w = 0; w <= 8 * taps; w += 2 * taps)
		{
			for (int frac = 0; frac < 2; ++frac)
			{
				*hevcasm_get_pred_bi_8to8(table, taps, w, 0, frac, frac, frac, frac)
					= get_pred_bi_8to8(taps, w, 0, frac, frac, frac, frac, mask);
			}
		}
	}
}



#define STRIDE_DST 192

typedef struct
{
	hevcasm_pred_bi_8to8 *f;
	HEVCASM_ALIGN(32, uint8_t, dst[64 * STRIDE_DST]);
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
	int taps;
}
bound_pred_bi;


int init_pred_bi_8to8(void *p, hevcasm_instruction_set mask)
{
	bound_pred_bi *s = (bound_pred_bi *)p;

	hevcasm_table_pred_bi_8to8 table;

	hevcasm_populate_pred_bi_8to8(&table, mask);

	s->f = *hevcasm_get_pred_bi_8to8(&table, s->taps, s->w, s->h, s->xFracA, s->yFracA, s->xFracB, s->yFracB);

	assert(s->f == get_pred_bi_8to8(s->taps, s->w, s->h, s->xFracA, s->yFracA, s->xFracB, s->yFracB, mask));

	if (s->f && mask == HEVCASM_C_REF)
	{
		printf("\t%dx%d %d-tap %s%s %s%s : ", s->w, s->h, s->taps, s->xFracA ? "H" : "", s->yFracA ? "V" : "", s->xFracB ? "H" : "", s->yFracB ? "V" : "");
	}

	memset(s->dst, 0, 64 * s->stride_dst);

	return !!s->f;

}


void invoke_pred_bi_8to8(void *p, int n)
{
	bound_pred_bi *s = (bound_pred_bi *)p;
	while (n--)
	{
		s->f(s->dst, s->stride_dst, s->refA, s->refB, s->stride_ref, s->w, s->h, s->xFracA, s->yFracA, s->xFracB, s->yFracB);
	}
}


int mismatch_pred_bi_8to8(void *boundRef, void *boundTest)
{
	bound_pred_bi *ref = (bound_pred_bi *)boundRef;
	bound_pred_bi *test = (bound_pred_bi *)boundTest;

	for (int y = 0; y < ref->h; ++y)
	{
		if (memcmp(&ref->dst[y*ref->stride_dst], &test->dst[y*test->stride_dst], ref->w)) return 1;
	}

	return 0;
}


static void test_partitions_bi(int *error_count, bound_pred_bi *b, hevcasm_instruction_set mask)
{
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

		b[0].w = nPbW * b[0].taps / 8;
		b[0].h = nPbH * b[0].taps / 8;

		b[1] = b[0];

		*error_count += hevcasm_test(&b[0], &b[1], init_pred_bi_8to8, invoke_pred_bi_8to8, mismatch_pred_bi_8to8, mask, 1000);
	}
}


void HEVCASM_API hevcasm_test_pred_bi(int *error_count, hevcasm_instruction_set mask)
{
	printf("\nhevcasm_pred_bi - Bireference Inter Prediction (two-reference motion compensation)\n");

	bound_pred_bi b[2];

#define STRIDE_REF 192
	HEVCASM_ALIGN(32, uint8_t, ref[2][80 * STRIDE_REF]);
	b[0].stride_dst = STRIDE_DST;
	b[0].stride_ref = STRIDE_REF;
#undef STRIDE_DST
#undef STRIDE_REF

	for (int x = 0; x < 80 * b[0].stride_ref; x++)
	{
		ref[0][x] = rand() & 0xff;
		ref[1][x] = rand() & 0xff;
	}

	b[0].refA = ref[0] + 8 * b[0].stride_ref;
	b[0].refB = ref[1] + 8 * b[0].stride_ref;

	for (b[0].taps = 8; b[0].taps >= 4; b[0].taps -= 4)
	{
		//b[0].xFracA = b[0].yFracA = b[0].xFracB = b[0].yFracB = 0;
		//test_partitions_bi(error_count, b, mask);

		b[0].xFracA = b[0].yFracA = b[0].xFracB = b[0].yFracB = 1;
		test_partitions_bi(error_count, b, mask);
	}
}
