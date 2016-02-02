// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.


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
