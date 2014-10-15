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


%if ARCH_X86_64 == 1

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

const_00080008000800080008000800080008
	times 8 dw 8

shuffle_018945cd018945cd:
	db 0, 1, 8, 9, 4, 5, 12, 13, 0, 1, 8, 9, 4, 5, 12, 13

shuffle_2367abef2367abef:
	db 2, 3, 6, 7, 10, 11, 14, 15, 2, 3, 6, 7, 10, 11, 14, 15

shuffle_2367236723672367:
	times 4 db 2, 3, 6, 7

shuffle_abefabefabefabef:
	times 4 db 10, 11, 14, 15

shuffle_45cd45cd45cd45cd:
	times 4 db 4, 5, 12, 13

shuffle_0189018901890189:
	times 4 db 0, 1, 8, 9

shuffle_efcdab8967452301:
	db 14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1

shuffle_23016745ab89efcd:
	db 2, 3, 0, 1, 6, 7, 4, 5, 10, 11, 8, 9, 14, 15, 12, 13

%macro V_TABLE_ENTRY_4 8
	times 4 dw %1, %2
	times 4 dw %3, %4
	times 4 dw %5, %6
	times 4 dw %7, %8
%endmacro

cosine_inverse_16:
	V_TABLE_ENTRY_4 90, 87, 80, 70, 57, 43, 25, 9
	V_TABLE_ENTRY_4 87, 57, 9, -43, -80, -90, -70, -25 
	V_TABLE_ENTRY_4 80, 9, -70, -87, -25, 57, 90, 43
	V_TABLE_ENTRY_4 70, -43, -87, 9, 90, 25, -80, -57
	V_TABLE_ENTRY_4 57, -80, -25, 90, -9, -87, 43, 70
	V_TABLE_ENTRY_4 43, -90, 57, 25, -87, 70, 9, -80
	V_TABLE_ENTRY_4 25, -70, 90, -80, 43, 9, -57, 87
	V_TABLE_ENTRY_4 9, -25, 43, -57, 70, -80, 87, -90

%macro H_TABLE_ENTRY 16
	dw %1, %9, %2, %10, %3, %11, %4, %12
	dw %5, %13, %6, %14, %7, %15, %8, %16
%endmacro

cosine_inverse_16_h:
	H_TABLE_ENTRY 90, 87, 80, 70, 57, 43, 25, 9,  87, 57, 9, -43, -80, -90, -70, -25
	H_TABLE_ENTRY 80, 9, -70, -87, -25, 57, 90, 43,  70, -43, -87, 9, 90, 25, -80, -57
	H_TABLE_ENTRY 57, -80, -25, 90, -9, -87, 43, 70,  43, -90, 57, 25, -87, 70, 9, -80
	H_TABLE_ENTRY 25, -70, 90, -80, 43, 9, -57, 87,  9, -25, 43, -57, 70, -80, 87, -90

%macro H_TABLE_ENTRY_8 8
	dw %1, %5, %2, %6, %3, %7, %4, %8
%endmacro

cosine_inverse_8_h2:
	H_TABLE_ENTRY_8 89, 75, 50, 18,  75, -18, -89, -50
	H_TABLE_ENTRY_8 50, -89, 18, 75,  18, -50, 75, -89 

const_00000200000002000000020000000200:
	times 4 dd 0x200

const_00000008000000080000000800000008:
	times 4 dd 8

%macro V_TABLE_ENTRY_16 16
	times 4 dw %1, %2
	times 4 dw %3, %4
	times 4 dw %5, %6
	times 4 dw %7, %8
	times 4 dw %9, %10
	times 4 dw %11, %12
	times 4 dw %13, %14
	times 4 dw %15, %16
%endmacro

cosine_8:
	V_TABLE_ENTRY_16 90, 87, 80, 70, 57, 43, 25, 9,  87, 57, 9, -43, -80, -90, -70, -25
	V_TABLE_ENTRY_16 80, 9, -70, -87, -25, 57, 90, 43,  70, -43, -87, 9, 90, 25, -80, -57
	V_TABLE_ENTRY_16 57, -80, -25, 90, -9, -87, 43, 70,  43, -90, 57, 25, -87, 70, 9, -80
	V_TABLE_ENTRY_16 25, -70, 90, -80, 43, 9, -57, 87,  9, -25, 43, -57, 70, -80, 87, -90

%macro V_TABLE_ENTRY_16_NEW 16
	dd %1, %2, %3, %4
	dd %5, %6, %7, %8
	dd %9, %10, %11, %12
	dd %13, %14, %15, %16
%endmacro

cosine_8_new:
	V_TABLE_ENTRY_16_NEW 90, 87, 80, 70, 57, 43, 25, 9,  87, 57, 9, -43, -80, -90, -70, -25
	V_TABLE_ENTRY_16_NEW 80, 9, -70, -87, -25, 57, 90, 43,  70, -43, -87, 9, 90, 25, -80, -57
	V_TABLE_ENTRY_16_NEW 57, -80, -25, 90, -9, -87, 43, 70,  43, -90, 57, 25, -87, 70, 9, -80
	V_TABLE_ENTRY_16_NEW 25, -70, 90, -80, 43, 9, -57, 87,  9, -25, 43, -57, 70, -80, 87, -90

%macro V_TABLE_ENTRY_8 8
	times 4 dw %1, %2
	times 4 dw %3, %4
	times 4 dw %5, %6
	times 4 dw %7, %8
%endmacro

cosine_4:
	V_TABLE_ENTRY_8 89, 75, 50, 18,  75, -18, -89, -50
	V_TABLE_ENTRY_8 50, -89, 18, 75,  18, -50, 75, -89

cosine_4_new:
	dd 89, 75, 50, 18,  75, -18, -89, -50
	dd 50, -89, 18, 75,  18, -50, 75, -89

cosine_2_alt:
	times 4 dw 83, 36
	times 4 dw 36, -83

cosine_2_new:
	dd 83, 36, 36, -83

cosine_1_h:
	dw 64, 64, -83, -36, -64, 64, -83, 36

%macro H_TABLE_ENTRY_16 16
	dw %1, %2, %3, %4, %5, %6, %7, %8
	dw %9, %10, %11, %12, %13, %14, %15, %16
%endmacro

cosine_8_h:
	H_TABLE_ENTRY_16 90, 87, 80, 70, 57, 43, 25, 9,  87, 57, 9, -43, -80, -90, -70, -25
	H_TABLE_ENTRY_16 80, 9, -70, -87, -25, 57, 90, 43,  70, -43, -87, 9, 90, 25, -80, -57
	H_TABLE_ENTRY_16 57, -80, -25, 90, -9, -87, 43, 70,  43, -90, 57, 25, -87, 70, 9, -80
	H_TABLE_ENTRY_16 25, -70, 90, -80, 43, 9, -57, 87,  9, -25, 43, -57, 70, -80, 87, -90

cosine_4_h:
	dw 89, 75, 50, 18,  50, 89, 18, -75 ; 75, -18, -89, -50
	dw 50, -89, 18, 75,  89, -75, 50, -18 ;18, -50, 75, -89

const_00000004000000040000000400000004:
	times 4 dd 4

SECTION .text


; void hevcasm_partial_butterfly_16v_ssse3(std::int16_t *dst, const std::int16_t *src, int shift);
INIT_XMM ssse3
cglobal partial_butterfly_16v, 3, 6, 16

	mov r5d, 4
	.loop_left_right

%macro COMPUTE_4 3
		movq m%1, [r1 + %1 * 16 * 2]
		punpcklwd m%1, m%1
		psrad m%1, 16
		; m%1 = src(3:0, %1)

		movq m8, [r1 + (15 - %1) * 16 * 2]
		punpcklwd m8, m8
		psrad m8, 16
		; m8 = src(3:0, 15-%1)
		
		%3 m%1, m8
		; m%1 = E[0](3:0) when %3 is paddd
		; m%1 = O[0](3:0) when %3 is psubd

		movq m%2, [r1 + (%2) * 16 * 2]
		punpcklwd m%2, m%2
		psrad m%2, 16
		; m%2 = src(3:0, %2)

		movq m9, [r1 + (15 - %2) * 16 * 2]
		punpcklwd m9, m9
		psrad m9, 16
		; m9 = src(3:0, 15-%2)

		%3 m%2, m9
		; m%2 = E[1](3:0) when %3 is paddd
		; m%2 = O[1](3:0) when %3 is psubd
%endmacro

		COMPUTE_4 0, 1, psubd
		COMPUTE_4 2, 3, psubd
		COMPUTE_4 4, 5, psubd
		COMPUTE_4 6, 7, psubd
		; mx = O[x](3:0)

		lea r0, [r0 + 1 * 16 * 2]
		lea r4, [cosine_8_new]
		mov r3d, 8
		.loop_write_odd
			mova m8, [const_00000200000002000000020000000200] 

			mova m10, [r4 + 0 * 16]
			pshufd m11, m10, ORDER(0, 0, 0, 0)
			pmulld m11, m0
			paddd m8, m11
			pshufd m12, m10, ORDER(1, 1, 1, 1)
			pmulld m12, m1
			paddd m8, m12
			pshufd m13, m10, ORDER(2, 2, 2, 2)
			pmulld m13, m2
			paddd m8, m13
			pshufd m14, m10, ORDER(3, 3, 3, 3)
			pmulld m14, m3
			paddd m8, m14

			mova m10, [r4 + 1 * 16]
			pshufd m11, m10, ORDER(0, 0, 0, 0)
			pmulld m11, m4
			paddd m8, m11
			pshufd m12, m10, ORDER(1, 1, 1, 1)
			pmulld m12, m5
			paddd m8, m12
			pshufd m13, m10, ORDER(2, 2, 2, 2)
			pmulld m13, m6
			paddd m8, m13
			pshufd m14, m10, ORDER(3, 3, 3, 3)
			pmulld m14, m7
			paddd m8, m14

			lea r4, [r4 + 2 * 16]

			psrad m8, 10

			packssdw m8, m8
			movq [r0], m8

			lea r0, [r0 + 2 * 16 * 2]

			dec r3d
			jg .loop_write_odd

		lea r0, [r0 - (1 + 8 * 2) * 16 * 2]

		COMPUTE_4 0, 1, paddd
		COMPUTE_4 2, 3, paddd
		COMPUTE_4 5, 4, paddd
		COMPUTE_4 7, 6, paddd
		; mx = E[x](3:0)

		psubd m8, m0, m7
		psubd m9, m1, m6
		psubd m10, m2, m5
		psubd m11, m3, m4
		; m8:11 = EO[0:3] (3:0)

		paddd m0, m7
		paddd m1, m6
		paddd m2, m5
		paddd m3, m4
		; m0:3 = EE[0:3] (3:0)

		psubd m4, m0, m3
		psubd m5, m1, m2
		; m4:5 = EEO[0:1] (3:0)

		paddd m0, m3
		paddd m1, m2
		; m0:1 = EEE[0:1] (3:0)

		paddd m0, [const_00000008000000080000000800000008]

		lea r0, [r0 + 2 * 16 * 2]
		lea r4, [cosine_4_new]
		mov r3d, 4
		.loop_write_even_odd
			mova m2, [const_00000200000002000000020000000200] 

			mova m3, [r4 + 0 * 16]
			pshufd m12, m3, ORDER(0, 0, 0, 0)
			pmulld m12, m8
			paddd m2, m12
			pshufd m13, m3, ORDER(1, 1, 1, 1)
			pmulld m13, m9
			paddd m2, m13
			pshufd m14, m3, ORDER(2, 2, 2, 2)
			pmulld m14, m10
			paddd m2, m14
			pshufd m15, m3, ORDER(3, 3, 3, 3)
			pmulld m15, m11
			paddd m2, m15

			psrad m2, 10
			packssdw m2, m2
			movq [r0], m2

			lea r4, [r4 + 1 * 16]
			lea r0, [r0 + 4 * 16 * 2]
			dec r3d
			jg .loop_write_even_odd
		lea r0, [r0 - (2 + 4 * 4) * 16 * 2]

		mova m3, [cosine_2_new]

		mova m2, [const_00000200000002000000020000000200] 
		pshufd m12, m3, ORDER(0, 0, 0, 0)
		pmulld m12, m4
		paddd m2, m12
		pshufd m13, m3, ORDER(1, 1, 1, 1)
		pmulld m13, m5
		paddd m2, m13
		psrad m2, 10
		packssdw m2, m2
		movq [r0 + (4) * 16 * 2], m2

		mova m2, [const_00000200000002000000020000000200] 
		pshufd m12, m3, ORDER(2, 2, 2, 2)
		pmulld m12, m4
		paddd m2, m12
		pshufd m13, m3, ORDER(3, 3, 3, 3)
		pmulld m13, m5
		paddd m2, m13
		psrad m2, 10
		packssdw m2, m2
		movq [r0 + (12) * 16 * 2], m2

		; m0:1 = EEE[0:1] (3:0)
		paddd m2, m0, m1
		psubd m0, m1

		psrad m2, 4
		psrad m0, 4

		packssdw m2, m2 ; 50% utilization 
		movq [r0 + (0) * 16 * 2], m2

		packssdw m0, m0 ; 50% utilization 
		movq [r0 + (8) * 16 * 2], m0

		lea r1, [r1 + 8]
		lea r0, [r0 + 8]
		dec r5d
		jg	.loop_left_right
	
	RET

; void transform_partial_butterfly_16h_ssse3(std::int16_t *dst, const std::int16_t *src, std::ptrdiff_t, srcStride, int shift);
; shift parameter ignored (r3)
INIT_XMM ssse3
cglobal partial_butterfly_16h, 4, 7, 16
	mova m3, [shuffle_efcdab8967452301]
	mova m4, [const_00000004000000040000000400000004]
	mov r4d, 16
.loop
		mova m0, [r1]
		mova m1, [r1+16]
		pshufb m1, m3
		
		paddw m2, m0, m1
		; m2 = E[7:0]
		
		psubw m0, m1
		; m0 = O[7:0]
		
		pmaddwd m8, m0, [cosine_8_h + 0 * 16]
		pmaddwd m9, m0, [cosine_8_h + 1 * 16]
		pmaddwd m10, m0, [cosine_8_h + 2 * 16]
		pmaddwd m11, m0, [cosine_8_h + 3 * 16]
		pmaddwd m12, m0, [cosine_8_h + 4 * 16]
		pmaddwd m13, m0, [cosine_8_h + 5 * 16]
		pmaddwd m14, m0, [cosine_8_h + 6 * 16]
		pmaddwd m15, m0, [cosine_8_h + 7 * 16]

		phaddd m8, m9
		phaddd m10, m11
		phaddd m12, m13
		phaddd m14, m15

		phaddd m8, m10
		phaddd m12, m14
	
		paddd m8, m4
		psrad m8, 3
		paddd m12, m4
		psrad m12, 3

		packssdw m8, m12
		; m8 = dst[15,13,11, 9, 7, 5, 3, 1]

		pshufb m5, m2, m3
		; m5 = E[0:7]

		psubw m7, m2, m5
		; m7 = -EO[0:3], EO[3:0]

		pmaddwd m6, m7, [cosine_4_h]
		pmaddwd m7, [cosine_4_h + 16]
		phaddd m6, m7
		paddd m6, m4
		psrad m6, 3
		; dst[14, 10, 6, 2]

		pslld m6, 16
		; m6 = dst[14], 0, dst[10], 0, dst[6], 0, dst[2], 0

		paddw m2, m5
		; m12 = EE[0,1,2,3,3,2,1,0]
	
		pshufd m11, m2, ORDER(3, 2, 3, 2)
		; m11 = EE[0,1,2,3,0,1,2,3]

		paddw m10, m11, m2
		; m10 = 2*EE[0,1,2,3], EEE[0,1,1,0]

		psubw m11, m2
		; m11= 0,0,0,0,EEO[0,1],-EEO[1,0]

		punpckldq m10, m11
		; m10 = EEO[0,1], EEE[0,1], -EEO[1,0], EEE[1,0]

		pmaddwd m10, [cosine_1_h]

		paddd m10, m4
		pslld m10, 16-3
		; m10 = dst[12, 8, 4, 0] << 16
		; m10 = dst[12], 0, dst[8], 0, dst[4], 0, dst[0], 0
		psrld m10, 16

		por m10, m6
		; m10 = dst[14,12,10, 8, 6, 4, 2, 0]

		punpcklwd m6, m10, m8
		mova [r0], m6

		punpckhwd m10, m8
		mova [r0+16], m10

		lea r0, [r0+2*16]
		lea r1, [r1+2*r2]
		dec r4d
		jg .loop

	RET


; this function potentially be combined with the horizontal feature with no
; need for temporary memory buffer in between.

; void transform_partial_butterfly_inverse_8v_ssse3(std::int16_t *dst, const std::int16_t *src, int shift);
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
	 

; void transform_partial_butterfly_inverse_8h_ssse3(std::int16_t *dst, const std::int16_t *src, int shift);
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




; void transform_partial_butterfly_inverse_16v_ssse3(std::int16_t *dst, const std::int16_t *src, int shift);
INIT_XMM ssse3
cglobal partial_butterfly_inverse_16v, 3, 6, 16, 8*2*16

	mov r5d, 2
.loop_horizontal

%macro INTERLEAVE 5
		mova %2, [r1 + %1 * 2 * 16]
		; %2 = src(7, %1), src(6, %1), src(5, %1), src(4, %1), src(3, %1), src(2, %1), src(1, %1), src(0, %1) 

		mova %5, [r1 + %3 * 2 * 16]
		; %5 = src(7, %3), src(6, %3), src(5, %3), src(4, %3), src(3, %3), src(2, %3), src(1, %3), src(0, %3) 

		punpckhwd %4, %2, %5
		; %4 = src(7, %3), src(7, %1), src(6, %3), src(6, %1), src(5, %3), src(5, %1), src(4, %3), src(4, %1) 

		punpcklwd %2, %5
		; %2 = src(3, %3), src(3, %1), src(2, %3), src(2, %1), src(1, %3), src(1, %1), src(0, %3), src(0, %1) 
%endmacro

		INTERLEAVE 1, m0, 3, m1, m9
		INTERLEAVE 5, m2, 7, m3, m9
		INTERLEAVE 9, m4, 11, m5, m9
		INTERLEAVE 13, m6, 15, m7, m9

		; m0 to m7 contain interleaved odd source rows

		lea r3, [cosine_inverse_16]
		mov r4d, 8

		.loop_compute_Ok
			pmaddwd m8, m0, [r3+0*16]
			pmaddwd m9, m1, [r3+0*16]

			pmaddwd m10, m2, [r3+1*16]
			pmaddwd m11, m3, [r3+1*16]
			paddd m8, m10
			paddd m9, m11

			pmaddwd m10, m4, [r3+2*16]
			pmaddwd m11, m5, [r3+2*16]
			paddd m8, m10
			paddd m9, m11

			pmaddwd m10, m6, [r3+3*16]
			pmaddwd m11, m7, [r3+3*16]
			paddd m8, m10
			paddd m9, m11

			mova [rsp], m8
			mova [rsp+16], m9

			lea r3, [r3+4*16]
			add rsp, 2*16
			dec r4d
			jg .loop_compute_Ok

		sub rsp, 8*2*16

		; now at esp is O[k] for 8 vertical positions (32-bit signed values)
	
		; the following is almost identical to 8x8 inverse transform

		lea r3, [cosine_inverse_4]
		lea r4, [dd_0040]

		mova m8, [r1 + 0 * 2 * 16] 
		mova m1, [r1 + 8 * 2 * 16]
		punpcklwd m0, m8, m1
		punpckhwd m8, m1

		pmaddwd m1, m0, [r3]
		paddd m1, [r4]
		; m1 = EEE[0] + add: 3, 2, 1, 0 

		pmaddwd m9, m8, [r3]
		paddd m9, [r4]
		; m9 = EEE[0] + add : 7, 6, 5, 4 

		pmaddwd m0, [r3+16]
		paddd m0, [r4]
		; m0 = EEE[1] + add : 3, 2, 1, 0 

		pmaddwd m8, [r3+16]
		paddd m8, [r4]
		; m8 = EEE[1] + add : 7, 6, 5, 4  

		mova m10, [r1 + 4 * 2 * 16] 
		mova m3, [r1 + 12 * 2 * 16]
		punpcklwd m2, m10, m3
		punpckhwd m10, m3
		pmaddwd m3, m2, [r3+32]
		; m3 = EEO[0] : 3, 2, 1, 0 

		pmaddwd m11, m10, [r3+32]
		; m11 = EEO[0] : 7, 6, 5, 4 

		pmaddwd m2, [r3+48]
		; m2 = EEO[1] : 3, 2, 1, 0 

		pmaddwd m10, [r3+48]
		; m10 = EEO[1] : 7, 6, 5, 4 

		paddd m4, m1, m3
		; m4 = EE[0] : 3, 2, 1, 0 

		paddd m12, m9, m11
		; m12 = EE[0] : 7, 6, 5, 4 

		psubd m1, m3
		; m1 = EE[3] : 3, 2, 1, 0 

		psubd m9, m11
		; m9 = EE[3] : 7, 6, 5, 4 

		paddd m3, m0, m2
		; m3 = E[1] : 3, 2, 1, 0 

		paddd m13, m8, m10
		; m13 = E[1] : 7, 6, 5, 4 

		psubd m0, m2
		; m0 = E[2] : 3, 2, 1, 0 

		psubd m8, m10
		; m8 = E[2] : 7, 6, 5, 4  

		; m4, m3, m0, m1 contain EE[] for words 3, 2, 1, 0
		; m12, m13, m8, m9 contain EE[] for words 7, 6, 5, 4

		mova m5, [r1 + 2 * 2 * 16]
		mova m2, [r1 + 6 * 2 * 16]
		punpckhwd m10, m5, m2
		punpcklwd m5, m2

		mova m6, [r1 + 10 * 2 * 16]
		mova m7, [r1 + 14 * 2 * 16]
		punpckhwd m11, m6, m7
		punpcklwd m6, m7

		; m4, m3, m0, m1 contain EE[] for words 3, 2, 1, 0
		; m12, m13, m8, m9 contain EE[] for words 7, 6, 5, 4
		; m5 contains source rows 2 and 6 for words 3, 2, 1, 0
		; m6 contains source rows 10 and 14 for words 3, 2, 1, 0
		; m10 contains source rows 2 and 6 for words 7, 6, 5, 4
		; m11 contains source rows 10 and 14 for words 7, 6, 5, 4

		lea r3, [cosine_inverse_8]

%macro OUTPUTROWS_B 3
		; k = %1
		; EE[k] is in registers %2 (3:0) and %3 (7:4)
		; O[k] is at [esp + 8 * 4 * k] 
		; EO[k] = 4-tap FIR
		; E[k] = EE[k] + EO[k]
		; dst[k] = E[k] + O[k]
		; dst[15 - k] = E[k] - O[k]
		; E[7-k] = EE[k] - EO[k]
		; dst[7-k] = E[7-k] + O[7-k]
		; dst[k+8] = E[7-k] - O[7-k]

		pmaddwd m2, m5, [r3+32*%1+0]
		pmaddwd m14, m10, [r3+32*%1+0]
		pmaddwd m7, m6, [r3+32*%1+16]
		pmaddwd m15, m11, [r3+32*%1+16]
		paddd m2, m7
		paddd m14, m15
		; m2 = EO[%1] : 3, 2, 1, 0 
		; m14 = EO[%1] : 7, 6, 5, 4

		mova m7, m2 
		mova m15, m14 
		paddd m7, %2
		paddd m15, %3
		; m7 = (E[%1] + add) = (EE[%1] + EO[%1] + add) : 3, 2, 1, 0
		; m15 = (E[%1] + add) = (EE[%1] + EO[%1] + add) : 7, 6, 5, 4

		; at this point, all registers are in use

		psubd %2, m2
		psubd %3, m14
		; %2 = (E[7-%1] + add) = (EE[%1] - EO[%1] + add) : 3, 2, 1, 0
		; %3 = (E[7-%1] + add) = (EE[%1] - EO[%1] + add) : 7, 6, 5, 4

		paddd m2, m7, [rsp+8*4*%1] ;
		; m2 = (E[%1] + O[%1] + add) : 3, 2, 1, 0

		paddd m14, m15, [rsp+8*4*%1 + 16] ;
		; m14 = (E[%1] + O[%1] + add) : 7, 6, 5, 4

		psrad m2, 7
		psrad m14, 7
		packssdw m2, m14
		mova [r0+16*2*%1], m2

		psubd m7, [rsp+8*4*%1] ; 
		; m7 = (E[%1] - O[%1] + add) : 3, 2, 1, 0

		psubd m15, [rsp+8*4*%1 + 16] ; 
		; m15 = (E[%1] - O[%1] + add) : 7, 6, 5, 4

		psrad m7, 7
		psrad m15, 7
		packssdw m7, m15
		mova [r0+16*2*(15-%1)], m7

		paddd m2, %2, [rsp+8*4*(7-%1)] ;
		; m2 = (E[7-%1] + O[7-%1] + add) : 3, 2, 1, 0

		paddd m14, %3, [rsp+8*4*(7-%1) + 16] ;
		; m14 = (E[7-%1] + O[7-%1] + add) : 7, 6, 5, 4

		psrad m2, 7
		psrad m14, 7
		packssdw m2, m14
		mova [r0+16*2*(7-%1)], m2

		psubd %2, [rsp+8*4*(7-%1)] ; 
		; %2 = (E[%1] - O[%1] + add) : 3, 2, 1, 0

		psubd %3, [rsp+8*4*(7-%1) + 16] ; 
		; %3 = (E[%1] - O[%1] + add) : 7, 6, 5, 4

		psrad %2, 7
		psrad %3, 7
		packssdw %2, %3
		mova [r0+16*2*(8+%1)], %2
%endmacro

		OUTPUTROWS_B 0, m4, m12
		OUTPUTROWS_B 1, m3, m13
		OUTPUTROWS_B 2, m0, m8
		OUTPUTROWS_B 3, m1, m9

		add r0, 16
		add r1, 16
		dec r5d
		jg .loop_horizontal

	RET


; void transform_partial_butterfly_inverse_16h_ssse3(std::int16_t *dst, const std::int16_t *src, int shift);
INIT_XMM ssse3
cglobal partial_butterfly_inverse_16h, 3, 5, 16
	mov r4d, 16

	.loop
		mova m0, [r1]
		; m0 = src[7:0]
		
		mova m1, [r1 + 16]
		; m1 = src[15:8]

		add r1, 32

		; 8 x 8-tap horizontal filters on odd positions

		pshufb m6, m0, [shuffle_2367236723672367] ; 3, 1
		pshufb m7, m0, [shuffle_abefabefabefabef] ; 7, 5
		pshufb m8, m1, [shuffle_2367236723672367]  ; 11, 9
		pshufb m9, m1, [shuffle_abefabefabefabef] ; 15, 13

		pmaddwd m2, m6, [cosine_inverse_16_h + 0 * 16]
		pmaddwd m6, [cosine_inverse_16_h + 1 * 16]

		pmaddwd m3, m7, [cosine_inverse_16_h + 2 * 16]
		pmaddwd m7, [cosine_inverse_16_h + 3 * 16]

		pmaddwd m4, m8, [cosine_inverse_16_h + 4 * 16]
		pmaddwd m8, [cosine_inverse_16_h + 5 * 16]

		pmaddwd m5, m9, [cosine_inverse_16_h + 6 * 16]
		pmaddwd m9, [cosine_inverse_16_h + 7 * 16]

		paddd m2, m3
		paddd m4, m5
		paddd m2, m4
		; m2 = O[3:0]

		paddd m6, m7
		paddd m8, m9
		paddd m6, m8
		; m6 = O[7:4]

		pshufb m3, m0, [shuffle_45cd45cd45cd45cd] ; 6, 2
		pshufb m4, m1, [shuffle_45cd45cd45cd45cd]  ; 14, 10

		pmaddwd m3, [cosine_inverse_8_h2 + 0 * 16]
		pmaddwd m4, [cosine_inverse_8_h2 + 1 * 16]

		paddd m3, m4
		; m3 = EO[3:0]

		pshufb m5, m0, [shuffle_0189018901890189] 
		; m5 = src[4, 0, 4, 0, 4, 0, 4, 0]
		
		pshufb m7, m1, [shuffle_0189018901890189] 
		; m7 = src[12, 8, 12, 8, 12, 8, 12, 8]

		punpcklwd m5, m7
		; m5 = src[12, 4, 8, 0, 12, 4, 8, 0]

		pmaddwd m5, [cosine_inverse_4_h]
		; m5 = EEO[0], EEE[0], EEO[1], EEE[1] 

		phsubd m4, m5
		; m4 = EE[3], EE[2], ?, ?

		pshufd m5, m5, ORDER(1, 0, 3, 2)
		; m5 = EEO[1], EEE[1], EEO[0], EEE[0]

		phaddd m5, m5
		; m5 = EE[1], EE[2], EE[1], EE[2]

		punpckhqdq m5, m4
		; m5 = EE[3:0]

		paddd m5, [dd_0800]
		; m5 = EE[3:0] + add

		paddd m0, m3, m5
		; m0 = E[3:0] + add

		psubd m5, m3
		; m5 = E[4:7] + add

		pshufd m10, m5, ORDER(0, 1, 2, 3)
		; m11 = E[7:4] + add

		paddd m11, m10, m6
		psrad m11, 12
		; m11 = dst[7:4]

		paddd m12, m0, m2
		psrad m12, 12
		; m12 = dst[3:0]

		packssdw m12, m11
		; m12 = dst[7:0]
		
		mova [r0], m12

		psubd m11, m10, m6
		psrad m11, 12
		; m11 = dst[8:11]

		psubd m12, m0, m2
		psrad m12, 12
		; m12 = dst[12:15]

		packssdw m12, m11
		; m12 = dst[8:15]
		
		pshufb m12, [shuffle_efcdab8967452301]

		mova [r0+16], m12
	
		add r0, 32

		dec r4d
		jg .loop

	RET


%endif