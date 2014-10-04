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
; extern "C" int ssd(const std::uint8_t *p1, const std::uint8_t *p2, int n)

INIT_XMM avx
cglobal ssd, 3, 3, 8
	shr r2, 4
	pxor m0, m0
	pxor m7, m7
.loop:
		mova m1, [r0]
		mova m2, [r1]
		punpckhbw m3, m1, m7
		punpckhbw m4, m2, m7
		punpcklbw m1, m7
		punpcklbw m2, m7
		psubw m1, m2
		psubw m3, m4
		pmaddwd m1, m1
		pmaddwd m3, m3
		paddd m0, m1
		paddd m0, m3
		add r0, mmsize
		add r1, mmsize
		dec r2
		jg .loop
	pshufd m1, m0, ORDER(3, 2, 3, 2)
	paddd m0, m1 ; Review: phaddd might be better here
	pshufd m1, m0, ORDER(3, 2, 0, 1)
	paddd m0, m1
    movd   eax, m0
	RET
