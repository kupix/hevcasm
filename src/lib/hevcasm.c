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


#include "pred_inter.h"
#include "residual_decode.h"
#include "sad.h"
#include "hevcasm.h"


#ifdef WIN32
#include <Windows.h>
#endif

#include <stdint.h>


#ifdef _MSC_VER
#include <intrin.h>
#endif


#ifdef __GNUC__

static void __cpuidex(int cpuInfo[4], int function_id, int subfunction_id)
{
	__asm__ __volatile__ ( "cpuid" 
		:
		"=a" ((cpuInfo)[0]),
		"=b" ((cpuInfo)[1]),
		"=c" ((cpuInfo)[2]),
		"=d" ((cpuInfo)[3]) 
		: 
		"0" (function_id),
		"2" (subfunction_id) );
}


static uint64_t _xgetbv(uint32_t index)
{
	uint32_t eax, edx;
	__asm__ __volatile__("xgetbv"
		:
		"=a" (eax),
		"=d" (edx)
		:
		"c" (index) );
	return ((uint64_t)edx << 32) | eax;
}


#endif


hevcasm_instruction_set hevcasm_instruction_set_support()
{
	hevcasm_instruction_set mask = HEVCASM_C_REF | HEVCASM_C_OPT;

	int cpuInfo[4]; // eax ... edx
	__cpuidex(cpuInfo, 0, 0);

	const int max_standard_level = cpuInfo[0];

	if (max_standard_level == 0) return mask;

	__cpuidex(cpuInfo, 1, 0);

	if (cpuInfo[3] & 0x04000000) mask |= HEVCASM_SSE2;
	if (cpuInfo[2] & 0x00000001) mask |= HEVCASM_SSE3;
	if (cpuInfo[2] & 0x00000200) mask |= HEVCASM_SSSE3;
	if (cpuInfo[2] & 0x00080000) mask |= HEVCASM_SSE41;
	if (cpuInfo[2] & 0x00100000) mask |= HEVCASM_SSE42;
	
	if ((cpuInfo[2] & 0x10000000) && (cpuInfo[2] & 0x08000000))
	{
		uint64_t xcr0 = _xgetbv(0);

		if ((xcr0 & 0x2) && (xcr0 & 0x4))
		{
			mask |= HEVCASM_AVX;
			if (max_standard_level >= 7)
			{
				__cpuidex(cpuInfo, 7, 0);
				
				if (cpuInfo[1] & 0x00000020) mask |= HEVCASM_AVX2;
			}
		}
	}

	return mask;
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
	error_count += hevcasm_test_pred_uni(mask);
	error_count += hevcasm_test_pred_bi(mask);

	return error_count;
}

