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

/* Declaration of assembly functions in quantize_a.asm */

#ifndef INCLUDED_pred_inter_a_h
#define INCLUDED_pred_inter_a_h

#include "pred_inter.h"

#include <stdlib.h>
#include <stdint.h>

typedef void hevcasm_pred_uni_filter_8to16(int16_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
typedef void hevcasm_pred_uni_filter_16to8(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
typedef void hevcasm_pred_uni_filter_16to16(int16_t *dst, ptrdiff_t stride_dst, const int16_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
typedef void hevcasm_pred_bi_v_filter_16to16(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *refAtop, const int16_t *refBtop, ptrdiff_t stride_ref, int nPbW, int nPbH, int yFracA, int yFracB);
typedef void hevcasm_pred_bi_8to8_copy(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref0, const uint8_t *ref1, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac0, int yFrac0, int xFrac1, int yFrac1);

hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_copy_8to8_16xh_sse2;
hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_copy_8to8_32xh_sse2;
hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_copy_8to8_48xh_sse2;
hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_copy_8to8_64xh_sse2;

hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_8tap_8to8_h_16xh_sse4;
hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_8tap_8to8_h_32xh_sse4;
hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_8tap_8to8_h_48xh_sse4;
hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_8tap_8to8_h_64xh_sse4;
hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_4tap_8to8_h_16xh_sse4;
hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_4tap_8to8_h_32xh_sse4;

hevcasm_pred_uni_filter_8to16 hevcasm_pred_uni_8tap_8to16_h_16xh_sse4;
hevcasm_pred_uni_filter_8to16 hevcasm_pred_uni_8tap_8to16_h_32xh_sse4;
hevcasm_pred_uni_filter_8to16 hevcasm_pred_uni_8tap_8to16_h_48xh_sse4;
hevcasm_pred_uni_filter_8to16 hevcasm_pred_uni_8tap_8to16_h_64xh_sse4;
hevcasm_pred_uni_filter_8to16 hevcasm_pred_uni_4tap_8to16_h_16xh_sse4;
hevcasm_pred_uni_filter_8to16 hevcasm_pred_uni_4tap_8to16_h_32xh_sse4;

hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_8tap_8to8_v_8xh_sse4;
hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_8tap_8to8_v_16xh_sse4;
hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_8tap_8to8_v_24xh_sse4;
hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_8tap_8to8_v_32xh_sse4;
hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_8tap_8to8_v_48xh_sse4;
hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_8tap_8to8_v_64xh_sse4;

hevcasm_pred_uni_filter_16to8 hevcasm_pred_uni_8tap_16to8_v_16xh_sse4;
hevcasm_pred_uni_filter_16to8 hevcasm_pred_uni_8tap_16to8_v_32xh_sse4;
hevcasm_pred_uni_filter_16to8 hevcasm_pred_uni_8tap_16to8_v_48xh_sse4;
hevcasm_pred_uni_filter_16to8 hevcasm_pred_uni_8tap_16to8_v_64xh_sse4;

hevcasm_pred_bi_v_filter_16to16 hevcasm_pred_bi_v_8tap_16to16_16xh_sse4;
hevcasm_pred_bi_v_filter_16to16 hevcasm_pred_bi_v_8tap_16to16_32xh_sse4;
hevcasm_pred_bi_v_filter_16to16 hevcasm_pred_bi_v_8tap_16to16_48xh_sse4;
hevcasm_pred_bi_v_filter_16to16 hevcasm_pred_bi_v_8tap_16to16_64xh_sse4;

hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_4tap_8to8_v_8xh_sse4;
hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_4tap_8to8_v_16xh_sse4;
hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_4tap_8to8_v_24xh_sse4;
hevcasm_pred_uni_filter_8to8 hevcasm_pred_uni_4tap_8to8_v_32xh_sse4;

hevcasm_pred_uni_filter_16to8 hevcasm_pred_uni_4tap_16to8_v_16xh_sse4;
hevcasm_pred_uni_filter_16to8 hevcasm_pred_uni_4tap_16to8_v_32xh_sse4;

hevcasm_pred_bi_v_filter_16to16 hevcasm_pred_bi_v_4tap_16to16_8xh_sse4;
hevcasm_pred_bi_v_filter_16to16 hevcasm_pred_bi_v_4tap_16to16_16xh_sse4;
hevcasm_pred_bi_v_filter_16to16 hevcasm_pred_bi_v_4tap_16to16_32xh_sse4;

hevcasm_pred_bi_8to8_copy hevcasm_pred_bi_8to8_copy_16xh_sse2;
hevcasm_pred_bi_8to8_copy hevcasm_pred_bi_8to8_copy_32xh_sse2;
hevcasm_pred_bi_8to8_copy hevcasm_pred_bi_8to8_copy_48xh_sse2;
hevcasm_pred_bi_8to8_copy hevcasm_pred_bi_8to8_copy_64xh_sse2;

#endif