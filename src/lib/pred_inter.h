// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.


#ifndef INCLUDED_hevcasm_prediction_inter_h
#define INCLUDED_hevcasm_prediction_inter_h

// HEVC inter prediction

// Note that these functions may write data to the right of the destination block.


#include "hevcasm.h"


#ifdef __cplusplus
extern "C"
{
#endif


// HEVC uni prediction

typedef void HEVCASM_API hevcasm_pred_uni_8to8(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac, int bitDepth);
typedef void HEVCASM_API hevcasm_pred_uni_16to16(uint16_t *dst, ptrdiff_t stride_dst, const uint16_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac, int bitDepth);

typedef struct
{
	hevcasm_pred_uni_8to8 * p[2][9][2][2];
}
hevcasm_table_pred_uni_8to8;

typedef struct
{
	hevcasm_pred_uni_16to16 * p[2][9][2][2];
}
hevcasm_table_pred_uni_16to16;

static hevcasm_pred_uni_8to8** hevcasm_get_pred_uni_8to8(hevcasm_table_pred_uni_8to8 *table, int taps, int w, int h, int xFrac, int yFrac)
{
	return &table->p[taps / 4 - 1][(w + taps - 1) / taps][xFrac ? 1 : 0][yFrac ? 1 : 0];
}

static hevcasm_pred_uni_16to16** hevcasm_get_pred_uni_16to16(hevcasm_table_pred_uni_16to16 *table, int taps, int w, int h, int xFrac, int yFrac)
{
	return &table->p[taps / 4 - 1][(w + taps - 1) / taps][xFrac ? 1 : 0][yFrac ? 1 : 0];
}

void HEVCASM_API hevcasm_populate_pred_uni_8to8(hevcasm_table_pred_uni_8to8 *table, hevcasm_code code);
void HEVCASM_API hevcasm_populate_pred_uni_16to16(hevcasm_table_pred_uni_16to16 *table, hevcasm_code code);

hevcasm_test_function hevcasm_test_pred_uni;


// HEVC bi prediction

typedef void hevcasm_pred_bi_8to8(uint8_t *dst0, ptrdiff_t stride_dst, const uint8_t *ref0, const uint8_t *ref1, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac0, int yFrac0, int xFrac1, int yFrac1, int bitDepth);
typedef void hevcasm_pred_bi_16to16(uint16_t *dst0, ptrdiff_t stride_dst, const uint16_t *ref0, const uint16_t *ref1, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac0, int yFrac0, int xFrac1, int yFrac1, int bitDepth);

typedef struct
{
	hevcasm_pred_bi_8to8 * p[2][5][2];
}
hevcasm_table_pred_bi_8to8;

typedef struct
{
	hevcasm_pred_bi_16to16 * p[2][5][2];
}
hevcasm_table_pred_bi_16to16;

static hevcasm_pred_bi_8to8** hevcasm_get_pred_bi_8to8(hevcasm_table_pred_bi_8to8 *table, int taps, int w, int h, int xFracA, int yFracA, int xFracB, int yFracB)
{
	const int frac = xFracA || yFracA || xFracB || yFracB;
	return &table->p[taps / 4 - 1][(w + 2 * taps - 1) / (2 * taps)][frac];
}

static hevcasm_pred_bi_16to16** hevcasm_get_pred_bi_16to16(hevcasm_table_pred_bi_16to16 *table, int taps, int w, int h, int xFracA, int yFracA, int xFracB, int yFracB)
{
	const int frac = xFracA || yFracA || xFracB || yFracB;
	return &table->p[taps / 4 - 1][(w + 2 * taps - 1) / (2 * taps)][frac];
}

void HEVCASM_API hevcasm_populate_pred_bi_8to8(hevcasm_table_pred_bi_8to8 *table, hevcasm_code code);
void HEVCASM_API hevcasm_populate_pred_bi_16to16(hevcasm_table_pred_bi_16to16 *table, hevcasm_code code);

hevcasm_test_function hevcasm_test_pred_bi;


#ifdef __cplusplus
}
#endif

#endif
