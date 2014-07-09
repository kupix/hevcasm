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

#include "residual_decode.h"
#include "hevcasm_test.h"

#include <stdlib.h>
#include <string.h>


// declaration of functions in residual_decode_a.asm
void hevcasm_partial_butterfly_inverse_8v_ssse3(int16_t *dst, const int16_t *src, int shift);
void hevcasm_partial_butterfly_inverse_8h_ssse3(int16_t *dst, const int16_t *src, int shift);




int hevcasm_clip3(int min, int max, int x)
{
	if (x > max) return max;
	if (x < min) return min;
	return x;
}

void hevcasm_inverse_partial_butterfly_8x8(int16_t dst[8 * 8], const int16_t src[8 * 8], int shift)
{
	const int add = 1 << (shift - 1);
	const int srcStride = 8;
	const int dstStride = 8;

	for (int j = 0; j<8; j++)
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
		for (int k = 0; k<4; k++)
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
		for (int k = 0; k<4; k++)
		{
			dst[k] = hevcasm_clip3(-32768, 32767, (E[k] + O[k] + add) >> shift);
			dst[k + 4] = hevcasm_clip3(-32768, 32767, (E[3 - k] - O[3 - k] + add) >> shift);
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

void hevcasm_idct_8x8_c(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *pred, ptrdiff_t stride_pred, const int16_t coeffs[8 * 8])
{
	int16_t temp[2][8 * 8];
	hevcasm_inverse_partial_butterfly_8x8(temp[0], coeffs, 7);
	hevcasm_inverse_partial_butterfly_8x8(temp[1], temp[0], 12);
	hevcasm_add_residual_8x8(dst, stride_dst, pred, stride_pred, temp[1]);
}

void hevcasm_idct_8x8_ssse3(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *pred, ptrdiff_t stride_pred, const int16_t coeffs[8 * 8])
{
	ALIGN(32, int16_t, temp[2][8 * 8]);
	hevcasm_partial_butterfly_inverse_8v_ssse3(temp[0], coeffs, 7);
	hevcasm_partial_butterfly_inverse_8h_ssse3(temp[1], temp[0], 12);
	hevcasm_add_residual_8x8(dst, stride_dst, pred, stride_pred, temp[1]);
}

void hevcasm_get_inverse_transform_add(hevcasm_inverse_transform_add **table, hevcasm_instruction_set_t mask)
{
	table[1] = 0;
	table[2] = 0;
	table[3] = 0;
	table[4] = 0;
	table[5] = 0;
	if (mask & HEVCASM_C)
	{
		table[3] = hevcasm_idct_8x8_c;
	}
	if (mask & HEVCASM_SSSE3)
	{
		table[3] = hevcasm_idct_8x8_ssse3;
	}
}

int hevcasm_validate_inverse_transform_add(hevcasm_instruction_set_t mask)
{
	int error_count = 0;
	printf("inverse_transform_add\n");
	hevcasm_inverse_transform_add *table_inverse_transform_add[HEVCASM_INSTRUCTION_SET_COUNT][6];
	for (hevcasm_instruction_set_idx_t i = 0; i < HEVCASM_INSTRUCTION_SET_COUNT; ++i)
	{
		hevcasm_get_inverse_transform_add(table_inverse_transform_add[i], 1 << i);
	}

	ALIGN(32, int16_t, coefficients[32 * 32]);
	ALIGN(32, uint8_t, predicted[32 * 32]);
	ALIGN(32, uint8_t, dst[2][32 * 32]);

	for (int x = 0; x < 8 * 8; x++) coefficients[x] = (rand() & 0x1ff) - 0x100;
	for (int x = 0; x < 8 * 8; x++) predicted[x] = rand() & 0xff;

	for (int j = 1; j < 6; ++j)
	{
		const int size = hevcasm_tranform_size(j);

		printf("%s %s : ", hevcasm_tranform_type_as_text(j), hevcasm_tranform_size_as_text(j));

		table_inverse_transform_add[C][3](dst[0], size, predicted, size, coefficients);

		for (hevcasm_instruction_set_idx_t i = 0; i < HEVCASM_INSTRUCTION_SET_COUNT; ++i)
		{
			hevcasm_inverse_transform_add *f = table_inverse_transform_add[i][j];
			if (f)
			{
				printf(" %s", hevcasm_instruction_set_idx_as_text(i));
				f(dst[1], size, predicted, size, coefficients);
				const int mismatch = memcmp(dst[0], dst[1], sizeof(uint8_t)* size * size);
				if (mismatch)
				{
					printf("-MISMATCH ");
					++error_count;
				}
			}
		}
		printf("\n");
	}
	printf("\n");
	return error_count;
}

void hevcasm_time_inverse_transform_add(hevcasm_instruction_set_t mask)
{
	printf("inverse_transform_add\n");
	hevcasm_inverse_transform_add *table_inverse_transform_add[HEVCASM_INSTRUCTION_SET_COUNT][6];
	for (hevcasm_instruction_set_idx_t i = 0; i < HEVCASM_INSTRUCTION_SET_COUNT; ++i)
	{
		hevcasm_get_inverse_transform_add(table_inverse_transform_add[i], 1 << i);
	}

	ALIGN(32, int16_t, coefficients[32 * 32]);
	ALIGN(32, uint8_t, predicted[32 * 32]);
	ALIGN(32, uint8_t, dst[32 * 32]);

	for (int j = 1; j < 6; ++j)
	{
		const int size = hevcasm_tranform_size(j);
		const int iterations = 1000;
		double first_result = 0.0;

		printf("%s %s : ", hevcasm_tranform_type_as_text(j), hevcasm_tranform_size_as_text(j));

		for (hevcasm_instruction_set_idx_t i = 0; i < HEVCASM_INSTRUCTION_SET_COUNT; ++i)
		{
			hevcasm_inverse_transform_add *f = table_inverse_transform_add[i][j];
			if (f)
			{
				printf(" %s:", hevcasm_instruction_set_idx_as_text(i));
				const hevcasm_timestamp_t start = hevcasm_get_timestamp();
				for (int n = 0; n < iterations; ++n)
				{
					f(dst, size, predicted, size, coefficients);
				}
				const int duration = (int)(hevcasm_get_timestamp() - start);
				const int average = (duration + iterations / 2) / iterations;
				printf("%d", average);
				if (first_result != 0.0)
				{
					printf("(x%.2f)", first_result / average);
				}
				else
				{
					first_result = average;
				}
			}
		}
		printf("\n");
	}
	printf("\n");
}