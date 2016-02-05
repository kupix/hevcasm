// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.


/* Functions for decoding HEVC residual: inverse transform and add the result to predictor */


#ifndef INCLUDED_hadamard_h
#define INCLUDED_hadamard_h

#include "hevcasm.h"



#ifdef __cplusplus
extern "C"
{
#endif



typedef int hevcasm_hadamard_satd(const uint8_t *srcA, ptrdiff_t stride_srcA, const uint8_t *srcB, ptrdiff_t stride_srcB);

typedef struct
{
	hevcasm_hadamard_satd *satd[3];
}
hevcasm_table_hadamard_satd;

static hevcasm_hadamard_satd** hevcasm_get_hadamard_satd(hevcasm_table_hadamard_satd *table, int log2TrafoSize)
{
	return &table->satd[log2TrafoSize - 1];
}

void HEVCASM_API hevcasm_populate_hadamard_satd(hevcasm_table_hadamard_satd *table, hevcasm_code code);

void HEVCASM_API hevcasm_test_hadamard_satd(int *error_count, hevcasm_instruction_set mask);


#ifdef __cplusplus
}
#endif

#endif
