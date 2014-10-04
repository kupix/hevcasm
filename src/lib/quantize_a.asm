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

ones_w:
	times 8 dw 1



SECTION .text

; void quantize_inverse_sse4(int16_t *dst, int16_t *src, int scale, int shift, int n);
INIT_XMM sse4
cglobal quantize_inverse, 5, 5, 6

	mova m0, [ones_w]

	movd m1, r3d 
	; m1 = shift

	add r3b, 15
	bts r2d, r3d 
	; r2d = (0x10000 << (shift - 1)) + scale
	movd m2, r2d
	pshufd m2, m2, 0
	; m2 = 1<<(shift-1), scale, 1<<(shift-1), scale, 1<<(shift-1), scale, 1<<(shift-1), scale

	shr r4d, 4 
	; r4 = n/16

	.loop

%assign offset 0
%rep 2

		mova m4, [r1 + offset]
		; m4 = src[7], src[6], src[5], src[4], src[3], src[2], src[1], src[0]

		punpcklwd m5, m4, m0
		; m5 = 1, src[3], 1, src[2], 1, src[1], 1, src[0]

		punpckhwd m4, m0
		; m4 = 1, src[7], 1, src[6], 1, src[5], 1, src[4]

		pmaddwd m4, m2
		; m4 = (1<<(shift-1)) + src[7] * scale , (1<<(shift-1)) + src[6] * scale, (1<<(shift-1)) + src[5] * scale, (1<<(shift-1)) + src[4] * scale

		pmaddwd m5, m2
		; m5 = (1<<(shift-1)) + src[3] * scale , (1<<(shift-1)) + src[2] * scale, (1<<(shift-1)) + src[1] * scale, (1<<(shift-1)) + src[0] * scale

		psrad m4, m1
		; m4 = ((1<<(shift-1)) + src[7] * scale)>>shift , ((1<<(shift-1)) + src[6] * scale)>>shift, ((1<<(shift-1)) + src[5] * scale)>>shift, ((1<<(shift-1)) + src[4] * scale)>>shift
		
		psrad m5, m1
		; m5 = ((1<<(shift-1)) + src[3] * scale)>>shift , ((1<<(shift-1)) + src[2] * scale)>>shift, ((1<<(shift-1)) + src[1] * scale)>>shift, ((1<<(shift-1)) + src[0] * scale)>>shift

		packssdw m5, m4
		; m5 = ((1<<(shift-1)) + src[7] * scale)>>shift , ((1<<(shift-1)) + src[6] * scale)>>shift, ((1<<(shift-1)) + src[5] * scale)>>shift, ((1<<(shift-1)) + src[4] * scale)>>shift, ((1<<(shift-1)) + src[3] * scale)>>shift , ((1<<(shift-1)) + src[2] * scale)>>shift, ((1<<(shift-1)) + src[1] * scale)>>shift, ((1<<(shift-1)) + src[0] * scale)>>shift

		mova [r0 + offset], m5

%assign offset offset+16
%endrep

		add r1, 32
		add r0, 32
		dec r4d
		jg .loop

	RET



; int quantize_sse4(int16_t *dst, const int16_t *src, int scale, int shift, int offset, int n);
INIT_XMM sse4
cglobal quantize, 6, 7, 8

	movd m1, r3d 
	; m1 = shift

	bts r2d, r3d 
	; r2d = (1 << shift) + scale
	movd m2, r2d
	pshufd m2, m2, 0
	; m2 = 1<<(shift-16), scale, 1<<(shift-16), scale, 1<<(shift-16), scale, 1<<(shift-16), scale

	movd m3, r4d
	pshuflw m3, m3, 0
	pshufd m3, m3, 0
	; m3 = offset, offset, offset, offset, offset, offset, offset, offset

	pxor m0, m0
	; m3 = 0

	shr r5d, 4 
	; r5 = n/16

	.loop

%assign offset 0
%rep 2
		mova m4, [r1 + offset] 
		; m4 = src[7], src[6], src[5], src[4], src[3], src[2], src[1], src[0]
	
		pabsw m5, m4 
		; m5 = abs(src[7]), abs(src[6]), abs(src[5]), abs(src[4]), abs(src[3]), abs(src[2]), abs(src[1]), abs(src[0])
		
		punpcklwd m6, m5, m3
		; m6 = offset, abs(src[3]), offset, abs(src[2]), offset, abs(src[1]), offset, abs(src[0])

		punpckhwd m5, m3
		; m5 = offset, abs(src[7]), offset, abs(src[6]), offset, abs(src[5]), offset, abs(src[4])

		pmaddwd m6, m2
		; m6 = (offset<<(shift-16))+abs(src[3])*scale, (offset<<(shift-16))+abs(src[2])*scale, (offset<<(shift-16))+abs(src[1])*scale, (offset<<(shift-16))+abs(src[0])*scale

		psrad m6, m1
		; m6 = (offset<<(shift-16))+abs(src[3])*scale>>shift, (offset<<(shift-16))+abs(src[2])*scale>>shift, (offset<<(shift-16))+abs(src[1])*scale>>shift, (offset<<(shift-16))+abs(src[0])*scale>>shift 

		pmaddwd m5, m2
		; m5 = (offset<<(shift-16))+abs(src[7])*scale, (offset<<(shift-16))+abs(src[6])*scale, (offset<<(shift-16))+abs(src[5])*scale, (offset<<(shift-16))+abs(src[4])*scale, 

		psrad m5, m1
		; m5 = (offset<<(shift-16))+abs(src[7])*scale>>shift, (offset<<(shift-16))+abs(src[6])*scale>>shift, (offset<<(shift-16))+abs(src[5])*scale>>shift, (offset<<(shift-16))+abs(src[4])*scale>>shift 

		punpcklwd m7, m4
		; m7 = (src[3]<<16)+0x????,(src[2]<<16)+0x????,(src[1]<<16)+0x????,(src[0]<<16)+0x????

		psignd m6, m7
		; m6 = ((offset<<(shift-16))+abs(src[3])*scale>>shift)*sign(src[3]), ((offset<<(shift-16))+abs(src[2])*scale>>shift)*sign(src[2]), ((offset<<(shift-16))+abs(src[1])*scale>>shift)*sign(src[1]), ((offset<<(shift-16))+abs(src[0])*scale>>shift)*sign(src[0]) 
		; m6 = dst[3], dst[2], dst[1], dst[0]

		punpckhwd m4, m4
		; m4 = (src[7]<<16)+0x????,(src[6]<<16)+0x????,(src[5]<<16)+0x????,(src[4]<<16)+0x????

		psignd m5, m4
		; m5 = ((offset<<(shift-16))+abs(src[7])*scale>>shift)*sign(src[7]), ((offset<<(shift-16))+abs(src[6])*scale>>shift)*sign(src[6]), ((offset<<(shift-16))+abs(src[5])*scale>>shift)*sign(src[5]), ((offset<<(shift-16))+abs(src[4])*scale>>shift)*sign(src[4]) 
		; m5 = dst[7], dst[6], dst[5], dst[4]

		packssdw m6, m5
		; m6 = dst[7], dst[6], dst[5], dst[4], dst[3], dst[2], dst[1], dst[0]

		por m0, m6
		; m0 is non-zero if we have seen any non-zero quantized coefficients 

		mova [r0 + offset], m6

%assign offset offset+16
%endrep

		add r1, 32
		add r0, 32
		dec r5d
		jg .loop

	; return zero only if m0 is zero - no non-zero quantized coefficients seen (cbf=0)
	packsswb m0, m0
	packsswb m0, m0
    movd eax, m0
	
	RET



; int hevcasm_quantize_reconstruct_4x4_sse4(uint8_t *recSamples, ptrdiff_t recStride, const uint8_t *predSamples, ptrdiff_t predStride, const int16_t *resSamples);
INIT_XMM sse4
cglobal quantize_reconstruct_4x4, 5, 6, 4
	pxor m0, m0
	mov r5d, 2
	.loop
		movd m1, [r2]
		movd m2, [r2+r3]
		lea r2, [r2+2*r3]

		punpckldq m1, m2
		punpcklbw m1, m0

		movu m3, [r4]
		paddw m1, m3
		lea r4, [r4+16]
		packuswb m1, m1

		movd [r0], m1
		pshufd m1, m1, ORDER(0, 0, 0, 1)
		movd [r0+r1], m1
		lea r0, [r0+2*r1]

		dec r5d
		jg .loop

	RET		



; int quantize_reconstruct_8x8_sse4(uint8_t *recSamples, ptrdiff_t recStride, const uint8_t *predSamples, ptrdiff_t predStride, const int16_t *resSamples);
INIT_XMM sse4
cglobal quantize_reconstruct_8x8, 5, 6, 2
	pxor m0, m0
	mov r5d, 8
	.loop
		movq m1, [r2]
		lea r2, [r2+r3]
		punpcklbw m1, m0
		; m1 = pred[7] ... pred[0]

		paddw m1, [r4]
		lea r4, [r4+16]
		; m1 = pred[7]+res[7] ... pred[0]+res[0]

		packuswb m1, m1

		movq [r0], m1
		lea r0, [r0+r1]

		dec r5d
		jg .loop

	RET		



; int quantize_reconstruct_16x16_sse4(uint8_t *recSamples, ptrdiff_t recStride, const uint8_t *predSamples, ptrdiff_t predStride, const int16_t *resSamples);
INIT_XMM sse4
cglobal quantize_reconstruct_16x16, 5, 6, 3
	pxor m0, m0
	mov r5d, 16
	.loop
		mova m1, [r2]
		lea r2, [r2+r3]
		punpcklbw m2, m1, m0
		; m2 = pred[7] ... pred[0]

		punpckhbw m1, m0
		; m1 = pred[15] ... pred[8]

		paddw m2, [r4]
		; m2 = pred[7]+res[7] ... pred[0]+res[0]

		paddw m1, [r4+16]
		lea r4, [r4+32]
		; m1 = pred[15]+res[15] ... pred[8]+res[8]

		packuswb m2, m1

		mova [r0], m2
		lea r0, [r0+r1]

		dec r5d
		jg .loop

	RET		
	


; int quantize_reconstruct_32x32_sse4(uint8_t *recSamples, ptrdiff_t recStride, const uint8_t *predSamples, ptrdiff_t predStride, const int16_t *resSamples);
INIT_XMM sse4
cglobal quantize_reconstruct_32x32, 5, 6, 3
	pxor m0, m0
	mov r5d, 32
	.loop

%assign offset 0
%rep 2
		mova m1, [r2 + 16 * offset]
		punpcklbw m2, m1, m0
		; m2 = pred[7] ... pred[0]

		punpckhbw m1, m0
		; m1 = pred[15] ... pred[8]

		paddw m2, [r4 + 32 * offset]
		; m2 = pred[7]+res[7] ... pred[0]+res[0]

		paddw m1, [r4 + 32 * offset + 16]
		; m1 = pred[15]+res[15] ... pred[8]+res[8]

		packuswb m2, m1

		mova [r0 + 16 * offset], m2

%assign offset offset + 1
%endrep

		lea r2, [r2+r3]
		lea r0, [r0+r1]
		lea r4, [r4+64]

		dec r5d
		jg .loop

	RET		