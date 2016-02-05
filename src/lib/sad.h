// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.


#ifndef INCLUDED_sad_h
#define INCLUDED_sad_h

#include "hevcasm.h"


#ifdef __cplusplus
extern "C"
{
#endif

#define X_HEVC_PU_SIZES \
	X(64, 64) \
	X(64, 48) \
	X(64, 32) \
	X(64, 16) \
	X(48, 64) \
	X(32, 64) \
	X(32, 32) \
	X(32, 24) \
	X(32, 16) \
	X(32, 8) \
	X(24, 32) \
	X(16, 64) \
	X(16, 32) \
	X(16, 16) \
	X(16, 12) \
	X(16, 8) \
	X(16, 4) \
	X(12, 16) \
	X(8, 32) \
	X(8, 16) \
	X(8, 8) \
	X(8, 4) \
	X(4, 8) \


//typedef std::function<int(const uint8_t * /*src*/, ptrdiff_t /*stride_src*/, const uint8_t * /*ref*/, ptrdiff_t /*stride_ref*/, uint32_t /*rect*/)> hevcasm_sad;

/* Rectangular SAD (Sum of Absolute Differences) with single reference */
typedef int hevcasm_sad(const uint8_t *src, ptrdiff_t stride_src, const uint8_t *ref, ptrdiff_t stride_ref, uint32_t rect);





typedef struct
{
#define X(w, h) \
	hevcasm_sad *sad ## w ## x ## h;
	
	X_HEVC_PU_SIZES
#undef X

	hevcasm_sad *sadGeneric;
}
hevcasm_table_sad;

static hevcasm_sad** hevcasm_get_sad(hevcasm_table_sad *table, int width, int height)
{
	switch (HEVCASM_RECT(width, height))
	{
#define X(w, h) \
	case HEVCASM_RECT(w, h): return &table->sad ## w ## x ## h;

	X_HEVC_PU_SIZES
#undef X
	default:;
	}
	return &table->sadGeneric;
}

void HEVCASM_API hevcasm_populate_sad(hevcasm_table_sad *table, hevcasm_code code);

hevcasm_test_function hevcasm_test_sad;


/* Rectangular SAD (Sum of Absolute Differences) with multiple references */
typedef void hevcasm_sad_multiref(const uint8_t *src, ptrdiff_t stride_src, const uint8_t *ref[], ptrdiff_t stride_ref, int sad[], uint32_t rect);

typedef struct
{
	hevcasm_sad_multiref *lookup[16][16];
	hevcasm_sad_multiref *sadGeneric_4;
}
hevcasm_table_sad_multiref;

static hevcasm_sad_multiref** hevcasm_get_sad_multiref(hevcasm_table_sad_multiref *table, int ways, int width, int height)
{
	if (ways != 4) return 0;
	return &table->lookup[(width>>2)-1][(height>>2)-1];
}

void HEVCASM_API hevcasm_populate_sad_multiref(hevcasm_table_sad_multiref *table, hevcasm_code code);

hevcasm_test_function hevcasm_test_sad_multiref;


#ifdef __cplusplus
}
#endif


#endif
