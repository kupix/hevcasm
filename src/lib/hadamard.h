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

void HEVCASM_API hevcasm_populate_hadamard_satd(hevcasm_table_hadamard_satd *table, hevcasm_instruction_set mask);

void HEVCASM_API hevcasm_test_hadamard_satd(int *error_count, hevcasm_instruction_set mask);


#ifdef __cplusplus
}
#endif

#endif
