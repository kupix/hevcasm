// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.


/* Functions for decoding HEVC residual: inverse transform and add the result to predictor */


#ifndef INCLUDED_hevcasm_residual_decode_h
#define INCLUDED_hevcasm_residual_decode_h

#include "hevcasm.h"
#include <cassert>


using hevcasm_inverse_transform = void(int16_t dst[], int16_t const coeffs[], int bitDepth);


struct hevcasm_table_inverse_transform
{
	hevcasm_inverse_transform *sine;
	hevcasm_inverse_transform *cosine[4];
};


static inline hevcasm_inverse_transform** hevcasm_get_inverse_transform(hevcasm_table_inverse_transform *table, int trType, int log2TrafoSize)
{
	if (trType)
	{
		assert(log2TrafoSize == 2);
		return &table->sine;
	}
	else
	{
		return &table->cosine[log2TrafoSize - 2];
	}
}


void hevcasm_populate_inverse_transform(hevcasm_table_inverse_transform *table, hevcasm_code code, int encoder);


template <typename Sample>
using hevcasm_inverse_transform_add = void(Sample *dst, intptr_t stride_dst, Sample const* pred, intptr_t stride_pred, int16_t const coeffs[], int bitDepth);


template <typename Sample>
struct hevcasm_table_inverse_transform_add
{
	hevcasm_inverse_transform_add<Sample> *sine;
	hevcasm_inverse_transform_add<Sample> *cosine[4];
};


template <typename Sample>
static inline hevcasm_inverse_transform_add<Sample>** hevcasm_get_inverse_transform_add(hevcasm_table_inverse_transform_add<Sample> *table, int trType, int log2TrafoSize)
{
	if (trType)
	{
		assert(log2TrafoSize == 2);
		return &table->sine;
	}
	else
	{
		return &table->cosine[log2TrafoSize - 2];
	}
}


template <typename Sample>
void hevcasm_populate_inverse_transform_add(hevcasm_table_inverse_transform_add<Sample> *table, hevcasm_code code, int encoder);


template <typename Sample>
void hevcasm_test_inverse_transform_add(int *error_count, hevcasm_instruction_set mask);


static int hevcasm_clip(int x, int bit_depth)
{
	if (x < 0) return 0;
	int const max = (int)(1 << bit_depth) - 1;
	if (x > max) return max;
	return x;
}


template <typename Sample>
void hevcasm_add_residual(int n, Sample* dst, intptr_t stride_dst, Sample const* pred, intptr_t stride_pred, int16_t *residual, int bitDepth)
{
	for (int y = 0; y < n; ++y)
	{
		for (int x = 0; x < n; ++x)
		{
			dst[x + y * stride_dst] = hevcasm_clip(pred[x + y * stride_pred] + residual[x + y * n], bitDepth);
		}
	}
}


// Review: this is an encode function in a file called "residual_decode.h"
typedef void hevcasm_transform(int16_t *coeffs, const int16_t *src, intptr_t src_stride);


template <int bitDepth>
struct hevcasm_table_transform
{
	hevcasm_transform *dst;
	hevcasm_transform *dct[4];
};


template <int bitDepth>
static hevcasm_transform** hevcasm_get_transform(hevcasm_table_transform<bitDepth> *table, int trType, int log2TrafoSize)
{
	if (trType)
	{
		assert(log2TrafoSize == 2);
		return &table->dst;
	}
	else
	{
		return &table->dct[log2TrafoSize - 2];
	}
}

template <int bitDepth>
void hevcasm_populate_transform(hevcasm_table_transform<bitDepth> *table, hevcasm_code code);


void hevcasm_test_transform(int *error_count, hevcasm_instruction_set mask);


#endif
