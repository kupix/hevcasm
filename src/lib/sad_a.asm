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

mask_24_32:
	times 24 db 255
	times 8 db 0

mask_12_16:
	times 12 db 255
	times 4 db 0
	times 12 db 255
	times 4 db 0


SECTION .text

%define ORDER(a, b, c, d) ((a << 6) | (b << 4) | (c << 2) | d)


%macro SAD_4REF_AVX2 1 ; %1=width
; Sum of absolute differences with four references
; typedef void hevcasm_sad_multiref(const uint8_t *src, ptrdiff_t stride_src, const uint8_t *ref[], ptrdiff_t stride_ref, int sad[], uint32_t rect);
INIT_YMM avx2
cglobal sad_multiref_4_%1xh, 6, 9, 9
	mov r6, r1 ; stride_src
	mov r7, r3 ; stride_ref
	and r5, 0xff
%if %1 <= 16
	shr r5, 1
%endif
	mov r8, r4 ; sad[]
	mov r1, [r2+0*8]
	mov r3, [r2+2*8]
	mov r4, [r2+3*8]
	mov r2, [r2+1*8]
	pxor m5, m5
	pxor m6, m6
	pxor m7, m7
	pxor m8, m8
	.loop:
%if %1 <= 16
		movu xm0, [r0]
		movu xm1, [r1]
		movu xm2, [r2]
		movu xm3, [r3]
		movu xm4, [r4]
		vinserti128  m0, m0, [r0+r6], 1
		vinserti128  m1, m1, [r1+r7], 1
		vinserti128  m2, m2, [r2+r7], 1
		vinserti128  m3, m3, [r3+r7], 1
		vinserti128  m4, m4, [r4+r7], 1
%if %1 == 12
		pand m0, [mask_12_16]
		pand m1, [mask_12_16]
		pand m2, [mask_12_16]
		pand m3, [mask_12_16]
		pand m4, [mask_12_16]
%endif
%else
		movu m0, [r0]
		movu m1, [r1]
		movu m2, [r2]
		movu m3, [r3]
		movu m4, [r4]
%endif
		psadbw m1, m0
		psadbw m2, m0
		psadbw m3, m0
		psadbw m4, m0
%if %1 == 24
		pand m1, [mask_24_32]
		pand m2, [mask_24_32]
		pand m3, [mask_24_32]
		pand m4, [mask_24_32]
%endif
		paddd m5, m1  
		paddd m6, m2  
		paddd m7, m3  
		paddd m8, m4  
%if %1 > 32
		movu m0, [r0+32]
		movu m1, [r1+32]
		movu m2, [r2+32]
		movu m3, [r3+32]
		movu m4, [r4+32]
%if %1 == 48
		psadbw xm1, xm0
		psadbw xm2, xm0
		psadbw xm3, xm0
		psadbw xm4, xm0
%else
		psadbw m1, m0
		psadbw m2, m0
		psadbw m3, m0
		psadbw m4, m0
%endif
		paddd m5, m1  
		paddd m6, m2  
		paddd m7, m3  
		paddd m8, m4  
%endif
%if %1 <= 16
		lea r0, [r0 + r6 * 2]
		lea r1, [r1 + r7 * 2]
		lea r2, [r2 + r7 * 2]
		lea r3, [r3 + r7 * 2]
		lea r4, [r4 + r7 * 2]
%else
		lea r0, [r0 + r6]
		lea r1, [r1 + r7]
		lea r2, [r2 + r7]
		lea r3, [r3 + r7]
		lea r4, [r4 + r7]
%endif
		dec r5
		jg .loop
	vextracti128 xm0, m5, 1
	vextracti128 xm1, m6, 1
	vextracti128 xm2, m7, 1
	vextracti128 xm3, m8, 1
	paddd xm5, xm0
	paddd xm6, xm1
	paddd xm7, xm2
	paddd xm8, xm3
%if 0 ; two different ways of achieving the same thing - both seem to take similar number of cycles
	pshufd xm0, xm5, ORDER(0, 0, 0, 2)
	pshufd xm1, xm6, ORDER(0, 0, 0, 2)
	pshufd xm2, xm7, ORDER(0, 0, 0, 2)
	pshufd xm3, xm8, ORDER(0, 0, 0, 2)
	paddd xm5, xm0
	paddd xm6, xm1
	paddd xm7, xm2
	paddd xm8, xm3
	movd [r8 + 0*4], xm5
	movd [r8 + 1*4], xm6
	movd [r8 + 2*4], xm7
	movd [r8 + 3*4], xm8
%else
	pslldq xm6, 4
	pslldq xm8, 4
	por xm5, xm6
	por xm7, xm8
	mova xm6, xm5
	mova xm8, xm7
	punpcklqdq xm5, xm7
	punpckhqdq xm6, xm8
	paddd xm5, xm6
	movu [r8], xm5
%endif
	RET
%endmacro

SAD_4REF_AVX2 64
SAD_4REF_AVX2 48
SAD_4REF_AVX2 32
SAD_4REF_AVX2 24
SAD_4REF_AVX2 16
SAD_4REF_AVX2 12

; Sum of absolute differences with four references
; typedef void hevcasm_sad_multiref(const uint8_t *src, ptrdiff_t stride_src, const uint8_t *ref[], ptrdiff_t stride_ref, int sad[], uint32_t rect);
INIT_XMM avx2
cglobal sad_multiref_4_8xh, 6, 9, 9
	and r5, 0xff
	shr r5, 1
	mov r6, [r2 + 0*8]
	mov r7, [r2 + 1*8]
	mov r8, [r2 + 2*8]
	mov r2, [r2 + 3*8]
	movh xm4, [r0]
	movh xm0, [r6]
	movh xm1, [r7]
	movh xm2, [r8]
	movh xm3, [r2]
	sub r7, r6
	sub r8, r6
	sub r2, r6
	lea r6, [r6+r3]
	movhps xm4, [r0+r1]
	movhps xm0, [r6]
	movhps xm1, [r7+r6]
	movhps xm2, [r8+r6]
	movhps xm3, [r2+r6]
	lea r6, [r6+r3]
	lea r0, [r0+r1*2]
	psadbw xm0, xm4
	psadbw xm1, xm4
	psadbw xm2, xm4
	psadbw xm3, xm4
	dec r5
	.loop:
		movh xm4, [r0]
		movh xm5, [r6]
		movh xm6, [r7+r6]
		movh xm7, [r8+r6]
		movh xm8, [r2+r6]
		lea r6, [r6+r3]
		movhps xm4, [r0+r1]
		movhps xm5, [r6]
		movhps xm6, [r7+r6]
		movhps xm7, [r8+r6]
		movhps xm8, [r2+r6]
		lea r6, [r6+r3]
		lea r0, [r0+r1*2]
		psadbw xm5, xm4
		psadbw xm6, xm4
		psadbw xm7, xm4
		psadbw xm8, xm4
		paddd xm0, xm5
		paddd xm1, xm6
		paddd xm2, xm7
		paddd xm3, xm8
		dec r5
		jg .loop
	pslldq xm1, 4
	pslldq xm3, 4
	por xm0, xm1
	por xm2, xm3
	mova xm1, xm0
	mova xm3, xm2
	punpcklqdq xm0, xm2
	punpckhqdq xm1, xm3
	paddd xm0, xm1
	movu [r4], xm0
	RET


; Sum of absolute differences with four references
; typedef void hevcasm_sad_multiref(const uint8_t *src, ptrdiff_t stride_src, const uint8_t *ref[], ptrdiff_t stride_ref, int sad[], uint32_t rect);
INIT_XMM avx2
cglobal sad_multiref_4_4xh, 6, 9, 9
	pxor m0, m0
	pxor m1, m1
	pxor m2, m2
	pxor m3, m3
	mov r6, [r2 + 0*8]
	mov r7, [r2 + 1*8]
	mov r8, [r2 + 2*8]
	mov r2, [r2 + 3*8]
	sub r7, r6
	sub r8, r6
	sub r2, r6
	and r5, 0xff
	shr r5, 1
	.loop:
		movd xm4, [r0]
		movd xm5, [r6]
		movd xm6, [r7+r6]
		movd xm7, [r8+r6]
		movd xm8, [r2+r6]
		lea r6, [r6+r3]
		punpckldq xm4, [r0+r1]
		punpckldq xm5, [r6]
		punpckldq xm6, [r7+r6]
		punpckldq xm7, [r8+r6]
		punpckldq xm8, [r2+r6]
		lea r6, [r6+r3]
		lea r0, [r0+r1*2]
		psadbw xm5, xm4
		psadbw xm6, xm4
		psadbw xm7, xm4
		psadbw xm8, xm4
		punpckldq xm5, xm6
		punpckldq xm7, xm8
		paddd xm0, xm5
		paddd xm2, xm7
		dec r5
		jg .loop
	pslldq xm1, 4
	pslldq xm3, 4
	por xm0, xm1
	por xm2, xm3
	mova xm1, xm0
	mova xm3, xm2
	punpcklqdq xm0, xm2
	punpckhqdq xm1, xm3
	paddd xm0, xm1
	movu [r4], xm0
	RET	

