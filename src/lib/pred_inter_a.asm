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


SECTION_RODATA 32

%macro CONSTANT 3
	constant_times_%1_%2_%3:
		times %1 %2 %3
%endmacro

CONSTANT 8, dw, 0x20
CONSTANT 8, dw, 0x40
CONSTANT 4, dd, 0x800


%macro PRED_INTER_8TAP_COEFFICIENT_PAIRS 2
	pred_inter_8tap_coefficient_pairs_%1_%2:
		; Frac = 0/4
		times %1 %2 0, 0
		times %1 %2 0, 64
		times %1 %2 0, 0
		times %1 %2 0, 0
		; Frac = 1/4
		times %1 %2 -1, 4
		times %1 %2 -10, 58 
		times %1 %2 17, -5 
		times %1 %2 1, 0
		; Frac = 2/4
		times %1 %2 -1, 4
		times %1 %2 -11, 40
		times %1 %2 40, -11
		times %1 %2 4, -1
		; Frac = 3/4
		times %1 %2 0, 1
		times %1 %2 -5, 17
		times %1 %2 58, -10
		times %1 %2 4, -1
%endmacro

PRED_INTER_8TAP_COEFFICIENT_PAIRS 8, db
PRED_INTER_8TAP_COEFFICIENT_PAIRS 4, dw


%macro PRED_INTER_4TAP_COEFFICIENT_PAIRS 2
	pred_inter_4tap_coefficient_pairs_%1_%2:
		; Frac = 0/8
		times %1 %2 0, 64
		times %1 %2 0, 0
		; Frac = 1/8
		times %1 %2 -2, 58
		times %1 %2 10, -2
		; Frac = 2/8
		times %1 %2 -4, 54
		times %1 %2 16, -2
		; Frac = 3/8
		times %1 %2 -6, 46
		times %1 %2 28, -4
		; Frac = 4/8
		times %1 %2 -4, 36
		times %1 %2 36, -4
		; Frac = 5/8
		times %1 %2 -4, 28
		times %1 %2 46, -6
		; Frac = 6/8
		times %1 %2 -2, 16
		times %1 %2 54, -4
		; Frac = 7/8
		times %1 %2 -2, 10
		times %1 %2 58, -2
%endmacro

PRED_INTER_4TAP_COEFFICIENT_PAIRS 8, db
PRED_INTER_4TAP_COEFFICIENT_PAIRS 4, dw


SECTION .text

; void hevcasm_pred_uni_copy_8to8_16xh_sse2(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
INIT_XMM sse2
cglobal pred_uni_copy_8to8_16xh, 8, 8, 1
.loop
	movu m0, [r2]
	movu [r0], m0
	lea r2, [r2 + r3]
	lea r0, [r0 + r1]
	dec r5d
	jg .loop
	RET

; void hevcasm_pred_uni_copy_8to8_32xh_sse2(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
INIT_XMM sse2
cglobal pred_uni_copy_8to8_32xh, 8, 8, 2
.loop
	movu m0, [r2]
	movu m1, [r2 + 16]
	movu [r0], m0
	movu [r0 + 16], m1
	lea r2, [r2 + r3]
	lea r0, [r0 + r1]
	dec r5d
	jg .loop
	RET

; void hevcasm_pred_uni_copy_8to8_48xh_sse2(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
INIT_XMM sse2
cglobal pred_uni_copy_8to8_48xh, 8, 8, 3
.loop
	movu m0, [r2]
	movu m1, [r2 + 16]
	movu m2, [r2 + 32]
	movu [r0], m0
	movu [r0 + 16], m1
	movu [r0 + 32], m2
	lea r2, [r2 + r3]
	lea r0, [r0 + r1]
	dec r5d
	jg .loop
	RET

; void hevcasm_pred_uni_copy_8to8_64xh_sse2(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
INIT_XMM sse2
cglobal pred_uni_copy_8to8_64xh, 8, 8, 4
.loop
	movu m0, [r2]
	movu m1, [r2 + 16]
	movu m2, [r2 + 32]
	movu m3, [r2 + 48]
	movu [r0], m0
	movu [r0 + 16], m1
	movu [r0 + 32], m2
	movu [r0 + 48], m3
	lea r2, [r2 + r3]
	lea r0, [r0 + r1]
	dec r5d
	jg .loop
	RET


; void hevcasm_pred_uni_copy_16to16_8xh_sse2(uint16_t *dst, ptrdiff_t stride_dst, const uint16_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
INIT_XMM sse2
cglobal pred_uni_copy_16to16_8xh, 8, 8, 1
.loop
	movu m0, [r2]
	movu [r0], m0
	lea r2, [r2 + 2 * r3]
	lea r0, [r0 + 2 * r1]
	dec r5d
	jg .loop
	RET


; void hevcasm_pred_uni_copy_16to16_16xh_sse2(uint16_t *dst, ptrdiff_t stride_dst, const uint16_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
INIT_XMM sse2
cglobal pred_uni_copy_16to16_16xh, 8, 8, 2
.loop
	movu m0, [r2]
	movu m1, [r2 + 16]
	movu [r0], m0
	movu [r0 + 16], m1
	lea r2, [r2 + 2 * r3]
	lea r0, [r0 + 2 * r1]
	dec r5d
	jg .loop
	RET


; void hevcasm_pred_uni_copy_16to16_32xh_sse2(uint16_t *dst, ptrdiff_t stride_dst, const uint16_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
INIT_XMM sse2
cglobal pred_uni_copy_16to16_32xh, 8, 8, 4
.loop
	movu m0, [r2]
	movu m1, [r2 + 16]
	movu m2, [r2 + 32]
	movu m3, [r2 + 48]
	movu [r0], m0
	movu [r0 + 16], m1
	movu [r0 + 32], m2
	movu [r0 + 48], m3
	lea r2, [r2 + 2 * r3]
	lea r0, [r0 + 2 * r1]
	dec r5d
	jg .loop
	RET


; void hevcasm_pred_uni_copy_16to16_48xh_sse2(uint16_t *dst, ptrdiff_t stride_dst, const uint16_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
INIT_XMM sse2
cglobal pred_uni_copy_16to16_48xh, 8, 8, 4
.loop
	movu m0, [r2]
	movu m1, [r2 + 16]
	movu m2, [r2 + 32]
	movu m3, [r2 + 48]
	movu [r0], m0
	movu [r0 + 16], m1
	movu [r0 + 32], m2
	movu [r0 + 48], m3
	movu m0, [r2 + 64]
	movu m1, [r2 + 80]
	movu [r0 + 64], m0
	movu [r0 + 80], m1
	lea r2, [r2 + 2 * r3]
	lea r0, [r0 + 2 * r1]
	dec r5d
	jg .loop
	RET


; void hevcasm_pred_uni_copy_16to16_64xh_sse2(uint16_t *dst, ptrdiff_t stride_dst, const uint16_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
INIT_XMM sse2
cglobal pred_uni_copy_16to16_64xh, 8, 8, 4
.loop
	movu m0, [r2]
	movu m1, [r2 + 16]
	movu m2, [r2 + 32]
	movu m3, [r2 + 48]
	movu [r0], m0
	movu [r0 + 16], m1
	movu [r0 + 32], m2
	movu [r0 + 48], m3
	movu m0, [r2 + 64]
	movu m1, [r2 + 80]
	movu m2, [r2 + 96]
	movu m3, [r2 + 112]
	movu [r0 + 64], m0
	movu [r0 + 80], m1
	movu [r0 + 96], m2
	movu [r0 + 112], m3
	lea r2, [r2 + 2 * r3]
	lea r0, [r0 + 2 * r1]
	dec r5d
	jg .loop
	RET


%macro PRED_UNI_H_16x1 3
	; %1 is number of filter taps (4 or 8)
	; %2 is size of output type (8 for uint8_t rounded, 16 for int16_t right shifted 6)
	; %3 is dx (horizontal offset as integer number of samples) 

	movu m2, [r2 + 1 - (%1/2) + %3]
	pmaddubsw m2, m4
	; m2 = eca86420 (even positions, first two taps)

	movu m1, [r2 + 2 - (%1/2) + %3]
	pmaddubsw m1, m4
	; m1 = fdb97531 (odd positions, first two taps)

	movu m0, [r2 + 3 - (%1/2) + %3]
	pmaddubsw m0, m5 
	; m0 = eca86420 (even positions, two taps)

	paddw m2, m0 
	; m2 = eca86420 (even positions)

	movu m0, [r2 + 4 - (%1/2) + %3] 
	pmaddubsw m0, m5 
	; m0 = fdb97531 (odd positions, two taps)

	paddw m1, m0
	; m1 = fdb97531 (odd positions)

	%if %1 == 8
		; need four more taps...

		movu m0, [r2 + 5 - (%1/2) + %3]
		pmaddubsw m0, m6 
		; m0 = eca86420 (even positions, two taps)

		paddw m2, m0 
		; m2 = eca86420 (even positions)

		movu m0, [r2 + 6 - (%1/2) + %3] 
		pmaddubsw m0, m6 
		; m0 = fdb97531 (odd positions, two taps)

		paddw m1, m0
		; m1 = fdb97531 (odd positions)

		movu m0, [r2 + 7 - (%1/2) + %3]
		pmaddubsw m0, m7 
		; m0 = eca86420 (even positions, two taps)

		paddw m2, m0 
		; m2 = eca86420 (even positions)

		movu m0, [r2 + 8 - (%1/2) + %3] 
		pmaddubsw m0, m7 
		; m0 = fdb97531 (odd positions, two taps)

		paddw m1, m0
		; m1 = fdb97531 (odd positions)

	%endif

	punpcklwd m0, m2, m1
	; m0 = 76543210

	punpckhwd m2, m1 
	; m2 = fedcba98

	%if %2 == 16
		mova [r0 + 2 * %3], m0 
		mova [r0 + 2 * %3 + 16], m2 
	%else
		paddw m0, [constant_times_8_dw_0x20]
		paddw m2, [constant_times_8_dw_0x20]
		psraw m0, 6
		psraw m2, 6
		packuswb m0, m2
		movu [r0 + %3], m0 
	%endif

%endmacro


%macro PRED_UNI_H_16NxH 3
	; %1 is number of filter taps (4 or 8)
	; %2 is size of output type (8 for uint8_t rounded, 16 for int16_t right shifted 6)
	; %3 is block width (number of samples, multiple of 16)

	; void hevcasm_pred_uni_%1tap_8to%2_h_%3xh_sse4(D *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
	INIT_XMM sse4
	cglobal pred_uni_%1tap_8to%2_h_%3xh, 8, 8, (6+%1/4)
    
		%if %1 == 8
			shl r6d, 6  ; frac *= 4 * 16
			lea r4, [pred_inter_8tap_coefficient_pairs_8_db ]
		%else
			shl r6d, 5  ; frac *= 2 * 16
			lea r4, [pred_inter_4tap_coefficient_pairs_8_db ]
		%endif

		mova m4, [r4 + r6+ 0 * 16]
		mova m5, [r4 + r6+ 1 * 16]
		%if %1 == 8
			mova m6, [r4 + r6+ 2 * 16]
			mova m7, [r4 + r6+ 3 * 16]
		%endif

		%if %2 == 16
			; dst is int16_t * so need double the stride
			shl r1, 1  ; 
		%endif

		.loop
			%assign dx 0
			%rep %3/16
				PRED_UNI_H_16x1 %1, %2, dx
				%assign dx dx+16	
			%endrep

			add r0, r1
			add r2, r3
			dec r5d
			jg .loop

		RET

%endmacro	

PRED_UNI_H_16NxH 8, 8, 16 
PRED_UNI_H_16NxH 8, 8, 32 
PRED_UNI_H_16NxH 8, 8, 48 
PRED_UNI_H_16NxH 8, 8, 64 

PRED_UNI_H_16NxH 8, 16, 16 
PRED_UNI_H_16NxH 8, 16, 32 
PRED_UNI_H_16NxH 8, 16, 48 
PRED_UNI_H_16NxH 8, 16, 64 

PRED_UNI_H_16NxH 4, 8, 16 
PRED_UNI_H_16NxH 4, 8, 32 

PRED_UNI_H_16NxH 4, 16, 16 
PRED_UNI_H_16NxH 4, 16, 32 



%macro PRED_UNI_V_8NxH 3
	; %1 is number of filter taps (4 or 8);
	; %2 is size of input type (8 for uint8_t, 16 for int16_t right shifted 6)
	; %3 is block width (number of samples, multiple of 8)

	; void hevcasm_pred_uni_%1tap_%2to8_v_%3xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac, int yFrac);
	INIT_XMM sse4
	cglobal pred_uni_%1tap_%2to8_v_%3xh, 8, 9, 6
    
		%if %2 == 16
			shl r3, 1
		%endif

		; adjust input pointer (subtract (taps/2-1) * stride)
		%rep %1/2-1
			sub r2, r3
		%endrep

		%if %2 == 16
			lea r4, [pred_inter_%1tap_coefficient_pairs_4_dw]
		%else
			lea r4, [pred_inter_%1tap_coefficient_pairs_8_db]
		%endif

		mov r6, r7m ; avoid probls
		shl r6d, 4+%1/4
		lea r4, [r4 + r6]

		.loop

			%rep %3/8
				; each iteration of this %rep operates on a row of 8-samples

				%if %2 == 16
					mova m3, [constant_times_4_dd_0x800]
					mova m5, m3
				%else
					mova m3, [constant_times_8_dw_0x20]
				%endif										

				%rep %1/2
					; each iteration of this loop performs two filter taps

					%if %2 == 16
						movu m0, [r2 + 0 * r3]
						movu m1, [r2 + 1 * r3]
						punpckhwd m2, m0, m1
						punpcklwd m0, m1
						pmaddwd m0, [r4]
						pmaddwd m2, [r4]

						paddd m3, m0
						paddd m5, m2
					%else
						movq m0, [r2 + 0 * r3]
						movq m1, [r2 + 1 * r3]
						punpcklbw m0, m1
						pmaddubsw m0, [r4]

						paddw m3, m0
					%endif										

					lea r2, [r2 + 2 * r3]
					
					add r4, 16
				%endrep

				neg r3
				lea r2, [r2 + %1 * r3]
				neg r3

				sub r4, %1 * 8
	
				%if %2 == 16
					psrad m3, 12
					psrad m5, 12
					packssdw m3, m5
				%else
					psraw m3, 6
				%endif

				packuswb m3, m3 

				movq [r0], m3

				lea r2, [r2 + %2]
				lea r0, [r0 + 8]

			%endrep

			sub r0, 8 * %3 / 8
			sub r2, %2 * %3 / 8

			add r0, r1
			add r2, r3

			dec r5d
			jg .loop

		RET

%endmacro	

PRED_UNI_V_8NxH 8, 8, 8
PRED_UNI_V_8NxH 8, 8, 16
PRED_UNI_V_8NxH 8, 8, 24
PRED_UNI_V_8NxH 8, 8, 32 
PRED_UNI_V_8NxH 8, 8, 48 
PRED_UNI_V_8NxH 8, 8, 64 

PRED_UNI_V_8NxH 8, 16, 8
PRED_UNI_V_8NxH 8, 16, 16
PRED_UNI_V_8NxH 8, 16, 24
PRED_UNI_V_8NxH 8, 16, 32
PRED_UNI_V_8NxH 8, 16, 48
PRED_UNI_V_8NxH 8, 16, 64

PRED_UNI_V_8NxH 4, 8, 8
PRED_UNI_V_8NxH 4, 8, 16
PRED_UNI_V_8NxH 4, 8, 24
PRED_UNI_V_8NxH 4, 8, 32 

PRED_UNI_V_8NxH 4, 16, 8
PRED_UNI_V_8NxH 4, 16, 16
PRED_UNI_V_8NxH 4, 16, 24
PRED_UNI_V_8NxH 4, 16, 32



%macro PRED_BI_V_8NxH 3
	; %1 is number of filter taps (4 or 8);
	; %2 is size of input type (8 for uint8_t, 16 for int16_t right shifted 6)
	; %3 is block width (number of samples, multiple of 8)

	%if %2==8
		%error "not yet implemented"
	%endif

	; void hevcasm_pred_bi_v_%1tap_16to16_%3xh_sse4(uint8_t *dst, ptrdiff_t stride_dst, const int16_t *refAtop, const int16_t *refBtop, ptrdiff_t stride_ref, int nPbW, int nPbH, int yFracA, int yFracB);
	INIT_XMM sse4
	cglobal pred_bi_v_%1tap_16to16_%3xh, 9, 9, 8
    
		shl r4, 1

		%if ARCH_X86_64
			shl r7d, 4+%1/4
			shl r8d, 4+%1/4
			lea r5, [pred_inter_%1tap_coefficient_pairs_4_dw]
			lea r7, [r5 + r7]
			lea r8, [r5 + r8]
			%define coeffA r7
			%define coeffB r8
			%define stride_dst r1
		%else
			mov r5, r7m
			shl r5, 4+%1/4
			lea r1, [pred_inter_%1tap_coefficient_pairs_4_dw + r5]
			mov r5, r8m
			shl r5, 4+%1/4
			lea r5, [pred_inter_%1tap_coefficient_pairs_4_dw + r5]
			%define coeffA r1
			%define coeffB r5
			%define stride_dst r1m
		%endif

		.loop
			%rep %3/8
				; each iteration of this loop operates on a row of 8-samples

				pxor m3, m3
				mova m5, m3
				mova m6, m3
				mova m7, m3

				%rep %1 / 2
					; each iteration of this loop performs two filter taps

					; reference picture A
					mova m0, [r2 + 0 * r4]
					mova m1, [r2 + 1 * r4]
					punpckhwd m2, m0, m1
					punpcklwd m0, m1
					pmaddwd m0, [coeffA]
					pmaddwd m2, [coeffA]
					paddd m3, m0
					paddd m5, m2
					lea r2, [r2 + 2 * r4]

					; reference picture B
					mova m0, [r3 + 0 * r4]
					mova m1, [r3 + 1 * r4]
					punpckhwd m2, m0, m1
					punpcklwd m0, m1
					pmaddwd m0, [coeffB]
					pmaddwd m2, [coeffB]
					paddd m6, m0
					paddd m7, m2
					lea r3, [r3 + 2 * r4]
					
					add coeffA, 16
					add coeffB, 16
				%endrep

				neg r4
				lea r2, [r2 + %1 * r4]
				lea r3, [r3 + %1 * r4]
				neg r4

				sub coeffA, 8 * %1
				sub coeffB, 8 * %1
	
				psrad m3, 6
				psrad m5, 6
				psrad m6, 6
				psrad m7, 6

				packssdw m3, m5
				packssdw m6, m7

				paddsw m3, m6
				paddsw m3, [constant_times_8_dw_0x40]
				psraw m3, 7

				packuswb m3, m3 

				movq [r0], m3

				lea r2, [r2 + 16]
				lea r3, [r3 + 16]
				lea r0, [r0 + 8]

			%endrep

			sub r2, 2 * %3
			sub r3, 2 * %3
			sub r0, %3

			add r2, r4
			add r3, r4
			add r0, stride_dst

			dec r6d
			jg .loop

		RET

	%undef coeffA
	%undef coeffB
	%undef stride_dst

%endmacro	

PRED_BI_V_8NxH 8, 16, 16
PRED_BI_V_8NxH 8, 16, 32
PRED_BI_V_8NxH 8, 16, 48
PRED_BI_V_8NxH 8, 16, 64

PRED_BI_V_8NxH 4, 16, 16
PRED_BI_V_8NxH 4, 16, 32



%macro PRED_BI_COPY 1
	; %1 width (in bytes)

	; void hevcasm_pred_bi_8to8_copy_%1xh_sse2(uint8_t *dst, ptrdiff_t stride_dst, const uint8_t *ref0, const uint8_t *ref1, ptrdiff_t stride_ref, int nPbW, int nPbH, int xFrac0, int yFrac0, int xFrac1, int yFrac1);
	INIT_XMM sse2
	cglobal pred_bi_8to8_copy_%1xh, 11, 11, 2
		.loop
			%assign dx 0 
			%rep %1/16
				movu m0, [r2 + dx]
				movu m1, [r3 + dx]
				pavgb m0, m1
				movu [r0 + dx], m0
				%assign dx dx+16
			%endrep
			lea r2, [r2 + r4]
			lea r3, [r3 + r4]
			lea r0, [r0 + r1]
			dec r6d
			jg .loop
		RET

%endmacro


PRED_BI_COPY 16
PRED_BI_COPY 32
PRED_BI_COPY 48
PRED_BI_COPY 64
