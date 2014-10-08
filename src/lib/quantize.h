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
	hevcasm_quantize_inverse *p;
}
hevcasm_table_quantize_inverse;

static hevcasm_quantize_inverse** hevcasm_get_quantize_inverse(hevcasm_table_quantize_inverse *table)
{
	return &table->p;
}

void HEVCASM_API hevcasm_populate_quantize_inverse(hevcasm_table_quantize_inverse *table, hevcasm_instruction_set mask);

void HEVCASM_API hevcasm_test_quantize_inverse(int *error_count, hevcasm_instruction_set mask);



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

void HEVCASM_API hevcasm_populate_quantize(hevcasm_table_quantize *table, hevcasm_instruction_set mask);

void HEVCASM_API hevcasm_test_quantize(int *error_count, hevcasm_instruction_set mask);



// Reconstruction function: adds a CU's predicted and residual values

typedef void hevcasm_quantize_reconstruct(uint8_t *rec, ptrdiff_t stride_rec, const uint8_t *pred, ptrdiff_t stride_pred, const int16_t *res, int n);

typedef struct
{
	hevcasm_quantize_reconstruct *p[4];
}
hevcasm_table_quantize_reconstruct;

static hevcasm_quantize_reconstruct** hevcasm_get_quantize_reconstruct(hevcasm_table_quantize_reconstruct *table, int log2TrafoSize)
{
	return &table->p[log2TrafoSize - 2];
}

void HEVCASM_API hevcasm_populate_quantize_reconstruct(hevcasm_table_quantize_reconstruct *table, hevcasm_instruction_set mask);

void HEVCASM_API hevcasm_test_quantize_reconstruct(int *error_count, hevcasm_instruction_set mask);


#ifdef __cplusplus
}
#endif

#endif
