// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.


#include "diff.h"
#include "hevcasm_test.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>


static int hevcasm_ssd_linear_c_ref(const uint8_t *p1, const uint8_t *p2, int n)
{
	int sum = 0;
	for (int i = 0; i<n; ++i)
	{
		const int diff = p1[i] - p2[i];
		sum += diff * diff;
	}
	return sum;
}


hevcasm_ssd_linear * HEVCASM_API hevcasm_get_ssd_linear(int size, hevcasm_instruction_set mask)
{
	hevcasm_ssd_linear *f = 0;
	
	if (mask & (HEVCASM_C_REF | HEVCASM_C_OPT)) f = hevcasm_ssd_linear_c_ref;

	// review:
	//if (mask & HEVCASM_AVX) f = hevcasm_ssd_linear_avx;

	return f;
}


#define BLOCK_SIZE 0x200


typedef struct
{
	HEVCASM_ALIGN(32, uint8_t, data[2][BLOCK_SIZE]);
	hevcasm_ssd_linear *f;
	int ssd;
}
bound_ssd_linear;


int get_ssd_linear(void *p, hevcasm_instruction_set mask)
{
	bound_ssd_linear *s = (bound_ssd_linear *)p;

	s->f = hevcasm_get_ssd_linear(BLOCK_SIZE, mask);

	if (mask == HEVCASM_C_REF) printf("\t%d:", BLOCK_SIZE);

	return !!s->f;
}


void invoke_ssd_linear(void *p, int n)
{
	bound_ssd_linear *s = (bound_ssd_linear *)p;
	while (n--)
	{
		s->ssd = s->f(s->data[0], s->data[1], BLOCK_SIZE);
	}
}


int mismatch_ssd_linear(void *boundRef, void *boundTest)
{
	bound_ssd_linear *ref = (bound_ssd_linear *)boundRef;
	bound_ssd_linear *test = (bound_ssd_linear *)boundTest;

	return  ref->ssd != test->ssd;
}


void HEVCASM_API hevcasm_test_ssd_linear(int *error_count, hevcasm_instruction_set mask)
{
	printf("\nhevcasm_ssd_linear - Linear Sum of Square Differences\n");

	bound_ssd_linear b[2];

	for (int n = 0; n < 2; ++n)
	{
		for (int x = 0; x < BLOCK_SIZE; ++x)
		{
			b[0].data[n][x] = rand();
		}
	}

	b[1] = b[0];

	*error_count += hevcasm_test(&b[0], &b[1], get_ssd_linear, invoke_ssd_linear, mismatch_ssd_linear, mask, 100000);
}
