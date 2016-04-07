// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.


/* Quantization and reconstruction functions */


#ifndef INCLUDED_quantize_h
#define INCLUDED_quantize_h

#include "hevcasm.h"

#include <stdint.h>
#include <stddef.h>


#ifdef __cplusplus
extern "C"
{
#endif


// HEVC inverse quantization ("scaling")

typedef void hevcasm_quantize_inverse(int16_t *dst, const int16_t *src, int scale, int shift, int n);

typedef struct
{
	hevcasm_quantize_inverse *p[2];
}
hevcasm_table_quantize_inverse;

static hevcasm_quantize_inverse** hevcasm_get_quantize_inverse(hevcasm_table_quantize_inverse *table, int scale, int shift)
{
	return &table->p[!!(scale & ((1 << shift) - 1))];
}

void hevcasm_populate_quantize_inverse(hevcasm_table_quantize_inverse *table, hevcasm_code code);

void hevcasm_test_quantize_inverse(int *error_count, hevcasm_instruction_set mask);



// HEVC simple quantization (similar to that in HM)

typedef int hevcasm_quantize(int16_t *dst, const int16_t *src, int scale, int shift, int offset, int n);

typedef struct
{
	hevcasm_quantize *p;
}
hevcasm_table_quantize;

static hevcasm_quantize** hevcasm_get_quantize(hevcasm_table_quantize *table)
{
	return &table->p;
}

void hevcasm_populate_quantize(hevcasm_table_quantize *table, hevcasm_code code);

void hevcasm_test_quantize(int *error_count, hevcasm_instruction_set mask);



// Reconstruction function: adds a CU's predicted and residual values

typedef void hevcasm_quantize_reconstruct(uint8_t *rec, intptr_t stride_rec, const uint8_t *pred, intptr_t stride_pred, const int16_t *res, int n);

typedef struct
{
	hevcasm_quantize_reconstruct *p[4];
}
hevcasm_table_quantize_reconstruct;

static hevcasm_quantize_reconstruct** hevcasm_get_quantize_reconstruct(hevcasm_table_quantize_reconstruct *table, int log2TrafoSize)
{
	return &table->p[log2TrafoSize - 2];
}

void hevcasm_populate_quantize_reconstruct(hevcasm_table_quantize_reconstruct *table, hevcasm_code code);

void hevcasm_test_quantize_reconstruct(int *error_count, hevcasm_instruction_set mask);


#ifdef __cplusplus
}
#endif

#endif
