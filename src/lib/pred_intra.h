/*
The copyright in this software is being made available under the BSD
License, included below. This software may be subject to other third party
and contributor rights, including patent rights, and no such rights are
granted under this license.


Copyright(c) 2011 - 2015, Parabola Research Limited
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
