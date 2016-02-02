// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.


#include "hevcasm_test.h"



int hevcasm_count_average_cycles(
	void *boundRef, void *boundTest,
	hevcasm_bound_invoke *f, 
	hevcasm_bound_mismatch *m, 
	double *first_result, 
	hevcasm_instruction_set set, 
	int iterations)
{
	hevcasm_timestamp sum = 0;
	int warmup = 100;
	int count = 0;
	while (count < iterations)
	{
		const hevcasm_timestamp start = hevcasm_get_timestamp();
		f(boundTest, 4);
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

		printf(" %s:", hevcasm_instruction_set_as_text(set));
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

	if (boundRef)
	{
		f(boundRef, 1);
		if (m(boundRef, boundTest))
		{
			printf("-MISMATCH");
			return 1;
		}
	}

	return 0;
}


int hevcasm_test(
	void *ref, 
	void *test, 
	hevcasm_bound_get *get, 
	hevcasm_bound_invoke *invoke, 
	hevcasm_bound_mismatch *mismatch, 
	hevcasm_instruction_set mask, 
	int iterations)
{
	int error_count = 0;

	if (get(ref, HEVCASM_C_REF))
	{
		invoke(ref, 1);

		double first_result = 0.0;
		for (hevcasm_instruction_set set = HEVCASM_C_OPT; set; set <<= 1)
		{
			if (get(test, set & mask))
			{
				error_count += hevcasm_count_average_cycles(ref, test, invoke, mismatch, &first_result, set, iterations);
			}
		}
		printf("\n");
	}

	return error_count;
}
