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

%if ARCH_X86_64 == 1

SECTION_RODATA 32

%macro CONSTANT 3
	constant_times_%1_%2_%3:
		times %1 %2 %3
%endmacro

CONSTANT 16, dw, 1

constant_010101010101010101ff01ff01ff01ff010101010101010101ff01ff01ff01ff:
	times 4 db 1, 1
	times 4 db 1, -1
	times 4 db 1, 1
	times 4 db 1, -1

constant_ffffffff00000000ffffffff00000000:
	times 2 dd -1, 0

constant_ffffffffffffffff0000000000000000:
	dq -1, 0

SECTION .text


%macro HADAMARD_8_H 2
	movq xm%1, [r0]  ; srcA(7..0, y)
	movq xm%2, [r2]  ; srcB(7..0, y)
	movq xm10, [r0+1*r1]  ; srcA(7..0, y+2)
	movq xm11, [r2+1*r3]  ; srcB(7..0, y+2)

	lea r0, [r0+2*r1]
	lea r2, [r2+2*r3]

	vinserti128 m%1, m%1, [r0], 1
	vinserti128 m%2, m%2, [r2], 1
	vinserti128 m10, m10, [r0+r1], 1
	vinserti128 m11, m11, [r2+r3], 1
	
	punpcklqdq m%1, m%1  ; b  srcA(7..0, y+1),  srcA(7..0, y+1),  srcA(7..0, y+0),  srcA(7..0, y+0)
	punpcklqdq m%2, m%2  ; b  srcB(7..0, y+1),  srcB(7..0, y+1),  srcB(7..0, y+0),  srcB(7..0, y+0)
	punpcklqdq m10, m10  ; b  srcA(7..0, y+3),  srcA(7..0, y+3),  srcA(7..0, y+2),  srcA(7..0, y+2)
	punpcklqdq m11, m11  ; b  srcB(7..0, y+3),  srcB(7..0, y+3),  srcB(7..0, y+2),  srcB(7..0, y+2)
	
	pmaddubsw m%1, m15 ; w  srcA(7,y+1)+srcA(6,y+1)..srcA(1,y+1)+srcA(0,y+1),  srcA(7,y+1)-srcA(6,y+1)..srcA(1,y+1)-srcA(0,y+1),  srcA(7,y+0)+srcA(6,y+0)..srcA(1,y+0)+srcA(0,y+0),  srcA(7,y+0)-srcA(6,y+0)..srcA(1,y+0)-srcA(0,y+0), 
	pmaddubsw m%2, m15 ; w  srcB(7,y+1)+srcB(6,y+1)..srcB(1,y+1)+srcB(0,y+1),  srcB(7,y+1)-srcB(6,y+1)..srcB(1,y+1)-srcB(0,y+1),  srcB(7,y+0)+srcB(6,y+0)..srcB(1,y+0)+srcB(0,y+0),  srcB(7,y+0)-srcB(6,y+0)..srcB(1,y+0)-srcB(0,y+0), 
	pmaddubsw m10, m15 ; w  srcA(7,y+3)+srcA(6,y+3)..srcA(1,y+3)+srcA(0,y+3),  srcA(7,y+3)-srcA(6,y+3)..srcA(1,y+3)-srcA(0,y+3),  srcA(7,y+2)+srcA(6,y+2)..srcA(1,y+2)+srcA(0,y+2),  srcA(7,y+2)-srcA(6,y+2)..srcA(1,y+2)-srcA(0,y+2), 
	pmaddubsw m11, m15 ; w  srcB(7,y+3)+srcB(6,y+3)..srcB(1,y+3)+srcB(0,y+3),  srcB(7,y+3)-srcB(6,y+3)..srcB(1,y+3)-srcB(0,y+3),  srcB(7,y+2)+srcB(6,y+2)..srcB(1,y+2)+srcB(0,y+2),  srcB(7,y+2)-srcB(6,y+2)..srcB(1,y+2)-srcB(0,y+2), 
	
	psubw m%1, m%2 ; w src(7,y+1)+src(6,y+1)..src(1,y+1)+src(0,y+1), src(7,y+1)-src(6,y+1)..src(1,y+1)-src(0,y+1), src(7,y+0)+src(6,y+0)..src(1,y+0)+src(0,y+0), src(7,y+0)-src(6,y+0)..src(1,y+0)-src(0,y+0),  
	psubw m10, m11 ; w src(7,y+3)+src(6,y+3)..src(1,y+3)+src(0,y+3), src(7,y+3)-src(6,y+3)..src(1,y+3)-src(0,y+3), src(7,y+2)+src(6,y+2)..src(1,y+2)+src(0,y+2), src(7,y+2)-src(6,y+2)..src(1,y+2)-src(0,y+2),  

	vphsubw m12, m%1, m10
	vphaddw m13, m%1, m10

	vpunpckldq m4, m12, m13
	vpunpckhdq m5, m12, m13

	vphsubw m6, m4, m4
	vphaddw m7, m4, m4
	vphsubw m8, m5, m5
	vphaddw m9, m5, m5

	vpunpcklwd m%1, m6, m7
	vpunpcklwd m%2, m8, m9

%endmacro

%macro PUNPCKHDQQQ 3
	vperm2i128 m%1, m%2, m%3, q0301
%endmacro

%macro PUNPCKLDQQQ 3
	vinserti128 m%1, m%2, xm%3, 1
%endmacro

; int hevcasm_hadamard_satd_8x8_avx2(const uint8_t *srcA, ptrdiff_t stride_srcA, const uint8_t *srcB, ptrdiff_t stride_srcB);
INIT_YMM avx2
cglobal hadamard_satd_8x8, 4, 4, 16
	mova m15, [constant_010101010101010101ff01ff01ff01ff010101010101010101ff01ff01ff01ff]

	HADAMARD_8_H 0, 1
	lea r0, [r0+2*r1]
	lea r2, [r2+2*r3]
	HADAMARD_8_H 2, 3

	 ; horizontal transform now done - output order is incorrect but same in each row so OK for SATD

	; (x, 0) in m0 low
	; (x, 1) in m1 low
	; (x, 2) in m0 high
	; (x, 3) in m1 high
	; (x, 4) in m2 low
	; (x, 5) in m3 low
	; (x, 6) in m2 high
	; (x, 7) in m3 high

	; m0  H=00+00000 L=+0000000
	; m1  H=000+0000 L=0+000000
	; m2  H=000000+0 L=0000+000
	; m3  H=0000000+ L=00000+00

	paddw m4, m0, m2 ; H=00+000+0 L=+000+000
	paddw m5, m1, m3 ; H=000+000+ L=0+000+00
	psubw m6, m0, m2 ; H=00+000-0 L=+000-000
	psubw m7, m1, m3 ; H=000+000- L=0+000-00

	paddw m0, m4, m5 ; H=00++00++ L=++00++00
	psubw m1, m4, m5 ; H=00+-00+- L=+-00+-00
	paddw m2, m6, m7 ; H=00++00-- L=++00--00
	psubw m3, m6, m7 ; H=00+-00-+ L=+-00-+00

	PUNPCKHDQQQ 4, 0, 1  ;  H=00+-00+- L=00++00++
	PUNPCKLDQQQ 5, 0, 1	  ;  H=+-00+-00 L=++00++00
	PUNPCKHDQQQ 6, 2, 3  ;  H=00+-00-+ L=00++00--
	PUNPCKLDQQQ 7, 2, 3	  ;  L=+-00-+00 L=++00--00

	paddw m0, m4, m5  ;  H=+-+-+-+- L=++++++++
	psubw m1, m5, m4  ;  H=+--++--+ L=++--++--
	paddw m2, m6, m7  ; H=+-+--+-+ L=++++----
	psubw m3, m7, m6  ; H=+--+-++- L=++----++

	; vertical transform now done too.

	pabsw m0, m0
	pabsw m1, m1
	pabsw m2, m2
	pabsw m3, m3

	; tranformed absolute differences now computed, just need to sum them and return

	paddw m0, m1
	paddw m2, m3

	paddw m0, m2 ; 16 w

	pmaddwd m0, m0, [constant_times_16_dw_1] ; 8 d
	vextracti128 xm1, m0, 1
	paddd xm0, xm1
	movhlps xm1, xm0
	paddd xm0, xm1
	pshuflw xm1, xm0, q0032
	paddd xm0, xm1
	movd  eax, xm0
	add eax, 2
	shr eax, 2

	RET

; %1 - destination 
; %2 and %3 - sources
; uses m5 and m7
%macro DIFF_BW 3
	movu %1,%2
	movu m5, %3
	punpcklbw %1, m7
	punpcklbw m5, m7
	psubw %1, m5
%endmacro


%macro BUTTERFLY_HORIZONTAL_4 2
	pshufd %2, %1, ORDER(1, 0, 3, 2)
	pxor %1, m6
	psubw %1, m6
	paddw %1, %2
%endmacro

%macro BUTTERFLY_HORIZONTAL_2 2
	pshufd %2, %1, ORDER(2, 3, 0, 1)
	pxor %1, m6
	psubw %1, m6
	paddw %1, %2
%endmacro

%macro BUTTERFLY_HORIZONTAL_1 2
	mova %2, %1 
	phaddw %2, %1
	phsubw %1, %1
	punpcklwd %1, %2
%endmacro


	
	

; int hevcasm_hadamard_satd_4x4_sse2(const uint8_t *srcA, ptrdiff_t stride_srcA, const uint8_t *srcB, ptrdiff_t stride_srcB);
INIT_XMM sse2
cglobal hadamard_satd_4x4, 4, 4, 8
	pxor m7, m7

	DIFF_BW m0, [r0], [r2]
	DIFF_BW m1, [r0+r1], [r2+r3]
	lea r0, [r0 + 2 * r1]
	lea r2, [r2 + 2 * r3]
	DIFF_BW m2, [r0], [r2]
	DIFF_BW m3, [r0+r1], [r2+r3]

	punpcklqdq m0, m2
	punpcklqdq m1, m3

	; diff in m0, m1, m2, m3

	mova m6, [constant_ffffffff00000000ffffffff00000000]
	BUTTERFLY_HORIZONTAL_2 m0, m4
	BUTTERFLY_HORIZONTAL_2 m1, m4

	BUTTERFLY_HORIZONTAL_1 m0, m4
	BUTTERFLY_HORIZONTAL_1 m1, m4

	; rows in m0, m1, m2, m3

	; vertical butterfly 2
	;mova m6, [constant_ffffffffffffffff0000000000000000]
	pshufd m6, m6, ORDER(3, 1, 2, 0)

	BUTTERFLY_HORIZONTAL_4 m0, m4
	BUTTERFLY_HORIZONTAL_4 m1, m4

	; rows in m0, m1

	; vertical butterfly 1
	psubw m2, m0, m1
	paddw m0, m1

	; rows in m0, m2

	pabsw m0, m0
	pabsw m2, m2

	paddw m0, m2
	phaddw m0, m0
	phaddw m0, m0
	phaddw m0, m0

	movd  eax, m0
	and eax, 0xffff
	add eax, 1
	shr eax, 1

	RET

%endif