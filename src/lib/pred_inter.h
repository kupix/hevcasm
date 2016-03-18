// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.


#ifndef INCLUDED_hevcasm_prediction_inter_h
#define INCLUDED_hevcasm_prediction_inter_h

// HEVC inter prediction

// Note that these functions may write data to the right of the destination block.


#include "hevcasm.h"


// HEVC uni prediction
template <typename Sample>
using HevcasmPredUni = void (Sample *dst, intptr_t stride_dst, Sample const *ref, intptr_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac, int bitDepth);
typedef HevcasmPredUni<uint8_t> hevcasm_pred_uni_8to8;
typedef HevcasmPredUni<uint16_t> hevcasm_pred_uni_16to16;

template <typename Sample>
struct HevcasmTablePredUni
{
	HevcasmPredUni<Sample>* p[3][2][9][2][2];
};


template <typename Sample>
static HevcasmPredUni<Sample>** hevcasmGetPredUni(HevcasmTablePredUni<Sample> *table, int taps, int w, int h, int xFrac, int yFrac, int bitDepth)
{
	return &table->p[bitDepth - 8][taps / 4 - 1][(w + taps - 1) / taps][xFrac ? 1 : 0][yFrac ? 1 : 0];
}


template <typename Sample>
void hevcasmPopulatePredUni(HevcasmTablePredUni<Sample> *table, hevcasm_code code);


hevcasm_test_function hevcasm_test_pred_uni;


// HEVC bi prediction

template <typename Sample>
using HevcasmPredBi = void(Sample *dst0, intptr_t stride_dst, const Sample *ref0, const Sample *ref1, intptr_t stride_ref, int nPbW, int nPbH, int xFrac0, int yFrac0, int xFrac1, int yFrac1, int bitDepth);

template <typename Sample>
struct HevcasmTablePredBi
{
	HevcasmPredBi<Sample> * p[3][2][5][2];
};

template <typename Sample>
static HevcasmPredBi<Sample>** hevcasmGetPredBi(HevcasmTablePredBi<Sample> *table, int taps, int w, int h, int xFracA, int yFracA, int xFracB, int yFracB, int bitDepth)
{
	const int frac = xFracA || yFracA || xFracB || yFracB;
	return &table->p[bitDepth - 8][taps / 4 - 1][(w + 2 * taps - 1) / (2 * taps)][frac];
}

template <typename Sample>
void hevcasmPopulatePredBi(HevcasmTablePredBi<Sample> *table, hevcasm_code code);

hevcasm_test_function hevcasm_test_pred_bi;

// review: rename, look for generic solution
template <typename Sample> struct Lookup;

template <> struct Lookup<uint8_t>
{
	typedef HevcasmTablePredUni<uint8_t> TablePredUni;
	constexpr static decltype(&hevcasmGetPredUni<uint8_t>) getPredUni() { return &hevcasmGetPredUni<uint8_t>; }
	typedef HevcasmTablePredBi<uint8_t> TablePredBi;
	constexpr static decltype(&hevcasmGetPredBi<uint8_t>) getPredBi() { return &hevcasmGetPredBi<uint8_t>; }
};

template <> struct Lookup<uint16_t>
{
	typedef HevcasmTablePredUni<uint16_t> TablePredUni;
	constexpr static decltype(&hevcasmGetPredUni<uint16_t>) getPredUni() { return &hevcasmGetPredUni<uint16_t>; }
	typedef HevcasmTablePredBi<uint16_t> TablePredBi;
	constexpr static decltype(&hevcasmGetPredBi<uint16_t>) getPredBi() { return &hevcasmGetPredBi<uint16_t>; }
};

#endif
