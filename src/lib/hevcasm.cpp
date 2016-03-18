// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.

#include "pred_inter.h"
#include "pred_intra.h"
#include "residual_decode.h"
#include "sad.h"
#include "ssd.h"
#include "diff.h"
#include "quantize.h"
#include "hadamard.h"
#include "hevcasm.h"
#include "Jit.h"
#ifdef WIN32
#include <Windows.h>
#endif
#include <stdint.h>
#include <type_traits>
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


static int bit_is_set(int value, int n)
{
	return value & (1 << n);
}


hevcasm_instruction_set hevcasm_instruction_set_support()
{
	std::underlying_type<hevcasm_instruction_set>::type mask = HEVCASM_C_REF | HEVCASM_C_OPT;

	enum { eax = 0, ebx = 1, ecx = 2, edx = 3 };

	int cpuInfo[4]; // eax ... edx

	__cpuidex(cpuInfo, 0, 0);

	const int max_standard_level = cpuInfo[0];

	if (max_standard_level == 0) return (hevcasm_instruction_set)mask;

	__cpuidex(cpuInfo, 1, 0);

	if (bit_is_set(cpuInfo[edx], 26)) mask |= HEVCASM_SSE2;
	if (bit_is_set(cpuInfo[ecx], 1)) mask |= HEVCASM_SSE3;
	if (bit_is_set(cpuInfo[ecx], 9)) mask |= HEVCASM_SSSE3;
	if (bit_is_set(cpuInfo[ecx], 19)) mask |= HEVCASM_SSE41;
	if (bit_is_set(cpuInfo[ecx], 20)) mask |= HEVCASM_SSE42;
	
	if (bit_is_set(cpuInfo[ecx], 28) && bit_is_set(cpuInfo[ecx], 27))
	{
		uint64_t xcr0 = _xgetbv(0);

		if ((xcr0 & 0x2) && (xcr0 & 0x4))
		{
			mask |= HEVCASM_AVX;
			if (max_standard_level >= 7)
			{
				__cpuidex(cpuInfo, 7, 0);
				
				if (bit_is_set(cpuInfo[ebx], 5)) mask |= HEVCASM_AVX2;
			}
		}
	}

	return (hevcasm_instruction_set)mask;
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


hevcasm_code hevcasm_new_code(hevcasm_instruction_set mask, int size)
{
	hevcasm_code code;
	code.implementation = new Jit::Buffer(size, mask);
	return code;
}


void hevcasm_delete_code(hevcasm_code code)
{
	delete (Jit::Buffer *)code.implementation;
}


int hevcasm_main(int argc, const char *argv[])
{
	hevcasm_instruction_set mask = hevcasm_instruction_set_support();

	printf("HEVCasm self test\n\n");

#ifdef WIN32
	if (!SetProcessAffinityMask(GetCurrentProcess(), 1))
	{
		printf("** SetProcessAffinityMask() failed **\n\n");
	}
#endif

	hevcasm_print_instruction_set_support(stdout, mask);

	int error_count = 0;

	hevcasm_test_sad_multiref(&error_count, mask);
	hevcasm_test_sad(&error_count, mask);
	hevcasm_test_ssd(&error_count, mask);
	hevcasm_test_pred_intra(&error_count, mask);
	hevcasm_test_hadamard_satd(&error_count, mask);
	hevcasm_test_quantize_inverse(&error_count, mask);
	hevcasm_test_quantize(&error_count, mask);
	hevcasm_test_quantize_reconstruct(&error_count, mask);
	hevcasm_test_pred_uni(&error_count, mask);
	hevcasm_test_pred_bi(&error_count, mask);
	hevcasm_test_inverse_transform_add8(&error_count, mask);
	hevcasm_test_transform(&error_count, mask);

	printf("\n");
	printf("HEVCasm self test: %d errors\n", error_count);

	return error_count;
}

