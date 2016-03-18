// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.


#ifndef INCLUDED_diff_h
#define INCLUDED_diff_h

#include "hevcasm.h"


#ifdef __cplusplus
extern "C"
{
#endif


/* Linear SSD (Sum of Squared Differences) */
typedef int hevcasm_ssd_linear(const uint8_t *src0, const uint8_t *src1, int size);

hevcasm_ssd_linear* hevcasm_get_ssd_linear(int size, hevcasm_code code);

hevcasm_test_function hevcasm_test_ssd_linear;


#ifdef __cplusplus
}
#endif


#endif
