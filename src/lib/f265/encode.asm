; Copyright (c) 2014, VANTRIX CORPORATION. All rights reserved. See LICENSE.txt
; for the full license text.

%include "x86inc.asm"

section .data

DECLARE_EXTERN f265_scan_map_data
DECLARE_EXTERN f265_scan_map_idx

align 4
pat_quant_dw_1:     dd 1
pat_dequant_dw_1:   dw 1,1
pat_sb_shuf_8:      db 3,1,2,0, 3,2,1,0, 3,1,2,0
pat_b_127:          times 4 db 127

align 8
pat_sb_neigh_32:    dq 0x7f7f7f7f7f7f7f7f

align 16
pat_sb_shuf_16:     db 15, 11, 14, 7, 10, 13, 3, 6, 9, 12, 2, 5, 8, 1, 4, 0
                    db 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
                    db 15, 11, 7, 3, 14, 10, 6, 2, 13, 9, 5, 1, 12, 8, 4, 0
pat_sb_merge_32:    dd 0, 4, 1, 5, 2, 6, 3, 7
; For some reason, the pattern for the second register is identical to the
; first, so we keep only the first.
pat_sb_shuf_32:     db 0x0f,0x07,0x0e,0x06,0x0d,0x05,0x0c,0x04,0x0b,0x03,0x0a,0x02,0x09,0x01,0x08,0x00 ; pshufb
                    db 0x0f,0x07,0x0e,0x06,0x0d,0x05,0x0c,0x04,0x0b,0x03,0x0a,0x02,0x09,0x01,0x08,0x00
                    db 0x0f,0x0e,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00
                    db 0x0f,0x0e,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00
                    db 0x0f,0x07,0x0e,0x06,0x0d,0x05,0x0c,0x04,0x0b,0x03,0x0a,0x02,0x09,0x01,0x08,0x00
                    db 0x0f,0x07,0x0e,0x06,0x0d,0x05,0x0c,0x04,0x0b,0x03,0x0a,0x02,0x09,0x01,0x08,0x00
                    dq 0xecc6183030200000 ; pdep
                    dq 0x131860c0c0c18400
                    dq 0x00218303030618c8
                    dq 0x0000040c0c186337
                    dq 0xffff000000000000
                    dq 0x0000ffff00000000
                    dq 0x00000000ffff0000
                    dq 0x000000000000ffff
                    dq 0xc0c0c0c0c0c0c0c0
                    dq 0x3030303030303030
                    dq 0x0c0c0c0c0c0c0c0c
                    dq 0x0303030303030303
pat_pp_null_sb:     dq 0x1, 0x8, 0x8000, 0x8000000000000000
pat_pp_reorder:     dw 0xffff,0xffff,0xffff,0x0f0e,0xffff,0xffff,0x0706,0x0d0c
                    dw 0x0302,0x0908,0xffff,0xffff,0x0100,0xffff,0xffff,0xffff
                    dw 0xffff,0xffff,0x0504,0x0b0a,0xffff,0x0302,0x0908,0x0100
                    dw 0x0f0e,0x0706,0x0d0c,0xffff,0x0504,0x0b0a,0xffff,0xffff
                    dw 0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff
                    dw 0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff
                    dw 0x0f0e,0x0d0c,0x0b0a,0x0908,0x0706,0x0504,0x0302,0x0100
                    dw 0x0f0e,0x0d0c,0x0b0a,0x0908,0x0706,0x0504,0x0302,0x0100
                    dw 0xffff,0xffff,0x0f0e,0x0706,0xffff,0xffff,0x0d0c,0x0504
                    dw 0x0b0a,0x0302,0xffff,0xffff,0x0908,0x0100,0xffff,0xffff
                    dw 0xffff,0xffff,0x0b0a,0x0302,0xffff,0xffff,0x0908,0x0100
                    dw 0x0f0e,0x0706,0xffff,0xffff,0x0d0c,0x0504,0xffff,0xffff


section .text
; ---------------------- QUANT/DEQUANT macros ---------------------
%macro MULTIPLY 3                           ; %1: tmp register, %2: src 0, %3: src 1.
    ; For QUANT: Interleaving gives - input, Add >> (Shift - 12) | input <n>, Add >> (Shift - 12).
    ; For DEQUANT: Interleaving gives - 1, input | 1, input.
    vpunpckhwd      y5, %2, %3
    vpunpcklwd      %1, %2, %3
    ; For QUANT: Multiplication gives- (input*mult) + ((Add >> (Shift - 12))*(1 << (Shift - 12))).
    ; For DEQUANT: Multiplication gives- (add + input*mult).
    vpmaddwd        y5, y5, y1              ; Multiply.
    vpmaddwd        %1, %1, y1
    vpsrad          y5, y5, x3              ; Shift.
    vpsrad          %1, %1, x3
    vpackssdw       %1, %1, y5              ; Pack to 16-bits.
%endmacro

; QUANT 4x4.
;
; Input parameters:
; - g0:     destination.
; - g1:     source.
; - g2:     bs.
; - g3:     multiplication value.
; - g4:     addition value.
; - g5:     shift value.
DEFFUN f265_lbd_quant_4_avx2, ia=6, at=884444, ti=0, tv=6, ym=1

    vmovdqu         y0, [g1]                ; Input.
    vmovd           x1, g3d                 ; Multiplication Factor.
    vmovd           x2, g4d                 ; Addition Factor.
    vmovd           x3, g5d                 ; Shift Factor.

    vpabsw          y4, y0                  ; Absolute value of input.
    vpbroadcastd    y1, x1
    vpbroadcastd    y2, x2
    ; We need to do the following multiplication: mult*input. However, since we use "vpmaddwd", the instruction does
    ; mult*input + x*y. Either x or y should be 0, and the other (y or x respectively) can be any value.
    ; In the instruction "vpbroadcastd  y1, x1", alternate 16-bit words of y1 are 0. Hence, we require that alternate
    ; 16-bit words of the input are redundant (to do mult*input + 0*y), and hence, we interleave with any register.
    vpunpckhwd      y5, y4, y0              ; Interleave to have the required input values on every alternate word.
    vpunpcklwd      y4, y4, y0
    vpmaddwd        y4, y4, y1              ; Multiply. The multiplication factor fits in 16 bits.
    vpmaddwd        y5, y5, y1
    vpaddd          y4, y4, y2              ; Add bias.
    vpaddd          y5, y5, y2
    vpsrad          y4, y4, x3              ; Shift.
    vpsrad          y5, y5, x3
    vpackssdw       y4, y4, y5              ; Pack to 16-bits.
    vpsignw         y5, y4, y0              ; Restore the sign.
    vmovdqu         [g0], y5                ; Store.
    ; Now, compute the non-zero flag: return 1 if there are coefficients, and 0 if no coefficients.
    mov             ga, 0                   ; Initialize return value to 0.
    vpcmpeqd        y1, y1                  ; Load '1' in all bits.
    vptest          y5, y1
    setnz           gab                     ; Coefficients present.
    RET

; DEQUANT 4x4.
;
; Input parameters:
; - g0:     destination.
; - g1:     source.
; - g2:     bs.
; - g3:     multiplication value.
; - g4:     addition value.
; - g5:     shift value.
DEFFUN f265_lbd_dequant_4_avx2, ia=6, at=884444, ti=0, tv=6, ym=1

    vmovd           x1, g3d                 ; Multiplication Factor.
    vmovd           x2, g4d                 ; Addition Factor.
    vmovdqu         y0, [g1]                ; Input.
    vpbroadcastd    y4, [pat_dequant_dw_1]

    vpbroadcastw    y1, x1                  ; Multiplier and bias both fit in 16 bits.
    vpbroadcastw    y2, x2
    vmovd           x3, g5d                 ; Shift Factor.
    vpblendw        y1, y1, y2, 0xAA        ; Interleave: add, mult | add, mult.
    MULTIPLY        y4, y0, y4              ; Interleave:  1, input | 1, input.  Multiply: (add + input*mult).
    vmovdqu         [g0], y4                ; Store.
    RET

; QUANT 32x32, 16x16 and 8x8.
;
; Input parameters:
; - g0:     destination.
; - g1:     source.
; - g2:     bs.
; - g3:     multiplication value.
; - g4:     addition value.
; - g5:     shift value.
DECLARE_FUNCTION f265_lbd_quant_32_avx2
DECLARE_FUNCTION f265_lbd_quant_16_avx2
DEFFUN f265_lbd_quant_8_avx2, ia=6, at=884444, ti=0, tv=7, ym=1

    vmovd           x3, g5d                 ; Shift Factor.
    vmovd           x1, g3d                 ; Multiplication Factor.
    sub             g5, 12                  ; Shift - 12.
    vmovd           x2, g4d                 ; Addition Factor.
    imul            g2, g2                  ; bs*bs.
    vpbroadcastd    y4, [pat_quant_dw_1]    ; Constant 1.
    vpbroadcastw    y1, x1
    shl             g2, 1
    vmovd           x5, g5d
    vpxor           y6, y6
    vpslld          y4, y4, x5              ; 1 << (Shift - 12).
    vpsrad          y2, y2, x5              ; Add >> (Shift - 12).
    xor             ga, ga                  ; Initialize return value to 0.
    xor             g3, g3
    vpblendw        y1, y1, y4, 0x55        ; Mult, 1 << (Shift - 12) | Mult, 1 << (Shift - 12).
    vpbroadcastw    y2, x2

    .loop_quant:
    vmovdqu         y0, [g1 + g3]           ; Input:  'n+1' | 'n'.
    vpabsw          y4, y0                  ; Absolute value of input.
    ; Interleaving:
    ; Input, Add >> (Shift - 12) | Input, Add >> (Shift - 12).
    ; Multiply: (input*mult) + (Add >> (Shift - 12)*1 << (Shift - 12)).
    MULTIPLY        y4, y2, y4
    vpsignw         y5, y4, y0              ; Restore the sign.
    vmovdqu         [g0 + g3], y5           ; Store.
    add             g3, 32
    vpor            y6, y6, y5
    cmp             g3, g2
    jnz             .loop_quant
    ; Now, compute the non-zero flag: return 1 if there are coefficients, and 0 if no coefficients.
    vpcmpeqd        y4, y4                  ; Load '1' in all bits.
    vptest          y6, y4
    setnz           gab                     ; Coefficients present.
    RET

; DEQUANT 16x16 and 8x8.
;
; Input parameters:
; - g0:     destination.
; - g1:     source.
; - g2:     bs.
; - g3:     multiplication value.
; - g4:     addition value.
; - g5:     shift value.
DECLARE_FUNCTION f265_lbd_dequant_32_avx2
DECLARE_FUNCTION f265_lbd_dequant_16_avx2
DEFFUN f265_lbd_dequant_8_avx2, ia=6, at=884444, ti=0, tv=6, ym=1

    vmovd           x1, g3d                 ; Multiplication Factor.
    vpbroadcastd    y4, [pat_dequant_dw_1]
    vmovd           x2, g4d                 ; Addition Factor.
    vpbroadcastw    y1, x1                  ; Multiplier fits in 16 bits.
    imul            g2, g2                  ; bs*bs.
    vpbroadcastw    y2, x2                  ; Bias fits in 16 bits.
    vpblendw        y1, y1, y2, 0xAA        ; Interleave: add, mult | add, mult.
    vmovd           x3, g5d                 ; Shift Factor.
    shl             g2, 1
    xor             g3, g3

    .loop_dequant:
    vmovdqu         y0, [g1 + g3]           ; Input.
    MULTIPLY        y2, y0, y4              ; Interleave:  1, input | 1, input. Multiply: (add + input*mult).
    vmovdqu         [g0 + g3], y2           ; Store.
    add             g3, 32
    cmp             g3, g2
    jnz             .loop_dequant
    RET

%unmacro CALC_LOOP_ITER 1
%unmacro MULTIPLY 3


; void fenc_get_sb_flags(f265_tb_enc *tb, int16_t *qc)
; Input parameters:
; - g0:     tb.
; - g1:     qc.
DEFFUN f265_lbd_get_sb_flags_4_avx2, ia=2, at=88, ti=0, tv=0, ym=0

    ; Assume there is one non-zero subblock.
    mov             qword [g0 + 8], 1
    mov             qword [g0 + 16], 0
    mov             qword [g0 + 24], 0
    RET

DEFFUN f265_lbd_get_sb_flags_8_avx2, ia=2, at=88, ti=1, tv=2, ym=1

    ; Get one byte per subblock.
    vmovdqu         y0, [g1+0]              ; OR rows together (0-2, 1-3, 4-6, 5-7).
    vpor            y0, [g1+32]
    vmovdqu         y1, [g1+64]
    vpor            y1, [g1+96]
    vpacksswb       y0, y1                  ; Collapse 8-byte subblock to 1-byte subblock.
    vpacksswb       y0, y0
    vpacksswb       y0, y0
    vextracti128    x1, y0, 1               ; OR with high lane.
    vpor            y0, y1
    vpxor           y1, y1                  ; Load zero.
    vpcmpeqb        y0, y1                  ; 0xff if subblock is non-zero.
    vpcmpeqb        y0, y1

    ; Shuffle the subblocks in encoding order.
    movzx           g1, byte [g0+1]         ; Load order.
    lea             g2, [pat_sb_shuf_8]     ; Shuffle.
    vpshufb         y1, y0, [g2 + 4*g1]

    ; Get the subblock flags.
    vpmovmskb       g1d, y0                 ; Raster order.
    and             g1, 0xf
    vpmovmskb       g2d, y1                 ; Encoding order.
    and             g2, 0xf
    mov             [g0+8], g2              ; Store in encoding order.

    ; Get the neighbour flags.
    mov             g2, g1                  ; Preserve.
    shr             g1, 1                   ; Move the columns left by 1.
    and             g1, 5                   ; Remove the non-existing column.
    mov             [g0+16], g1             ; Store right.
    shr             g2, 2                   ; Move the rows up by 1.
    mov             [g0+24], g2             ; Store bottom.
    RET

DEFFUN f265_lbd_get_sb_flags_16_avx2, ia=2, at=88, ti=1, tv=3, ym=1

    ; Get one byte per subblock.
    %macro LOAD_ROW 2                       ; %1: output, %2: true if g1 must be incremented.
    vmovdqu         %1, [g1+0]              ; OR rows together.
    vpor            %1, [g1+32]
    vpor            %1, [g1+64]
    vpor            %1, [g1+96]
    %if %2
    sub             g1, -128                ; Increment g1 to minimize overall code size.
    %endif
    %endmacro
    LOAD_ROW        y0, 1
    LOAD_ROW        y1, 1
    vpacksswb       y0, y1                  ; Collapse.
    LOAD_ROW        y1, 1
    LOAD_ROW        y2, 0
    vpacksswb       y1, y2
    vpacksswb       y0, y1
    vpacksswb       y0, y0
    vextracti128    x1, y0, 1               ; Unpack with high lane.
    vpunpcklwd      y0, y1
    vpxor           y1, y1                  ; Load zero.
    vpcmpeqb        y0, y1                  ; 0xff if subblock is non-zero.
    vpcmpeqb        y0, y1
    %unmacro LOAD_ROW 2

    ; Shuffle the subblocks in encoding order.
    movzx           g1, byte [g0+1]
    shl             g1, 4
    lea             g2, [pat_sb_shuf_16]
    vpshufb         y1, y0, [g2+g1]

    ; Get the subblock flags.
    vpmovmskb       g1d, x0
    vpmovmskb       g2d, x1
    mov             [g0+8], g2

    ; Get the neighbour flags.
    mov             g2, g1
    shr             g1, 1
    and             g1, 0x7777
    mov             [g0+16], g1
    shr             g2, 4
    mov             [g0+24], g2
    RET

DEFFUN f265_lbd_get_sb_flags_32_avx2, ia=2, at=88, ti=3, tv=6, ym=1

    ; Get one byte per subblock. Final output in y2 and y3.
    vmovdqu         y5, [pat_sb_merge_32]   ; Load row merge pattern.
    %macro LOAD_REG 2                       ; %1: output, %2: tmp.
    call .load_sb_row                       ; Load and merge every row.
    vmovdqu         %1, y0                  ; Preserve.
    call .load_sb_row
    vpacksswb       %1, %1, y0              ; Pack and reorder.
    vpermd          %1, y5, %1
    call .load_sb_row
    vmovdqu         %2, y0
    call .load_sb_row
    vpacksswb       %2, %2, y0
    vpermd          %2, y5, %2
    vpacksswb       %1, %1, %2
    vpermq          %1, %1, 0xd8
    %endmacro
    LOAD_REG        y2, y3
    LOAD_REG        y3, y4
    %unmacro LOAD_REG 2
    vpxor           y0, y0                  ; 0xff if subblock is non-zero.
    vpcmpeqb        y2, y0
    vpcmpeqb        y2, y0
    vpcmpeqb        y3, y0
    vpcmpeqb        y3, y0

    ; Shuffle the subblocks in encoding order.
    movzx           g1, byte [g0+1]
    shl             g1, 5
    lea             g3, [pat_sb_shuf_32]    ; Pshufb base.
    lea             g2, [g3+g1+32*3]        ; Pdep pointer.
    vpshufb         y0, y2, [g3+g1]         ; Reorder within each lane.
    vpshufb         y1, y3, [g3+g1]
    vpmovmskb       g1, y0                  ; Reorder using the lane bitfields.
    pdep            g4, g1, [g2]            ; Low 16-bit, first register.
    shr             g1, 16                  ; High 16-bit.
    pdep            g3, g1, [g2+8]
    or              g4, g3
    vpmovmskb       g1, y1                  ; Second register.
    pdep            g3, g1, [g2+16]
    or              g4, g3
    shr             g1, 16
    pdep            g3, g1, [g2+24]
    or              g4, g3
    mov             [g0+8], g4              ; Store in encoding order.

    ; Get the neighbour flags.
    vpmovmskb       g1, y2                  ; Combine into a single bitfield.
    vpmovmskb       g2, y3
    shl             g2, 32
    or              g1, g2
    mov             g2, g1
    shr             g1, 1
    and             g1, [pat_sb_neigh_32]
    mov             [g0+16], g1
    shr             g2, 8
    mov             [g0+24], g2
    RET

    ; Load one row of subblocks. Output register y0, tmp register y1.
    .load_sb_row:
    vmovdqu         y0, [g1+0*32]
    vmovdqu         y1, [g1+1*32]
    vpor            y0, [g1+2*32]
    vpor            y1, [g1+3*32]
    vpor            y0, [g1+4*32]
    vpor            y1, [g1+5*32]
    vpor            y0, [g1+6*32]
    vpor            y1, [g1+7*32]
    add             g1, 8*32
    vpacksswb       y0, y1
    ret


; void fenc_preprocess_tb(f265_tt_enc *tt, int16_t *qc, uint8_t *spill)
; Input parameters:
; - g0:     tt.
; - g1:     qc.
; - g2:     spill buffer.
;
; We use two passes to minimize latency. The first pass stores the reordered
; coefficients. The second pass processes them.
;
; Register usage, pass 1.
; - ga:     tmp.
; - g0:     tt.
; - g1:     qc.
; - g2:     iteration counter preserved for pass 2.
; - g3:     tmp.
; - g4:     tt->nz_flags[0].
; - g5:     offset map.
; - g6:     stride.
; - g7:     stride*3.
; - g8:     iteration counter.
; - g9:     null subblock flag.
; - y0-1:   tmp.
; - y2-3:   patterns.
;
; Register usage, pass 2.
; - ga:     tmp.
; - g0:     tt.
; - g1:     tmp.
; - g2:     iteration counter.
; - g3:     nz.
; - g4:     gt1.
; - g5:     gt2.
; - g6:     tt->sb.
; - g7:     tt->levels.
; - g8:     iteration end.
; - y0-2:   tmp.
; - y3-8:   patterns.
;
; TODO: add special case for 4x4, possibly from the function dispatcher to avoid a branch.
DEFFUN f265_lbd_preprocess_tb_avx2, ia=3, at=888, ti=7, tv=9, ym=1

    ; Initialize first pass.
    mov             g3, [g0+8]              ; tt->tb.
    mov             g4, [g3+8]              ; tb->nz_flags[0].
    movzx           ga, byte [g3+0]         ; tb->lg_bs.
    movzx           g3, byte [g3+1]         ; tb->order.
    lea             g5, [pat_pp_null_sb]    ; Remember whether there is a null subblock.
    xor             g9, g9
    test            g4, [g5+8*ga-16]
    setz            g9b
    mov             g6, 2                   ; Stride.
    shlx            g6, g6, ga
    lea             g7, [3*g6]              ; 3*stride.
    lea             g5, [f265_scan_map_idx] ; Coefficient offset map.
    lea             ga, [3*ga-3*2]          ; 3*(lg_bs-2).
    add             ga, g5
    movzx           ga, byte [ga+g3]
    lea             g5, [f265_scan_map_data + 256]
    add             g5, ga
    lea             ga, [pat_pp_reorder]    ; Reorder patterns.
    shl             g3, 6
    vmovdqu         y2, [ga+g3]
    vmovdqu         y3, [ga+g3+32]
    mov             g8, g2                  ; Iteration counter.

    ; First pass.
    tzcnt           ga, g4                  ; Position of the next subblock.
    .loop_pass1:
    movzx           g3, byte [g5+ga]        ; Subblock offset.
    blsi            ga, g4                  ; Update the non-zero bitfield.
    xor             g4, ga

    lea             ga, [g1 + 8*g3]         ; Load the coefficients.
    vmovdqu         y0, [ga]
    vpunpcklqdq     y0, [ga + g6]
    vmovdqu         y1, [ga + 2*g6 - 16]    ; Offset to avoid cross-lane merges.
    vpunpcklqdq     y1, [ga + g7 - 16]
    vpblendd        y0, y0, y1, 0xf0

    vpshufb         y1, y0, y3              ; Reorder within lanes, leaving holes for missing values.
    vpshufb         y0, y0, y2
    vpermq          y1, y1, 0x4e
    vpor            y0, y1

    vpacksswb       y1, y0, y0              ; Pack to 8-bit.
    vpermq          y1, y1, 8
    vpabsw          y0, y0                  ; Make the 16-bit levels absolute.
    vmovdqu         [g8], y0                ; Store the 16-bit levels and 8-bit levels aligned (64 bytes).
    vmovdqu         [g8+32], y1
    add             g8, 64

    tzcnt           ga, g4                  ; Pass to the next subblock.
    jnc             .loop_pass1

    ; Initialize second pass.
    mov             g6, [g0+16]             ; tt->sb.
    mov             g7, [g0+24]             ; tt->levels.
    vpbroadcastd    y8, [pat_b_127]         ; 127.
    vmovdqu         y7, [pat_sb_shuf_16+16] ; Reverse order (borrowed from horizontal order).
    vpcmpeqb        y3, y3                  ; -1.
    vpxor           y4, y4                  ; 0.
    vpabsb          y5, y3                  ; 1.
    vpaddb          y6, y5, y5              ; 2.
    add             qword [g0+8], 40        ; tb++.

    ; Second pass.
    .loop_pass2:
    vmovdqu         y0, [g2]                ; 16-bit absolute levels.
    vmovdqu         y1, [g2+32]             ; 8-bit signed levels.
    vpabsb          y2, y1                  ; 8-bit absolute levels.
    vpminub         y2, y8                  ; Convert -128 to 127 to avoid signed issues below.
    add             g2, 64

    vpshufb         y1, y7                  ; sign reverse.
    vpcmpgtb        y1, y1, y3
    vpmovmskb       ga, x1
    not             gaw
    vpcmpgtb        y1, y2, y4              ; nz.
    vpmovmskb       g3, x1
    vpshufb         y1, y7                  ; nz reverse.
    vpmovmskb       g1, x1
    vpcmpgtb        y1, y2, y5              ; gt1.
    vpmovmskb       g4, x1
    vpcmpgtb        y1, y2, y6              ; gt2.
    vpmovmskb       g5, x1

    pext            ga, ga, g1              ; Extract the signs.
    mov             [g6+0], g3w             ; Store the non-zero flags and the signs.
    mov             [g6+2], gaw

    pext            g4, g4, g3              ; Extract the 8 gt1 flags.
    and             g4, 0xff
    mov             [g6+6], g4b             ; Store the gt1 flags.

    pext            g5, g5, g3              ; Extract all gt2 flags.
    blsi            ga, g4                  ; Extract the first gt1 flag set, or 0 if none.
    and             g5, ga                  ; Extract the first gt2 flag, or 0 if none.
    setnz           gab                     ; True if the gt2 flag is 1.
    popcnt          g1, g3                  ; Count the number of non-zero flags.
    shl             ga, 5                   ; Store packed_data.
    or              ga, g1
    mov             [g6+7], gab

    blsi            ga, g4                  ; Extract the first gt1 flag set, or 0 if none.
    xor             ga, g4                  ; Clear the gt2 position in the gt1 flags.
    or              ga, g5                  ; Set the gt2 position in the gt1 flags if the gt2 flag is set.
    or              ga, 0xff00              ; Set the bit of all coefficients that haven't been inferred.
    pdep            ga, ga, g3
    mov             [g6+4], gaw             ; Store the remaining flags.

    vmovdqu         [g7], y0                ; Store the levels tentatively.
    xor             g1, g1                  ; levels++ if there are uninferred coefficients.
    test            ga, ga
    setnz           g1b
    shl             g1, 5
    add             g7, g1

    add             g6, 8                   ; Pass to the next subblock.
    cmp             g2, g8
    jnz             .loop_pass2

    ; Finish.
    mov             qword [g6], 0           ; Add the null subblock as needed.
    lea             g6, [g6+8*g9]
    mov             [g0+16], g6             ; tt->sb.
    mov             [g0+24], g7             ; tt->levels.
    RET

