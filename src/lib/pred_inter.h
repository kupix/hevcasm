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

typedef void HEVCASM_API hevcasm_pred_uni_8to8(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);

typedef struct
{
	hevcasm_pred_uni_8to8 * p[2][9][2][2];
}
hevcasm_table_pred_uni_8to8;

static hevcasm_pred_uni_8to8** hevcasm_get_pred_uni_8to8(hevcasm_table_pred_uni_8to8 *table, int taps, int w, int h, int xFrac, int yFrac)
{
	return &table->p[taps / 4 - 1][(w + taps - 1) / taps][xFrac ? 1 : 0][yFrac ? 1 : 0];
}

void HEVCASM_API hevcasm_populate_pred_uni_8to8(hevcasm_table_pred_uni_8to8 *table, hevcasm_instruction_set mask);

hevcasm_test_function hevcasm_test_pred_uni;


// HEVC bi prediction

typedef void hevcasm_pred_bi_8to8(uint8_t *dst0, ptrdiff_t stride_dst, const uint8_t *ref0, const uint8_t *ref1, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac0, int yFrac0, int xFrac1, int yFrac1);

typedef struct
{
	hevcasm_pred_bi_8to8 * p[2][5][2];
}
hevcasm_table_pred_bi_8to8;

static hevcasm_pred_bi_8to8** hevcasm_get_pred_bi_8to8(hevcasm_table_pred_bi_8to8 *table, int taps, int w, int h, int xFracA, int yFracA, int xFracB, int yFracB)
{
	const int frac = xFracA || yFracA || xFracB || yFracB;
	return &table->p[taps / 4 - 1][(w + 2 * taps - 1) / (2 * taps)][frac];
}

void HEVCASM_API hevcasm_populate_pred_bi_8to8(hevcasm_table_pred_bi_8to8 *table, hevcasm_instruction_set mask);

hevcasm_test_function hevcasm_test_pred_bi;


#ifdef __cplusplus
}
#endif

#endif
