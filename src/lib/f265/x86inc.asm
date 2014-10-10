; Copyright (c) 2014, VANTRIX CORPORATION. All rights reserved. See LICENSE.txt
; for the full license text.
;
;
; This file handles OS-specific idiosyncrasies. In particular it unifies the
; amd64 (Unix) and x64 (Windows) calling conventions. It is heavily inspired by
; x264's x86inc.asm.
;
;
; The DECLARE_FUNCTION macro declares a function so that it is callable from
; other functions in the codec (and only in the codec).
;
; The PROLOGUE and EPILOG macros provide a uniform interface to the arguments
; passed to an assembly function that is called from the C code. The prologue
; places the arguments in the appropriate registers and saves the callee-saved
; registers. The epilog restores the callee-saved registers. The prologue code
; assumes the floating point arguments appear last in the argument list. The
; token 'stack_off' is set to the number of bytes pushed by the prologue code.
;
; The PROLOGUE arguments can be declared in any order, as follow:
; - ia=X:  number of integer arguments.
; - at=X:  type of the integer arguments (see below).
; - fa=X:  number of floating point arguments.
; - ti=X:  number of temporary integer registers, excluding rax and the
;          integer argument registers. The temporary registers are referred
;          to as if they were extra arguments to the function.
; - tv=X:  number of temporary vector registers, excluding the floating
;          point argument registers.
; - ym=X:  true if YMM registers are used (execute VZEROUPPER on return).
;
; The 'at' argument specifies the size in bytes of each integer argument,
; left-to-right. Hence, 'at=48' specifies that the first argument has size 4
; and the second argument has size 8.
;
; The DEFFUN macro declares the function and adds the prologue if there is at
; least one additional parameter besides the function name. The RET macro adds
; the epilog and the return instruction.
;
; The DECLARE_GPR macro renames the registers to follow the implicit register
; order defined by the calling convention.
;
; The general-purpose registers (GPR) are named gN, where N is an integer
; argument position. For example, g0 and g1 are the registers that contain the
; first and the second integer argument of a function respectively. gN is
; 64-bit, gNd is 32-bit, gNw is 16-bit, gNb is 8-bit. As a special case, ga is
; the rax register and gs is the rsp register.
;
; The vector registers (VEC) are named xN and yN, where N is a floating point
; argument position. For example, x0 and x1 are the registers that contain the
; first and the second floating argument of a function respectively. xN is XMM,
; yN is YMM.
;
; The GPR names are different for amd64 and x64. For instance, g0 is rdi on
; amd64 and rcx on x64. The rax register is not used for argument passing and
; it is available as a temporary and return register on both platforms. The VEC
; names are the same on both platforms. x0 is xmm0, x1 is xmm1 and so on.
;
;
; The upshot is that you can write assembly code once without using
; platform-specific %ifdef. This is not a panacea though. There are several
; limitations.
;
; Haswell is the first processor that is "register clean". Its non-legacy
; instructions do not use hardcoded register names. Prior processors hardcode
; rcx for variable shifts and xmm0 for blending. xmm0 being hardcoded is not a
; problem since x0 is always xmm0. For rcx, the code can be unified by using
; %ifdef to perform a register swap on x64. Although this could be handled
; automatically by dynamically changing the GPR names, it would then create
; mapping problems across function calls, so we don't do it.
;
; It is difficult to call another assembly function from its C interface. The
; stack must be aligned properly, the registers must be saved and restored, the
; shadow space must be catered for, etc. It is easier and usually more efficient
; to define a custom calling convention for the assembly functions that are
; called from other assembly functions. Simply specify how the arguments are
; passed and which registers are spilled. If a common function is also meant to
; be called from C, you can define a stub for the C-to-assembly prologue and
; call the common function from there.
;
;
; The stack should only be used to store 8-byte data, if at all. The prologue
; keeps the stack aligned to an 8-byte boundary only. Use the spill buffer to
; store vector register data and block data. Keep in mind that the calling
; convention requires the stack pointer to be aligned on 8 at all times and
; forbids data from being stored after the stack pointer.
;
; Please optimize the code for the amd64 calling convention only. x64 support
; is a nice-to-have that should not slow down the code on the primary platform.
;
;
; FIXME: add support for declaring data without bleeding symbols.

; Check if the calling convention is specified.
%ifdef ARCH_AMD64
%elifdef ARCH_X64
%else
    %fatal "ARCH_AMD64 or ARCH_x64 must be defined"
%endif

; Verify an assertion.
%macro ASSERT 1
    %if (%1) == 0
        %defstr _msg assertion failure: %1
        %error _msg
    %endif
%endmacro

; Declare the general-purpose registers.
%macro DECLARE_GPR 5        ; %1: register name, %2: "q" name, %3, "d" name, %4, "w" name, %5: "b" name.
    %define g%1  %2
    %define g%1d %3
    %define g%1w %4
    %define g%1b %5
%endmacro

%ifdef ARCH_AMD64
DECLARE_GPR 0,  rdi, edi,  di, dil
DECLARE_GPR 1,  rsi, esi,  si, sil
DECLARE_GPR 2,  rdx, edx,  dx, dl
DECLARE_GPR 3,  rcx, ecx,  cx, cl
DECLARE_GPR 4,  r8,  r8d,  r8w, r8b
DECLARE_GPR 5,  r9,  r9d,  r9w, r9b
DECLARE_GPR 6,  r10, r10d, r10w, r10b
DECLARE_GPR 7,  r11, r11d, r11w, r11b
%else
DECLARE_GPR 0,  rcx, ecx,  cx, cl
DECLARE_GPR 1,  rdx, edx,  dx, dl
DECLARE_GPR 2,  r8,  r8d,  r8w, r8b
DECLARE_GPR 3,  r9,  r9d,  r9w, r9b
DECLARE_GPR 4,  r10, r10d, r10w, r10b
DECLARE_GPR 5,  r11, r11d, r11w, r11b
DECLARE_GPR 6,  rdi, edi,  di, dil
DECLARE_GPR 7,  rsi, esi,  si, sil
%endif

DECLARE_GPR 8,  rbx, ebx,  bx, bl
DECLARE_GPR 9,  rbp, ebp,  bp, bpl
DECLARE_GPR 10, r12, r12d, r12w, r12b
DECLARE_GPR 11, r13, r13d, r13w, r13b
DECLARE_GPR 12, r14, r14d, r14w, r14b
DECLARE_GPR 13, r15, r15d, r15w, r15b
DECLARE_GPR a,  rax, eax,  ax, al
DECLARE_GPR s,  rsp, esp,  sp, spl

; Declare the vector registers.
%assign _count 0
%rep 16
    %xdefine x%[_count] xmm%[_count]
    %xdefine y%[_count] ymm%[_count]
    %assign _count _count+1
%endrep
%undef _count

; Set the number of arguments passed by register, the number of scratch
; registers (GPR), and the offset of the first argument on the stack.
%ifdef ARCH_AMD64
%assign nb_argument_gpr 6
%assign nb_scratch_gpr  8
%assign arg_stack_off   8
%else
%assign nb_argument_gpr 4
%assign nb_scratch_gpr  6
%assign arg_stack_off   40
%endif

; Define an alias for the public symbol specified for OSes that mangle symbol
; names.
%macro ALIAS_PUBLIC_SYM 1   ; %1: symbol.
%ifidn __OUTPUT_FORMAT__, macho64
%define %1 _%1
%endif
%endmacro

; Declare an extern symbol.
%macro DECLARE_EXTERN 1     ; %1: symbol.
    ALIAS_PUBLIC_SYM %1
    extern %1
%endmacro

; Define a function aligned on a 16-bytes boundary.
%macro DECLARE_FUNCTION 1   ; %1: function name.
    ALIAS_PUBLIC_SYM %1
    align 16
    %ifidn __OUTPUT_FORMAT__,elf
    global %1: function hidden
    %else
    global %1
    %endif
    %1:
%endmacro

; Macros to push/pop values while updating the stack offset.
%macro ADD_TO_STACK_OFF 1   ; %1: offset added to stack_off.
    %assign stack_off stack_off + %1
%endmacro

%macro PUSH 1
    push %1
    ADD_TO_STACK_OFF 8
%endmacro

%macro POP 1
    pop %1
    ADD_TO_STACK_OFF -8
%endmacro

; Extract an argument from the list of arguments provided.
; WARNING: avoid prolonged contact. Keep out of the reach of children.
%macro EXTRACT_ARG 2-*      ; %1: key, %2: default, %3+: user arguments.
                            ; Output: _key=val, _match_count updated.
    ; Key token.
    %define _key %1
    ; Default token.
    %define _default %2
    ; Searched "key=".
    %defstr _key_str _key %+ =
    ; Searched "key=" length.
    %strlen _key_len _key_str
    ; Skip the first two arguments.
    %rotate 2
    ; Parse every user argument.
    %rep %0 - 2
        ; Argument "key=val".
        %defstr _arg_str %1
        ; Argument "key=".
        %substr _arg_key_str, _arg_str 1, _key_len
        ; Got a match.
        %if _key_str == _arg_key_str
            ; Argument "val".
            %substr _arg_val_str, _arg_str _key_len+1,-1
            ; Set _key to val.
            %deftok %[_ %+ %[_key]] _arg_val_str
            ; Remember the match.
            %define _match_flag
        %endif
        ; Pass to the next argument.
        %rotate 1
    %endrep
    ; Set default argument if no match.
    %ifndef _match_flag
        %xdefine %[_ %+ %[_key]] _default
    ; Update match count.
    %else
        %assign _match_count _match_count+1
    %endif
    ; Clean up.
    %undef _key
    %undef _default
    %undef _key_str
    %undef _key_len
    %undef _arg_str
    %undef _arg_key_str
    %undef _arg_val_str
    %undef _match_flag
%endmacro

; Define the function prologue.
%macro PROLOGUE 0-*

    ; Clean up the previous invocation.
    %undef stack_off
    %undef _ia
    %undef _at
    %undef _fa
    %undef _ti
    %undef _tv
    %undef _ym
    %undef _x64_xmm_save_size


    ; Extract the parameters as _key tokens.

    ; Number of arguments that matched.
    %assign _match_count 0

    ; Nasm is buggy and won't properly expand %{1:-1}, so we have to expand
    ; the parameters manually. This is fixed in the development version of
    ; nasm, remove this when it's stable.
    EXTRACT_ARG ia, 0, %1, %2, %3, %4, %5, %6
    EXTRACT_ARG at,  , %1, %2, %3, %4, %5, %6
    EXTRACT_ARG fa, 0, %1, %2, %3, %4, %5, %6
    EXTRACT_ARG ti, 0, %1, %2, %3, %4, %5, %6
    EXTRACT_ARG tv, 0, %1, %2, %3, %4, %5, %6
    EXTRACT_ARG ym, 0, %1, %2, %3, %4, %5, %6

    ; Make '_at' a string.
    %defstr _at _at

    ; Check if all arguments matched. Disabled until Nasm gets fixed and
    ; we don't pass blank arguments anymore.
    %if _match_count != %0
        ;%fatal "invalid argument in function prologue"
    %endif

    ; Sanity check.
    ASSERT(_ia + _ti <= 14)
    ASSERT(_fa + _tv <= 16)

    ; Verify the argument types.
    %strlen _tmp _at
    %if _tmp != _ia
    %fatal Integer argument types not set properly (count mismatch).
    %endif
    %assign _iter 0
    %rep _ia
        %substr _tmp _at _iter+1
        %if _tmp != '4' && _tmp != '8'
        %fatal Integer argument _iter has invalid type _tmp
        %endif
        %assign _iter _iter + 1
    %endrep

    ; Save and load registers.
    %assign stack_off 0

    ; Save the GPRs (negative count is harmless).
    %assign _reg_count _ia + _ti - nb_scratch_gpr
    %assign _reg_idx nb_scratch_gpr
    %rep _reg_count
        PUSH g%[_reg_idx]
        %assign _reg_idx _reg_idx + 1
    %endrep

    ; Save the XMM registers.
    %ifdef ARCH_X64
    %assign _reg_count _fa + _tv - 6
    %if _reg_count >= 0
        ; Align the stack to 16 for the store.
        %assign _stack_align_off (stack_off + 8) % 16

        ; Reserve space and remember how much we reserved.
        %assign _x64_xmm_save_size _stack_align_off + 16*_reg_count
        ADD_TO_STACK_OFF(_x64_xmm_save_size)
        sub gs, _x64_xmm_save_size

        ; Store.
        %assign _iter 0
        %rep _reg_count
            %assign _reg_idx 6 + _iter
            movdqa [gs + _iter*16], x%[_reg_idx]
            %assign _iter _iter + 1
        %endrep
    %endif
    %endif

    ; Get the number of floating point arguments in registers.
    %ifdef ARCH_AMD64

    %if _fa > 8
        %assign _fa_reg 8
    %else
        %assign _fa_reg _fa
    %endif

    %else

    %if _ia + _fa <= 4
        %assign _fa_reg _fa
    %elif _ia < 4
        %assign _fa_reg 4 - _ia
    %else
        %assign _fa_reg 0
    %endif

    %endif

    ; Move the arguments in xmm0 and the subsequent registers.
    %ifdef ARCH_X64
    %if _ia != 0
        %assign _iter 0
        %rep _fa_reg
            %assign _reg_idx _ia + _iter
            movsd x%[_iter], x%[_reg_idx]
            %assign _iter _iter + 1
        %endrep
    %endif
    %endif

    ; Load the floating point arguments from the stack.
    %assign _fa_stack _fa - _fa_reg
    %assign _iter 0
    %rep _fa_stack
        %assign _reg_idx _fa_reg + _iter
        movsd x%[_reg_idx], [gs + stack_off + arg_stack_off + _iter*8]
        %assign _iter _iter + 1
    %endrep

    ; Load the GPRs.
    %assign _reg_count _ia - nb_argument_gpr
    %assign _iter 0
    %rep _reg_count
        %assign _reg_idx nb_argument_gpr + _iter
        %substr _tmp _at _reg_idx+1
        %if _tmp == '4'
        %define _reg_dst g%[_reg_idx]d
        %else
        %define _reg_dst g%[_reg_idx]
        %endif
        mov _reg_dst, [gs + stack_off + arg_stack_off + (_fa_stack+_iter)*8]
        %assign _iter _iter + 1
    %endrep


    ; Clean up.
    %undef _match_count
    %undef _stack_align_off
    %undef _iter
    %undef _tmp
    %undef _reg_count
    %undef _reg_idx
    %undef _reg_dst
    %undef _fa_reg
    %undef _fa_stack

%endmacro

; Define the function epilog. A function may have multiple return statements,
; so the epilog can be present multiple times.
%macro EPILOG 0

    ; Back-up stack_off.
    %assign stack_off_bak stack_off

    ; Clear the high YMM registers.
    %if _ym
        vzeroupper
    %endif

    ; Restore the XMM registers.
    %ifdef ARCH_X64
    %assign _reg_count _fa + _tv - 6
    %if _reg_count >= 0
        ; Load the registers.
        %assign _iter 0
        %rep _reg_count
            %assign _reg_idx 6 + _iter
            movdqa x%[_reg_idx], [gs + _iter*16]
            %assign _iter _iter + 1
        %endrep

        ; Free space.
        add gs, _x64_xmm_save_size
        ADD_TO_STACK_OFF(-_x64_xmm_save_size)
    %endif
    %endif

    ; Restore the GPRs.
    %assign _reg_count _ia + _ti - nb_scratch_gpr
    %assign _reg_idx nb_scratch_gpr + _reg_count - 1
    %rep _reg_count
        POP g%[_reg_idx]
        %assign _reg_idx _reg_idx - 1
    %endrep

    ASSERT(stack_off == 0)

    ; Restore stack_off.
    %assign stack_off stack_off_bak

    ; Clean up.
    %undef _iter
    %undef _reg_idx
    %undef _reg_count

%endmacro

; Function declaration + epilog as needed.
%macro DEFFUN 1-*           ; %1: function name, the rest are the prologue arguments.
    DECLARE_FUNCTION %1
    %if %0 > 1
       PROLOGUE %2, %3, %4, %5, %6, %7
    %endif
%endmacro

; Epilog + return.
%macro RET 0
    EPILOG
    ret
%endmacro

; Use RIP-relative addressing by default.
default rel

