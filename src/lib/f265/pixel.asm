; Copyright (c) 2014, VANTRIX CORPORATION. All rights reserved. See LICENSE.txt
; for the full license text.

%include "x86inc.asm"

section .data

align 4

pat_b_127:          times 4 db 127
pat_b_2:            times 4 db 2
pat_w_1:            dw 1, 1
pat_w_256:          dw 256, 256
pat_w_512:          dw 512, 512
pat_dw_2048:        dd 2048

align 16
pat_if_h_group:     db 0,1, 1,2, 2,3, 3,4, 4,5, 5,6, 6,7, 7,8
pat_if_luma_8bit:   db -1,4,-1,4, -10,58,-10,58, 17,-5,17,-5, 1,0,1,0
                    db -1,4,-1,4, -11,40,-11,40, 40,-11,40,-11, 4,-1,4,-1
                    db 0,1,0,1, -5,17,-5,17, 58,-10,58,-10, 4,-1,4,-1
pat_if_luma_16bit:  dw -1,4, -10,58, 17,-5, 1,0
                    dw -1,4, -11,40, 40,-11, 4,-1
                    dw 0,1, -5,17, 58,-10, 4,-1
pat_avg_pix_12:     dd 5, 6, 0, 0, 0, 1, 2, 4

section .text

; int sad(f265_pix *src, int src_stride, f265_pix *ref, int ref_stride, int packed_dims)
; Input parameters:
; - g0:     source.
; - g1:     source stride.
; - g2:     reference.
; - g3:     reference stride.
; - g4:     packed_dims.
%macro DEFINE_SAD 1                         ; %1: block width.

; Declare the function. We do just enough work per loop iteration to amortize the
; loop overhead (either two or four rows per iteration).
%if %1 == 32
DEFFUN f265_lbd_fsad_%1_avx2, ia=5, at=84844, ti=2, tv=3, ym=1
%else
DEFFUN f265_lbd_fsad_%1_avx2, ia=5, at=84844, ti=0, tv=3, ym=1
%endif
    ; Initialization.
    vpxor           y0, y0, y0              ; Initialize the SAD accumulator.
    and             g4, 0x7f                ; Get the height.
    %if %1 == 32
    lea             g5, [3*g1]              ; Load 3*stride.
    lea             g6, [3*g3]
    %endif

    ; Loop body before pointer update.
    .loop:
    %if %1 == 64
    vmovdqu         y1, [g0]                ; Load source.
    vpsadbw         y1, y1, [g2]            ; SAD with reference.
    vpaddd          y0, y0, y1              ; Accumulate.

    vmovdqu         y1, [g0 + 32]
    vpsadbw         y1, y1, [g2 + 32]
    vpaddd          y0, y0, y1

    vmovdqu         y1, [g0 + g1]
    vpsadbw         y1, y1, [g2 + g3]
    vpaddd          y0, y0, y1

    vmovdqu         y1, [g0 + g1 + 32]
    vpsadbw         y1, y1, [g2 + g3 + 32]
    vpaddd          y0, y0, y1

    %elif %1 == 32
    vmovdqu         y1, [g0]
    vpsadbw         y1, y1, [g2]
    vpaddd          y0, y0, y1

    vmovdqu         y1, [g0 + g1]
    vpsadbw         y1, y1, [g2 + g3]
    vpaddd          y0, y0, y1

    vmovdqu         y1, [g0 + 2*g1]
    vpsadbw         y1, y1, [g2 + 2*g3]
    vpaddd          y0, y0, y1

    vmovdqu         y1, [g0 + g5]
    vpsadbw         y1, y1, [g2 + g6]
    vpaddd          y0, y0, y1

    %elif %1 == 16
    vmovdqu         y1, [g0]
    vinserti128     y1, y1, [g0 + g1], 1
    vmovdqu         y2, [g2]
    vinserti128     y2, y2, [g2 + g3], 1
    vpsadbw         y1, y1, y2
    vpaddd          y0, y0, y1

    %elif %1 == 8
    vmovq           x1, [g0]
    vmovhps         x1, [g0 + g1]
    vmovq           x2, [g2]
    vmovhps         x2, [g2 + g3]
    vpsadbw         y1, y1, y2
    vpaddd          y0, y0, y1

    %elif %1 == 4
    vmovd           x1, [g0]
    vpunpckldq      x1, x1, [g0 + g1]
    vmovd           x2, [g2]
    vpunpckldq      x2, x2, [g2 + g3]
    vpsadbw         y1, y1, y2
    vpaddd          y0, y0, y1
    %endif

    ; Pointer update.
    %if %1 == 32
    lea             g0, [g0 + 4*g1]
    lea             g2, [g2 + 4*g3]
    sub             g4, 4
    %else
    lea             g0, [g0 + 2*g1]
    lea             g2, [g2 + 2*g3]
    sub             g4, 2
    %endif
    jnz             .loop

    ; Final combination.
    %if %1 >= 16
    vextracti128    x1, y0, 1
    vpaddd          y0, y0, y1
    %endif
    %if %1 >= 8
    vpshufd         y1, y0, 2
    vpaddd          y0, y0, y1
    %endif
    vmovd           gad, x0
    RET
%endmacro
DEFINE_SAD 4
DEFINE_SAD 8
DEFINE_SAD 16
DEFINE_SAD 32
DEFINE_SAD 64
%unmacro DEFINE_SAD 1


; void sadx(int *costs, f265_pix *src, int src_stride, f265_pix **refs, int ref_stride, int packed_dims).
;
; Input parameters:
; - g0:     costs.
; - g1:     source.
; - g2:     source stride.
; - g3:     references.
; - g4:     reference stride.
; - g5:     packed_dims.
;
; Register usage:
; - g0:     costs.
; - g1:     source.
; - g2:     source stride.
; - g3:     even stride accumulator (0, 2*stride, 4*stride, ...)
;           OR single stride accumulator (block sizes 32 and 64).
; - g4:     odd stride accumulator (stride, 3*stride, 5*stride, ...)
;           OR first reference pointer (block sizes 32 and 64).
; - ga:     2*stride OR 1*stride.
; - g5:     height.
; - g6-9:   reference pointers.
; - y0:     scratch register.
; - y1:     source pixels.
; - y2-5:   SAD accumulators.
%macro DEFINE_SAD 2                         ; %1: block width, %2: number of references.

; We process single rows for widths 32 and 64.
; We need one extra vector register for SAD4.
%assign nbti 2
%assign nbtv 5
%if %1 <= 16
%assign nbti nbti+1
%endif
%if %2 == 4
%assign nbti nbti+1
%assign nbtv nbtv+1
%endif
DEFFUN f265_lbd_sad%2_%1_avx2, ia=6, at=884844, ti=nbti, tv=nbtv, ym=1
    ; 16 and below.
    %if %1 <= 16
    mov             g6, [g3]                ; Load the reference pointers.
    mov             g7, [g3+8]
    mov             g8, [g3+16]
    %if %2 == 4
    mov             g9, [g3+24]
    %endif
    lea             ga, [2*g4]              ; Load the stride increment.

    ; 32 and above.
    %else
    mov             ga, g4                  ; Load the stride increment.
    mov             g4, [g3]
    mov             g6, [g3+8]
    mov             g7, [g3+16]
    %if %2 == 4
    mov             g8, [g3+24]
    %endif
    %endif

    xor             g3, g3                  ; Initialize the stride counter.
    and             g5, 0xff                ; Extract the height.
    vpxor           y2, y2, y2              ; Initialize the SAD accumulators.
    vpxor           y3, y3, y3
    vpxor           y4, y4, y4
    %if %2 == 4
    vpxor           y5, y5, y5
    %endif

    .loop:                                  ; Process 1 or 2 rows per loop iteration.

    ; Width 64.
    %if %1 == 64
    vmovdqu         y1, [g1]
    vpsadbw         y0, y1, [g4 + g3]
    vpaddd          y2, y2, y0
    vpsadbw         y0, y1, [g6 + g3]
    vpaddd          y3, y3, y0
    vpsadbw         y0, y1, [g7 + g3]
    vpaddd          y4, y4, y0
    %if %2 == 4
    vpsadbw         y0, y1, [g8 + g3]
    vpaddd          y5, y5, y0
    %endif

    vmovdqu         y1, [g1 + 32]
    vpsadbw         y0, y1, [g4 + g3 + 32]
    vpaddd          y2, y2, y0
    vpsadbw         y0, y1, [g6 + g3 + 32]
    vpaddd          y3, y3, y0
    vpsadbw         y0, y1, [g7 + g3 + 32]
    vpaddd          y4, y4, y0
    %if %2 == 4
    vpsadbw         y0, y1, [g8 + g3 + 32]
    vpaddd          y5, y5, y0
    %endif

    ; Width 32.
    %elif %1 == 32
    vmovdqu         y1, [g1]

    vpsadbw         y0, y1, [g4 + g3]
    vpaddd          y2, y2, y0

    vpsadbw         y0, y1, [g6 + g3]
    vpaddd          y3, y3, y0

    vpsadbw         y0, y1, [g7 + g3]
    vpaddd          y4, y4, y0

    %if %2 == 4
    vpsadbw         y0, y1, [g8 + g3]
    vpaddd          y5, y5, y0
    %endif

    ; Width 16.
    %elif %1 == 16
    vmovdqu         y1, [g1]
    vinserti128     y1, y1, [g1 + g2], 1

    vmovdqu         y0, [g6 + g3]
    vinserti128     y0, y0, [g6 + g4], 1
    vpsadbw         y0, y0, y1
    vpaddd          y2, y2, y0

    vmovdqu         y0, [g7 + g3]
    vinserti128     y0, y0, [g7 + g4], 1
    vpsadbw         y0, y0, y1
    vpaddd          y3, y3, y0

    vmovdqu         y0, [g8 + g3]
    vinserti128     y0, y0, [g8 + g4], 1
    vpsadbw         y0, y0, y1
    vpaddd          y4, y4, y0

    %if %2 == 4
    vmovdqu         y0, [g9 + g3]
    vinserti128     y0, y0, [g9 + g4], 1
    vpsadbw         y0, y0, y1
    vpaddd          y5, y5, y0
    %endif

    ; Width 8.
    %elif %1 == 8
    vmovq           x1, [g1]
    vmovhps         x1, [g1 + g2]

    vmovq           x0, [g6 + g3]
    vmovhps         x0, [g6 + g4]
    vpsadbw         y0, y0, y1
    vpaddd          y2, y2, y0

    vmovq           x0, [g7 + g3]
    vmovhps         x0, [g7 + g4]
    vpsadbw         y0, y0, y1
    vpaddd          y3, y3, y0

    vmovq           x0, [g8 + g3]
    vmovhps         x0, [g8 + g4]
    vpsadbw         y0, y0, y1
    vpaddd          y4, y4, y0

    %if %2 == 4
    vmovq           x0, [g9 + g3]
    vmovhps         x0, [g9 + g4]
    vpsadbw         y0, y0, y1
    vpaddd          y5, y5, y0
    %endif

    ; Width 4.
    %elif %1 == 4

    vmovd           x1, [g1]
    vpunpckldq      x1, x1, [g1 + g2]

    vmovd           x0, [g6 + g3]
    vpunpckldq      x0, x0, [g6 + g4]
    vpsadbw         y0, y0, y1
    vpaddd          y2, y2, y0

    vmovd           x0, [g7 + g3]
    vpunpckldq      x0, x0, [g7 + g4]
    vpsadbw         y0, y0, y1
    vpaddd          y3, y3, y0

    vmovd           x0, [g8 + g3]
    vpunpckldq      x0, x0, [g8 + g4]
    vpsadbw         y0, y0, y1
    vpaddd          y4, y4, y0

    %if %2 == 4
    vmovd           x0, [g9 + g3]
    vpunpckldq      x0, x0, [g9 + g4]
    vpsadbw         y0, y0, y1
    vpaddd          y5, y5, y0
    %endif

    %endif

    %if %1 <= 16
    lea             g1, [g1 + 2*g2]         ; Pointer update.
    add             g3, ga
    add             g4, ga
    sub             g5, 2
    %else
    add             g1, g2
    add             g3, ga
    sub             g5, 1
    %endif
    jnz             .loop

    ; Width 8 and above.
    %if %1 >= 8
    vpsllq          y3, y3, 32              ; Interleave costs 0 and 1.
    vpor            y2, y2, y3
    %if %2 == 4
    vpsllq          y5, y5, 32              ; Interleave costs 2 and 3.
    vpor            y4, y4, y5
    %endif

    %if %1 >= 16
    vperm2i128      y1, y2, y4, 0x20        ; Combine lanes.
    vperm2i128      y2, y2, y4, 0x31
    vpaddd          y2, y1, y2
    %else
    vinserti128     y2, y2, x4, 1
    %endif
    vpshufd         y1, y2, 14              ; Combine low and high (xxxx 1110 == 14).
    vpaddd          y2, y1, y2
    vpermq          y2, y2, 8               ; Put quadwords 0 and 2 together (xxxx 1000 == 8).

    ; Width 4.
    %else
    vpunpckldq      y2, y2, y3
    %if %2 == 4
    vpunpckldq      y4, y4, y5
    %endif
    vpunpcklqdq     y2, y2, y4
    %endif

    vmovdqu         [g0], x2                ; Store.
    RET
%endmacro
DEFINE_SAD 4, 3
DEFINE_SAD 4, 4
DEFINE_SAD 8, 3
DEFINE_SAD 8, 4
DEFINE_SAD 16, 3
DEFINE_SAD 16, 4
DEFINE_SAD 32, 3
DEFINE_SAD 32, 4
DEFINE_SAD 64, 3
DEFINE_SAD 64, 4
%unmacro DEFINE_SAD 2


; int fenc_fssd(f265_pix *src0, int stride0, f265_pix *src1, int stride1, int width)
; Input parameters:
; - g0:     source 0.
; - g1:     source 0 stride.
; - g2:     source 1.
; - g3:     source 1 stride.
; - g4:     width.
%macro DEFINE_SSD 1                         ; %1: width.

DEFFUN f265_lbd_fssd_%1_avx2, ia=4, at=8484, ti=0, tv=6, ym=1

    ; Initialize.
    vpbroadcastd    y4, [pat_b_127]         ; Maximum pixel value.
    vpbroadcastd    y5, [pat_w_1]           ; Multiply-add two 16-bit values.
    %if %1 >= 16
    vpxor           y0, y0                  ; Accumulator.
    %endif

    %macro SSD_REG 6                        ; %1: acc, %2: src0, %3: src1, %4: tmp, %5: 16-bit pat, %6: 32-bit pat.
    vpsubusb        y%4, y%2, y%3           ; unsigned_saturate(A-B)|unsigned_saturate(B-A).
    vpsubusb        y%2, y%3, y%2
    vpor            y%2, y%4
    vpminub         y%2, y%5                ; Avoid overflows.
    vpmaddubsw      y%2, y%2                ; Compute 16-bit SSDs (sum of two signed 14-bit values fit in 16-bit).
    vpmaddwd        y%2, y%6                ; Sum the 32-bit SSDs.
    %if %1 != %2
    vpaddd          y%1, y%2                ; Accumulate.
    %endif
    %endmacro

    ; Width 4.
    %if %1 == 4
    vmovdqu         y0, [g0]
    vpunpckldq      y0, [g0+g1]
    vmovdqu         y2, [g2]
    vpunpckldq      y2, [g2+g3]
    lea             g0, [g0+2*g1-4]
    lea             g2, [g2+2*g3]
    vmovdqu         y1, [g0]
    vpunpckldq      y1, [g0+g1]
    vmovdqu         y3, [g2]
    vpunpckldq      y3, [g2+g3]
    vpblendd        y0, y1, 0x0c
    vpunpcklqdq     y2, y3
    SSD_REG         0, 0, 2, 1, 4, 5

    ; Width 8.
    %elif %1 == 8
    %macro ONE_PASS 3                       ; %1: src0, %2: src1, %3: tmp.
    vmovdqu         y%1, [g0]
    vpunpcklqdq     y%1, [g0+g1]
    vmovdqu         y%2, [g2]
    vpunpcklqdq     y%2, [g2+g3]
    lea             g0, [g0+2*g1-16]
    lea             g2, [g2+2*g3-16]
    vmovdqu         y%3, [g0]
    vpunpcklqdq     y%3, [g0+g1]
    vpblendd        y%1, y%3, 0xf0
    vmovdqu         y%3, [g2]
    vpunpcklqdq     y%3, [g2+g3]
    vpblendd        y%2, y%3, 0xf0
    SSD_REG         %1, %1, %2, %3, 4, 5
    %endmacro
    ONE_PASS        0, 1, 2
    lea             g0, [g0+2*g1+16]
    lea             g2, [g2+2*g3+16]
    ONE_PASS        1, 2, 3
    vpaddd          y0, y1
    %unmacro ONE_PASS 3

    ; Width 16.
    %elif %1 == 16
    mov             ga, 8                   ; Loop counter.
    .loop:
    vmovdqu         y1, [g0]
    vpblendd        y1, [g0+g1-16], 0xf0
    vmovdqu         y2, [g2]
    vpblendd        y2, [g2+g3-16], 0xf0
    lea             g0, [g0+2*g1]
    lea             g2, [g2+2*g3]
    SSD_REG         0, 1, 2, 3, 4, 5
    sub             ga, 1
    jnz             .loop

    ; Width 32.
    %elif %1 == 32
    mov             ga, 16                  ; Loop counter.
    .loop:
    vmovdqu         y1, [g0]
    vmovdqu         y2, [g2]
    SSD_REG         0, 1, 2, 3, 4, 5
    vmovdqu         y1, [g0+g1]
    vmovdqu         y2, [g2+g3]
    SSD_REG         0, 1, 2, 3, 4, 5
    lea             g0, [g0+2*g1]
    lea             g2, [g2+2*g3]
    sub             ga, 1
    jnz             .loop

    ; Width 64.
    %elif %1 == 64
    mov             ga, 64                  ; Loop counter.
    .loop:
    vmovdqu         y1, [g0]
    vmovdqu         y2, [g2]
    SSD_REG         0, 1, 2, 3, 4, 5
    vmovdqu         y1, [g0+32]
    vmovdqu         y2, [g2+32]
    SSD_REG         0, 1, 2, 3, 4, 5
    add             g0, g1
    add             g2, g3
    sub             ga, 1
    jnz             .loop
    %endif

    ; Combine.
    vpshufd         y1, y0, 0xe
    vpaddd          y0, y1
    vpshufd         y1, y0, 1
    vpaddd          y0, y1
    %if %1 != 4
    vpermq          y1, y0, 2
    vpaddd          y0, y1
    %endif
    vmovd           gad, x0
    RET
    %unmacro SSD_REG 6

%endmacro
DEFINE_SSD 4
DEFINE_SSD 8
DEFINE_SSD 16
DEFINE_SSD 32
DEFINE_SSD 64
%unmacro DEFINE_SSD 1


; void fenc_avg_pix(f265_pix *dst, f265_pix *src0, int src0_stride, f265_pix *src1, int src1_stride, int packed_dims)
; Input parameters:
; - g0:     destination.
; - g1:     source 0.
; - g2:     source 0 stride.
; - g3:     source 1.
; - g4:     source 1 stride.
; - g5:     packed_dims.
%macro DEFINE_AVG_PIX 1                     ; %1: width.

%assign nbti 0
%if %1 <= 16 || %1 == 32
%assign nbti 2
%endif

DEFFUN f265_lbd_avg_pix_%1_avx2, ia=6, at=884844, ti=nbti, tv=3, ym=1
    ; Initialize.
    and             g5, 0x7f                ; Height.
    %if %1 <= 16 || %1 == 32
    lea             g6, [3*g2]              ; 3*stride.
    lea             g7, [3*g4]
    %endif
    %if %1 == 12
    vmovdqu          y2, [pat_avg_pix_12]
    %endif

    ; Compute the average.
    .loop:

    ; Width 4.
    %if %1 == 4
    vmovdqu         y0, [g1]
    vpunpckldq      y0, y0, [g1+g2]
    vmovdqu         y1, [g3]
    vpunpckldq      y1, y1, [g3+g4]
    vpavgb          y0, y0, y1
    vmovq           [g0], x0

    vmovdqu         y0, [g1+2*g2]
    vpunpckldq      y0, y0, [g1+g6]
    vmovdqu         y1, [g3+2*g4]
    vpunpckldq      y1, y1, [g3+g7]
    vpavgb          y0, y0, y1
    vmovq           [g0+8], x0

    ; Width 8.
    %elif %1 == 8
    vmovdqu         x0, [g1]
    vmovhps         x0, [g1+g2]
    vmovdqu         x1, [g3]
    vmovhps         x1, [g3+g4]
    vpavgb          y0, y0, y1
    vmovdqu         [g0], x0

    vmovdqu         x0, [g1+2*g2]
    vmovhps         x0, [g1+g6]
    vmovdqu         x1, [g3+2*g4]
    vmovhps         x1, [g3+g7]
    vpavgb          y0, y0, y1
    vmovdqu         [g0+16], x0

    ; Width 16.
    %elif %1 == 16
    vmovdqu         y0, [g1]
    vinserti128     y0, y0, [g1+g2], 1
    vmovdqu         y1, [g3]
    vinserti128     y1, y1, [g3+g4], 1
    vpavgb          y0, y0, y1
    vmovdqu         [g0], y0

    vmovdqu         y0, [g1+2*g2]
    vinserti128     y0, y0, [g1+g6], 1
    vmovdqu         y1, [g3+2*g4]
    vinserti128     y1, y1, [g3+g7], 1
    vpavgb          y0, y0, y1
    vmovdqu         [g0+32], y0

    ; Width 32.
    %elif %1 == 32
    vmovdqu         y0, [g1]
    vpavgb          y0, y0, [g3]
    vmovdqu         [g0], y0

    vmovdqu         y0, [g1+g2]
    vpavgb          y0, y0, [g3+g4]
    vmovdqu         [g0+32], y0

    vmovdqu         y0, [g1+2*g2]
    vpavgb          y0, y0, [g3+2*g4]
    vmovdqu         [g0+64], y0

    vmovdqu         y0, [g1+g6]
    vpavgb          y0, y0, [g3+g7]
    vmovdqu         [g0+96], y0

    ; Width 64.
    %elif %1 == 64
    vmovdqu         y0, [g1]
    vpavgb          y0, y0, [g3]
    vmovdqu         [g0], y0

    vmovdqu         y0, [g1+32]
    vpavgb          y0, y0, [g3+32]
    vmovdqu         [g0+32], y0

    vmovdqu         y0, [g1+g2]
    vpavgb          y0, y0, [g3+g4]
    vmovdqu         [g0+64], y0

    vmovdqu         y0, [g1+g2+32]
    vpavgb          y0, y0, [g3+g4+32]
    vmovdqu         [g0+96], y0

    ; Width 12.
    %elif %1 == 12
    vmovdqu         y0, [g1]
    vinserti128     y0, y0, [g1+g2], 1
    vmovdqu         y1, [g3]
    vinserti128     y1, y1, [g3+g4], 1
    vpavgb          y0, y0, y1
    vpermd          y0, y2, y0              ; Pack 16 bytes high, 8 bytes low.
    vextracti128    [g0], y0, 1
    vmovq           [g0+16], x0

    vmovdqu         y0, [g1+2*g2]
    vinserti128     y0, y0, [g1+g6], 1
    vmovdqu         y1, [g3+2*g4]
    vinserti128     y1, y1, [g3+g7], 1
    vpavgb          y0, y0, y1
    vpermd          y0, y2, y0
    vextracti128    [g0+24], y0, 1
    vmovq           [g0+40], x0

    ; Width 24.
    %elif %1 == 24
    vmovdqu         y0, [g1]
    vpavgb          y0, y0, [g3]
    vmovdqu         [g0], x0
    vextracti128    x0, y0, 1
    vmovq           [g0+16], x0

    vmovdqu         y0, [g1+g2]
    vpavgb          y0, y0, [g3+g4]
    vmovdqu         [g0+24], x0
    vextracti128    x0, y0, 1
    vmovq           [g0+40], x0

    ; Width 48.
    %elif %1 == 48
    vmovdqu         y0, [g1]
    vpavgb          y0, y0, [g3]
    vmovdqu         [g0+0*16], y0

    vmovdqu         y0, [g1+32]
    vpavgb          y0, y0, [g3+32]
    vmovdqu         [g0+2*16], x0

    vmovdqu         y0, [g1+g2]
    vpavgb          y0, y0, [g3+g4]
    vmovdqu         [g0+3*16], y0

    vmovdqu         y0, [g1+g2+32]
    vpavgb          y0, y0, [g3+g4+32]
    vmovdqu         [g0+5*16], x0

    %endif

    ; Update the pointer and loop.
    %if %1 <= 16 || %1 == 32
    add             g0, %1*4
    lea             g1, [g1+4*g2]
    lea             g3, [g3+4*g4]
    sub             g5, 4
    %else
    add             g0, %1*2
    lea             g1, [g1+2*g2]
    lea             g3, [g3+2*g4]
    sub             g5, 2
    %endif
    jnz             .loop
    RET
%endmacro
DEFINE_AVG_PIX 4
DEFINE_AVG_PIX 8
DEFINE_AVG_PIX 16
DEFINE_AVG_PIX 32
DEFINE_AVG_PIX 64
DEFINE_AVG_PIX 12
DEFINE_AVG_PIX 24
DEFINE_AVG_PIX 48
%unmacro DEFINE_AVG_PIX 1


; void fenc_interpol_luma_qpel_pix(f265_pix *dst, int dst_stride, f265_pix *src, int src_stride, int frac,
;                                  int packed_dims, uint8_t *spill)
; Input parameters:
; - g0:     destination.
; - g1:     destination stride.
; - g2:     source.
; - g3:     source stride.
; - g4:     interpolation fraction.
; - g5:     packed dimensions.
; - g6:     spill.

; Luma horizontal interpolation. This macro handles 9 cases.
%macro LUMA_INTERPOL_H 2                    ; %1: width, %2: output ("pix", "s16", "dia").
    %assign WIDTH  %1
    %assign OUTPUT %2

    ; Register usage:
    ; - y0-1:  tmp.
    ; - y2-3:  accumulators.
    ; - y4-7:  grouping patterns.
    ; - y8-11: factors.
    ; - y12:   round (for "pix" only).
    ; - y15:   used for "dia". See the documentation in LUMA_INTERPOL_D.

    ; Initialize.
    %if OUTPUT == "s16"
    add             g1, g1                  ; Double the destination stride for 16-bit stores.
    %endif

    %if OUTPUT == "dia"
    vmovq           x15, g6                 ; Preserve the spill pointer.
    vpinsrd         x15, g4d, 2             ; Preserve the fraction.
    %endif

    and             g4, 3                   ; Get the factor index.
    shl             g4, 4
    lea             ga, [pat_if_luma_8bit - 16] ; Load the factor base.
    lea             ga, [ga + g4]           ; Add the factor index.
    vbroadcasti128  y4, [pat_if_h_group]    ; Load the grouping patterns.
    vpbroadcastd    y0, [pat_b_2]
    vpaddb          y5, y4, y0
    vpaddb          y6, y5, y0
    vpaddb          y7, y6, y0
    vpbroadcastd    y8, [ga + 0]            ; Broadcast the factors.
    vpbroadcastd    y9, [ga + 4]
    vpbroadcastd    y10, [ga + 8]
    vpbroadcastd    y11, [ga + 12]
    %if OUTPUT == "pix"
    vpbroadcastd    y12, [pat_w_512]        ; Load the shift&round factor.
    %endif

    and             g5, 0x7f                ; Get the height.
    %if OUTPUT == "dia"
    vpinsrd         x15, g5d, 3             ; Preserve the height.
    add             g5, 8                   ; Process height+8 rows.
    %endif

    %if OUTPUT != "dia"
    sub             g2, 3                   ; Offset the source at -3.
    %else
    lea             ga, [3*g3 + 3]          ; Offset the source at -3 - 3*stride.
    sub             g2, ga
    %endif

    %if WIDTH != 16 && OUTPUT != "dia"
    lea             ga, [3*g1]              ; 3 * destination stride.
    %endif
    %if WIDTH != 16
    lea             g4, [3*g3]              ; 3 * source stride.
    %endif


    ; Align the pixels with the factors, multiply, and add.
    %macro ADD_FACTORS 5                    ; %1: accumulator, %2: tmp, %3: source, %4: grouping, %5: factor.
    vpshufb         %2, %3, %4
    vpmaddubsw      %2, %2, %5
    %if %1 != %2
    vpaddw          %1, %2
    %endif
    %endmacro

    ; Load a row in each lane and process them.
    %macro PROCESS_PAIR 3                   ; %1: accumulator, %2: first source, %3: second source.
    vmovdqu         y0, [%2]                ; Load the rows at offset -3.
    vinserti128     y0, y0, [%3], 1
    ADD_FACTORS     %1, %1, y0, y4, y8      ; Factors -3,-2.
    ADD_FACTORS     %1, y1, y0, y5, y9      ; Factors -1,0.
    ADD_FACTORS     %1, y1, y0, y6, y10     ; Factors 1,2.
    ADD_FACTORS     %1, y1, y0, y7, y11     ; Factors 3,4.
    %if OUTPUT == "pix"
    vpmulhrsw       %1, %1, y12             ; Add the bias and shift.
    %endif
    %endmacro

    ; Process the rows.
    .loop:

    ; Width 4, dia; width 8, pix.
    %if (WIDTH == 4 && OUTPUT == "dia") || (WIDTH == 8 && OUTPUT == "pix")
    PROCESS_PAIR    y2, g2, g2+2*g3         ; Rows 0 and 2.
    PROCESS_PAIR    y3, g2+g3, g2+g4        ; Rows 1 and 3.

    ; Width 4, pix/s16; width 8, s16/dia.
    %elif WIDTH <= 8
    PROCESS_PAIR    y2, g2, g2+g3           ; Rows 0 and 1.
    PROCESS_PAIR    y3, g2+2*g3, g2+g4      ; Rows 2 and 3.

    ; Width 16, pix.
    %elif WIDTH == 16 && OUTPUT == "pix"
    PROCESS_PAIR    y2, g2, g2+g3           ; Row 0 first half, row 1 first half.
    PROCESS_PAIR    y3, g2+8, g2+g3+8       ; Row 0 second half, row 1 second half.

    ; Width 16, s16/dia.
    %elif WIDTH == 16
    PROCESS_PAIR    y2, g2, g2+8            ; Row 0 first half, row 0 second half.
    PROCESS_PAIR    y3, g2+g3, g2+g3+8      ; Row 1 first half, row 1 second half.
    %endif


    ; Store.

    ; Width 4, pix.
    %if WIDTH == 4 && OUTPUT == "pix"
    vpackuswb       y2, y2, y2              ; Convert to 8-bit in place.
    vpackuswb       y3, y3, y3
    vextracti128    x0, y2, 1
    vextracti128    x1, y3, 1
    vmovd           [g0], x2
    vmovd           [g0 + g1], x0
    vmovd           [g0 + 2*g1], x3
    vmovd           [g0 + ga], x1

    ; Width 4, s16.
    %elif WIDTH == 4 && OUTPUT == "s16"
    vpunpcklqdq     y0, y2, y3              ; Interleave the rows.
    vextracti128    x1, y0, 1
    vmovq           [g0], x0
    vmovq           [g0 + g1], x1
    vmovhps         [g0 + 2*g1], x0
    vmovhps         [g0 + ga], x1

    ; Width 4, dia.
    %elif WIDTH == 4 && OUTPUT == "dia"
    vpunpcklqdq     y0, y2, y3              ; Interleave the rows.
    vmovdqu         [g6], y0
    add             g6, 4*8

    ; Width 8, pix.
    %elif WIDTH == 8 && OUTPUT == "pix"
    vpackuswb       y0, y2, y3              ; Convert to 8-bit and pack the rows together.
    vextracti128    x1, y0, 1
    vmovq           [g0], x0
    vmovhps         [g0 + g1], x0
    vmovq           [g0 + 2*g1], x1
    vmovhps         [g0 + ga], x1

    ; Width 8, s16.
    %elif WIDTH == 8 && OUTPUT == "s16"
    vmovdqu         [g0], x2
    vextracti128    [g0 + g1], y2, 1
    vmovdqu         [g0 + 2*g1], x3
    vextracti128    [g0 + ga], y3, 1

    ; Width 8, dia.
    %elif WIDTH == 8 && OUTPUT == "dia"
    vmovdqu         [g6], y2
    vmovdqu         [g6+2*16], y3
    add             g6, 4*16

    ; Width 16, pix.
    %elif WIDTH == 16 && OUTPUT == "pix"
    vpackuswb       y0, y2, y3              ; Convert to 8-bit and pack the rows together.
    vmovdqu         [g0], x0
    vextracti128    [g0 + g1], y0, 1

    ; Width 16, s16.
    %elif WIDTH == 16 && OUTPUT == "s16"
    vmovdqu         [g0], y2
    vmovdqu         [g0 + g1], y3

    ; Width 16, dia.
    %elif WIDTH == 16 && OUTPUT == "dia"
    vmovdqu         [g6], y2
    vmovdqu         [g6 + 32], y3
    add             g6, 2*32
    %endif


    ; Next loop iteration.

    ; Width 4/8.
    %if WIDTH <= 8
    %if OUTPUT != "dia"
    lea             g0, [g0 + 4*g1]
    %endif
    lea             g2, [g2 + 4*g3]
    sub             g5, 4

    ; Width 16.
    %else
    %if OUTPUT != "dia"
    lea             g0, [g0 + 2*g1]
    %endif
    lea             g2, [g2 + 2*g3]
    sub             g5, 2
    %endif

    ; Loop.
    jnz             .loop
    %if OUTPUT != "dia"
    RET
    %endif
    %unmacro ADD_FACTORS 5
    %unmacro PROCESS_PAIR 3
    %undef WIDTH
    %undef HEIGHT
%endmacro

; Luma vertical interpolation. This macro handles 6 cases.
%macro LUMA_INTERPOL_V 2                    ; %1: width, %2: output ("pix", "s16").
    %assign WIDTH  %1
    %assign OUTPUT %2

    ; The even/odd rows are processed in the low/high lane. We pack as shown
    ; below, then we interleave rows for pmadd.
    ;   1 | 0
    ;   2 | 1
    ;   3 | 2
    ;   4 | 3
    ;
    ; In the comments below, block line 0 refers to the first line of the block.
    ; Row 0 refers to the first pixel row loaded (3 lines above block line 0).

    ; Initialize.
    %if OUTPUT == "s16"
    add             g1, g1                  ; Double the destination stride for 16-bit stores.
    %endif

    shr             g4, 2                   ; Get the factor index.
    shl             g4, 4
    lea             ga, [pat_if_luma_8bit - 16] ; Load the factor base.
    lea             ga, [ga + g4]           ; Add the factor index.
    vpbroadcastd    y1, [ga + 0]            ; Broadcast the factors.
    vpbroadcastd    y2, [ga + 4]
    vpbroadcastd    y3, [ga + 8]
    vpbroadcastd    y4, [ga + 12]
    %if OUTPUT == "pix" && WIDTH <= 8
    vpbroadcastd    y11, [pat_w_512]        ; Load the shift&round factor.
    %elif OUTPUT == "pix"
    vpbroadcastd    y14, [pat_w_512]
    %endif

    and             g5, 0x7f                ; Get the height.

    %if WIDTH <= 8
    lea             ga, [3*g1]              ; 3 * destination stride.
    %endif
    lea             g4, [3*g3]              ; 3 * source stride.
    sub             g2, g4                  ; Offset the source at -3*stride.

    ; Width 4/8.
    %if WIDTH <= 8

    ; Register usage:
    ; - y0:   tmp.
    ; - y1-4: factors.
    ; - y5-6: rows loaded by the loop iteration.
    ; - y7:   next row, incomplete.
    ; - y8-9: accumulators for the current loop iteration.
    ; - y10:  accumulator for the next row iteration.
    ; - y11:  round (for "pix" only).

    ; Broadcast a row and blend it in the previous row.
    %macro LOAD_ROW 4                       ; %1: broadcast dst, %2: blend dst, %3: blend src, %4: load src.
    %if WIDTH == 8
    vpbroadcastq    %1, [%4]                ; Broadcast the row.
    vpblendd        %2, %3, %1, 0xf0        ; Blend in the previous row, high lane.
    %else
    vpbroadcastd    %1, [%4]                ; Broadcast the row.
    vpblendd        %2, %3, %1, 2           ; Blend in the previous row, second dword.
    %endif
    %endmacro

    ; Regroup a pair of rows.
    %macro LOAD_PAIR 5                      ; %1: dst, %2: tmp, %3: outstanding row, %4-5: sources.
    LOAD_ROW        %2, %1, %3, %4          ; Load the first row.
    LOAD_ROW        %3, %2, %2, %5          ; Load the second row.
    vpunpcklbw      %1, %1, %2              ; Interleave both rows.
    %endmacro

    ; Process an interleaved pair of rows. Sum if prev sum is not 0.
    %macro PROCESS_PAIR 5                   ; %1: dst, %2: prev sum, %3: tmp, %4: source, %5: factor.
    vpmaddubsw      %3, %4, %5              ; Multiply.
    %if %2 != 0
    vpaddw          %1, %2, %3              ; Add.
    %endif
    %endmacro

    ; Load rows (0,1) and update the source.
    vmovq           x7, [g2]
    LOAD_PAIR       y5, y0, y7, g2+g3, g2+2*g3
    PROCESS_PAIR    y10, 0, y10, y5, y1     ; Factors 01 for block line 0 (loop unification).
    lea             g2, [g2+g4]

    ; Load rows (2,3) and (4,5) and update the source.
    LOAD_PAIR       y5, y0, y7, g2,      g2+g3
    LOAD_PAIR       y6, y0, y7, g2+2*g3, g2+g4
    lea             g2, [g2+4*g3]

    .loop:

    ; Process rows (2,3) and (4,5).
    PROCESS_PAIR    y8, y10, y8, y5, y2     ; Factors 23 for block line 0 (added to factors 01 above).
    PROCESS_PAIR    y8, y8, y0, y6, y3      ; Factors 45 for block line 0.
    PROCESS_PAIR    y9, 0, y9, y5, y1       ; Factors 01 for block line 2.
    PROCESS_PAIR    y9, y9, y0, y6, y2      ; Factors 23 for block line 2.
    PROCESS_PAIR    y10, 0, y10, y6, y1     ; Factors 01 for block line 4.

    ; Load rows (6,7) and (8,9) and update the source.
    LOAD_PAIR       y5, y0, y7, g2,      g2+g3
    LOAD_PAIR       y6, y0, y7, g2+2*g3, g2+g4
    lea             g2, [g2+4*g3]

    ; Process rows (6,7) and (8,9).
    PROCESS_PAIR    y8, y8, y0, y5, y4      ; Factors 67 for block line 0.
    PROCESS_PAIR    y9, y9, y0, y5, y3      ; Factors 45 for block line 2.
    PROCESS_PAIR    y9, y9, y0, y6, y4      ; Factors 67 for block line 2.

    ; Add the bias, shift and pack.
    %if OUTPUT == "pix"
    vpmulhrsw       y8, y11, y8
    vpmulhrsw       y0, y11, y9
    vpackuswb       y0, y8, y0
    %endif


    ; Store.

    ; Width 4, pix.
    %if WIDTH == 4 && OUTPUT == "pix"
    vmovd           [g0], x0                ; Row 0.
    vpsrlq          x8, x0, 32
    vmovd           [g0+g1], x8             ; Row 1.
    vpshufd         x0, x0, 0x0e
    vmovd           [g0+2*g1], x0           ; Row 2.
    vpsrlq          x8, x0, 32
    vmovd           [g0+ga], x8             ; Row 3.

    ; Width 4, s16.
    %elif WIDTH == 4
    vmovq           [g0], x8                ; Row 0.
    vmovhps         [g0+g1], x8             ; Row 1.
    vmovq           [g0+2*g1], x9           ; Row 2.
    vmovhps         [g0+ga], x9             ; Row 3.

    ; Width 8, pix.
    %elif WIDTH == 8 && OUTPUT == "pix"
    vmovq           [g0], x0                ; Row 0.
    vmovhps         [g0+2*g1], x0           ; Row 2.
    vextracti128    x0, y0, 1
    vmovq           [g0+g1], x0             ; Row 1.
    vmovhps         [g0+ga], x0             ; Row 3.

    ; Width 8, s16.
    %elif WIDTH == 8
    vmovdqu         [g0], x8                ; Row 0.
    vextracti128    [g0+g1], y8, 1          ; Row 1.
    vmovdqu         [g0+2*g1], x9           ; Row 2.
    vextracti128    [g0+ga], y9, 1          ; Row 3.
    %endif


    ; Loop.
    lea             g0, [g0+4*g1]
    sub             g5, 4
    jnz             .loop
    %unmacro LOAD_ROW 4
    %unmacro LOAD_PAIR 5
    %unmacro PROCESS_PAIR 5


    ; Width 16.
    %else

    ; Register usage:
    ; - y0:     tmp.
    ; - y1-4:   factors.
    ; - y5-8:   2 row pairs loaded by the loop iteration.
    ; - y9:     next row, incomplete.
    ; - y10-11: accumulators for the current loop iteration.
    ; - y12-13: accumulators for the next iteration.
    ; - y14:    round (for "pix" only).

    ; Broadcast a row and blend it in the previous row.
    %macro LOAD_ROW 4                       ; %1: broadcast dst, %2: blend dst, %3: blend src, %4: load src.
    vbroadcasti128  %1, [%4]                ; Broadcast the row.
    vpblendd        %2, %3, %1, 0xf0        ; Blend in the previous row, high lane.
    %endmacro

    ; Regroup a pair of rows.
    %macro LOAD_PAIR 6                      ; %1-2 dst, %3: tmp, %4: outstanding row, %5-6: sources.
    LOAD_ROW        %2, %1, %4, %5          ; Load the first row.
    LOAD_ROW        %4, %3, %2, %6          ; Load the second row.
    vpunpckhbw      %2, %1, %3              ; Interleave both rows.
    vpunpcklbw      %1, %1, %3
    %endmacro

    ; Process an interleaved pair of rows. Sum if prev sum is not 0.
    %macro PROCESS_PAIR 8                   ; %1-2: dst, %3-4: prev sums, %5: tmp, %6-7: sources, %8: factor.
    %if %3 == 0
    vpmaddubsw      %1, %6, %8              ; Multiply.
    vpmaddubsw      %2, %7, %8
    %else
    vpmaddubsw      %5, %6, %8              ; Multiply.
    vpaddw          %1, %3, %5              ; Add.
    vpmaddubsw      %5, %7, %8
    vpaddw          %2, %4, %5
    %endif
    %endmacro

    ; Load rows (0,1).
    vmovdqu         x9, [g2]
    LOAD_PAIR       y5,y6, y0, y9, g2+g3, g2+2*g3
    PROCESS_PAIR    y12,y13, 0,0, y0, y5,y6, y1 ; Factors 01 for block line 0 (loop unification).
    lea             g2, [g2+g4]

    ; Load rows (2,3) and (4,5) and update the source.
    LOAD_PAIR       y5,y6, y0, y9, g2, g2+g3
    LOAD_PAIR       y7,y8, y0, y9, g2+2*g3, g2+g4
    lea             g2, [g2+4*g3]

    .loop:

    ; Process rows (2,3) and (4,5).
    PROCESS_PAIR    y10,y11, y12,y13, y0, y5,y6, y2 ; Factors 23 for block line 0 (added to factors 01 above).
    PROCESS_PAIR    y10,y11, y10,y11, y0, y7,y8, y3 ; Factors 45 for block line 0.
    PROCESS_PAIR    y12,y13, 0,0,     y0, y5,y6, y1 ; Factors 01 for block line 2.

    ; Clobber rows (2,3) with (4,5).
    vmovdqu         y5, y7
    vmovdqu         y6, y8

    ; Load rows (6,7) and update the source.
    LOAD_PAIR       y7,y8, y0, y9, g2, g2+g3
    lea             g2, [g2+2*g3]

    ; Process rows (6,7).
    PROCESS_PAIR    y10,y11, y10,y11, y0, y7,y8, y4 ; Factors 67 for block line 0.

    ; Add the bias, shift and pack.
    %if OUTPUT == "pix"
    vpmulhrsw       y10, y14, y10
    vpmulhrsw       y0, y14, y11
    vpackuswb       y0, y10, y0
    %endif


    ; Store.

    ; Width 16, pix.
    %if OUTPUT == "pix"
    vmovdqu         [g0], x0                ; Row 0.
    vextracti128    [g0+g1], y0, 1          ; Row 1.

    ; Width 16, s16.
    %else
    vperm2i128      y0, y10, y11, 0x20
    vmovdqu         [g0], y0                ; Row 0.
    vperm2i128      y0, y10, y11, 0x31
    vmovdqu         [g0+g1], y0             ; Row 1.
    %endif


    ; Loop.
    lea             g0, [g0+2*g1]
    sub             g5, 2
    jnz             .loop
    %unmacro LOAD_ROW 4
    %unmacro LOAD_PAIR 6
    %unmacro PROCESS_PAIR 8
    %endif
    RET

    %undef WIDTH
    %undef HEIGHT
%endmacro

; This macro performs the 32-bit vertical interpolation used in the diagonal
; interpolation. This macro handles 6 cases (6 more eventually).
%macro LUMA_INTERPOL_D 2                    ; %1: width, %2: output ("pix", "s16").
    %assign WIDTH  %1
    %assign OUTPUT %2

    ; The logic and the register usage is as follow.
    ; - 4xN: same as the 8xN pure vertical case above.
    ; - 8xN: same as the 16xN pure vertical case above.
    ; - 16xN: handled as 2*8xN using an horizontal loop.
    ;
    ; Even though the source stride is known in some cases, we hold it in a
    ; register to keep the macro flexible. The performance loss is negligible.
    ;
    ; The destination pointer and stride are unmodified by the H macro. For the
    ; 16xN case, we copy the destination pointer in 'g6' for the next
    ; iteration. The spill pointer, fraction and height are stored in 'x15' by
    ; the H macro, in that order.

    ; Initialization after the H macro.
    vmovq           g2, x15                 ; Source.
    vpextrd         g4, x15, 2              ; Fraction.
    vpextrd         g5, x15, 3              ; Height.

    %if OUTPUT == "s16"
    add             g1, g1                  ; Double the destination stride for 16-bit stores.
    %endif

    shr             g4, 2                   ; Get the factor index.
    shl             g4, 4
    lea             ga, [pat_if_luma_16bit - 16] ; Load the factor base.
    lea             ga, [ga + g4]           ; Add the factor index.
    vpbroadcastd    y1, [ga + 0]            ; Broadcast the factors.
    vpbroadcastd    y2, [ga + 4]
    vpbroadcastd    y3, [ga + 8]
    vpbroadcastd    y4, [ga + 12]
    %if OUTPUT == "pix" && WIDTH == 4
    vpbroadcastd    y11, [pat_dw_2048]      ; Bias.
    %elif OUTPUT == "pix"
    vpbroadcastd    y14, [pat_dw_2048]
    %endif

    %if WIDTH == 4
    lea             ga, [3*g1]              ; 3 * destination stride.
    %endif
    %if WIDTH == 4
    mov             g3, 8                   ; Source stride.
    mov             g4, 8*3                 ; 3 * source stride.
    %elif WIDTH == 8
    mov             g3, 16
    mov             g4, 16*3
    %else
    mov             g3, 32
    mov             g4, 32*3
    %endif

    ; Handle the 16x16 case.
    %if WIDTH == 16
    mov             ga, 2                   ; Horizontal iteration count.
    mov             g6, g0                  ; Store the destination pointer for the second iteration.
    %endif


    ; Width 4.
    %if WIDTH == 4

    ; === BEGIN COPY&PASTE FROM V CASE (with 16-bit => 32-bit) ===

    ; Broadcast a row and blend it in the previous row.
    %macro LOAD_ROW 4                       ; %1: broadcast dst, %2: blend dst, %3: blend src, %4: load src.
    vpbroadcastq    %1, [%4]                ; Broadcast the row.
    vpblendd        %2, %3, %1, 0xf0        ; Blend in the previous row, high lane.
    %endmacro

    ; Regroup a pair of rows.
    %macro LOAD_PAIR 5                      ; %1: dst, %2: tmp, %3: outstanding row, %4-5: sources.
    LOAD_ROW        %2, %1, %3, %4          ; Load the first row.
    LOAD_ROW        %3, %2, %2, %5          ; Load the second row.
    vpunpcklwd      %1, %1, %2              ; Interleave both rows.
    %endmacro

    ; Process an interleaved pair of rows. Sum if prev sum is not 0.
    %macro PROCESS_PAIR 5                   ; %1: dst, %2: prev sum, %3: tmp, %4: source, %5: factor.
    vpmaddwd        %3, %4, %5              ; Multiply.
    %if %2 != 0
    vpaddd          %1, %2, %3              ; Add.
    %endif
    %endmacro

    ; Load rows (0,1) and update the source.
    vmovq           x7, [g2]
    LOAD_PAIR       y5, y0, y7, g2+g3, g2+2*g3
    PROCESS_PAIR    y10, 0, y10, y5, y1     ; Factors 01 for block line 0 (loop unification).
    lea             g2, [g2+g4]

    ; Load rows (2,3) and (4,5) and update the source.
    LOAD_PAIR       y5, y0, y7, g2,      g2+g3
    LOAD_PAIR       y6, y0, y7, g2+2*g3, g2+g4
    lea             g2, [g2+4*g3]

    .loop_dv:

    ; Process rows (2,3) and (4,5).
    PROCESS_PAIR    y8, y10, y8, y5, y2     ; Factors 23 for block line 0 (added to factors 01 above).
    PROCESS_PAIR    y8, y8, y0, y6, y3      ; Factors 45 for block line 0.
    PROCESS_PAIR    y9, 0, y9, y5, y1       ; Factors 01 for block line 2.
    PROCESS_PAIR    y9, y9, y0, y6, y2      ; Factors 23 for block line 2.
    PROCESS_PAIR    y10, 0, y10, y6, y1     ; Factors 01 for block line 4.

    ; Load rows (6,7) and (8,9) and update the source.
    LOAD_PAIR       y5, y0, y7, g2,      g2+g3
    LOAD_PAIR       y6, y0, y7, g2+2*g3, g2+g4
    lea             g2, [g2+4*g3]

    ; Process rows (6,7) and (8,9).
    PROCESS_PAIR    y8, y8, y0, y5, y4      ; Factors 67 for block line 0.
    PROCESS_PAIR    y9, y9, y0, y5, y3      ; Factors 45 for block line 2.
    PROCESS_PAIR    y9, y9, y0, y6, y4      ; Factors 67 for block line 2.

    ; === END COPY&PASTE FROM V CASE ===


    ; Add the bias, shift and pack.
    %if OUTPUT == "pix"
    vpaddd          y8, y11, y8
    vpaddd          y0, y11, y9
    vpsrad          y8, y8, 12
    vpsrad          y0, y0, 12
    vpackssdw       y0, y8, y0
    vpackuswb       y0, y0, y0

    ; Shift and pack.
    %else
    vpsrad          y8, y8, 6
    vpsrad          y0, y9, 6
    vpackssdw       y0, y8, y0
    %endif


    ; Store.

    ; Width 4, pix.
    %if OUTPUT == "pix"
    vmovd           [g0], x0                ; Row 0.
    vpsrlq          x8, x0, 32
    vmovd           [g0+2*g1], x8           ; Row 2.
    vextracti128    x0, y0, 1
    vmovd           [g0+g1], x0             ; Row 1.
    vpsrlq          x0, x0, 32
    vmovd           [g0+ga], x0             ; Row 3.

    ; Width 4, s16.
    %else
    vmovq           [g0], x0                ; Row 0.
    vmovhps         [g0+2*g1], x0           ; Row 2.
    vextracti128    x0, y0, 1
    vmovq           [g0+g1], x0             ; Row 1.
    vmovhps         [g0+ga], x0             ; Row 3.
    %endif


    ; Loop.
    lea             g0, [g0+4*g1]
    sub             g5, 4
    jnz             .loop_dv
    %unmacro LOAD_ROW 4
    %unmacro LOAD_PAIR 5
    %unmacro PROCESS_PAIR 5


    ; Width 8/16.
    %else
    ; === BEGIN COPY&PASTE FROM V CASE (with 16-bit => 32-bit) ===

    ; Broadcast a row and blend it in the previous row.
    %macro LOAD_ROW 4                       ; %1: broadcast dst, %2: blend dst, %3: blend src, %4: load src.
    vbroadcasti128  %1, [%4]                ; Broadcast the row.
    vpblendd        %2, %3, %1, 0xf0        ; Blend in the previous row, high lane.
    %endmacro

    ; Regroup a pair of rows.
    %macro LOAD_PAIR 6                      ; %1-2 dst, %3: tmp, %4: outstanding row, %5-6: sources.
    LOAD_ROW        %2, %1, %4, %5          ; Load the first row.
    LOAD_ROW        %4, %3, %2, %6          ; Load the second row.
    vpunpckhwd      %2, %1, %3              ; Interleave both rows.
    vpunpcklwd      %1, %1, %3
    %endmacro

    ; Process an interleaved pair of rows. Sum if prev sum is not 0.
    %macro PROCESS_PAIR 8                   ; %1-2: dst, %3-4: prev sums, %5: tmp, %6-7: sources, %8: factor.
    %if %3 == 0
    vpmaddwd        %1, %6, %8              ; Multiply.
    vpmaddwd        %2, %7, %8
    %else
    vpmaddwd        %5, %6, %8              ; Multiply.
    vpaddd          %1, %3, %5              ; Add.
    vpmaddwd        %5, %7, %8
    vpaddd          %2, %4, %5
    %endif
    %endmacro

    .loop_dh:

    ; Load rows (0,1).
    vmovdqu         x9, [g2]
    LOAD_PAIR       y5,y6, y0, y9, g2+g3, g2+2*g3
    PROCESS_PAIR    y12,y13, 0,0, y0, y5,y6, y1 ; Factors 01 for block line 0 (loop unification).
    lea             g2, [g2+g4]

    ; Load rows (2,3) and (4,5) and update the source.
    LOAD_PAIR       y5,y6, y0, y9, g2, g2+g3
    LOAD_PAIR       y7,y8, y0, y9, g2+2*g3, g2+g4
    lea             g2, [g2+4*g3]

    .loop_dv:

    ; Process rows (2,3) and (4,5).
    PROCESS_PAIR    y10,y11, y12,y13, y0, y5,y6, y2 ; Factors 23 for block line 0 (added to factors 01 above).
    PROCESS_PAIR    y10,y11, y10,y11, y0, y7,y8, y3 ; Factors 45 for block line 0.
    PROCESS_PAIR    y12,y13, 0,0,     y0, y5,y6, y1 ; Factors 01 for block line 2.

    ; Clobber rows (2,3) with (4,5).
    vmovdqu         y5, y7
    vmovdqu         y6, y8

    ; Load rows (6,7) and update the source.
    LOAD_PAIR       y7,y8, y0, y9, g2, g2+g3
    lea             g2, [g2+2*g3]

    ; Process rows (6,7).
    PROCESS_PAIR    y10,y11, y10,y11, y0, y7,y8, y4 ; Factors 67 for block line 0.

    ; === END COPY&PASTE FROM V CASE ===


    ; Add the bias, shift and pack.
    %if OUTPUT == "pix"
    vpaddd          y10, y14, y10
    vpaddd          y0, y14, y11
    vpsrad          y10, y10, 12
    vpsrad          y0, y0, 12
    vpackssdw       y0, y10, y0
    vpackuswb       y0, y0, y0

    ; Shift and pack.
    %else
    vpsrad          y10, y10, 6
    vpsrad          y0, y11, 6
    vpackssdw       y0, y10, y0
    %endif


    ; Store.

    ; Width 8/16, pix.
    %if OUTPUT == "pix"
    vmovq           [g0], x0                ; Row 0.
    vextracti128    x0, y0, 1
    vmovq           [g0+g1], x0             ; Row 1.

    ; Width 8/16, s16.
    %else
    vmovdqu         [g0], x0                ; Row 0.
    vextracti128    [g0+g1], y0, 1          ; Row 1.
    %endif


    ; Loop vertical.
    lea             g0, [g0+2*g1]
    sub             g5, 2
    jnz             .loop_dv

    ; Loop horizontal for 16xN.
    %if WIDTH == 16
    %if OUTPUT == "pix"
    lea             g0, [g6 + 8]            ; Destination for the second iteration.
    %else
    lea             g0, [g6 + 16]
    %endif
    vmovq           g2, x15                 ; Source for the second iteration.
    add             g2, 16
    vpextrd         g5, x15, 3              ; Height.
    sub             ga, 1                   ; Loop.
    jnz             .loop_dh
    %endif
    %unmacro LOAD_ROW 4
    %unmacro LOAD_PAIR 6
    %unmacro PROCESS_PAIR 8
    %endif
    RET

    %undef WIDTH
    %undef HEIGHT
%endmacro

; Declare the function, and the body with macro calls.
%macro LUMA_INTERPOL 3                      ; %1: width, %2: output ("pix", "s16"), %3: type ("h", "v", "d").
    %if %3 == "h"
        %if %2 == "pix"
            DEFFUN f265_lbd_interpol_luma_qpel_pix_%1_h_avx2, ia=7, at=8484448, ti=0, tv=13, ym=1
        %else
            DEFFUN f265_lbd_interpol_luma_qpel_s16_%1_h_avx2, ia=7, at=8484448, ti=0, tv=12, ym=1
        %endif
        LUMA_INTERPOL_H %1, %2

    %elif %3 == "v"
        %if %1 <= 8 && %2 == "pix"
            DEFFUN f265_lbd_interpol_luma_qpel_pix_%1_v_avx2, ia=7, at=8484448, ti=0, tv=12, ym=1
        %elif %1 <= 8 && %2 == "s16"
            DEFFUN f265_lbd_interpol_luma_qpel_s16_%1_v_avx2, ia=7, at=8484448, ti=0, tv=11, ym=1
        %elif %1 == 16 && %2 == "pix"
            DEFFUN f265_lbd_interpol_luma_qpel_pix_%1_v_avx2, ia=7, at=8484448, ti=0, tv=15, ym=1
        %elif %1 == 16 && %2 == "s16"
            DEFFUN f265_lbd_interpol_luma_qpel_s16_%1_v_avx2, ia=7, at=8484448, ti=0, tv=14, ym=1
        %endif
        LUMA_INTERPOL_V %1, %2

    %elif %3 == "d"
        %if %2 == "pix"
            DEFFUN f265_lbd_interpol_luma_qpel_pix_%1_d_avx2, ia=7, at=8484448, ti=0, tv=16, ym=1
        %else
            DEFFUN f265_lbd_interpol_luma_qpel_s16_%1_d_avx2, ia=7, at=8484448, ti=0, tv=16, ym=1
        %endif
        LUMA_INTERPOL_H %1, "dia"
        LUMA_INTERPOL_D %1, %2
    %endif
%endmacro
LUMA_INTERPOL 4, "pix", "h"
LUMA_INTERPOL 4, "s16", "h"
LUMA_INTERPOL 4, "pix", "v"
LUMA_INTERPOL 4, "s16", "v"
LUMA_INTERPOL 4, "pix", "d"
LUMA_INTERPOL 4, "s16", "d"
LUMA_INTERPOL 8, "pix", "h"
LUMA_INTERPOL 8, "s16", "h"
LUMA_INTERPOL 8, "pix", "v"
LUMA_INTERPOL 8, "s16", "v"
LUMA_INTERPOL 8, "pix", "d"
LUMA_INTERPOL 8, "s16", "d"
LUMA_INTERPOL 16, "pix", "h"
LUMA_INTERPOL 16, "s16", "h"
LUMA_INTERPOL 16, "pix", "v"
LUMA_INTERPOL 16, "s16", "v"
LUMA_INTERPOL 16, "pix", "d"
LUMA_INTERPOL 16, "s16", "d"
%unmacro LUMA_INTERPOL_H 2
%unmacro LUMA_INTERPOL_V 2
%unmacro LUMA_INTERPOL_D 2
%unmacro LUMA_INTERPOL 3


; void fenc_avg_pix_s16(f265_pix *dst, int dst_stride, int16_t *src0, int16_t *src1, int src_stride, int packed_dims)
; Input parameters:
; - g0:     destination.
; - g1:     destination stride.
; - g2:     source0.
; - g3:     source1.
; - g4:     source stride.
; - g5:     packed dimensions.
;
; The operation, before clipping, is (src0 + src1 + 64)>>7.
;
; The interpolated pixels src0 and src1 are 16-bit signed values. The worst
; case happens with the diagonal interpolation. The factors 4, 40, 40, 4 are
; applied twice per interpolated pixel and the intermediate value is shifted
; right by 6. The maximum interpolated pixel range is thus
; (255*(4+40+40+4)^2)/2^6 = 30855, and the signed sum (src0 + src1 + 64) does
; not fit in 16-bit.
;
; If we compute (src0 + src1) with signed saturation, the result is within
; [-32768, 32767]. Adding the bias in 32-bit and shifting right by 7 yields a
; value within [-256, 256], which is what we need.

; Declare the function, and the body with macro calls. For simplicity, we
; declare the temporary register count for the worst case since it doesn't
; affect the output.
%macro AVG_PIX 1                            ; %1: width.
DEFFUN f265_lbd_avg_pix_s16_%1_avx2, ia=6, at=848844, ti=1, tv=4, ym=1
    %assign WIDTH %1

    vpbroadcastd    y3, [pat_w_256]         ; Round&shift factor.
    and             g5, 0x7f                ; Get the height.
    add             g4, g4                  ; Double the source stride.

    ; Special case for the uncommon 8x2/8x6 block.
    %if WIDTH == 8
    bt              g5, 1
    jb              .loop_8x2
    %endif

    ; Process 4 rows per loop iteration.
    %if WIDTH >= 8 && WIDTH <= 24
    lea             g6, [g1*3]              ; 3*destination stride.
    lea             ga, [g4*3]              ; 3*source stride.
    %endif

    ; Process two 8-byte chunks without packing. y0 is temp.
    %macro Chunk8 3                         ; %1: first load off, %2: second load off, %3: destination register.
    vmovdqu         x%3, [g2+%1]            ; First load.
    vmovdqu         x0, [g3+%1]
    vinserti128     y%3, y%3, [g2+%2], 1    ; Second load.
    vinserti128     y0, y0, [g3+%2], 1
    vpaddsw         y%3, y0
    vpmulhrsw       y%3, y3
    %endmacro

    ; Process two 16-byte chunks and pack. y0 is destination, y1 is temp.
    %macro Chunk16 2                        ; %1: first load off, %2: second load off.
    vmovdqu         y0, [g2+%1]             ; First load.
    vpaddsw         y0, [g3+%1]
    vpmulhrsw       y0, y3

    vmovdqu         y1, [g2+%2]             ; Second load.
    vpaddsw         y1, [g3+%2]
    vpmulhrsw       y1, y3

    vpackuswb       y0, y1                  ; Pack the results.
    %endmacro

    ; Helper 8x4.
    %macro Block8 8                         ; %1-4 load off, %5-8: dst off.
    Chunk8          %1, %3, 1               ; Rows 0 and 2.
    Chunk8          %2, %4, 2               ; Rows 1 and 3.
    vpackuswb       y0, y1, y2              ; Pack the results.
    vmovq           [g0+%5], x0             ; Row 0.
    vmovhps         [g0+%6], x0             ; Row 1.
    vextracti128    x0, y0, 1
    vmovq           [g0+%7], x0             ; Row 2.
    vmovhps         [g0+%8], x0             ; Row 3.
    %endmacro

    ; Helper 12x2.
    %macro Block12 4                        ; %1: first load off, %2: second load off, %3-4: dst offsets.
    Chunk16         %1, %2                  ; Pack two rows.
    vmovq           [g0+%3], x0             ; Row 0, first 8-byte.
    vmovhps         [g0+%4], x0             ; Row 1, first 8-byte.
    vextracti128    x0, y0, 1
    vmovd           [g0+%3+8], x0           ; Row 0, last 4-byte.
    vpshufd         x0, x0, 2
    vmovd           [g0+%4+8], x0           ; Row 1, last 4-byte.
    %endmacro

    ; Helper 16x2.
    %macro Block16 4                        ; %1: first load off, %2: second load off, %3-4: dst offsets.
    Chunk16         %1, %2                  ; Pack 2 rows.
    vpermq          y0, y0, 0b11011000      ; Reorder.
    vmovdqu         [g0+%3], x0             ; Store.
    vextracti128    [g0+%4], y0, 1
    %endmacro

    ; Helper 32x1.
    %macro Block32 3                        ; %1: first load off, %2: second load off, %3: dst offset.
    Chunk16         %1, %2                  ; Load two consecutive 16-byte chunks.
    vpermq          y0, y0, 0b11011000      ; Reorder.
    vmovdqu         [g0+%3], y0             ; Store.
    %endmacro

    .loop:

    ; Width 4, 2 rows.
    %if WIDTH == 4
    vmovq           x0, [g2]                ; Rows 0 and 1.
    vmovhps         x0, [g2+g4]
    vmovq           x1, [g3]
    vmovhps         x1, [g3+g4]
    vpaddsw         y0, y1
    vpmulhrsw       y0, y3
    vpackuswb       y0, y0, y0              ; Pack the results.
    vmovd           [g0], x0                ; Row 0.
    vpsrlq          x0, x0, 32
    vmovd           [g0+g1], x0             ; Row 1.

    ; Width 6, 2 rows.
    %elif WIDTH == 6
    Chunk8          0, g4, 1                ; Rows 0 and 1.
    vpackuswb       y0, y1, y1              ; Pack the results.
    vmovd           [g0], x0                ; Row 0, with overlapping stores.
    vpsrlq          x1, x0, 16
    vmovd           [g0+2], x1
    vextracti128    x0, y0, 1               ; Row 1
    vmovd           [g0+g1], x0
    vpsrlq          x1, x0, 16
    vmovd           [g0+g1+2], x1

    ; Width 8, 4 rows.
    %elif WIDTH == 8
    Block8          0, g4, 2*g4, ga,  0, g1, 2*g1, g6

    ; Width 12, 4 rows.
    %elif WIDTH == 12
    Block12         0, g4, 0, g1
    Block12         2*g4, ga, 2*g1, g6

    ; Width 16, 4 rows.
    %elif WIDTH == 16
    Block16         0, g4, 0, g1
    Block16         2*g4, ga, 2*g1, g6

    ; Width 24, 4 rows.
    %elif WIDTH == 24
    Block16         0, g4, 0, g1
    Block16         2*g4, ga, 2*g1, g6
    Block8          32, 32+g4, 32+2*g4, 32+ga,  16, 16+g1, 16+2*g1, 16+g6

    ; Width 32, 2 rows.
    %elif WIDTH == 32
    Block32         0, 32, 0
    Block32         g4, g4+32, g1

    ; Width 48, 2 rows.
    %elif WIDTH == 48
    Block32         0, 32, 0
    Block32         g4, g4+32, g1
    Block16         64, g4+64, 32, g1+32

    ; Width 64, 1 row.
    %elif WIDTH == 64
    Block32         0, 32, 0
    Block32         64, 96, 32
    %endif


    ; Skip one row.
    %if WIDTH == 64
    add             g0, g1
    add             g2, g4
    add             g3, g4
    sub             g5, 1

    ; Skip two rows.
    %elif WIDTH == 4 || WIDTH == 6 || WIDTH == 32 || WIDTH == 48
    lea             g0, [g0 + 2*g1]
    lea             g2, [g2 + 2*g4]
    lea             g3, [g3 + 2*g4]
    sub             g5, 2

    ; Skip four rows.
    %else
    lea             g0, [g0 + 4*g1]
    lea             g2, [g2 + 4*g4]
    lea             g3, [g3 + 4*g4]
    sub             g5, 4
    %endif

    ; Loop.
    jnz             .loop
    RET


    ; Special case for the uncommon 8x2/8x6 block.
    %if WIDTH == 8
    .loop_8x2:
    Chunk8          0, g4, 1                ; Rows 0 and 1.
    vpackuswb       y0, y1, y1              ; Pack the results.
    vmovq           [g0], x0                ; Row 0.
    vextracti128    x0, y0, 1
    vmovq           [g0+g1], x0             ; Row 1.
    lea             g0, [g0 + 2*g1]         ; Update the pointers and loop.
    lea             g2, [g2 + 2*g4]
    lea             g3, [g3 + 2*g4]
    sub             g5, 2
    jnz             .loop_8x2
    RET
    %endif

    %unmacro Chunk8 3
    %unmacro Chunk16 2
    %unmacro Block8 8
    %unmacro Block12 4
    %unmacro Block16 4
    %unmacro Block32 3
    %undef WIDTH
%endmacro
AVG_PIX 4
AVG_PIX 6
AVG_PIX 8
AVG_PIX 12
AVG_PIX 16
AVG_PIX 24
AVG_PIX 32
AVG_PIX 48
AVG_PIX 64
%unmacro AVG_PIX 1


; void fenc_scale_qpel(int16_t *dst, int dst_stride, f265_pix *src, int src_stride, int packed_dims)
; Input parameters:
; - g0:     destination.
; - g1:     destination stride.
; - g2:     source.
; - g3:     source stride.
; - g4:     packed dimensions.

; This function isn't called often. It's not worth unrolling the loop. We do
; one or two rows per loop for all block sizes.
%macro SCALE_QPEL 1                         ; %1: width.
DEFFUN f265_lbd_scale_qpel_%1_avx2, ia=5, at=84844, ti=0, tv=1, ym=1
    %assign WIDTH %1

    and             g4, 0x7f                ; Get the height.
    add             g1, g1                  ; Double the destination stride.

    ; Helper macros, %1: src, %2: dst.
    %macro Block4 2
    vpmovzxbw       x0, [%1]
    vpsllw          y0, y0, 6
    vmovq           [%2], x0
    %endmacro

    %macro Block6 2
    vpmovzxbw       x0, [%1]
    vpsllw          y0, y0, 6
    vmovq           [%2], x0
    vpshufd         x0, x0, 2
    vmovd           [%2+8], x0
    %endmacro

    %macro Block8 2
    vpmovzxbw       x0, [%1]
    vpsllw          y0, y0, 6
    vmovdqu         [%2], x0
    %endmacro

    %macro Block12 2
    vpmovzxbw       y0, [%1]
    vpsllw          y0, y0, 6
    vmovdqu         [%2], x0
    vextracti128    x0, y0, 1
    vmovq           [%2+16], x0
    %endmacro

    %macro Block16 2
    vpmovzxbw       y0, [%1]
    vpsllw          y0, y0, 6
    vmovdqu         [%2], y0
    %endmacro

    .loop:

    ; Width 4.
    %if WIDTH == 4
    Block4          g2, g0
    Block4          g2+g3, g0+g1

    ; Width 6.
    %elif WIDTH == 6
    Block6          g2, g0
    Block6          g2+g3, g0+g1

    ; Width 8.
    %elif WIDTH == 8
    Block8          g2, g0
    Block8          g2+g3, g0+g1

    ; Width 12.
    %elif WIDTH == 12
    Block12         g2, g0
    Block12         g2+g3, g0+g1

    ; Width 16.
    %elif WIDTH == 16
    Block16         g2, g0
    Block16         g2+g3, g0+g1

    ; Width 24.
    %elif WIDTH == 24
    Block16         g2, g0
    Block16         g2+g3, g0+g1
    Block8          g2+16, g0+32
    Block8          g2+g3+16, g0+g1+32

    ; Width 32.
    %elif WIDTH == 32
    Block16         g2, g0
    Block16         g2+g3, g0+g1
    Block16         g2+16, g0+32
    Block16         g2+g3+16, g0+g1+32

    ; Width 48.
    %elif WIDTH == 48
    Block16         g2, g0
    Block16         g2+16, g0+32
    Block16         g2+32, g0+64

    ; Width 64.
    %elif WIDTH == 64
    Block16         g2, g0
    Block16         g2+16, g0+32
    Block16         g2+32, g0+64
    Block16         g2+48, g0+96
    %endif

    ; Skip one row.
    %if WIDTH >= 48
    add             g0, g1
    add             g2, g3
    sub             g4, 1

    ; Skip two rows.
    %else
    lea             g0, [g0 + 2*g1]
    lea             g2, [g2 + 2*g3]
    sub             g4, 2
    %endif

    ; Loop.
    jnz             .loop
    RET

    %unmacro Block4 2
    %unmacro Block6 2
    %unmacro Block8 2
    %unmacro Block12 2
    %unmacro Block16 2
    %undef WIDTH
%endmacro
SCALE_QPEL 4
SCALE_QPEL 6
SCALE_QPEL 8
SCALE_QPEL 12
SCALE_QPEL 16
SCALE_QPEL 24
SCALE_QPEL 32
SCALE_QPEL 48
SCALE_QPEL 64
%unmacro SCALE_QPEL 1

