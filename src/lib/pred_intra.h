// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.


#ifndef INCLUDED_hevcasm_prediction_intra_h
#define INCLUDED_hevcasm_prediction_intra_h

// HEVC intra prediction


#include "hevcasm.h"


#ifdef __cplusplus
extern "C"
{
#endif

// HEVC intra prediction

typedef int hevcasm_pred_intra_packed;

// Function prototype compatible with that of f265
typedef void HEVCASM_API hevcasm_pred_intra(uint8_t *dst, const uint8_t *neighbours, int intraPredMode, hevcasm_pred_intra_packed packed);

// Packing compatible with f265
static __inline hevcasm_pred_intra_packed hevcasm_pred_intra_pack(int cIdx, int log2CbSize)
{
	const int bit_depth = 8;
	const int edge_flag = (cIdx == 0 && log2CbSize < 5) ? 1 : 0;
	return (bit_depth << 8) | (log2CbSize << 1) | edge_flag;
}

typedef struct
{
	hevcasm_pred_intra *p[4 /* log2CbSize - 2 */][35 /* intraPredModeY */];
	hevcasm_pred_intra *p_dc_filter[3 /* log2CbSize - 2 */];
}
hevcasm_table_pred_intra;

static __inline hevcasm_pred_intra** hevcasm_get_pred_intra(hevcasm_table_pred_intra *table, int intraPredMode, hevcasm_pred_intra_packed packed)
{
	const int edge_flag = packed & 0x1;
	const int log2CbSize = (packed >> 1) & 0x7f;
	if (intraPredMode == 1 && edge_flag)
	{
		return &table->p_dc_filter[log2CbSize - 2];
	}
	else
	{
		return &table->p[log2CbSize - 2][intraPredMode];
	}
}

void HEVCASM_API hevcasm_populate_pred_intra(hevcasm_table_pred_intra *table, hevcasm_instruction_set mask);

hevcasm_test_function hevcasm_test_pred_intra;

#ifdef __cplusplus
}
#endif

#endif
