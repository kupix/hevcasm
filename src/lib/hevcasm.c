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


#include "residual_decode.h"
#include "sad.h"
#include "hevcasm.h"


#ifdef WIN32
#include <Windows.h>
#endif


/* declaration for assembly function in instrset.asm */
uint16_t hevcasm_get_instruction_set();


hevcasm_instruction_set hevcasm_instruction_set_support()
{
	long long int set = hevcasm_get_instruction_set();
	return (2 << set) - 1;
}


void hevcasm_print_instruction_set_support(FILE *f, hevcasm_instruction_set mask)
{
	f = stdout;
	fprintf(f, "HEVCasm processor instruction set support:\n");
#define X(value, name, description) fprintf(f, "[%c] " #name " (" description ")\n", ((1 << value) & mask) ? 'x' : ' ');
	HEVCASM_INSTRUCTION_SET_XMACRO
#undef X
	fprintf(f, "\n");
}


int hevcasm_main(int argc, const char *argv[])
{
	hevcasm_instruction_set mask = hevcasm_instruction_set_support();

	printf("HEVCasm self test\n");
	printf("\n");

#ifdef WIN32
	if (!SetProcessAffinityMask(GetCurrentProcess(), 1))
	{
		printf("** SetProcessAffinityMask() failed **\n\n");
	}
#endif

	hevcasm_print_instruction_set_support(stdout, mask);
	printf("\n");

	int error_count = 0;

	error_count += hevcasm_test_inverse_transform_add(mask);
	error_count += hevcasm_test_sad(mask);
	error_count += hevcasm_test_sad_multiref(mask);

	return error_count;
}

