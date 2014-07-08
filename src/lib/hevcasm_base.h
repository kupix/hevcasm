/*
The copyright in this software is being made available under the BSD
License, included below. This software may be subject to other third party
and contributor rights, including patent rights, and no such rights are
granted under this license.


Copyright(c) 2011 - 2014, Parabola Research Limited
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met :

*Redistributions of source code must retain the above copyright notice,
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


#ifndef INCLUDED_hevcasm_base_h
#define INCLUDED_hevcasm_base_h

#include <stddef.h>
#include <inttypes.h>
#include <stdio.h>


typedef void(*hevcasm_transform_function)(int16_t *dst, ptrdiff_t stride_dst, int16_t *src, ptrdiff_t stride_src);


#define HEVCASM_INSTRUCTION_SET_XMACRO \
	X(1<<1, SSE2) \
	X(1<<2, SSE3) \
	X(1<<3, SSSE3) \
	X(1<<4, SSE41) \
	X(1<<5, POPCNT) \
	X(1<<6, SSE42) \
	X(1<<7, AVX) \
	X(1<<8, RDRAND) \
	X(1<<9, PCLMUL_AES) \
	X(1<<10, AVX2)

typedef enum {
#define X(value, name) HEVCASM_ ## name = value,
	HEVCASM_INSTRUCTION_SET_XMACRO
#undef X
} hevcasm_instruction_set_t;

hevcasm_instruction_set_t hevcasm_instruction_set_support();

void hevcasm_print_instruction_set_support(FILE *f, hevcasm_instruction_set_t mask);


void hevcasm_get_forward_transforms(hevcasm_transform_function *table, hevcasm_instruction_set_t mask);
void hevcasm_get_inverse_transforms(hevcasm_transform_function *table, hevcasm_instruction_set_t mask);




#endif
