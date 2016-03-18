// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.


/* Functions for decoding HEVC residual: inverse transform and add the result to predictor */


#ifndef INCLUDED_hadamard_h
#define INCLUDED_hadamard_h

#include "hevcasm.h"


template <typename Sample>
using hevcasm_hadamard_satd = int(Sample const *srcA, intptr_t stride_srcA, Sample const *srcB, intptr_t stride_srcB);


template <typename Sample>
struct hevcasm_table_hadamard_satd
{
	hevcasm_hadamard_satd<Sample> *satd[3];
};


template <typename Sample>
hevcasm_hadamard_satd<Sample>** hevcasm_get_hadamard_satd(hevcasm_table_hadamard_satd<Sample> *table, int log2TrafoSize)
{
	return &table->satd[log2TrafoSize - 1];
}


template <typename Sample>
void hevcasm_populate_hadamard_satd(hevcasm_table_hadamard_satd<Sample> *table, hevcasm_code code);


void hevcasm_test_hadamard_satd(int *error_count, hevcasm_instruction_set mask);


#endif
