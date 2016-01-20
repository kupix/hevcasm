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


#ifndef INCLUDED_hevcasm_test_h
#define INCLUDED_hevcasm_test_h

#include "hevcasm.h"

#include <inttypes.h>



static const char *hevcasm_instruction_set_as_text(hevcasm_instruction_set set)
{
#define X(value, name, description) if (set == (1 << value)) return #name;
	HEVCASM_INSTRUCTION_SET_XMACRO
#undef X
	return 0;
}

typedef void hevcasm_bound_invoke(void *bound, int n);

typedef int hevcasm_bound_mismatch(void *boundRef, void *boundTest);

typedef int hevcasm_bound_get(void *bound, hevcasm_instruction_set set);


int hevcasm_count_average_cycles(
	void *boundRef, void *boundTest,
	hevcasm_bound_invoke *f,
	hevcasm_bound_mismatch *m,
	double *first_result,
	hevcasm_instruction_set set,
	int iterations);


int hevcasm_test(
	void *ref,
	void *test,
	hevcasm_bound_get *get,
	hevcasm_bound_invoke *invoke,
	hevcasm_bound_mismatch *mismatch,
	hevcasm_instruction_set mask,
	int iterations);


#endif
