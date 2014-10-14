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

/* Functions for decoding HEVC residual: inverse transform and add the result to predictor */


#ifndef INCLUDED_hevcasm_residual_decode_h
#define INCLUDED_hevcasm_residual_decode_h

#include "hevcasm.h"

#include "assert.h"


#ifdef __cplusplus
extern "C"
{
#endif



typedef void hevcasm_inverse_transform_add(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *pred, ptrdiff_t stride_pred, const int16_t *coeffs);

typedef struct
{
	hevcasm_inverse_transform_add *dst;
	hevcasm_inverse_transform_add *dct[4];
}
hevcasm_table_inverse_transform_add;

static hevcasm_inverse_transform_add** hevcasm_get_inverse_transform_add(hevcasm_table_inverse_transform_add *table, int trType, int log2TrafoSize)
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

void HEVCASM_API hevcasm_populate_inverse_transform_add(hevcasm_table_inverse_transform_add *table, hevcasm_instruction_set mask);

void HEVCASM_API hevcasm_test_inverse_transform_add(int *error_count, hevcasm_instruction_set mask);


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

void HEVCASM_API hevcasm_populate_transform(hevcasm_table_transform *table, hevcasm_instruction_set mask);

void HEVCASM_API hevcasm_test_transform(int *error_count, hevcasm_instruction_set mask);



#ifdef __cplusplus
}
#endif

#endif
