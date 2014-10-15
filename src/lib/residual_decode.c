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


// Declaration for f265 functions
void __fastcall f265_lbd_idct_16_avx2(uint8_t *dst, int dst_stride, const uint8_t *pred, int pred_stride, const int16_t coeffs[16*16], uint8_t *spill);


static int Clip3(int min, int max, int x)
{
	if (x > max) return max;
	if (x < min) return min;
	return x;
}


static void hevcasm_inverse_partial_butterfly_4x4_dst_c_opt(int16_t dst[4 * 4], const int16_t src[4 * 4], int shift)
{
	const int add = 1 << (shift - 1);
	const int src_stride = 4;
	const int dst_stride = 4;

	for (int i = 0; i < 4; i++)
	{
		int c[4];
		c[0] = src[i] + src[2 * src_stride + i];
		c[1] = src[2 * src_stride + i] + src[3 * src_stride + i];
		c[2] = src[i] - src[3 * src_stride + i];
		c[3] = 74 * src[1 * src_stride + i];

		dst[dst_stride*i + 0] = Clip3(-32768, 32767, (29 * c[0] + 55 * c[1] + c[3] + add) >> shift);
		dst[dst_stride*i + 1] = Clip3(-32768, 32767, (55 * c[2] - 29 * c[1] + c[3] + add) >> shift);
		dst[dst_stride*i + 2] = Clip3(-32768, 32767, (74 * (src[i] - src[2 * src_stride + i] + src[3 * src_stride + i]) + add) >> shift);
		dst[dst_stride*i + 3] = Clip3(-32768, 32767, (55 * c[0] + 29 * c[2] - c[3] + add) >> shift);
	}
}


static void hevcasm_inverse_partial_butterfly_4x4_c_opt(int16_t dst[4 * 4], const int16_t src[4 * 4], int shift)
{
	const int add = 1 << (shift - 1);
	const int src_stride = 4;
	const int dst_stride = 4;

	for (int j = 0; j<4; ++j)
	{
		static const int16_t table[4][4] =
		{
			{ 64, 64, 64, 64 },
			{ 83, 36, -36, -83 },
			{ 64, -64, -64, 64 },
			{ 36, -83, 83, -36 }
		};

		int E[2], O[2];
		O[0] = table[1][0] * src[1 * src_stride] + table[3][0] * src[3 * src_stride];
		O[1] = table[1][1] * src[1 * src_stride] + table[3][1] * src[3 * src_stride];
		E[0] = table[0][0] * src[0 * src_stride] + table[2][0] * src[2 * src_stride];
		E[1] = table[0][1] * src[0 * src_stride] + table[2][1] * src[2 * src_stride];

		dst[0] = Clip3(-32768, 32767, (E[0] + O[0] + add) >> shift);
		dst[1] = Clip3(-32768, 32767, (E[1] + O[1] + add) >> shift);
		dst[2] = Clip3(-32768, 32767, (E[1] - O[1] + add) >> shift);
		dst[3] = Clip3(-32768, 32767, (E[0] - O[0] + add) >> shift);

		++src;
		dst += dst_stride;
	}
}


static void hevcasm_inverse_partial_butterfly_8x8_c_opt(int16_t dst[8 * 8], const int16_t src[8 * 8], int shift)
{
	const int add = 1 << (shift - 1);
	const int src_stride = 8;
	const int dst_stride = 8;

	for (int j = 0; j<8; ++j)
	{
		static const int16_t table[8][8] =
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
			O[k] = table[1][k] * src[1 * src_stride] + table[3][k] * src[3 * src_stride] + table[5][k] * src[5 * src_stride] + table[7][k] * src[7 * src_stride];
		}

		int EE[2], EO[2];
		EO[0] = table[2][0] * src[2 * src_stride] + table[6][0] * src[6 * src_stride];
		EO[1] = table[2][1] * src[2 * src_stride] + table[6][1] * src[6 * src_stride];
		EE[0] = table[0][0] * src[0 * src_stride] + table[4][0] * src[4 * src_stride];
		EE[1] = table[0][1] * src[0 * src_stride] + table[4][1] * src[4 * src_stride];

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
		dst += dst_stride;
	}
}


static void hevcasm_inverse_partial_butterfly_16x16_c_opt(int16_t dst[16 * 16], const int16_t src[16 * 16], int shift)
{
	const int add = 1 << (shift - 1);
	const int src_stride = 16;
	const int dst_stride = 16;

	for (int j = 0; j<16; ++j)
	{
		static const int16_t table[16][16] =
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
			O[k] = table[1][k] * src[src_stride] + table[3][k] * src[3 * src_stride] + table[5][k] * src[5 * src_stride] + table[7][k] * src[7 * src_stride] +
				table[9][k] * src[9 * src_stride] + table[11][k] * src[11 * src_stride] + table[13][k] * src[13 * src_stride] + table[15][k] * src[15 * src_stride];
		}

		int EO[4];
		for (int k = 0; k<4; ++k)
		{
			EO[k] = table[2][k] * src[2 * src_stride] + table[6][k] * src[6 * src_stride] + table[10][k] * src[10 * src_stride] + table[14][k] * src[14 * src_stride];
		}

		int EEE[2], EEO[2];
		EEO[0] = table[4][0] * src[4 * src_stride] + table[12][0] * src[12 * src_stride];
		EEE[0] = table[0][0] * src[0 * src_stride] + table[8][0] * src[8 * src_stride];
		EEO[1] = table[4][1] * src[4 * src_stride] + table[12][1] * src[12 * src_stride];
		EEE[1] = table[0][1] * src[0 * src_stride] + table[8][1] * src[8 * src_stride];

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
		dst += dst_stride;
	}
}


static void hevcasm_inverse_partial_butterfly_32x32_c_opt(int16_t dst[32 * 32], const int16_t src[32 * 32], int shift)
{
	const int add = 1 << (shift - 1);
	const int src_stride = 32;
	const int dst_stride = 32;

	for (int j = 0; j<32; j++)
	{
		static const int16_t table[32][32] =
		{
			{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
			{ 90, 90, 88, 85, 82, 78, 73, 67, 61, 54, 46, 38, 31, 22, 13, 4, -4, -13, -22, -31, -38, -46, -54, -61, -67, -73, -78, -82, -85, -88, -90, -90 },
			{ 90, 87, 80, 70, 57, 43, 25, 9, -9, -25, -43, -57, -70, -80, -87, -90, -90, -87, -80, -70, -57, -43, -25, -9, 9, 25, 43, 57, 70, 80, 87, 90 },
			{ 90, 82, 67, 46, 22, -4, -31, -54, -73, -85, -90, -88, -78, -61, -38, -13, 13, 38, 61, 78, 88, 90, 85, 73, 54, 31, 4, -22, -46, -67, -82, -90 },
			{ 89, 75, 50, 18, -18, -50, -75, -89, -89, -75, -50, -18, 18, 50, 75, 89, 89, 75, 50, 18, -18, -50, -75, -89, -89, -75, -50, -18, 18, 50, 75, 89 },
			{ 88, 67, 31, -13, -54, -82, -90, -78, -46, -4, 38, 73, 90, 85, 61, 22, -22, -61, -85, -90, -73, -38, 4, 46, 78, 90, 82, 54, 13, -31, -67, -88 },
			{ 87, 57, 9, -43, -80, -90, -70, -25, 25, 70, 90, 80, 43, -9, -57, -87, -87, -57, -9, 43, 80, 90, 70, 25, -25, -70, -90, -80, -43, 9, 57, 87 },
			{ 85, 46, -13, -67, -90, -73, -22, 38, 82, 88, 54, -4, -61, -90, -78, -31, 31, 78, 90, 61, 4, -54, -88, -82, -38, 22, 73, 90, 67, 13, -46, -85 },
			{ 83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83 },
			{ 82, 22, -54, -90, -61, 13, 78, 85, 31, -46, -90, -67, 4, 73, 88, 38, -38, -88, -73, -4, 67, 90, 46, -31, -85, -78, -13, 61, 90, 54, -22, -82 },
			{ 80, 9, -70, -87, -25, 57, 90, 43, -43, -90, -57, 25, 87, 70, -9, -80, -80, -9, 70, 87, 25, -57, -90, -43, 43, 90, 57, -25, -87, -70, 9, 80 },
			{ 78, -4, -82, -73, 13, 85, 67, -22, -88, -61, 31, 90, 54, -38, -90, -46, 46, 90, 38, -54, -90, -31, 61, 88, 22, -67, -85, -13, 73, 82, 4, -78 },
			{ 75, -18, -89, -50, 50, 89, 18, -75, -75, 18, 89, 50, -50, -89, -18, 75, 75, -18, -89, -50, 50, 89, 18, -75, -75, 18, 89, 50, -50, -89, -18, 75 },
			{ 73, -31, -90, -22, 78, 67, -38, -90, -13, 82, 61, -46, -88, -4, 85, 54, -54, -85, 4, 88, 46, -61, -82, 13, 90, 38, -67, -78, 22, 90, 31, -73 },
			{ 70, -43, -87, 9, 90, 25, -80, -57, 57, 80, -25, -90, -9, 87, 43, -70, -70, 43, 87, -9, -90, -25, 80, 57, -57, -80, 25, 90, 9, -87, -43, 70 },
			{ 67, -54, -78, 38, 85, -22, -90, 4, 90, 13, -88, -31, 82, 46, -73, -61, 61, 73, -46, -82, 31, 88, -13, -90, -4, 90, 22, -85, -38, 78, 54, -67 },
			{ 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64 },
			{ 61, -73, -46, 82, 31, -88, -13, 90, -4, -90, 22, 85, -38, -78, 54, 67, -67, -54, 78, 38, -85, -22, 90, 4, -90, 13, 88, -31, -82, 46, 73, -61 },
			{ 57, -80, -25, 90, -9, -87, 43, 70, -70, -43, 87, 9, -90, 25, 80, -57, -57, 80, 25, -90, 9, 87, -43, -70, 70, 43, -87, -9, 90, -25, -80, 57 },
			{ 54, -85, -4, 88, -46, -61, 82, 13, -90, 38, 67, -78, -22, 90, -31, -73, 73, 31, -90, 22, 78, -67, -38, 90, -13, -82, 61, 46, -88, 4, 85, -54 },
			{ 50, -89, 18, 75, -75, -18, 89, -50, -50, 89, -18, -75, 75, 18, -89, 50, 50, -89, 18, 75, -75, -18, 89, -50, -50, 89, -18, -75, 75, 18, -89, 50 },
			{ 46, -90, 38, 54, -90, 31, 61, -88, 22, 67, -85, 13, 73, -82, 4, 78, -78, -4, 82, -73, -13, 85, -67, -22, 88, -61, -31, 90, -54, -38, 90, -46 },
			{ 43, -90, 57, 25, -87, 70, 9, -80, 80, -9, -70, 87, -25, -57, 90, -43, -43, 90, -57, -25, 87, -70, -9, 80, -80, 9, 70, -87, 25, 57, -90, 43 },
			{ 38, -88, 73, -4, -67, 90, -46, -31, 85, -78, 13, 61, -90, 54, 22, -82, 82, -22, -54, 90, -61, -13, 78, -85, 31, 46, -90, 67, 4, -73, 88, -38 },
			{ 36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36 },
			{ 31, -78, 90, -61, 4, 54, -88, 82, -38, -22, 73, -90, 67, -13, -46, 85, -85, 46, 13, -67, 90, -73, 22, 38, -82, 88, -54, -4, 61, -90, 78, -31 },
			{ 25, -70, 90, -80, 43, 9, -57, 87, -87, 57, -9, -43, 80, -90, 70, -25, -25, 70, -90, 80, -43, -9, 57, -87, 87, -57, 9, 43, -80, 90, -70, 25 },
			{ 22, -61, 85, -90, 73, -38, -4, 46, -78, 90, -82, 54, -13, -31, 67, -88, 88, -67, 31, 13, -54, 82, -90, 78, -46, 4, 38, -73, 90, -85, 61, -22 },
			{ 18, -50, 75, -89, 89, -75, 50, -18, -18, 50, -75, 89, -89, 75, -50, 18, 18, -50, 75, -89, 89, -75, 50, -18, -18, 50, -75, 89, -89, 75, -50, 18 },
			{ 13, -38, 61, -78, 88, -90, 85, -73, 54, -31, 4, 22, -46, 67, -82, 90, -90, 82, -67, 46, -22, -4, 31, -54, 73, -85, 90, -88, 78, -61, 38, -13 },
			{ 9, -25, 43, -57, 70, -80, 87, -90, 90, -87, 80, -70, 57, -43, 25, -9, -9, 25, -43, 57, -70, 80, -87, 90, -90, 87, -80, 70, -57, 43, -25, 9 },
			{ 4, -13, 22, -31, 38, -46, 54, -61, 67, -73, 78, -82, 85, -88, 90, -90, 90, -90, 88, -85, 82, -78, 73, -67, 61, -54, 46, -38, 31, -22, 13, -4 }
		};

		int O[16];
		for (int k = 0; k<16; k++)
		{
			O[k] = table[1][k] * src[src_stride] + table[3][k] * src[3 * src_stride] + table[5][k] * src[5 * src_stride] + table[7][k] * src[7 * src_stride] +
				table[9][k] * src[9 * src_stride] + table[11][k] * src[11 * src_stride] + table[13][k] * src[13 * src_stride] + table[15][k] * src[15 * src_stride] +
				table[17][k] * src[17 * src_stride] + table[19][k] * src[19 * src_stride] + table[21][k] * src[21 * src_stride] + table[23][k] * src[23 * src_stride] +
				table[25][k] * src[25 * src_stride] + table[27][k] * src[27 * src_stride] + table[29][k] * src[29 * src_stride] + table[31][k] * src[31 * src_stride];
		}

		int EO[8];
		for (int k = 0; k<8; k++)
		{
			EO[k] = table[2][k] * src[2 * src_stride] + table[6][k] * src[6 * src_stride] + table[10][k] * src[10 * src_stride] + table[14][k] * src[14 * src_stride] +
				table[18][k] * src[18 * src_stride] + table[22][k] * src[22 * src_stride] + table[26][k] * src[26 * src_stride] + table[30][k] * src[30 * src_stride];
		}

		int EEO[4];
		for (int k = 0; k<4; k++)
		{
			EEO[k] = table[4][k] * src[4 * src_stride] + table[12][k] * src[12 * src_stride] + table[20][k] * src[20 * src_stride] + table[28][k] * src[28 * src_stride];
		}

		int EEEO[2];
		EEEO[0] = table[8][0] * src[8 * src_stride] + table[24][0] * src[24 * src_stride];
		EEEO[1] = table[8][1] * src[8 * src_stride] + table[24][1] * src[24 * src_stride];

		int EEEE[2];
		EEEE[0] = table[0][0] * src[0] + table[16][0] * src[16 * src_stride];
		EEEE[1] = table[0][1] * src[0] + table[16][1] * src[16 * src_stride];

		int EEE[4];
		EEE[0] = EEEE[0] + EEEO[0];
		EEE[3] = EEEE[0] - EEEO[0];
		EEE[1] = EEEE[1] + EEEO[1];
		EEE[2] = EEEE[1] - EEEO[1];

		int EE[8];
		for (int k = 0; k<4; k++)
		{
			EE[k] = EEE[k] + EEO[k];
			EE[k + 4] = EEE[3 - k] - EEO[3 - k];
		}

		int E[16];
		for (int k = 0; k<8; k++)
		{
			E[k] = EE[k] + EO[k];
			E[k + 8] = EE[7 - k] - EO[7 - k];
		}
		for (int k = 0; k<16; k++)
		{
			dst[k] = Clip3(-32768, 32767, (E[k] + O[k] + add) >> shift);
			dst[k + 16] = Clip3(-32768, 32767, (E[15 - k] - O[15 - k] + add) >> shift);
		}
		src++;
		dst += dst_stride;
	}
}


uint8_t hevcasm_clip(int16_t x, int bit_depth)
{
	const uint8_t max = (int)(1 << bit_depth) - 1;
	if (x > max) return max;
	if (x < 0) return 0;
	return (uint8_t)x;
}


void hevcasm_add_residual(int n, uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *pred, ptrdiff_t stride_pred, int16_t *residual)
{
	for (int y = 0; y < n; ++y)
	{
		for (int x = 0; x < n; ++x)
		{
			dst[x + y * stride_dst] = hevcasm_clip(pred[x + y * stride_pred] + residual[x + y * n], 8);
		}
	}
}


void hevcasm_idct_4x4_c_opt(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *pred, ptrdiff_t stride_pred, const int16_t coeffs[4 * 4])
{
	int16_t temp[2][4 * 4];
	hevcasm_inverse_partial_butterfly_4x4_c_opt(temp[0], coeffs, 7);
	hevcasm_inverse_partial_butterfly_4x4_c_opt(temp[1], temp[0], 12);
	hevcasm_add_residual(4, dst, stride_dst, pred, stride_pred, temp[1]);
}


void hevcasm_idst_4x4_c_opt(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *pred, ptrdiff_t stride_pred, const int16_t coeffs[4 * 4])
{
	int16_t temp[2][4 * 4];
	hevcasm_inverse_partial_butterfly_4x4_dst_c_opt(temp[0], coeffs, 7);
	hevcasm_inverse_partial_butterfly_4x4_dst_c_opt(temp[1], temp[0], 12);
	hevcasm_add_residual(4, dst, stride_dst, pred, stride_pred, temp[1]);
}


void hevcasm_idct_8x8_c_opt(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *pred, ptrdiff_t stride_pred, const int16_t coeffs[8 * 8])
{
	int16_t temp[2][8 * 8];
	hevcasm_inverse_partial_butterfly_8x8_c_opt(temp[0], coeffs, 7);
	hevcasm_inverse_partial_butterfly_8x8_c_opt(temp[1], temp[0], 12);
	hevcasm_add_residual(8, dst, stride_dst, pred, stride_pred, temp[1]);
}


void hevcasm_idct_16x16_c_opt(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *pred, ptrdiff_t stride_pred, const int16_t coeffs[8 * 8])
{
	int16_t temp[2][16 * 16];
	hevcasm_inverse_partial_butterfly_16x16_c_opt(temp[0], coeffs, 7);
	hevcasm_inverse_partial_butterfly_16x16_c_opt(temp[1], temp[0], 12);
	hevcasm_add_residual(16, dst, stride_dst, pred, stride_pred, temp[1]);
}


void hevcasm_idct_32x32_c_opt(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *pred, ptrdiff_t stride_pred, const int16_t coeffs[32 * 32])
{
	int16_t temp[2][32 * 32];
	hevcasm_inverse_partial_butterfly_32x32_c_opt(temp[0], coeffs, 7);
	hevcasm_inverse_partial_butterfly_32x32_c_opt(temp[1], temp[0], 12);
	hevcasm_add_residual(32, dst, stride_dst, pred, stride_pred, temp[1]);
}


#ifdef HEVCASM_X64
void hevcasm_idct_8x8_ssse3(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *pred, ptrdiff_t stride_pred, const int16_t coeffs[8 * 8])
{
	HEVCASM_ALIGN(32, int16_t, temp[2][8 * 8]);
	hevcasm_partial_butterfly_inverse_8v_ssse3(temp[0], coeffs, 7);
	hevcasm_partial_butterfly_inverse_8h_ssse3(temp[1], temp[0], 12);
	hevcasm_add_residual(8, dst, stride_dst, pred, stride_pred, temp[1]);
}
#else
/* hevcasm_idct_8x8_ssse3 uses too many xmm registers for a 32-bit build */
hevcasm_inverse_transform_add *hevcasm_idct_8x8_ssse3 = 0;
#endif


#ifdef HEVCASM_X64
void hevcasm_idct_16x16_ssse3(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *pred, ptrdiff_t stride_pred, const int16_t coeffs[16 * 16])
{
	HEVCASM_ALIGN(32, int16_t, temp[2][16 * 16]);
	hevcasm_partial_butterfly_inverse_16v_ssse3(temp[0], coeffs, 7);
	hevcasm_partial_butterfly_inverse_16h_ssse3(temp[1], temp[0], 12);
	hevcasm_add_residual(16, dst, stride_dst, pred, stride_pred, temp[1]);
}
#else
/* hevcasm_idct_16x16_ssse3 uses too many xmm registers for a 32-bit build */
hevcasm_inverse_transform_add *hevcasm_idct_16x16_ssse3 = 0;
#endif


#ifdef HEVCASM_X64
void hevcasm_idct_16x16_avx2(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *pred, ptrdiff_t stride_pred, const int16_t coeffs[16 * 16])
{
	HEVCASM_ALIGN(32, uint8_t, spill[4096]);

	f265_lbd_idct_16_avx2(dst, (int)stride_dst, pred, (int)stride_pred, coeffs, &spill[2048]);
}
#else
/* f265_lbd_idct_16_avx2 only builds on 64-bit */
hevcasm_inverse_transform_add *hevcasm_idct_16x16_avx2 = 0;
#endif


static hevcasm_inverse_transform_add* get_inverse_transform_add(int trType, int log2TrafoSize, hevcasm_instruction_set mask)
{
	const int nCbS = 1 << log2TrafoSize;

	hevcasm_inverse_transform_add *f = 0;

	if (mask & (HEVCASM_C_REF | HEVCASM_C_OPT))
	{
		if (nCbS == 4) f = trType ? hevcasm_idst_4x4_c_opt : hevcasm_idct_4x4_c_opt;
		if (nCbS == 8) f = hevcasm_idct_8x8_c_opt;
		if (nCbS == 16) f = hevcasm_idct_16x16_c_opt;
		if (nCbS == 32) f = hevcasm_idct_32x32_c_opt;
	}

	if (mask & HEVCASM_SSSE3)
	{
		if (nCbS == 8 && hevcasm_idct_8x8_ssse3) f = hevcasm_idct_8x8_ssse3;
		if (nCbS == 16 && hevcasm_idct_16x16_ssse3) f = hevcasm_idct_16x16_ssse3;
	}

	if (mask & HEVCASM_AVX2)
	{
		if (nCbS == 16 && hevcasm_idct_16x16_avx2) f = hevcasm_idct_16x16_avx2;
	}
	return f;
}


void HEVCASM_API hevcasm_populate_inverse_transform_add(hevcasm_table_inverse_transform_add *table, hevcasm_instruction_set mask)
{
	*hevcasm_get_inverse_transform_add(table, 1, 2) = get_inverse_transform_add(1, 2, mask);
	for (int log2TrafoSize = 2; log2TrafoSize <= 5; ++log2TrafoSize)
	{
		*hevcasm_get_inverse_transform_add(table, 0, log2TrafoSize) = get_inverse_transform_add(0, log2TrafoSize, mask);
	}
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


int init_inverse_transform_add(void *p, hevcasm_instruction_set mask)
{
	bind_inverse_transform_add *s = p;

	hevcasm_table_inverse_transform_add table;

	hevcasm_populate_inverse_transform_add(&table, mask);

	s->f = *hevcasm_get_inverse_transform_add(&table, s->trType, s->log2TrafoSize);

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

	return memcmp(ref->dst, test->dst, nCbS * nCbS * sizeof(int8_t));
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

		*error_count += hevcasm_test(&b[0], &b[1], init_inverse_transform_add, invoke_inverse_transform_add, mismatch_transform_add, mask, 100000);
	}
}


void hevcasm_partial_butterfly_4x4_dst_c_opt(int16_t *dst, const int16_t *src, ptrdiff_t src_stride, int shift)
{
	const int add = 1 << (shift - 1);
	const int dst_stride = 4;

	for (int i = 0; i<4; i++)
	{
		int c[4];
		c[0] = src[src_stride*i + 0] + src[src_stride*i + 3];
		c[1] = src[src_stride*i + 1] + src[src_stride*i + 3];
		c[2] = src[src_stride*i + 0] - src[src_stride*i + 1];
		c[3] = 74 * src[src_stride*i + 2];

		dst[0 * dst_stride + i] = (29 * c[0] + 55 * c[1] + c[3] + add) >> shift;
		dst[1 * dst_stride + i] = (74 * (src[src_stride*i + 0] + src[src_stride*i + 1] - src[src_stride*i + 3]) + add) >> shift;
		dst[2 * dst_stride + i] = (29 * c[2] + 55 * c[0] - c[3] + add) >> shift;
		dst[3 * dst_stride + i] = (55 * c[2] - 29 * c[1] + c[3] + add) >> shift;
	}
}


void hevcasm_partial_butterfly_4x4_c_opt(int16_t *dst, const int16_t *src, ptrdiff_t src_stride, int shift)
{
	const int add = 1 << (shift - 1);
	const int dst_stride = 4;

	for (int j = 0; j<4; j++)
	{
		int E[2] = { src[0] + src[3], src[1] + src[2] };
		int O[2] = { src[0] - src[3], src[1] - src[2] };

		static const int16_t table[4][4] =
		{
			{ 64, 64, 64, 64 },
			{ 83, 36, -36, -83 },
			{ 64, -64, -64, 64 },
			{ 36, -83, 83, -36 }
		};

		dst[0 * dst_stride] = (table[0][0] * E[0] + table[0][1] * E[1] + add) >> shift;
		dst[2 * dst_stride] = (table[2][0] * E[0] + table[2][1] * E[1] + add) >> shift;
		dst[1 * dst_stride] = (table[1][0] * O[0] + table[1][1] * O[1] + add) >> shift;
		dst[3 * dst_stride] = (table[3][0] * O[0] + table[3][1] * O[1] + add) >> shift;

		src += src_stride;
		dst++;
	}
}


void hevcasm_partial_butterfly_8x8_c_opt(int16_t *dst, const int16_t *src, ptrdiff_t src_stride, int shift)
{
	const int add = 1 << (shift - 1);
	const int dst_stride = 8;

	for (int j = 0; j<8; j++)
	{
		int E[4], O[4];
		for (int k = 0; k<4; k++)
		{
			E[k] = src[k] + src[7 - k];
			O[k] = src[k] - src[7 - k];
		}

		int EE[2], EO[2];
		EE[0] = E[0] + E[3];
		EO[0] = E[0] - E[3];
		EE[1] = E[1] + E[2];
		EO[1] = E[1] - E[2];

		static const int16_t table[8][8] =
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

		dst[0 * dst_stride] = (table[0][0] * EE[0] + table[0][1] * EE[1] + add) >> shift;
		dst[4 * dst_stride] = (table[4][0] * EE[0] + table[4][1] * EE[1] + add) >> shift;
		dst[2 * dst_stride] = (table[2][0] * EO[0] + table[2][1] * EO[1] + add) >> shift;
		dst[6 * dst_stride] = (table[6][0] * EO[0] + table[6][1] * EO[1] + add) >> shift;

		dst[1 * dst_stride] = (table[1][0] * O[0] + table[1][1] * O[1] + table[1][2] * O[2] + table[1][3] * O[3] + add) >> shift;
		dst[3 * dst_stride] = (table[3][0] * O[0] + table[3][1] * O[1] + table[3][2] * O[2] + table[3][3] * O[3] + add) >> shift;
		dst[5 * dst_stride] = (table[5][0] * O[0] + table[5][1] * O[1] + table[5][2] * O[2] + table[5][3] * O[3] + add) >> shift;
		dst[7 * dst_stride] = (table[7][0] * O[0] + table[7][1] * O[1] + table[7][2] * O[2] + table[7][3] * O[3] + add) >> shift;

		src += src_stride;
		dst++;
	}
}


void hevcasm_partial_butterfly_16x16_c_opt(int16_t *dst, const int16_t *src, ptrdiff_t src_stride, int shift)
{
	const int add = 1 << (shift - 1);
	const ptrdiff_t dst_stride = 16;

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

		static const int16_t table[16][16] =
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

		dst[0 * dst_stride] = (table[0][0] * EEE[0] + table[0][1] * EEE[1] + add) >> shift;
		dst[8 * dst_stride] = (table[8][0] * EEE[0] + table[8][1] * EEE[1] + add) >> shift;
		dst[4 * dst_stride] = (table[4][0] * EEO[0] + table[4][1] * EEO[1] + add) >> shift;
		dst[12 * dst_stride] = (table[12][0] * EEO[0] + table[12][1] * EEO[1] + add) >> shift;

		for (int k = 2; k<16; k += 4)
		{
			dst[k*dst_stride] = (table[k][0] * EO[0] + table[k][1] * EO[1] + table[k][2] * EO[2] + table[k][3] * EO[3] + add) >> shift;
		}

		for (int k = 1; k<16; k += 2)
		{
			dst[k*dst_stride] = (table[k][0] * O[0] + table[k][1] * O[1] + table[k][2] * O[2] + table[k][3] * O[3] +
				table[k][4] * O[4] + table[k][5] * O[5] + table[k][6] * O[6] + table[k][7] * O[7] + add) >> shift;
		}

		src += src_stride;
		++dst;
	}
}


void hevcasm_partial_butterfly_32x32_c_opt(int16_t *dst, const int16_t *src, ptrdiff_t src_stride, int shift)
{
	const int add = 1 << (shift - 1);
	const int dst_stride = 32;

	for (int j = 0; j<32; j++)
	{
		int E[16], O[16];
		for (int k = 0; k<16; k++)
		{
			E[k] = src[k] + src[31 - k];
			O[k] = src[k] - src[31 - k];
		}

		int EE[8], EO[8];
		for (int k = 0; k<8; k++)
		{
			EE[k] = E[k] + E[15 - k];
			EO[k] = E[k] - E[15 - k];
		}

		int EEE[4], EEO[4];
		for (int k = 0; k<4; k++)
		{
			EEE[k] = EE[k] + EE[7 - k];
			EEO[k] = EE[k] - EE[7 - k];
		}

		int EEEE[2], EEEO[2];
		EEEE[0] = EEE[0] + EEE[3];
		EEEO[0] = EEE[0] - EEE[3];
		EEEE[1] = EEE[1] + EEE[2];
		EEEO[1] = EEE[1] - EEE[2];

		static const int16_t table[32][32] =
		{
			{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
			{ 90, 90, 88, 85, 82, 78, 73, 67, 61, 54, 46, 38, 31, 22, 13, 4, -4, -13, -22, -31, -38, -46, -54, -61, -67, -73, -78, -82, -85, -88, -90, -90 },
			{ 90, 87, 80, 70, 57, 43, 25, 9, -9, -25, -43, -57, -70, -80, -87, -90, -90, -87, -80, -70, -57, -43, -25, -9, 9, 25, 43, 57, 70, 80, 87, 90 },
			{ 90, 82, 67, 46, 22, -4, -31, -54, -73, -85, -90, -88, -78, -61, -38, -13, 13, 38, 61, 78, 88, 90, 85, 73, 54, 31, 4, -22, -46, -67, -82, -90 },
			{ 89, 75, 50, 18, -18, -50, -75, -89, -89, -75, -50, -18, 18, 50, 75, 89, 89, 75, 50, 18, -18, -50, -75, -89, -89, -75, -50, -18, 18, 50, 75, 89 },
			{ 88, 67, 31, -13, -54, -82, -90, -78, -46, -4, 38, 73, 90, 85, 61, 22, -22, -61, -85, -90, -73, -38, 4, 46, 78, 90, 82, 54, 13, -31, -67, -88 },
			{ 87, 57, 9, -43, -80, -90, -70, -25, 25, 70, 90, 80, 43, -9, -57, -87, -87, -57, -9, 43, 80, 90, 70, 25, -25, -70, -90, -80, -43, 9, 57, 87 },
			{ 85, 46, -13, -67, -90, -73, -22, 38, 82, 88, 54, -4, -61, -90, -78, -31, 31, 78, 90, 61, 4, -54, -88, -82, -38, 22, 73, 90, 67, 13, -46, -85 },
			{ 83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83 },
			{ 82, 22, -54, -90, -61, 13, 78, 85, 31, -46, -90, -67, 4, 73, 88, 38, -38, -88, -73, -4, 67, 90, 46, -31, -85, -78, -13, 61, 90, 54, -22, -82 },
			{ 80, 9, -70, -87, -25, 57, 90, 43, -43, -90, -57, 25, 87, 70, -9, -80, -80, -9, 70, 87, 25, -57, -90, -43, 43, 90, 57, -25, -87, -70, 9, 80 },
			{ 78, -4, -82, -73, 13, 85, 67, -22, -88, -61, 31, 90, 54, -38, -90, -46, 46, 90, 38, -54, -90, -31, 61, 88, 22, -67, -85, -13, 73, 82, 4, -78 },
			{ 75, -18, -89, -50, 50, 89, 18, -75, -75, 18, 89, 50, -50, -89, -18, 75, 75, -18, -89, -50, 50, 89, 18, -75, -75, 18, 89, 50, -50, -89, -18, 75 },
			{ 73, -31, -90, -22, 78, 67, -38, -90, -13, 82, 61, -46, -88, -4, 85, 54, -54, -85, 4, 88, 46, -61, -82, 13, 90, 38, -67, -78, 22, 90, 31, -73 },
			{ 70, -43, -87, 9, 90, 25, -80, -57, 57, 80, -25, -90, -9, 87, 43, -70, -70, 43, 87, -9, -90, -25, 80, 57, -57, -80, 25, 90, 9, -87, -43, 70 },
			{ 67, -54, -78, 38, 85, -22, -90, 4, 90, 13, -88, -31, 82, 46, -73, -61, 61, 73, -46, -82, 31, 88, -13, -90, -4, 90, 22, -85, -38, 78, 54, -67 },
			{ 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64 },
			{ 61, -73, -46, 82, 31, -88, -13, 90, -4, -90, 22, 85, -38, -78, 54, 67, -67, -54, 78, 38, -85, -22, 90, 4, -90, 13, 88, -31, -82, 46, 73, -61 },
			{ 57, -80, -25, 90, -9, -87, 43, 70, -70, -43, 87, 9, -90, 25, 80, -57, -57, 80, 25, -90, 9, 87, -43, -70, 70, 43, -87, -9, 90, -25, -80, 57 },
			{ 54, -85, -4, 88, -46, -61, 82, 13, -90, 38, 67, -78, -22, 90, -31, -73, 73, 31, -90, 22, 78, -67, -38, 90, -13, -82, 61, 46, -88, 4, 85, -54 },
			{ 50, -89, 18, 75, -75, -18, 89, -50, -50, 89, -18, -75, 75, 18, -89, 50, 50, -89, 18, 75, -75, -18, 89, -50, -50, 89, -18, -75, 75, 18, -89, 50 },
			{ 46, -90, 38, 54, -90, 31, 61, -88, 22, 67, -85, 13, 73, -82, 4, 78, -78, -4, 82, -73, -13, 85, -67, -22, 88, -61, -31, 90, -54, -38, 90, -46 },
			{ 43, -90, 57, 25, -87, 70, 9, -80, 80, -9, -70, 87, -25, -57, 90, -43, -43, 90, -57, -25, 87, -70, -9, 80, -80, 9, 70, -87, 25, 57, -90, 43 },
			{ 38, -88, 73, -4, -67, 90, -46, -31, 85, -78, 13, 61, -90, 54, 22, -82, 82, -22, -54, 90, -61, -13, 78, -85, 31, 46, -90, 67, 4, -73, 88, -38 },
			{ 36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36 },
			{ 31, -78, 90, -61, 4, 54, -88, 82, -38, -22, 73, -90, 67, -13, -46, 85, -85, 46, 13, -67, 90, -73, 22, 38, -82, 88, -54, -4, 61, -90, 78, -31 },
			{ 25, -70, 90, -80, 43, 9, -57, 87, -87, 57, -9, -43, 80, -90, 70, -25, -25, 70, -90, 80, -43, -9, 57, -87, 87, -57, 9, 43, -80, 90, -70, 25 },
			{ 22, -61, 85, -90, 73, -38, -4, 46, -78, 90, -82, 54, -13, -31, 67, -88, 88, -67, 31, 13, -54, 82, -90, 78, -46, 4, 38, -73, 90, -85, 61, -22 },
			{ 18, -50, 75, -89, 89, -75, 50, -18, -18, 50, -75, 89, -89, 75, -50, 18, 18, -50, 75, -89, 89, -75, 50, -18, -18, 50, -75, 89, -89, 75, -50, 18 },
			{ 13, -38, 61, -78, 88, -90, 85, -73, 54, -31, 4, 22, -46, 67, -82, 90, -90, 82, -67, 46, -22, -4, 31, -54, 73, -85, 90, -88, 78, -61, 38, -13 },
			{ 9, -25, 43, -57, 70, -80, 87, -90, 90, -87, 80, -70, 57, -43, 25, -9, -9, 25, -43, 57, -70, 80, -87, 90, -90, 87, -80, 70, -57, 43, -25, 9 },
			{ 4, -13, 22, -31, 38, -46, 54, -61, 67, -73, 78, -82, 85, -88, 90, -90, 90, -90, 88, -85, 82, -78, 73, -67, 61, -54, 46, -38, 31, -22, 13, -4 }
		};

		dst[0 * dst_stride] = (table[0][0] * EEEE[0] + table[0][1] * EEEE[1] + add) >> shift;
		dst[16 * dst_stride] = (table[16][0] * EEEE[0] + table[16][1] * EEEE[1] + add) >> shift;
		dst[8 * dst_stride] = (table[8][0] * EEEO[0] + table[8][1] * EEEO[1] + add) >> shift;
		dst[24 * dst_stride] = (table[24][0] * EEEO[0] + table[24][1] * EEEO[1] + add) >> shift;
		for (int k = 4; k<32; k += 8)
		{
			dst[k*dst_stride] = (table[k][0] * EEO[0] + table[k][1] * EEO[1] + table[k][2] * EEO[2] + table[k][3] * EEO[3] + add) >> shift;
		}
		for (int k = 2; k<32; k += 4)
		{
			dst[k*dst_stride] = (table[k][0] * EO[0] + table[k][1] * EO[1] + table[k][2] * EO[2] + table[k][3] * EO[3] +
				table[k][4] * EO[4] + table[k][5] * EO[5] + table[k][6] * EO[6] + table[k][7] * EO[7] + add) >> shift;
		}
		for (int k = 1; k<32; k += 2)
		{
			dst[k*dst_stride] = (table[k][0] * O[0] + table[k][1] * O[1] + table[k][2] * O[2] + table[k][3] * O[3] +
				table[k][4] * O[4] + table[k][5] * O[5] + table[k][6] * O[6] + table[k][7] * O[7] +
				table[k][8] * O[8] + table[k][9] * O[9] + table[k][10] * O[10] + table[k][11] * O[11] +
				table[k][12] * O[12] + table[k][13] * O[13] + table[k][14] * O[14] + table[k][15] * O[15] + add) >> shift;
		}
		src += src_stride;
		dst++;
	}
}


void hevcasm_dst_4x4_c_opt(int16_t coeffs[4 * 4], const int16_t *src, ptrdiff_t src_stride)
{
	int16_t temp[4 * 4];
	hevcasm_partial_butterfly_4x4_dst_c_opt(temp, src, src_stride,1 );
	hevcasm_partial_butterfly_4x4_dst_c_opt(coeffs, temp, 4, 8);
}


void hevcasm_dct_4x4_c_opt(int16_t coeffs[4 * 4], const int16_t *src, ptrdiff_t src_stride)
{
	int16_t temp[4 * 4];
	hevcasm_partial_butterfly_4x4_c_opt(temp, src, src_stride, 1);
	hevcasm_partial_butterfly_4x4_c_opt(coeffs, temp, 4, 8);
}


void hevcasm_dct_8x8_c_opt(int16_t coeffs[8 * 8], const int16_t *src, ptrdiff_t src_stride)
{
	int16_t temp[8 * 8];
	hevcasm_partial_butterfly_8x8_c_opt(temp, src, src_stride, 2);
	hevcasm_partial_butterfly_8x8_c_opt(coeffs, temp, 8, 9);
}


void hevcasm_dct_16x16_c_opt(int16_t coeffs[16 * 16], const int16_t *src, ptrdiff_t src_stride)
{
	int16_t temp[16 * 16];
	hevcasm_partial_butterfly_16x16_c_opt(temp, src, src_stride, 3);
	hevcasm_partial_butterfly_16x16_c_opt(coeffs, temp, 16, 10);
}


void hevcasm_dct_32x32_c_opt(int16_t coeffs[32 * 32], const int16_t *src, ptrdiff_t src_stride)
{
	int16_t temp[32 * 32];
	hevcasm_partial_butterfly_32x32_c_opt(temp, src, src_stride, 4);
	hevcasm_partial_butterfly_32x32_c_opt(coeffs, temp, 32, 11);
}



#ifdef HEVCASM_X64
void hevcasm_dct_16x16_ssse3(int16_t *coeffs, const int16_t *src, ptrdiff_t src_stride)
{
	HEVCASM_ALIGN(32, int16_t, temp[16 * 16]);
	hevcasm_partial_butterfly_16h_ssse3(temp, src, src_stride, 3);
	hevcasm_partial_butterfly_16v_ssse3(coeffs, temp, 10);
}
#else
/* hevcasm_dct_16x16_ssse3 uses too many xmm registers for a 32-bit build */
hevcasm_transform *hevcasm_dct_16x16_ssse3 = 0;
#endif


hevcasm_transform* HEVCASM_API get_transform(int trType, int log2TrafoSize, hevcasm_instruction_set mask)
{
	const int nCbS = 1 << log2TrafoSize;

	hevcasm_transform *f = 0;

	if (mask & (HEVCASM_C_REF | HEVCASM_C_OPT))
	{
		if (nCbS == 4) f = trType ? hevcasm_dst_4x4_c_opt : hevcasm_dct_4x4_c_opt;
		if (nCbS == 8) f = hevcasm_dct_8x8_c_opt;
		if (nCbS == 16) f = hevcasm_dct_16x16_c_opt;
		if (nCbS == 32) f = hevcasm_dct_32x32_c_opt;
	}

	if (mask & HEVCASM_SSSE3)
	{
		if (nCbS == 16 && hevcasm_dct_16x16_ssse3) f = hevcasm_dct_16x16_ssse3;
	}

	return f;
}


void HEVCASM_API hevcasm_populate_transform(hevcasm_table_transform *table, hevcasm_instruction_set mask)
{
	*hevcasm_get_transform(table, 1, 2) = get_transform(1, 2, mask);
	for (int log2TrafoSize = 2; log2TrafoSize <= 5; ++log2TrafoSize)
	{
		*hevcasm_get_transform(table, 0, log2TrafoSize) = get_transform(0, log2TrafoSize, mask);
	}
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


int init_transform(void *p, hevcasm_instruction_set mask)
{
	bound_transform *s = p;

	hevcasm_table_transform table;
	hevcasm_populate_transform(&table, mask);

	s->f = *hevcasm_get_transform(&table, s->trType, s->log2TrafoSize);
	assert(s->f == get_transform(s->trType, s->log2TrafoSize, mask));

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

		*error_count += hevcasm_test(&b[0], &b[1], init_transform, invoke_transform, mismatch_transform, mask, 100000);
	}
}
