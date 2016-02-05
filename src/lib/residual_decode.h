// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.


/* Functions for decoding HEVC residual: inverse transform and add the result to predictor */


#ifndef INCLUDED_hevcasm_residual_decode_h
#define INCLUDED_hevcasm_residual_decode_h

#include "hevcasm.h"

#include "assert.h"


#ifdef __cplusplus
extern "C"
{
#endif



typedef void hevcasm_inverse_transform_add8(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *pred, ptrdiff_t stride_pred, const int16_t *coeffs, int bitDepth);
typedef void hevcasm_inverse_transform_add16(uint16_t *dst, ptrdiff_t stride_dst, const uint16_t *pred, ptrdiff_t stride_pred, const int16_t *coeffs, int bitDepth);

typedef struct
{
	hevcasm_inverse_transform_add8 *dst;
	hevcasm_inverse_transform_add8 *dct[4];
}
hevcasm_table_inverse_transform_add8;

typedef struct
{
	hevcasm_inverse_transform_add16 *dst;
	hevcasm_inverse_transform_add16 *dct[4];
}
hevcasm_table_inverse_transform_add16;

static hevcasm_inverse_transform_add8** hevcasm_get_inverse_transform_add8(hevcasm_table_inverse_transform_add8 *table, int trType, int log2TrafoSize)
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

static hevcasm_inverse_transform_add16** hevcasm_get_inverse_transform_add16(hevcasm_table_inverse_transform_add16 *table, int trType, int log2TrafoSize)
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

void HEVCASM_API hevcasm_populate_inverse_transform_add8(hevcasm_table_inverse_transform_add8 *table, hevcasm_code code, int encoder);
void HEVCASM_API hevcasm_populate_inverse_transform_add16(hevcasm_table_inverse_transform_add16 *table, hevcasm_code code, int encoder);

void HEVCASM_API hevcasm_test_inverse_transform_add8(int *error_count, hevcasm_instruction_set mask);
void HEVCASM_API hevcasm_test_inverse_transform_add16(int *error_count, hevcasm_instruction_set mask);


// Review: this is an encode function in a file called "residual_decode.h"
typedef void hevcasm_transform(int16_t *coeffs, const int16_t *src, ptrdiff_t src_stride);

typedef struct
{
	hevcasm_transform *dst;
	hevcasm_transform *dct[4];
}
hevcasm_table_transform;

static hevcasm_transform** hevcasm_get_transform(hevcasm_table_transform *table, int trType, int log2TrafoSize)
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

void HEVCASM_API hevcasm_populate_transform(hevcasm_table_transform *table, hevcasm_code code);

void HEVCASM_API hevcasm_test_transform(int *error_count, hevcasm_instruction_set mask);



#ifdef __cplusplus
}
#endif

#endif
