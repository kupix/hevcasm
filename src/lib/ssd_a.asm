; The copyright in this software is being made available under the BSD
; License, included below. This software may be subject to other third party
; and contributor rights, including patent rights, and no such rights are
; granted under this license.
; 
; 
; Copyright(c) 2011 - 2014, Parabola Research Limited
; All rights reserved.
; 
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met :
; 
; * Redistributions of source code must retain the above copyright notice,
; this list of conditions and the following disclaimer.
; * Redistributions in binary form must reproduce the above copyright notice,
; this list of conditions and the following disclaimer in the documentation
; and / or other materials provided with the distribution.
; * Neither the name of the copyright holder nor the names of its contributors may
; be used to endorse or promote products derived from this software without
; specific prior written permission.
; 
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
; ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
; BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
; CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
; SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
; CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE)
; ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
; THE POSSIBILITY OF SUCH DAMAGE.


%define private_prefix hevcasm
%include "x86inc.asm"

%define ORDER(a, b, c, d) ((a << 6) | (b << 4) | (c << 2) | d)


; Sum of square differences
; extern "C" int ssd_16x16(const uint8_t *srcA, ptrdiff_t stride_srcA, const uint8_t *srcB, ptrdiff_t stride_srcB, int w, int h)

INIT_XMM avx
cglobal ssd_16x16, 6, 6, 6
	pxor m0, m0
	pxor m5, m5
.loop:
		mova m1, [r0]
		mova m2, [r2]
		punpckhbw m3, m1, m5
		punpckhbw m4, m2, m5
		punpcklbw m1, m5
		punpcklbw m2, m5
		psubw m1, m2
		psubw m3, m4
		pmaddwd m1, m1
		pmaddwd m3, m3
		paddd m0, m1
		paddd m0, m3
		lea r0, [r0 + r1]
		lea r2, [r2 + r3]
		dec r5d
		jg .loop
	pshufd m1, m0, ORDER(3, 2, 3, 2)
	paddd m0, m1 ; Review: phaddd might be better here
	pshufd m1, m0, ORDER(3, 2, 0, 1)
	paddd m0, m1
    movd   eax, m0
	RET


; Sum of square differences
; extern "C" int ssd_32x32(const uint8_t *srcA, ptrdiff_t stride_srcA, const uint8_t *srcB, ptrdiff_t stride_srcB, int w, int h)

INIT_XMM avx
cglobal ssd_32x32, 6, 6, 6
	pxor m0, m0
	pxor m5, m5
.loop:
		mova m1, [r0]
		mova m2, [r2]
		punpckhbw m3, m1, m5
		punpckhbw m4, m2, m5
		punpcklbw m1, m5
		punpcklbw m2, m5
		psubw m1, m2
		psubw m3, m4
		pmaddwd m1, m1
		pmaddwd m3, m3
		paddd m0, m1
		paddd m0, m3
		mova m1, [r0+16]
		mova m2, [r2+16]
		punpckhbw m3, m1, m5
		punpckhbw m4, m2, m5
		punpcklbw m1, m5
		punpcklbw m2, m5
		psubw m1, m2
		psubw m3, m4
		pmaddwd m1, m1
		pmaddwd m3, m3
		paddd m0, m1
		paddd m0, m3
		lea r0, [r0 + r1]
		lea r2, [r2 + r3]
		dec r5d
		jg .loop
	pshufd m1, m0, ORDER(3, 2, 3, 2)
	paddd m0, m1 ; Review: phaddd might be better here
	pshufd m1, m0, ORDER(3, 2, 0, 1)
	paddd m0, m1
    movd   eax, m0
	RET


; Sum of square differences
; extern "C" int ssd_64x64(const uint8_t *srcA, ptrdiff_t stride_srcA, const uint8_t *srcB, ptrdiff_t stride_srcB, int w, int h)

INIT_XMM avx
cglobal ssd_64x64, 6, 6, 6
	pxor m0, m0
	pxor m5, m5
.loop:
		mova m1, [r0]
		mova m2, [r2]
		punpckhbw m3, m1, m5
		punpckhbw m4, m2, m5
		punpcklbw m1, m5
		punpcklbw m2, m5
		psubw m1, m2
		psubw m3, m4
		pmaddwd m1, m1
		pmaddwd m3, m3
		paddd m0, m1
		paddd m0, m3
		mova m1, [r0+16]
		mova m2, [r2+16]
		punpckhbw m3, m1, m5
		punpckhbw m4, m2, m5
		punpcklbw m1, m5
		punpcklbw m2, m5
		psubw m1, m2
		psubw m3, m4
		pmaddwd m1, m1
		pmaddwd m3, m3
		paddd m0, m1
		paddd m0, m3
		mova m1, [r0+32]
		mova m2, [r2+32]
		punpckhbw m3, m1, m5
		punpckhbw m4, m2, m5
		punpcklbw m1, m5
		punpcklbw m2, m5
		psubw m1, m2
		psubw m3, m4
		pmaddwd m1, m1
		pmaddwd m3, m3
		paddd m0, m1
		paddd m0, m3
		mova m1, [r0+48]
		mova m2, [r2+48]
		punpckhbw m3, m1, m5
		punpckhbw m4, m2, m5
		punpcklbw m1, m5
		punpcklbw m2, m5
		psubw m1, m2
		psubw m3, m4
		pmaddwd m1, m1
		pmaddwd m3, m3
		paddd m0, m1
		paddd m0, m3
		lea r0, [r0 + r1]
		lea r2, [r2 + r3]
		dec r5d
		jg .loop
	pshufd m1, m0, ORDER(3, 2, 3, 2)
	paddd m0, m1 ; Review: phaddd might be better here
	pshufd m1, m0, ORDER(3, 2, 0, 1)
	paddd m0, m1
    movd   eax, m0
	RET
