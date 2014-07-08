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
; *Redistributions of source code must retain the above copyright notice,
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



         [BITS 64]                        ; 64 bit segment

        global hevcasm_instruction_set                      ; visibility for ld

        section .text                    ; code

; hevcasm_instruction_set

; Purpose:         Inspect the available instruction set.

; Input:           none

; Output:          an instruction set index reflecting what ISA is available
;                   1 = SSE2 supported
;                   2 = SSE3 supported
;                   3 = Supplementary SSE3 (SSSE3) supported
;                   4 = SSE4.1 supported
;                   5 = POPCNT supported
;                   6 = SSE4.2 supported
;                   7 = AVX supported (CPU and OS)
;                   8 = RDRAND supported
;                   9 = PCLMUL and AES supported
;                  10 = AVX2 supported



hevcasm_instruction_set:

        push       rbx                   ; ebx is affected by CPUID
        push       rdi                   ; result holder
        mov        r8, 1                 ; r8 = increment value
        xor        rax, rax

        ; We're in a 64 bit OS. So, at least SSE2 must be available.

        mov        rdi, 1                ; indicate SSE2
        mov        eax, 1                ; get version information
        cpuid

        ; check SSE3

        test       ecx, 1                ; supported?
        jz         .done                 ; no: jump
        add        rdi, r8               ; yes: indicate SSE3

        ; check SSSE3

        bt         ecx, 9                ; supported?
        jnc        .done                 ; no: jump
        add        rdi, r8               ; yes: indicate SSSE3

        ; check SSE4.1

        bt         ecx, 19               ; supported?
        jnc        .done                 ; no: jump
        add        rdi, r8               ; yes: indicate SSE4.1

        ; check POPCNT support

        bt         ecx, 23               ; supported?
        jnc        .done                 ; no: jump
        add        rdi, r8               ; yes: indicate POPCNT support

        ; check SSE4.2

        bt         ecx, 20               ; supported?
        jnc        .done                 ; no: jump
        add        rdi, r8               ; yes: indicate SSE4.2

        ; check AVX support by the Operating System

        bt         ecx, 27               ; XGETBV supported?
        jnc        .done                 ; no: jump
        push       rcx
        push       rdx
        xor        ecx, ecx
        xgetbv
        and        eax, 6
        cmp        eax, 6                ; AVX supported by OS?
        pop        rdx
        pop        rcx
        jne        .done                 ; no: jump

        ; check AVX support by the CPU

        bt         ecx, 28               ; AVX support by CPU?
        jnc        .done                 ; no: jump
        add        rdi, r8               ; yes: indicate AVX

        ; check RDRAND support

        bt         ecx, 30               ; RDRAND supported?
        jnc        .done                 ; no: jump
        add        rdi, r8               ; yes: indicate RDRAND

        ; check PCLMUL support

        bt         ecx, 1                ; supported?
        jnc        .done                 ; no: jump

        ; check AES support

        bt         ecx, 25               ; supported?
        jnc        .done                 ; no: jump
        add        rdi, r8               ; yes: indicate PCLMUL and AES

        ; check AVX2

        mov        eax, 7
        xor        ecx, ecx
        cpuid                            ; check AVX2 support
        bt         ebx, 5                ; supported?
        jnc        .done                 ; no: jump
        add        rdi, r8               ; yes: indicate AVX2
        
.done:  

        mov        rax, rdi              ; load function result
        pop        rdi
        pop        rbx
        ret
