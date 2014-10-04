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

#include "residual_decode_a.h"
#include "residual_decode.h"
#include "hevcasm_test.h"

#include <stdlib.h>
#include <string.h>


static int Clip3(int min, int max, int x)
{
	if (x > max) return max;
	if (x < min) return min;
	return x;
}


static void hevcasm_inverse_partial_butterfly_8x8_c_opt(int16_t dst[8 * 8], const int16_t src[8 * 8], int shift)
{
	const int add = 1 << (shift - 1);
	const int srcStride = 8;
	const int dstStride = 8;

	for (int j = 0; j<8; ++j)
	{
		static const int16_t lookup[8][8] =
		{
			{ 64, 64, 64, 64, 64, 64, 64, 64 },
			{ 89, 75, 50, 18, -18, -50, -75, -89 },
			{ 83, 36, -36, -83, -83, -36, 36, 83 },
			{ 75, -18, -89, -50, 50, 89, 18, -75 },
			{ 64, -64, -64, 64, 64, -64, -64, 64 },
			{ 50, -89, 18, 75, -75, -18, 89, -50 },
			{ 36, -83, 83, -36, -36, 83, -83, 36 },
			{ 18, -50, 75, -89, 89, -75, 50, -18 }
		};

		int O[4];
		for (int k = 0; k<4; ++k)
		{
			O[k] = lookup[1][k] * src[1 * srcStride] + lookup[3][k] * src[3 * srcStride] + lookup[5][k] * src[5 * srcStride] + lookup[7][k] * src[7 * srcStride];
		}

		int EE[2], EO[2];
		EO[0] = lookup[2][0] * src[2 * srcStride] + lookup[6][0] * src[6 * srcStride];
		EO[1] = lookup[2][1] * src[2 * srcStride] + lookup[6][1] * src[6 * srcStride];
		EE[0] = lookup[0][0] * src[0 * srcStride] + lookup[4][0] * src[4 * srcStride];
		EE[1] = lookup[0][1] * src[0 * srcStride] + lookup[4][1] * src[4 * srcStride];

		int E[4];
		E[0] = EE[0] + EO[0];
		E[3] = EE[0] - EO[0];
		E[1] = EE[1] + EO[1];
		E[2] = EE[1] - EO[1];

		for (int k = 0; k<4; ++k)
		{
			dst[k] = Clip3(-32768, 32767, (E[k] + O[k] + add) >> shift);
			dst[k + 4] = Clip3(-32768, 32767, (E[3 - k] - O[3 - k] + add) >> shift);
		}

		src++;
		dst += dstStride;
	}
}


static void hevcasm_inverse_partial_butterfly_16x16_c_opt(int16_t dst[16 * 16], const int16_t src[16 * 16], int shift)
{
	const int add = 1 << (shift - 1);
	const int srcStride = 16;
	const int dstStride = 16;

	for (int j = 0; j<16; ++j)
	{
		static const int16_t lookup[16][16] =
		{
			{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
			{ 90, 87, 80, 70, 57, 43, 25, 9, -9, -25, -43, -57, -70, -80, -87, -90 },
			{ 89, 75, 50, 18, -18, -50, -75, -89, -89, -75, -50, -18, 18, 50, 75, 89 },
			{ 87, 57, 9, -43, -80, -90, -70, -25, 25, 70, 90, 80, 43, -9, -57, -87 },
			{ 83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83 },
			{ 80, 9, -70, -87, -25, 57, 90, 43, -43, -90, -57, 25, 87, 70, -9, -80 },
			{ 75, -18, -89, -50, 50, 89, 18, -75, -75, 18, 89, 50, -50, -89, -18, 75 },
			{ 70, -43, -87, 9, 90, 25, -80, -57, 57, 80, -25, -90, -9, 87, 43, -70 },
			{ 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64 },
			{ 57, -80, -25, 90, -9, -87, 43, 70, -70, -43, 87, 9, -90, 25, 80, -57 },
			{ 50, -89, 18, 75, -75, -18, 89, -50, -50, 89, -18, -75, 75, 18, -89, 50 },
			{ 43, -90, 57, 25, -87, 70, 9, -80, 80, -9, -70, 87, -25, -57, 90, -43 },
			{ 36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36 },
			{ 25, -70, 90, -80, 43, 9, -57, 87, -87, 57, -9, -43, 80, -90, 70, -25 },
			{ 18, -50, 75, -89, 89, -75, 50, -18, -18, 50, -75, 89, -89, 75, -50, 18 },
			{ 9, -25, 43, -57, 70, -80, 87, -90, 90, -87, 80, -70, 57, -43, 25, -9 }
		};

		int O[8];
		for (int k = 0; k<8; ++k)
		{
			O[k] = lookup[1][k] * src[srcStride] + lookup[3][k] * src[3 * srcStride] + lookup[5][k] * src[5 * srcStride] + lookup[7][k] * src[7 * srcStride] +
				lookup[9][k] * src[9 * srcStride] + lookup[11][k] * src[11 * srcStride] + lookup[13][k] * src[13 * srcStride] + lookup[15][k] * src[15 * srcStride];
		}

		int EO[4];
		for (int k = 0; k<4; ++k)
		{
			EO[k] = lookup[2][k] * src[2 * srcStride] + lookup[6][k] * src[6 * srcStride] + lookup[10][k] * src[10 * srcStride] + lookup[14][k] * src[14 * srcStride];
		}

		int EEE[2], EEO[2];
		EEO[0] = lookup[4][0] * src[4 * srcStride] + lookup[12][0] * src[12 * srcStride];
		EEE[0] = lookup[0][0] * src[0 * srcStride] + lookup[8][0] * src[8 * srcStride];
		EEO[1] = lookup[4][1] * src[4 * srcStride] + lookup[12][1] * src[12 * srcStride];
		EEE[1] = lookup[0][1] * src[0 * srcStride] + lookup[8][1] * src[8 * srcStride];

		int EE[4];
		for (int k = 0; k<2; ++k)
		{
			EE[k] = EEE[k] + EEO[k];
			EE[k + 2] = EEE[1 - k] - EEO[1 - k];
		}

		int E[8];
		for (int k = 0; k<4; ++k)
		{
			E[k] = EE[k] + EO[k];
			E[k + 4] = EE[3 - k] - EO[3 - k];
		}

		for (int k = 0; k<8; ++k)
		{
			dst[k] = Clip3(-32768, 32767, (E[k] + O[k] + add) >> shift);
			dst[k + 8] = Clip3(-32768, 32767, (E[7 - k] - O[7 - k] + add) >> shift);
		}
		src++;
		dst += dstStride;
	}
}


uint8_t hevcasm_clip(int16_t x, int bit_depth)
{
	const uint8_t max = (int)(1 << bit_depth) - 1;
	if (x > max) return max;
	if (x < 0) return 0;
	return (uint8_t)x;
}


void hevcasm_add_residual_8x8(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *pred, ptrdiff_t stride_pred, int16_t residual[8 * 8])
{
	for (int y = 0; y < 8; ++y)
	{
		for (int x = 0; x < 8; ++x)
		{
			dst[x + y * stride_dst] = hevcasm_clip(pred[x + y * stride_pred] + residual[x + y * 8], 8);
		}
	}
}


void hevcasm_add_residual_16x16(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *pred, ptrdiff_t stride_pred, int16_t residual[8 * 8])
{
	for (int y = 0; y < 16; ++y)
	{
		for (int x = 0; x < 16; ++x)
		{
			dst[x + y * stride_dst] = hevcasm_clip(pred[x + y * stride_pred] + residual[x + y * 16], 16);
		}
	}
}


void hevcasm_idct_8x8_c_opt(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *pred, ptrdiff_t stride_pred, const int16_t coeffs[8 * 8])
{
	int16_t temp[2][8 * 8];
	hevcasm_inverse_partial_butterfly_8x8_c_opt(temp[0], coeffs, 7);
	hevcasm_inverse_partial_butterfly_8x8_c_opt(temp[1], temp[0], 12);
	hevcasm_add_residual_8x8(dst, stride_dst, pred, stride_pred, temp[1]);
}


void hevcasm_idct_16x16_c_opt(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *pred, ptrdiff_t stride_pred, const int16_t coeffs[8 * 8])
{
	int16_t temp[2][16 * 16];
	hevcasm_inverse_partial_butterfly_16x16_c_opt(temp[0], coeffs, 7);
	hevcasm_inverse_partial_butterfly_16x16_c_opt(temp[1], temp[0], 12);
	hevcasm_add_residual_16x16(dst, stride_dst, pred, stride_pred, temp[1]);
}


#ifdef HEVCASM_X64
void hevcasm_idct_8x8_ssse3(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *pred, ptrdiff_t stride_pred, const int16_t coeffs[8 * 8])
{
	HEVCASM_ALIGN(32, int16_t, temp[2][8 * 8]);
	hevcasm_partial_butterfly_inverse_8v_ssse3(temp[0], coeffs, 7);
	hevcasm_partial_butterfly_inverse_8h_ssse3(temp[1], temp[0], 12);
	hevcasm_add_residual_8x8(dst, stride_dst, pred, stride_pred, temp[1]);
}
#else
/* hevcasm_idct_8x8_ssse3 uses too many xmm registers for a 32-bit build */
hevcasm_inverse_transform_add *hevcasm_idct_8x8_ssse3 = 0;
#endif


void hevcasm_idct_16x16_ssse3(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *pred, ptrdiff_t stride_pred, const int16_t coeffs[8 * 8])
{
	HEVCASM_ALIGN(32, int16_t, temp[2][16 * 16]);
	hevcasm_partial_butterfly_inverse_16v_ssse3(temp[0], coeffs, 7);
	hevcasm_partial_butterfly_inverse_16h_ssse3(temp[1], temp[0], 12);
	hevcasm_add_residual_16x16(dst, stride_dst, pred, stride_pred, temp[1]);
}


hevcasm_inverse_transform_add* HEVCASM_API hevcasm_get_inverse_transform_add(int log2TrafoSize, int trType, hevcasm_instruction_set mask)
{
	const int nCbS = 1 << log2TrafoSize;

	hevcasm_inverse_transform_add *f = 0;

	if (mask & (HEVCASM_C_REF | HEVCASM_C_OPT))
	{
		if (nCbS == 8) f = hevcasm_idct_8x8_c_opt;
		if (nCbS == 16) f = hevcasm_idct_16x16_c_opt;
	}

	if (mask & HEVCASM_SSSE3)
	{
		if (nCbS == 8) f = hevcasm_idct_8x8_ssse3;
		if (nCbS == 16) f = hevcasm_idct_16x16_ssse3;
	}

	return f;
}


typedef struct
{
	const int16_t *coefficients;
	const uint8_t *predicted;
	hevcasm_inverse_transform_add *f;
	int log2TrafoSize;
	int trType;
	uint8_t dst[32 * 32];
} 
bind_inverse_transform_add;


int get_inverse_transform_add(void *p, hevcasm_instruction_set mask)
{
	bind_inverse_transform_add *s = p;

	s->f = hevcasm_get_inverse_transform_add(s->log2TrafoSize, s->trType, mask);

	if (s->f && mask == HEVCASM_C_REF)
	{
		const int nCbS = 1 << s->log2TrafoSize;
		printf("\t%s %dx%d : ", s->trType ? "sine" : "cosine", nCbS, nCbS);
	}

	return !!s->f;
}


void invoke_inverse_transform_add(void *p, int n)
{
	bind_inverse_transform_add *s = p;

	while (n--)
	{
		s->f(s->dst, (ptrdiff_t)1 << s->log2TrafoSize, s->predicted, (ptrdiff_t)1 << s->log2TrafoSize, s->coefficients);
	}
}


int mismatch_transform_add(void *boundRef, void *boundTest)
{
	bind_inverse_transform_add *ref = boundRef;
	bind_inverse_transform_add *test = boundTest;

	const int nCbS = 1 << ref->log2TrafoSize;

	return memcmp(ref->dst, test->dst, nCbS * nCbS * sizeof(int16_t));
}


void hevcasm_test_inverse_transform_add(int *error_count, hevcasm_instruction_set mask)
{
	printf("\ninverse_transform_add - Inverse Transform, then add to predicted\n");

	HEVCASM_ALIGN(32, int16_t, coefficients[32 * 32]);
	HEVCASM_ALIGN(32, uint8_t, predicted[32 * 32]);

	for (int x = 0; x < 32 * 32; x++) coefficients[x] = (rand() & 0x1ff) - 0x100;
	for (int x = 0; x < 32 * 32; x++) predicted[x] = rand() & 0xff;

	bind_inverse_transform_add b[2];
	b[0].coefficients = coefficients;
	b[0].predicted = predicted;

	for (int j = 1; j < 6; ++j)
	{
		b[0].trType = (j == 1) ? 1 : 0;
		b[0].log2TrafoSize = (j == 1) ? 2 : j;
		b[1] = b[0];

		*error_count += hevcasm_test(&b[0], &b[1], get_inverse_transform_add, invoke_inverse_transform_add, mismatch_transform_add, mask, 100000);
	}
}


void hevcasm_partial_butterfly_16x16_c_opt(int16_t *dst, const int16_t *src, ptrdiff_t srcStride, int shift)
{
	const ptrdiff_t dstStride = 16;

	const int add = 1 << (shift - 1);

	for (int j = 0; j<16; ++j)
	{
		int E[8], O[8];
		for (int k = 0; k<8; ++k)
		{
			E[k] = src[k] + src[15 - k];
			O[k] = src[k] - src[15 - k];
		}

		int EE[4], EO[4];
		for (int k = 0; k<4; ++k)
		{
			EE[k] = E[k] + E[7 - k];
			EO[k] = E[k] - E[7 - k];
		}

		int EEE[2], EEO[2];
		EEE[0] = EE[0] + EE[3];
		EEO[0] = EE[0] - EE[3];
		EEE[1] = EE[1] + EE[2];
		EEO[1] = EE[1] - EE[2];

		static const int16_t lookup[16][16] =
		{
			{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
			{ 90, 87, 80, 70, 57, 43, 25, 9, -9, -25, -43, -57, -70, -80, -87, -90 },
			{ 89, 75, 50, 18, -18, -50, -75, -89, -89, -75, -50, -18, 18, 50, 75, 89 },
			{ 87, 57, 9, -43, -80, -90, -70, -25, 25, 70, 90, 80, 43, -9, -57, -87 },
			{ 83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83 },
			{ 80, 9, -70, -87, -25, 57, 90, 43, -43, -90, -57, 25, 87, 70, -9, -80 },
			{ 75, -18, -89, -50, 50, 89, 18, -75, -75, 18, 89, 50, -50, -89, -18, 75 },
			{ 70, -43, -87, 9, 90, 25, -80, -57, 57, 80, -25, -90, -9, 87, 43, -70 },
			{ 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64 },
			{ 57, -80, -25, 90, -9, -87, 43, 70, -70, -43, 87, 9, -90, 25, 80, -57 },
			{ 50, -89, 18, 75, -75, -18, 89, -50, -50, 89, -18, -75, 75, 18, -89, 50 },
			{ 43, -90, 57, 25, -87, 70, 9, -80, 80, -9, -70, 87, -25, -57, 90, -43 },
			{ 36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36 },
			{ 25, -70, 90, -80, 43, 9, -57, 87, -87, 57, -9, -43, 80, -90, 70, -25 },
			{ 18, -50, 75, -89, 89, -75, 50, -18, -18, 50, -75, 89, -89, 75, -50, 18 },
			{ 9, -25, 43, -57, 70, -80, 87, -90, 90, -87, 80, -70, 57, -43, 25, -9 }
		};

		dst[0 * dstStride] = (lookup[0][0] * EEE[0] + lookup[0][1] * EEE[1] + add) >> shift;
		dst[8 * dstStride] = (lookup[8][0] * EEE[0] + lookup[8][1] * EEE[1] + add) >> shift;
		dst[4 * dstStride] = (lookup[4][0] * EEO[0] + lookup[4][1] * EEO[1] + add) >> shift;
		dst[12 * dstStride] = (lookup[12][0] * EEO[0] + lookup[12][1] * EEO[1] + add) >> shift;

		for (int k = 2; k<16; k += 4)
		{
			dst[k*dstStride] = (lookup[k][0] * EO[0] + lookup[k][1] * EO[1] + lookup[k][2] * EO[2] + lookup[k][3] * EO[3] + add) >> shift;
		}

		for (int k = 1; k<16; k += 2)
		{
			dst[k*dstStride] = (lookup[k][0] * O[0] + lookup[k][1] * O[1] + lookup[k][2] * O[2] + lookup[k][3] * O[3] +
				lookup[k][4] * O[4] + lookup[k][5] * O[5] + lookup[k][6] * O[6] + lookup[k][7] * O[7] + add) >> shift;
		}

		src += srcStride;
		++dst;
	}
}


void hevcasm_dct_16x16_c_opt(int16_t *coeffs, const int16_t *src, ptrdiff_t src_stride)
{
	int16_t temp[16 * 16];
	hevcasm_partial_butterfly_16x16_c_opt(temp, src, src_stride, 3);
	hevcasm_partial_butterfly_16x16_c_opt(coeffs, temp, 16, 10);
}


void hevcasm_dct_16x16_ssse3(int16_t *coeffs, const int16_t *src, ptrdiff_t src_stride)
{
	HEVCASM_ALIGN(32, int16_t, temp[16 * 16]);
	hevcasm_partial_butterfly_16h_ssse3(temp, src, src_stride, 3);
	hevcasm_partial_butterfly_16v_ssse3(coeffs, temp, 10);
}


hevcasm_transform* HEVCASM_API hevcasm_get_transform(int log2TrafoSize, int trType, hevcasm_instruction_set mask)
{
	const int nCbS = 1 << log2TrafoSize;

	hevcasm_transform *f = 0;

	if (mask & (HEVCASM_C_REF | HEVCASM_C_OPT))
	{
		if (nCbS == 16) f = hevcasm_dct_16x16_c_opt;
	}

	if (mask & HEVCASM_SSSE3)
	{
		if (nCbS == 16) f = hevcasm_dct_16x16_ssse3;
	}

	return f;
}


typedef struct
{
	hevcasm_transform *f;
	HEVCASM_ALIGN(32, int16_t, dst[32 * 32]);
	int16_t *src;
	ptrdiff_t src_stride;
	int trType;
	int log2TrafoSize;
}
bound_transform;


int get_transform(void *p, hevcasm_instruction_set mask)
{
	bound_transform *s = p;

	s->f = hevcasm_get_transform(s->log2TrafoSize, s->trType, mask);

	if (s->f && mask == HEVCASM_C_REF)
	{
		const int nCbS = 1 << s->log2TrafoSize;
		printf("\t%s %dx%d : ", s->trType ? "sine" : "cosine", nCbS, nCbS);
	}

	return !!s->f;
}


void invoke_transform(void *p, int n)
{
	bound_transform *s = p;

	while (n--)
	{
		s->f(s->dst, s->src, s->src_stride);
	}
}


int mismatch_transform(void *boundRef, void *boundTest)
{
	bound_transform *ref = boundRef;
	bound_transform *test = boundTest;
	
	const int nCbS = 1 << ref->log2TrafoSize;

	return memcmp(ref->dst, test->dst, nCbS * nCbS * sizeof(int16_t));
}


void hevcasm_test_transform(int *error_count, hevcasm_instruction_set mask)
{
	printf("\nhevcasm_transform - Forward Transform\n");

	HEVCASM_ALIGN(32, uint16_t, src[32 * 32]);
	for (int x = 0; x < 32 * 32; x++) src[x] = (rand() & 0x1ff) - 0x100;

	bound_transform b[2];
	b[0].src = src;
	b[0].src_stride = 32;

	for (int j = 1; j < 6; ++j)
	{
		b[0].trType = (j == 1) ? 1 : 0;
		b[0].log2TrafoSize = (j == 1) ? 2 : j;
		
		b[1] = b[0];

		*error_count += hevcasm_test(&b[0], &b[1], get_transform, invoke_transform, mismatch_transform, mask, 100000);
	}
}
