// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.



/* Sum of square differences */


#ifndef INCLUDED_ssd_h
#define INCLUDED_ssd_h

#include "hevcasm.h"


template <typename Sample>
using hevcasm_ssd = uint32_t(Sample const *srcA, intptr_t stride_srcA, Sample const *srcB, intptr_t stride_srcB, int w, int h);


template <typename Sample>
struct hevcasm_table_ssd
{
	hevcasm_ssd<Sample> *ssd[5];
};


template <typename Sample>
static hevcasm_ssd<Sample>** hevcasm_get_ssd(hevcasm_table_ssd<Sample> *table, int log2TrafoSize)
{

	return &table->ssd[log2TrafoSize - 2];
}


template <typename Sample>
void hevcasm_populate_ssd(hevcasm_table_ssd<Sample> *table, hevcasm_code code);


void hevcasm_test_ssd(int *error_count, hevcasm_instruction_set mask);


#endif
