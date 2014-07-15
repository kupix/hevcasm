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


typedef enum {
#define X(value, name, description) name = value,
	HEVCASM_INSTRUCTION_SET_XMACRO
#undef X
} hevcasm_instruction_set_idx_t;

static const char *hevcasm_instruction_set_idx_as_text(hevcasm_instruction_set_idx_t i)
{
#define X(value, name, description) if (i == value) return #name;
	HEVCASM_INSTRUCTION_SET_XMACRO
#undef X
	return 0;
}

typedef void hevc_bound_function(void *p, int n);

static void hevcasm_count_average_cycles(hevc_bound_function *f, void *p, double *first_result, hevcasm_instruction_set i, int iterations)
{
	hevcasm_timestamp sum = 0;
	int warmup = 100;
	int count = 0;
	while (count < iterations)
	{
		const hevcasm_timestamp start = hevcasm_get_timestamp();
		f(p, 4);
		const hevcasm_timestamp duration = hevcasm_get_timestamp() - start;

		if (warmup == 0)
		{
			if (8 * duration * count < 7 * 4 * sum)
			{
				sum = 0;
				count = 0;
				warmup = 100;
			}
			else 	if (7 * duration * count <= 8 * 4 * sum)
			{
				count += 4;
				sum += duration;
			}
			else
			{
				warmup = 100;
			}
		}
		else
		{
			--warmup;
		}
	}
	
	{
		const int average = (int)((sum + count / 2) / count);

		printf(" %s:", hevcasm_instruction_set_idx_as_text(i));
		printf("%d", average);
		if (*first_result != 0.0)
		{
			printf("(x%.2f)", *first_result / average);
		}
		else
		{
			*first_result = average;
		}
	}
}

#endif
