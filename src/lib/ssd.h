// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.



/* Sum of square differences */


#ifndef INCLUDED_ssd_h
#define INCLUDED_ssd_h

#include "hevcasm.h"



#ifdef __cplusplus
extern "C"
{
#endif



typedef int hevcasm_ssd(const uint8_t *srcA, ptrdiff_t stride_srcA, const uint8_t *srcB, ptrdiff_t stride_srcB, int w, int h);
typedef int hevcasm_ssd16(const uint16_t *srcA, ptrdiff_t stride_srcA, const uint16_t *srcB, ptrdiff_t stride_srcB, int w, int h);

typedef struct
{
	hevcasm_ssd *satd[5];
}
hevcasm_table_ssd;

typedef struct
{
	hevcasm_ssd16 *satd[5];
}
hevcasm_table_ssd16;

static hevcasm_ssd** hevcasm_get_ssd(hevcasm_table_ssd *table, int log2TrafoSize)
{
	return &table->satd[log2TrafoSize - 2];
}

static hevcasm_ssd16** hevcasm_get_ssd16(hevcasm_table_ssd16 *table, int log2TrafoSize)
{
	return &table->satd[log2TrafoSize - 2];
}

void HEVCASM_API hevcasm_populate_ssd(hevcasm_table_ssd *table, hevcasm_code code);

void HEVCASM_API hevcasm_populate_ssd16(hevcasm_table_ssd16 *table, hevcasm_code code);

void HEVCASM_API hevcasm_test_ssd(int *error_count, hevcasm_instruction_set mask);


#ifdef __cplusplus
}

template <typename Sample>
struct HevcasmSsd;

template <>
struct HevcasmSsd<uint8_t>
{
	typedef hevcasm_ssd Function;
	typedef hevcasm_table_ssd Table;
	static Function **get(Table *table, int log2TrafoSize)
	{
		return hevcasm_get_ssd(table, log2TrafoSize);
	}
};

template <>
struct HevcasmSsd<uint16_t>
{
	typedef hevcasm_ssd16 Function;
	typedef hevcasm_table_ssd16 Table;
	static Function **get(Table *table, int log2TrafoSize)
	{
		return hevcasm_get_ssd16(table, log2TrafoSize);
	}
};

#endif

#endif
