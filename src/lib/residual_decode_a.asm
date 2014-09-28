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


SECTION_RODATA 32

cosine_inverse_4:
	times 4 dw 64, 64
	times 4 dw 64, -64
	times 4 dw 83, 36
	times 4 dw 36, -83

cosine_inverse_4_h:
	dw 64, -64, 36, -83, 64, 64, 83, 36

cosine_inverse_8:
	times 4 dw 89, 75
	times 4 dw 50, 18
	times 4 dw 75, -18
	times 4 dw -89, -50
	times 4 dw 50, -89
	times 4 dw 18, 75
	times 4 dw 18, -50
	times 4 dw 75, -89

cosine_inverse_8_h:
	dw 89, 75, 50, 18
	dw 75, -18, -89, -50
	dw 50, -89, 18, 75
	dw 18, -50, 75, -89

dd_0040:
	times 4 dd 0x0040

dd_0800:
	times 4 dd 0x0800

shuffle_018945cd018945cd:
	db 0, 1, 8, 9, 4, 5, 12, 13, 0, 1, 8, 9, 4, 5, 12, 13

shuffle_2367abef2367abef:
	db 2, 3, 6, 7, 10, 11, 14, 15, 2, 3, 6, 7, 10, 11, 14, 15

SECTION .text


%if ARCH_X86_64

; this function potentially be combined with the horizontal feature with no
; need for temporary memory buffer in between.

; void hevcasm_partial_butterfly_inverse_8v_ssse3(int16_t *dst, const int16_t *src, int shift);
INIT_XMM ssse3
cglobal partial_butterfly_inverse_8v, 3, 5, 16
	
	lea r3, [cosine_inverse_4]

	mova m8, [r1 + 0 * 2 * 8] 
	; m8 = src(7, 0), src(6, 0), src(5, 0), src(4, 0), src(3, 0), src(2, 0), src(1, 0), src(0, 0) 
	
	mova m1, [r1 + 4 * 2 * 8]
	; m1 = src(7, 4), src(6, 4), src(5, 4), src(4, 4), src(3, 4), src(2, 4), src(1, 4), src(0, 4) 

	punpcklwd m0, m8, m1
	; m0 = src(3, 4), src(3, 0), src(2, 4), src(2, 0), src(1, 4), src(1, 0), src(0, 4), src(0, 0) 

	punpckhwd m8, m1
	; m8 = src(7, 4), src(7, 0), src(6, 4), src(6, 0), src(5, 4), src(5, 0), src(4, 4), src(4, 0) 

	pmaddwd m1, m0, [r3]
	; m1 = EE[0] : 3, 2, 1, 0 

	pmaddwd m9, m8, [r3]
	; m9 = EE[0] : 7, 6, 5, 4 

	pmaddwd m0, [r3+16]
	; m0 = EE[1] : 3, 2, 1, 0 

	pmaddwd m8, [r3+16]
	; m8 = EE[1] : 7, 6, 5, 4  

	mova m10, [r1 + 2 * 2 * 8] 
	; m10 = src(7, 2), src(6, 2), src(5, 2), src(4, 2), src(3, 2), src(2, 2), src(1, 2), src(0, 2) 
	
	mova m3, [r1 + 6 * 2 * 8]
	; m3 = src(7, 6), src(6, 6), src(5, 6), src(4, 6), src(3, 6), src(2, 6), src(1, 6), src(0, 6) 

	punpcklwd m2, m10, m3
	; m2 = src(3, 6), src(3, 2), src(2, 6), src(2, 2), src(1, 6), src(1, 2), src(0, 6), src(0, 2) 

	punpckhwd m10, m3
	; m10 = src(7, 6), src(7, 2), src(6, 6), src(6, 2), src(5, 6), src(5, 2), src(4, 6), src(4, 2) 

	pmaddwd m3, m2, [r3+32]
	; m3 = EO[0] : 3, 2, 1, 0 

	pmaddwd m11, m10, [r3+32]
	; m11 = EO[0] : 7, 6, 5, 4 

	pmaddwd m2, [r3+48]
	; m2 = EO[1] : 3, 2, 1, 0 

	pmaddwd m10, [r3+48]
	; m10 = EO[1] : 7, 6, 5, 4 

	paddd m4, m1, m3
	; m4 = E[0] : 3, 2, 1, 0 

	paddd m12, m9, m11
	; m12 = E[0] : 7, 6, 5, 4 

	psubd m1, m3
	; m1 = E[3] : 3, 2, 1, 0 

	psubd m9, m11
	; m9 = E[3] : 7, 6, 5, 4 

	paddd m3, m0, m2
	; m3 = E[1] : 3, 2, 1, 0 

	paddd m13, m8, m10
	; m13 = E[1] : 7, 6, 5, 4 

	psubd m0, m2
	; m0 = E[2] : 3, 2, 1, 0 

	psubd m8, m10
	; m8 = E[2] : 7, 6, 5, 4  

	; m4, m3, m0, m1 contain E[] for words 3, 2, 1, 0
	; m12, m13, m8, m9 contain E[] for words 7, 6, 5, 4
	; could add rounding here to E[] instead of later

	mova m5, [r1 + 1 * 2 * 8]
	; m5 = src(7, 1), src(6, 1), src(5, 1), src(4, 1), src(3, 1), src(2, 1), src(1, 1), src(0, 1) 

	mova m2, [r1 + 3 * 2 * 8]
	; m2 = src(7, 3), src(6, 3), src(5, 3), src(4, 3), src(3, 3), src(2, 3), src(1, 3), src(0, 3) 

	punpckhwd m10, m5, m2
	; m10 = src(7, 3), src(7, 1), src(6, 3), src(6, 1), src(5, 3), src(5, 1), src(4, 3), src(4, 1) 

	punpcklwd m5, m2
	; m5 = src(3, 3), src(3, 1), src(2, 3), src(2, 1), src(1, 3), src(1, 1), src(0, 3), src(0, 1) 

	mova m6, [r1 + 5 * 2 * 8]
	; m6 = src(7, 5), src(6, 5), src(5, 5), src(4, 5), src(3, 5), src(2, 5), src(1, 5), src(0, 5) 

	mova m7, [r1 + 7 * 2 * 8]
	; m7 = src(7, 7), src(6, 7), src(5, 7), src(4, 7), src(3, 7), src(2, 7), src(1, 7), src(0, 7) 

	punpckhwd m11, m6, m7
	; m11 = src(7, 7), src(7, 5), src(6, 7), src(6, 5), src(5, 7), src(5, 5), src(4, 7), src(4, 5) 

	punpcklwd m6, m7
	; m6 = src(3, 7), src(3, 5), src(2, 7), src(2, 5), src(1, 7), src(1, 5), src(0, 7), src(0, 5) 

	; m4, m3, m0, m1 contain E[] for words 3, 2, 1, 0
	; m12, m13, m8, m9 contain E[] for words 7, 6, 5, 4
	; m5 contains source rows 1 and 3 for words 3, 2, 1, 0
	; m6 contains source rows 5 and 7 for words 3, 2, 1, 0
	; m10 contains source rows 1 and 3 for words 7, 6, 5, 4
	; m11 contains source rows 5 and 7 for words 7, 6, 5, 4

	lea r3, [cosine_inverse_8]
	lea r4, [dd_0040]

%macro OUTPUTROWS 3
	pmaddwd m2, m5, [r3+32*%1+0]
	pmaddwd m14, m10, [r3+32*%1+0]
	pmaddwd m7, m6, [r3+32*%1+16]
	pmaddwd m15, m11, [r3+32*%1+16]
	paddd m2, m7
	paddd m14, m15
	; m2 = O[%1] : 3, 2, 1, 0 
	; m14 = O[%1] : 7, 6, 5, 4

	movu m7, [r4]
	movu m15, [r4]
	paddd m7, m2 
	paddd m15, m14 
	paddd m7, %2
	paddd m15, %3
	psrad m7, 7
	psrad m15, 7
	; m7 = (E[%1] + O[%1] + add) >> shift : 3, 2, 1, 0
	; m15 = (E[%1] + O[%1] + add) >> shift : 7, 6, 5, 4

	packssdw m7, m15
	mova [r0+16*%1], m7

	paddd %2, [r4] 
	paddd %3, [r4] 
	psubd %2, m2
	psubd %3, m14
	psrad %2, 7
	psrad %3, 7
	; %2 = (E[%1] - O[%1] + add) >> shift : 3, 2, 1, 0
	; %3 = (E[%1] - O[%1] + add) >> shift : 7, 6, 5, 4

	packssdw %2, %3
	mova [r0+16*(7-%1)], %2
%endmacro

	OUTPUTROWS 0, m4, m12
	OUTPUTROWS 1, m3, m13
	OUTPUTROWS 2, m0, m8
	OUTPUTROWS 3, m1, m9

	RET
	 

; void hevcasm_partial_butterfly_inverse_8h_ssse3(int16_t *dst, const int16_t *src, int shift);
INIT_XMM ssse3
cglobal partial_butterfly_inverse_8h, 3, 5, 16
	mov r4d, 8
.loop
		mova m2, [r1]
		pshufb m0, m2, [shuffle_018945cd018945cd]
		; m0 = src[6], src[2], src[4], src[0], src[6], src[2], src[4], src[0]
		
		pmaddwd m0, [cosine_inverse_4_h]
		; m0 = EO[0], EE[0], EO[1], EE[1] 

		phsubd m1, m0
		; m1 = E[3], E[2], ?, ?

		pshufd m0, m0, ORDER(1, 0, 3, 2)
		; m0 = EO[1], EE[1], EO[0], EE[0]

		phaddd m0, m0
		; m0 = E[1], E[2], ?, ?

		punpckhqdq m0, m1
		; m0 = E[3], E[2], E[1], E[0]

		paddd m0, [dd_0800]
		; m0 = E[3]+add, E[2]+add, E[1]+add, E[0]+add

		pshufb m2, [shuffle_2367abef2367abef]

		pmaddwd m3, m2, [cosine_inverse_8_h]
		pmaddwd m2, [cosine_inverse_8_h+16]
		phaddd m3, m2
		; m3 = O[3], O[2], O[1], O[0]
		; m0 = E[3], E[2], E[1], E[0]

		paddd m1, m0, m3
		; m1 = (E[k] + O[k] + add) >> shift : 3, 2, 1, 0
		psrad m1, 12

		psubd m0, m3
		psrad m0, 12
		; m0 = (E[k] - O[k] + add) >> shift : 3, 2, 1, 0

		pshufd m0, m0, ORDER(0, 1, 2, 3)
		; m0 = (E[k] - O[k] + add) >> shift : 0, 1, 2, 3

		packssdw m1, m0

		mova [r0], m1

		add r0, 16
		add r1, 16
		dec r4d
		jg .loop

	RET

%endif