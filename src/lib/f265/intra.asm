; Copyright (c) 2014, VANTRIX CORPORATION. All rights reserved. See LICENSE.txt
; for the full license text.

%include "x86inc.asm"

section .data
align 32

planar_8_left:      db  14,15, 12,13, 6,7, 4,5, 0,0,0,0,0,0,0,0, ; Shuffle pattern to regroup the left and top-right
                    db  10,11,  8, 9, 2,3, 0,1, 0,0,0,0,0,0,0,0, ; pixels together for rows 0/2, 1/3, 4/6, 5/7.

angle_mul_ver:      dw  1, 2, 5, 6,  0, 0, 0, 0, ; Row index, shuffled to do 2 rows at the time.
                    dw  3, 4, 7, 8,  0, 0, 0, 0, ; Used to get the weight and offset of each row on vertical angles.

triple_last_lane:   db  1,0, 1,0, 1,0, 1,0, 1,0, 1,0, 1,0, 1,0, ; Multiply high lane by 3 while keeping the
                    db  3,0, 3,0, 3,0, 3,0, 3,0, 3,0, 3,0, 3,0, ; low lane as-is.

neigh_1_of_2:       db  0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15,
                    db  8, 0, 9, 1, 10, 2, 11, 3, 12, 4, 13, 5, 14, 6, 15, 7,

neigh_shift_pair:   db  14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,
                    db  2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,

intra_ox_4:         dw  0,0,0,0, 1,1,1,1, 2,2,2,2, 3,3,3,3, ; Effect of row.
intra_inv_ox_4:     dw  3,3,3,3, 2,2,2,2, 1,1,1,1, 0,0,0,0, ; Inverted effect.
intra_ver_4:        dw  1,1,1,1, 2,2,2,2, 3,3,3,3, 4,4,4,4, ; Vertical angle multipliers.
intra_hor_4:        db  3,3,3,3, 2,2,2,2, 1,1,1,1, 0,0,0,0, ; Horizontal angle multipliers.
                    db  1,1,1,1, 0,0,0,0, 3,3,3,3, 2,2,2,2
intra_p_hor_4:      db  3,1,3,1, 3,1,3,1, 2,2,2,2, 2,2,2,2, ; Horizontal angle multipliers for planar.
                    db  1,3,1,3, 1,3,1,3, 0,4,0,4, 0,4,0,4,


align 16

; Repeat values on a whole 8x8 row. Inversed for use in pure horizontal.
ang_hor_8:          db  3, 3, 3, 3, 3, 3, 3, 3,  2, 2, 2, 2, 2, 2, 2, 2,
                    db  1, 1, 1, 1, 1, 1, 1, 1,  0, 0, 0, 0, 0, 0, 0, 0,

; Repeat values on a whole 8x8 row. Inversed for use in pure horizontal.
pair_low:           db  0,1, 1,2, 2,3, 3,4, 4,5, 5,6, 6,7, 7,8,
pair_high:          db  7,8, 8,9, 9,10, 10,11,  11,12, 12,13, 13,14, 14,15

angle_mul_hor:      dw  1, 2, 3, 4, 5, 6, 7, 8,     ; Row index. Used to get the weight and offset of each row on
                                                    ; horizontal angles.
angle_inv_mul_hor:  dw  0, 1, 2, 3, 4, 5, 6, 7,     ; Multiplier for inv_angle on horizontal angles.
angle_inv_mul_ver:  dw  7, 6, 5, 4, 3, 2, 1, 0,     ; Multiplier for inv_angle on vertical angles.

dia_bot_left_8:     db  14, 13, 12, 11, 10, 9, 8, 7 ; Invert byte order.
                    db  6, 5, 4, 3, 2, 1, 0, 15

planar_wgt_hor:     db  7, 1,  6, 2,  5, 3,  4, 4,  ; Weight pair, used for planar row weighting.
                    db  3, 5,  2, 6,  1, 7,  0, 8,

; Manage neighbour filtering edge case.
neig_bl_unav_8:     db  0,0,0,0,0,0,0,0, 0, 1, 2, 3, 4, 5, 6, 7

pat_b_m1_to_14:     db  -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14

intra_dia_bl_4:     db  6,5,4,3, 5,4,3,2, 4,3,2,1, 3,2,1,0  ; Dia bottom left re-ordering pattern.
intra_dia_tl_4:     db  0,8,9,10, 7,0,8,9, 6,7,0,8, 5,6,7,0 ; Dia top-left re-ordering pattern.
intra_dia_tr_4:     db  1,2,3,4, 2,3,4,5, 3,4,5,6, 4,5,6,7, ; Dia top right re-ordering pattern.


align 4

; Seed on which the the neighbours offset of inversed angles are calculated.
; As words (repeated 4 times) for speed-ups.
inv_angle:          dw  256, 256
                    dw  315, 315
                    dw  390, 390
                    dw  482, 482
                    dw  630, 630
                    dw  910, 910
                    dw  1638, 1638

; Seed on which the angles weights and offsets are calculated.
; As words (repeated 4 times) for speed-ups.
intra_angle:        db  2, 2, 2, 2,
                    db  5, 5, 5, 5,
                    db  9, 9, 9, 9,
                    db  13, 13, 13, 13,
                    db  17, 17, 17, 17,
                    db  21, 21, 21, 21,
                    db  26, 26, 26, 26,

; Pattern used as mask, bias, offset, ...
; As double to use the more efficient vpbroadcastd.
intra_p_ver_4:      db  3,1, 2,2, 1,3, 0,4,
intra_tl_4:         db  3,4, 4,5, 5,6, 6,7
neigh_last_b_of_d:  db  3, 7, 11, 15,
pat_q_255:          dq  0xff
pat_w_8192:         dw  8192, 8192,
pat_w_4096:         dw  4096, 4096,
pat_w_2048:         dw  2048, 2048,
pat_w_1024:         dw  1024, 1024,
pat_w_128:          dw  128, 128
pat_w_32:           dw  32, 32,
pat_w_31:           dw  31, 31,
pat_w_8:            dw  8, 8,
pat_b_14_15:        db  14,15, 14,15,
pat_b_7_8:          db  7,8, 7,8,
pat_b_4_5:          db  4,5, 4,5,
pat_b_0_1:          db  0,1, 0,1,
pat_b_128:          db  128, 128, 128, 128
pat_b_15:           db  15, 15, 15, 15,
pat_b_7:            db  7, 7, 7, 7
pat_b_1:            db  1, 1, 1, 1,


section .text

; 8x8 intra prediction functions.
; There are 11 assembly function to cover all 8x8 intra prediction modes.
; - Planar and DC.
; - 3 pure diagonals:
;   - dia_bot_left.
;   - dia_top_left.
;   - dia_top_right.
; - Pure vertical and horizontal.
; - 4 diagonals:
;   - hor_bot.
;   - hor_top.
;   - ver_left.
;   - ver_right.
;
; They all have the same input parameters, although some input parameters may be ignored.
; - g0:     Destination.
; - g1:     Neighbours. 48 is the bottommost left neighbour. 63 is the topmost left neighbour.
;           64 is the leftmost top neighbours. 79 is the rightmost top neighbours. 128 is the top-left neighbour.
; - g2:     Mode.
; - g3:     Filter edge flag.


; Intra DC 8x8.
DEFFUN f265_lbd_predict_intra_dc_8_avx2, ia=4, at=8844, ti=0, tv=6, ym=1
    ; Logic:
    ; Sum all neighbours, except the corners.
    ; Divide with bias by the number of samples.

    vpmovzxbw       x1, [g1+56]             ; Load all data.
    vpmovzxbw       x0, [g1+64]

    vinserti128     y2, y0, x1, 1           ; Keep a copy for filtering.

    vpaddw          y1, y0                  ; Add them together.

    vpalignr        y0, y1, 8               ; At each step, fold the register in 2...
    vpaddw          y1, y0                  ; ... then add each value together.

    vpalignr        y0, y1, 4
    vpaddw          y1, y0

    vpalignr        y0, y1, 2
    vpaddw          y1, y0

    vmovd           x0, [pat_w_2048]
    vpmulhrsw       y1, y1, y0              ; Round.

    vpbroadcastb    y1, x1                  ; Replicate the value.
    vmovdqa         y0, y1

    and             g3, 1
    jz              .SKIP_FILTER

    ; 3 cases:
    ; - Top-left = 2*base + top + left.
    ; - Top =  3*base + top.
    ; - Left = 3*base + left.

    vmovd           g2d, x1                 ; Extract base.
    and             g2, 0xff

    lea             g3, [3*g2+2]            ; Base * 3 + rounding bias.
    vmovd           x3, g3d
    vpbroadcastw    y3, x3                  ; Broadcast base * 3 + rounding bias.

    movzx           g3, byte [g1+64]        ; Load the first top and left value.
    movzx           ga, byte [g1+63]

    vpaddw          y2, y3                  ; 3 * base + neighbours + rounding bias.
    vpsrlw          y2, 2                   ; Divide by 4.

    vpackuswb       y2, y2                  ; Word to byte.

    vpblendd        y0, y2, y0, 0xfc        ; Save in top row.

    vpermq          y2, y2, 0b10_10_10_10   ; Broadcast left column.

    vmovdqu         y3, [ang_hor_8]
    vpbroadcastq    y5, [pat_q_255]

    vpshufb         y4, y2, y3              ; Replicate 8x the 4 lower values.
    vpsrldq         y2, y2, 4               ; Shift by 4 to do the 4 last rows.
    vpblendvb       y1, y1, y4, y5          ; Blend only the first value of each row.

    vpshufb         y4, y2, y3              ; Replicate 8x the 4 lower values.
    vpblendvb       y0, y0, y4, y5          ; Blend only the first value of each row.

    ; Do top-left.
    add             g3, ga                  ; Top + left.
    lea             g2, [2*g2+g3+2]         ; Top + left + 2*base + bias.
    shr             g2, 2                   ; Get the average.

    vmovdqa         y2, y0
    vpinsrb         x2, g2b, 0
    vinserti128     y0, y0, x2, 0

    .SKIP_FILTER:

    vmovdqu         [g0], y0
    vmovdqu         [g0+0x20], y1           ; Save the value.

    RET


; Intra planar 8x8.
DEFFUN f265_lbd_predict_intra_planar_8_avx2, ia=4, at=8844, ti=0, tv=8, ym=1
    ; value = ((8-x-1)*left + (8-y-1)*top + (x+1)*top_right + (y+1)*bottom_left + 8) >> 4);

    vmovd           x6, [g1+56-1]           ; Load & broadcast bottom-left.
    vpbroadcastb    x6, x6

    vpmovzxbw       x1, [g1+64]             ; Load top neighbours.
    vpmovzxbw       x6, x6

    vmovq           x7, [g1+72]             ; Load & broadcast top right.
    vpbroadcastb    y7, x7

    vpbroadcastd    y0, [pat_b_0_1]         ; Weight distribution pattern.

    vpsllw          y2, y1, 3               ; Top row * 8.
    vpsubw          y1, y6                  ; Row delta (top neighbour - bottom-left).

    vpsubw          y2, y1                  ; Top row * 7 + bottom-left.

    vpsllw          y3, y1, 1               ;
    vpsubw          y6, y2, y3              ; Top row * 5 + 3*bottom-left.
    vinserti128     y2, y2, x6, 1           ; Get row 2 values.
    vinserti128     y1, y1, x1, 1           ; Double the vertical delta removed at each line.

    ; Register usage:
    ; - y1: row delta.
    ; - y2: row sum.

    vpbroadcastq    y3, [g1+64-8]           ; Load left column.
    vpunpcklbw      y3, y7                  ; Merge top right with left col.
    vpshufb         y3, [planar_8_left]     ; Shuffle to do 2 columns at a time.

    vbroadcasti128  y4, [planar_wgt_hor]    ; Load weights.
    vpbroadcastd    y5, [pat_w_2048]        ; Load rounding bias.

    ; Register usage:
    ; - y0: weight distribution pattern.
    ; - y1: row vertical delta.
    ; - y2: row vertical sum.
    ; - y3: column values.
    ; - y4: column weights.
    ; - y5: rounding bias.

    %macro DO_ROW 2                         ; %1: alignment offset, %2: destination register.
    %if %1 != 0
    vpsubw          y2, y1                  ; Add delta to row sum.
    vpalignr        y%2, y3, %1*2           ; Offset column.
    vpshufb         y%2, y%2, y0            ; Repeat the column.
    %else
    vpshufb         y%2, y3, y0             ; Repeat the column.
    %endif

    vpmaddubsw      y%2, y4                 ; Get the sum of all factors.
    vpaddusw        y%2, y2                 ; Add vertical.
    vpmulhrsw       y%2, y5                 ; Round.
    %endmacro

    DO_ROW          0, 6                    ; Do row 0 and 2.
    DO_ROW          1, 7                    ; Do row 1 and 3.

    vpackuswb       y6, y7
    vmovdqu         [g0], y6

    vpsubw          y2, y1                  ; Add offset to row value.
    vpsubw          y2, y1                  ;

    DO_ROW          2, 6                    ; Do row 4 and 6.
    DO_ROW          3, 7                    ; Do row 5 and 7.

    vpackuswb       y6, y7
    vmovdqu         [g0+0x20], y6
    %unmacro DO_ROW 2
    RET


; Intra pure diagonal bottom-left 8x8.
DEFFUN f265_lbd_predict_intra_dia_bot_left_8_avx2, ia=4, at=8844, ti=0, tv=3, ym=1

    vmovdqu         x0, [g1+48]             ; Load all data.
    vpshufb         y0, [dia_bot_left_8]    ; Re-order it.

    vpalignr        y1, y0, 2               ; Offset the pixels in the high lane to build rows 2 and 3.
    vinserti128     y0, y0, x1, 1           ;

    vpalignr        y1, y0, 1               ; Create row 1 and 3.
    vpunpcklqdq     y2, y0, y1              ; Merge them with rows 0 and 2.
    vmovdqu         [g0], y2                ; Save row 0 to 3.

    vpalignr        y1, y0, 5               ; Offset to generate rows 4 to 7.

    vpalignr        y0, y0, 4               ; Repeat operation above for rows 4 to 7.
    vpunpcklqdq     y2, y0, y1              ;
    vmovdqu         [g0+0x20], y2           ;
    RET


; Do horizontal prediction on a single row.
; Input:
; - y0: weights.
; - y1: neighbours offset pattern.
; - y2: rounding bias.
; - y5: neighbour offset increment for each row.
; - y6: left neighbours.
; - %1: row offset.
; - %2: output register.
%macro DO_ROW       2
    %if %1 != 0
    vpsubb          y1, y5                  ; Update neighbours offset.
    %endif

    vpshufb         y%2, y6, y1             ; Generate neighbour pair.

    ; Calculate row values.
    vpmaddubsw      y%2, y%2, y0            ; Multiply with weight.
    vpmulhrsw       y%2, y%2, y2            ; Round.
%endmacro

; Predict intra from left neighbours.
; Input:
; - y0: weights. (for DO_ROW)
; - y1: neighbours offset pattern.
; - y6: left neighbours.
; Register usage:
; - y2: rounding bias.
; - y3: temp.
; - y4: temp.
; - y5: neighbour offset increment for each row.
%macro PRED_LEFT 0
    ; Load patterns.
    vpbroadcastd    y2, [pat_w_1024]        ; Load rounding bias.

    ; Calculate the offset for the high lane.
    vpbroadcastd    y5, [pat_b_1]           ; Load neighbour position offsets.
    vpsubb          y3, y1, y5              ; Pre-offset by 2 the neighbour position.
    vpsubb          y3, y3, y5              ; Will be used to calculate 2 rows at once.
    vinserti128     y1, y1, x3, 1           ; Put the offsetted load pattern on the high lane.

    DO_ROW          0, 3                    ; Do row 0 and 2.
    DO_ROW          1, 4                    ; Do row 1 and 3.

    vpackuswb       y3, y4                  ; Merge value.
    vmovdqu         [g0+0x00], y3           ; Save result.

    vpsubb          y1, y5                  ; Skip from rows 1|3 to rows 4|6.
    vpsubb          y1, y5

    DO_ROW          4, 3                    ; Do row 4 and 6.
    DO_ROW          5, 4                    ; Do row 5 and 7.

    vpackuswb       y3, y4                  ; Merge value.
    vmovdqu         [g0+0x20], y3           ; Save result.
%endmacro

; Generate offset and weight for intra left prediction.
; Input:
; - g2: mode.
; - %1: 1 for hor top. 0 for hoz bottom.
; Register usage:
; - ga: temp.
; - y2: temp.
; Output:
; - y0: weights.
; - y1: neighbours offset pattern.
%macro GEN_LEFT_WEIGHT 1
    ; Generate weight and offset.
    lea             ga, [intra_angle]
    %if %1
    vpbroadcastd    y1, [ga+g2*4-11*4]      ; Load angle factor.
    neg             g2                      ; Get the angle's inversed offset.
    %else
    neg             g2                      ; Get the angle's inversed offset.
    vpbroadcastd    y1, [ga+g2*4+9*4]       ; Load angle factor.
    %endif
    vbroadcasti128  y2, [angle_mul_hor]     ; Load multiplication table.
    vpmaddubsw      y1, y1, y2              ; Result in offset and weight for each column.

    ; Generate weight.
    vpbroadcastd    y2, [pat_w_31]          ; Load weight mask.
    vpand           y2, y2, y1              ; Extract weight.

    ; Generate weight pairs.
    vpbroadcastd    y0, [pat_w_32]          ; Load weight complement base.
    vpsubw          y0, y2                  ; Get weight complements.

    vpackuswb       y2, y2, y2              ; Word to byte.
    vpackuswb       y0, y0, y0              ; Word to byte.
    %if %1
    vpunpcklbw      y0, y0, y2              ; Make the pair. Final weight.
    %else
    vpunpcklbw      y0, y2, y0              ; Make the pair. Final weight.
    %endif

    ; Generate offsets.
    vpsrlw          y1, y1, 5               ; Extract neighbour offset.
    vpsllw          y2, y1, 8               ; Double the offset (twice for each pair).
    vpor            y1, y2

    %if %1
    vpbroadcastd    y2, [pat_b_7_8]         ; Load base offset pattern.
    vpaddw          y1, y2, y1              ; Add offset with base. Result in actual neighbour position.
    %else
    vpbroadcastd    y2, [pat_b_14_15]       ; Load base offset pattern.
    vpsubw          y1, y2, y1              ; Add the angle offset to the base offset.
    %endif
%endmacro


; Intra angular horizontal bottom 8x8.
DEFFUN f265_lbd_predict_intra_hor_bot_8_avx2, ia=4, at=8844, ti=0, tv=7, ym=1
    vbroadcasti128  y6, [g1+48]             ; Load left column.

    GEN_LEFT_WEIGHT 0
    PRED_LEFT
    RET


; Intra pure horizontal 8x8.
DEFFUN f265_lbd_predict_intra_hor_8_avx2, ia=4, at=8844, ti=0, tv=4, ym=1
    vmovdqu         y0, [ang_hor_8]         ; Load shuffle mask.

    vpbroadcastd    y1, [g1+63-3]           ; Load the first 4 rows.
    vpbroadcastd    y2, [g1+63-7]           ; Load the second 4 rows.

    vpshufb         y1, y1, y0              ; Replicate 8 times each value.
    vpshufb         y2, y2, y0              ;

    and             g3, 1
    jz              .SKIP_FILTER

    vpmovzxbw       x0, [g1+64]             ; Load top row.
    vmovd           x3, [g1+128]            ; Load & broadcast top-left.
    vpbroadcastb    x3, x3

    vpmovzxbw       x3, x3                  ; Byte to word.

    vpsubw          x0, x3                  ; Top - top-left.
    vpsraw          x0, 1                   ; (top - top-left)/2.

    vmovd           x3, [g1+63]             ; Load left.
    vpbroadcastb    x3, x3
    vpmovzxbw       x3, x3                  ; Byte to word.
    vpaddw          x0, x3                  ; Left + (top - top-left)/2.

    vpxor           x3, x3                  ; Replace negative values by 0.
    vpmaxsw         x0, x3                  ;

    vpackuswb       x0, x0                  ; Word to byte with unsigned saturation.

    vpblendd        y1, y0, y1, 0xfc        ; Update the first 8 bytes.

    .SKIP_FILTER:
    vmovdqu         [g0], y1                ; Save it.
    vmovdqu         [g0+0x20], y2           ;

    RET


; Intra angular horizontal top 8x8.
DEFFUN f265_lbd_predict_intra_hor_top_8_avx2, ia=4, at=8844, ti=0, tv=7, ym=1
    GEN_LEFT_WEIGHT 1

    vmovdqu         x5, [g1+64]             ; Load top neighbour.
    vpalignr        x5, x5, x5, 15
    vpinsrb         x5, [g1+128], 0         ; Insert the top-left neighbour.

    ; Import top neighbour with the left ones.
    lea             g3, [inv_angle]
    vpbroadcastd    y4, [g3+g2*4+18*4]      ; Load the inversed angle values.
    vmovdqu         x3, [angle_inv_mul_hor] ; Load the weight values.
    vpmullw         y4, y4, y3              ; Get the weight. Some neighbour will have an invalid offset.
                                            ; Since we never read them, it's ok.
    vpbroadcastd    y3, [pat_w_128]         ; Load inversed angle bias.
    vpaddw          y4, y3                  ; Add inversed angle bias.
    vpsraw          y4, 8                   ; Get inversed neighbour offset.
    vpackuswb       y4, y4                  ; Word to byte.
    vpshufb         y5, y4                  ; Re-order left neighbours.

    ; Load patterns.
    vmovq           x4, [g1+56]             ; Load left data.
    vpblendd        y5, y4, y5, 0xfc        ; Blend left neighbours with top neighbours.
    vinserti128     y6, y5, x5, 1           ; Double data.

    PRED_LEFT
    RET

%unmacro GEN_LEFT_WEIGHT 1
%unmacro PRED_LEFT 0
%unmacro DO_ROW 2


; Intra pure diagonal top-left 8x8.
DEFFUN f265_lbd_predict_intra_dia_top_left_8_avx2, ia=4, at=8844, ti=0, tv=3, ym=1
    vmovq           x0, [g1+64-7]           ; Load top row.
    vmovhps         x0, [g1+64]             ; Load left row.
    vpinsrb         x0, [g1+128], 7         ; Load & insert top-left.

    vpalignr        y1, y0, 2               ; Offset the pixels in the high lane to build rows 6 and 7.
    vinserti128     y0, y1, x0, 1           ;

    vpalignr        y1, y0, 1               ; Create row 5 and 7.
    vpunpcklqdq     y2, y1, y0              ; Merge them with rows 4 and 6.
    vmovdqu         [g0+0x20], y2           ;

    vpalignr        y0, y0, 4               ; Offset to generate rows 0 to 3.

    vpalignr        y1, y0, 1               ; Repeat operation above for row 0 to 3.
    vpunpcklqdq     y2, y1, y0              ;
    vmovdqu         [g0], y2                ; Save rows 0 to 3.
    RET


; Generate vertical intra prediction of 2 rows.
; Input:
; - %1: row index.
; - %3: 0 for vertical right. 1 for vertical left.
; - y0: rounding bias.
; - y1: broadcast weights paterns.
; - y2: pre-calculated weights.
; - y5: temp.
; - y6: base offset pattern
; - y7: angle sum.
; - y8: angle increment.
; - y9: shift mask.
; - y10: neighbours.
; Output:
; - %2: predicted row.
%macro DO_ROW 3
    %if %1 != 0
    vpaddb          y7, y8                  ; Add the angle to the current angle sum. Generate the offset.
    %endif

    ; Generate the neighbours pairs.
    vpsrlw          y%2, y7, 5              ; Generate neighbour offset.
    vpand           y%2, y9                 ; Shift can only be on word or greater value. Mask to simulate byte shift.
    %if %3
    vpsubb          y%2, y6, y%2,           ; Generate pair offset.
    %else
    vpaddb          y%2, y6                 ; Add offset to pairing mask.
    %endif
    vpshufb         y%2, y10, y%2           ; Generate pair.

    ; Broadcast the current weights.
    %if %1 != 0
    vpalignr        y5, y2, %1*2            ; Get weights.
    vpshufb         y5, y1                  ; Broadcast weights.
    %else
    vpshufb         y5, y2, y1              ; Broadcast weights.
    %endif

    ; Calculates row predictions.
    vpmaddubsw      y%2, y%2, y5            ; Multiply values with weight.
    vpmulhrsw       y%2, y%2, y0            ; Round.
%endmacro

; Input:
; - %1: 0 for vertical right. 1 for vertical left.
; - g0: Result array.
; - y10: Top row. Replicated.
; Register usage :
; - y0: Rounding bias.
; - y1: Word replication pattern.
; - y2: Weights, distributed to do 2 rows at a time.
; - y3: 2 rows of results [0|2].
; - y4: 2 rows of results [1|3].
; - y5: Temp.
; - y6: Generate pair.
; - y7: Angle sum. Used to generate the offset.
; - y8: Angle value. Add it to the sum at each row.
; - y9: "Word shift as byte shift" mask pattern.
%macro ANG_VERTICAL_PRED 1
    ; Calculate the angle offset base.
    lea             g3, [intra_angle]
    %if %1
    neg             g2
    vpbroadcastd    y8, [g3+g2*4+25*4]      ; Load angle factor.
    %else
    vpbroadcastd    y8, [g3+g2*4-27*4]      ; Load angle factor.
    %endif

    vpmaddubsw      y7, y8, [triple_last_lane]  ; Multiply high lane by 3. Offset required to do 2 rows at the time.
    vpackuswb       y7, y7                  ; This is the angle sum for each row.

    ; Calculate the weight.
    %if %1
    vmovdqu         y3, [angle_mul_ver]     ; Load multiplication table.
    vpmaddubsw      y2, y3, y8              ; Offset and weight for all rows.
    %else
    vpmaddubsw      y2, y8, [angle_mul_ver] ; Offset and weight for all rows.
    %endif
    vpbroadcastd    y3, [pat_w_31]          ; Load mask.
    vpand           y3, y3, y2              ; Weight.

    vpbroadcastd    y4, [pat_w_32]          ; Load weights complement base.
    vpsubw          y4, y3                  ; Get the weight complement.
    %if %1
    vpunpcklbw      y2, y3, y4              ; Make the pair. Final weight.
    %else
    vpunpcklbw      y2, y4, y3              ; Make the pair. Final weight.
    %endif

    ; Load patterns.
    %if %1
    vbroadcasti128  y6, [pair_high]         ; Load pair making pattern.
    %else
    vbroadcasti128  y6, [pair_low]          ; Load pair making pattern.
    %endif
    vpbroadcastd    y0, [pat_w_1024]        ; Load rounding bias.
    vpbroadcastd    y1, [pat_b_0_1]         ; Load weight distribution pattern.
    vpbroadcastd    y9, [pat_b_7]           ; Load "word shift as byte shift" mask pattern.

    DO_ROW          0, 3, %1                ; Do row 0 and 2.
    DO_ROW          2, 4, %1                ; Do row 1 and 3.

    vpackuswb       y3, y4                  ; Merge value.
    vmovdqu         [g0+0x00], y3           ; Save the result.

    vpaddb          y7, y8                  ; Skip from rows 1|3 to rows 4|6.
    vpaddb          y7, y8

    DO_ROW          4, 3, %1                ; Do row 4 and 6.
    DO_ROW          6, 4, %1                ; Do row 5 and 7.

    vpackuswb       y3, y4                  ; Merge value.
    vmovdqu         [g0+0x20], y3           ; Save the result.
%endmacro


; Intra angular vertical left 8x8.
DEFFUN f265_lbd_predict_intra_ver_left_8_avx2, ia=4, at=8844, ti=0, tv=11, ym=1
    vmovq           x0, [g1+64-8]           ; Load top and left data.
    vpinsrb         x0, [g1+128], 8         ; Load top-left.

    ; Re-order the left neighbours.
    lea             g3, [inv_angle]
    vpbroadcastd    x2, [g3+g2*4-18*4]      ; Load the inversed angle values.
    vmovdqu         x3, [angle_inv_mul_ver] ; Load the inversed weight values.
    vpmullw         x2, x2, x3              ; Get the weight. Some neighbour will have an invalid offset.
                                            ; Since we never use them, it's ok.
    vpbroadcastd    x3, [pat_w_128]         ; Load inversed angle bias.
    vpaddw          x2, x3                  ; Add inversed angle bias.
    vpsraw          x2, 8                   ; Get inversed neighbour offset.
    vpbroadcastd    x3, [pat_w_8]           ; Load inversed angle offset.
    vpsubb          x2, x3, x2              ; Invert the index.
    vpackuswb       x2, x2                  ; Word to byte.
    vpshufb         x0, x2                  ; Re-order left neighbours.

    ; Blend re-ordered neighbours with the top neighbours.
    vmovhps         x0, [g1+64]
    vinserti128     y10, y0, x0, 1          ; Double top row.

    ANG_VERTICAL_PRED 1
    RET


; Intra pure vertical 8x8.
DEFFUN f265_lbd_predict_intra_ver_8_avx2, ia=4, at=8844, ti=0, tv=6, ym=1
    vpbroadcastq    y0, [g1+64]             ; Copy the top neighbours 4 times. Holds row 0 to 3.
    vmovdqa         y4, y0                  ; Copy it. Holds row 4 to 7.

    and             g3, 1
    jz              .SKIP_FILTER

    vmovd           x3, [g1+128]            ; Load left.
    vpbroadcastb    x3, x3
    vpmovzxbw       x2, [g1+64-8]           ; Load left neighbours.
    vpbroadcastb    x1, x0                  ; Broadcast top neighbours.

    vpmovzxbw       x3, x3                  ; Word to byte.
    vpmovzxbw       x1, x1

    vpsubw          x2, x3                  ; Left - top-left.
    vpsraw          x2, 1                   ; Signed divide by 2.
    vpaddw          x2, x1                  ; Top + (left - top-left)/2.

    vpxor           x3, x3
    vpmaxsw         x2, x3                  ; Clip negative value to 0.
    vpackuswb       x2, x2                  ; Word to byte with unsigned saturation.
    vinserti128     y2, x2, 1               ; Double the data.

    vmovdqu         y3, [ang_hor_8]         ; Load replication pattern.
    vpbroadcastq    y1, [pat_q_255]         ; Pattern that blends in a word out of 8.

    vpshufb         y5, y2, y3              ; Replicate 8x the 4 lower values.

    vpsrldq         y2, y2, 4               ; Shift by 4 to do the 4 last rows.

    vpblendvb       y4, y5, y1              ; Blend only the first value of each row.

    vpshufb         y5, y2, y3              ; Replicate 8x the 4 lower value.
    vpblendvb       y0, y5, y1              ; Blend only the first value of each row.

    .SKIP_FILTER:
    vmovdqu         [g0+0x00], y0           ; Save it.
    vmovdqu         [g0+0x20], y4           ;

    RET


; Intra angular vertical right 8x8.
DEFFUN f265_lbd_predict_intra_ver_right_8_avx2, ia=4, at=8844, ti=0, tv=11, ym=1

    vbroadcasti128  y10, [g1+64]            ; Load top row.

    ANG_VERTICAL_PRED 0
    RET

%unmacro DO_ROW 3
%unmacro ANG_VERTICAL_PRED 1


; Intra angular top right 8x8.
DEFFUN f265_lbd_predict_intra_dia_top_right_8_avx2, ia=4, at=8844, ti=0, tv=3, ym=1
    vmovdqu         x0, [g1+64]             ; Load all data.

    vpalignr        y1, y0, 3               ; Offset the pixels in the high lane to build rows 2 and 3.
    vpalignr        y0, y0, 1               ;
    vinserti128     y0, y0, x1, 1           ; Push offsetted value in high lane.

    vpalignr        y1, y0, 1               ; Create rows 1 and 3.
    vpunpcklqdq     y2, y0, y1              ; Merge them with rows 0 and 2.
    vmovdqu         [g0], y2                ; Save rows 0 to 3.

    vpalignr        y1, y0, 5               ; Offset to generate rows 4 to 7.

    vpalignr        y0, y0, 4               ; Repeat operation above for rows 4 to 7.
    vpunpcklqdq     y2, y0, y1              ;
    vmovdqu         [g0+0x20], y2           ;

    RET


; Extract and filter neighbours for intra prediction.
;
; Input format:
; EAABB
; C
; C
; D
; D
;
; Output format:
;   padding   [48]  [64]  padding [128]
; [ ...       DDCC  AACC  ...     E]
;
; Input parameters:
; - g0: nbuf[2][160].
; - g1: pred.
; - g2: pred_stride.
; - g3: avail[2].
; - g4: filter_flag.
; - g5: packed (ignored).
DEFFUN f265_lbd_extract_intra_neigh_8_avx2, ia=6, at=884844, ti=1, tv=8, ym=1
    ; Load availability.
    movzx           g5, byte [g3]           ; Load availx.
    movzx           g6, byte [g3+4]         ; Load availy.

    ; Test for special case: no left neighbours.
    cmp             g6, 0
    jz              .LEFT_NOT_AVAILABLE

    ; Left neighbours are available.

    ; Get C from the prediction buffer.
    ; Pseudo-code:
    ; - Load & broadcast as dword the left neighbour of each row.
    ; - Blend the rows together.
    ; - Keep in mind the order needs to be inversed.

    ; Get 4 left neighbours.
    ; Input:
    ; - %1: the xmm register in which to save the value.
    ; - %2: temp.
    ; - %3: temp.
    ; - ga: first row address. Must be aligned on the dword left of the row.
    ; - g2: pred_stride.
    ; - g3: 3*pred_stride.
    %macro load     3
    vpbroadcastd    %1, [ga]                ; Load & broadcast the left neighbour.
    vpbroadcastd    %2, [ga+g2]             ; Load & broadcast the next left neighbour.
    vpblendd        %1, %1, %2, 0b0101_0101 ; Mix even and odd row: result 1 0 1 0.

    vpbroadcastd    %2, [ga+g2*2]           ; Load & broadcast the next left neighbour.
    vpbroadcastd    %3, [ga+g3]             ; Load & broadcast the next left neighbour.
    vpblendd        %2, %2, %3, 0b0101_0101 ; Mix even and odd row: result 3 2 3 2.

    vpblendd        %1, %1, %2, 0b0011_0011 ; Mix 1 0 and 3 2. Result 3 2 1 0.
    vpshufb         %1, x7                  ; Keep the last byte of each dword.
    %endmacro

    vpbroadcastd    x7, [neigh_last_b_of_d] ; Load shuffle mask.

    lea             ga, [g1-4]
    lea             g3, [g2*3]
    load            x0, x1, x2              ; Load C0 to C3.

    lea             ga, [ga+g2*4]
    load            x3, x1, x2              ; Load C4 to C7.

    vpblendd        x0, x0, x3, 0b01010101  ; Get C7..C0.

    ; Special case: no top neighbours.
    cmp             g5, 0
    jz              .TOP_NOT_AVAILABLE

    ; Load top (A and B) neighbour from pred.
    mov             ga, g2
    neg             ga                      ; Move up 1 row (negative pred_stride).
    vmovdqu         x1, [g1+ga]             ; Load A|B from prediction.
    vmovd           x2, [g1+ga-1]           ; Load top-left (E).

    .LEFT_AND_TOP_FETCHED:

    ; Test if bottom-left is available.
    cmp             g6, 8
    ja              .BOTTOM_AVAILABLE

    ; Bottom-left not available.
    vpshufb         x0, [neig_bl_unav_8]    ; Expand the last value.

    .BOTTOM_FETCHED:

    vmovdqu         [g0+48], x0             ; Save partial top and left to allow easy byte extraction.
    vmovdqu         [g0+64], x1

    lea             ga, [g5-1]              ; Available offset can go up to 128.
    movd            x3, gad                 ; Since vpcmpgtb is signed, we subtract one.
    vpbroadcastb    x3, x3
    vpcmpgtb        x3, [pat_b_m1_to_14]
    vmovd           x4, [g0+63+g5]          ; Broadcast the last available block.
    vpbroadcastb    x4, x4
    vpblendvb       x1, x4, x1, x3          ; Replace (blend) invalid value with the broadcasted last valid values.

    vmovdqu         [g0+48], x0             ; Save values.
    vmovdqu         [g0+64], x1
    vmovdqu         [g0+128], x2

    ; Filter only if required.
    cmp             g4, 0
    je              .END

    ; Pseudo code:
    ; Register ordering : D7, D6 ... D0, C7, ... C0, E, A0, ..., A7, B0, ... B6, B7.
    ; V[i] = (V[i-1] + 2*V[i] + V[i+1] + 2) >> 2
    ; D7 = D7, B7 = B7

    vpbroadcastd    y6, [pat_b_1]           ; Load pmadd pattern (actually, just an add and zero extend).
    vpbroadcastd    y5, [pat_w_8192]        ; Load rounding bias.

    vpslldq         x4, x2, 15              ; Move the top-left (e) to the last byte of the xmm register.
    vpalignr        x3, x2, x0, 1           ; Remove D7 and insert E next to C0.
                                            ; All bytes are shifted by one. Named D|C*.
    vpalignr        x4, x1, x4, 15          ; Remove B7 and insert E next to A0.
                                            ; All bytes are shifted by one. Named A|B*.

    vinserti128     y0, y0, x1, 1           ; Pack D|C with A|B.
    vinserti128     y3, y3, x4, 1           ; Pack D|C* with A|B*.

    vpmaddubsw      y0, y0, y6              ; Add the neighbours together.
    vpmaddubsw      y3, y3, y6              ; As D|C|AB* is DC|A|B offsetted by one byte, this will generate all
                                            ; D|C and A|B peer. The innermost value of D|C|A|B* will be C0+E and E+A0.

    vpaddw          y1, y0, y3              ; Add D|C|A|B to D|C|A|B*.
    vpmulhrsw       y1, y1, y5              ; Round.

    vpshufb         y3, [neigh_shift_pair]  ;
    vpaddw          y0, y3, y0              ; Generate the missing pair sums.
    vpmulhrsw       y0, y0, y5              ; Round.

    vpackuswb       y0, y0, y1              ; Word to byte.
    vpshufb         y0, [neigh_1_of_2]      ; Interleave the result.

    vextracti128    x1, y0, 1

    vpinsrb         x0, [g0+48], 0          ; Manage D7.
    vmovdqu         [g0+160+48], x0         ; Save it.

    vpinsrb         x1, [g0+79], 15         ; Manage B7.
    vmovdqu         [g0+160+64], x1         ; Save it.

    ; Filter top-left.
    movzx           g2, byte [g0+128]       ; Load top-left.
    movzx           g3, byte [g0+63]        ; Load top.
    movzx           g4, byte [g0+64]        ; Load left.
    lea             g2, [g2*2+g3+2]         ; Top-left * 2 + top + bias.
    add             g2, g4                  ; Top-left * 2 + top + left + bias.
    shr             g2, 2                   ; Round.
    mov             [g0+160+128], g2b       ; Save filtered top-left.

    .END:
    RET


    .LEFT_NOT_AVAILABLE:

    ; Test if top is available.
    cmp             g5, 0
    jz              .NOTHING_AVAILABLE

    mov             ga, g2
    neg             ga
    vmovdqu         x1, [g1+ga]             ; Load top value
    vpbroadcastb    x0, x1                  ; Broadcast the first byte as the left value.
    vmovdqa         x2, x1                  ; Set top-left.
    jmp             .LEFT_AND_TOP_FETCHED


    .TOP_NOT_AVAILABLE:

    vpbroadcastd    x2, [pat_b_15]
    vpshufb         x1, x0, x2              ; Replicate C0 as the top neighbours.
    vmovdqa         x2, x1                  ; Set top-left.
    jmp             .LEFT_AND_TOP_FETCHED


    .BOTTOM_AVAILABLE:

    ; Get D from the pred buffer.
    lea             ga, [g1-4+8*g2]
    load            x3, x4, x5

    lea             ga, [ga+g2*4]
    load            x4, x5, x6
    %unmacro load 3

    vpblendd        x3, x3, x4, 0b0101

    ; Merge C and D.
    vpblendd        x0, x0, x3, 0b0011      ; [f e d c b a 9 8 7 6 5 4 3 2 1 0].

    cmp             g6, 12
    ja             .BOTTOM_FETCHED

    ; Broadcast the 5th value over the invalid neighbours.
    vpalignr        x3, x0, x0, 4
    vpbroadcastb    x3, x3
    vpblendd        x0, x0, x3, 0b0001
    jmp             .BOTTOM_FETCHED


    .NOTHING_AVAILABLE:

    vpbroadcastd    y0, [pat_b_128]         ; Store 128 everywhere.

    vmovdqu         [g0+48], x0             ; Save it.
    vmovdqu         [g0+64], x0
    vmovd           [g0+128], x0

    vmovdqu         [g0+160+48], x0         ; Save the filtered version.
    vmovdqu         [g0+160+64], x0
    vmovd           [g0+160+128], x0
    RET



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Duplicate above functions for 4x4 block size.


; Intra DC 4x4.
DEFFUN f265_lbd_predict_intra_dc_4_avx2, ia=4, at=8844, ti=0, tv=4, ym=0
    ; Logic:
    ; Sum all direct neighbours.
    ; Divide with bias by the number of samples.

    vmovd           x2, [g1+60]             ; Load left neighbours.
    vmovd           x3, [g1+64]             ; Load top neighbours.

    vpmovzxbw       x2, x2                   ; Byte to word.
    vpmovzxbw       x3, x3

    ; Add top and left neighbours.
    vpaddw          x0, x2, x3

    ; Fold in two the subresult.
    vpalignr        x1, x0, x0, 4
    vpaddw          x0, x0, x1

    ; Get a single sum.
    vphaddw         x0, x0, x0

    ; Round.
    vmovd           x1, [pat_w_4096]
    vpmulhrsw       x0, x0, x1              ; Round.
    vpbroadcastb    x0, x0

    ; Should it be filtered?
    test            g3, 1
    jz              .SKIP_FILTER

    ; 3 cases:
    ; - Top-left = 2*base + top + left.
    ; - Top =  3*base + top.
    ; - Left = 3*base + left.

    ; Blend top and left neighbours.
    vpalignr        x3, x3, x3, 8
    vpblendd        x2, x3, x2, 0b0011

    movd            g2d, x0                 ; Extract base.
    and             g2, 0xff

    lea             g3, [3*g2+2]            ; Base * 3 + rounding bias.
    movd            x1, g3d
    vpbroadcastw    x1, x1                  ; Broadcast base * 3 + rounding bias.

    vpaddw          x2, x1                  ; 3 * base + neighbours + rounding bias.
    vpsrlw          x2, 2                   ; Divide by 4.
    vpackuswb       x2, x2                  ; Word to byte.

    vpalignr        x1, x2, 4
    vpblendd        x0, x1, x0, 0xfe        ; Blend in the top row.

    vpshufb         x2, x2, [intra_hor_4]
    vpbroadcastd    x1, [pat_q_255]
    vpblendvb       x0, x0, x2, x1

    ; Do top-left.
    movzx           g3, byte [g1+64]        ; Load the first top and left value.
    movzx           ga, byte [g1+63]
    add             g3, ga                  ; Top + left.
    lea             g2, [2*g2+g3+2]         ; Top + left + 2*base + bias.
    shr             g2, 2                   ; Get the average.
    vpinsrb         x0, g2b, 0              ; Insert the result.

    .SKIP_FILTER:

    vmovdqu         [g0], x0                ; Save the results.

    RET


; Intra planar 4x4.
DEFFUN f265_lbd_predict_intra_planar_4_avx2, ia=4, at=8844, ti=0, tv=4, ym=1
    ; Value[x,y] = ((3-x)*left + (3-y)*top + (x+1)*top_right + (y+1)*bottom_left + 4) >> 3);

    ; Get top neighbour weighted values.
    vpbroadcastd    y1, [g1+64]             ; Load & broadcast top neighbours.
    vmovd           x2, [g1+59]             ; Load & broadcast bottom-left.
    vpbroadcastb    y2, x2
    vpunpcklbw      y1, y1, y2              ; Mix top with bottom-left neighbours.
    vpmaddubsw      y1, y1, [intra_p_hor_4] ; Get the value of each column.

    ; Get left neighbour weighted values.
    vpbroadcastd    y3, [g1+60]             ; Load left neighbours.
    vpshufb         y3, [intra_hor_4]       ; Re-order them.
    vmovd           x2, [g1+68]             ; Load & broadcast top-right.
    vpbroadcastb    y2, x2
    vpunpcklbw      y3, y3, y2              ; Mix top right with left neighbours.
    vpbroadcastq    y0, [intra_p_ver_4]     ; Load & broadcast weight.
    vpmaddubsw      y3, y3, y0              ; Get the value of each row.

    vpaddw          y1, y3                  ; Get the final sums.
    vpbroadcastd    y0, [pat_w_4096]        ; Round the sums.
    vpmulhrsw       y1, y1, y0
    vpackuswb       y1, y1

    vmovq           [g0], x1                ; Save the result.
    vextracti128    x1, y1, 1
    vmovq           [g0+8], x1
    RET


; Intra pure diagonal bottom-left 4x4.
DEFFUN f265_lbd_predict_intra_dia_bot_left_4_avx2, ia=4, at=8844, ti=0, tv=1, ym=0
    vmovq           x0, [g1+56]             ; Load left neighbours.
    vpshufb         x0, [intra_dia_bl_4]    ; Re-order them.
    vmovdqu         [g0], x0                ; Save the results.
    RET


; Calculate weight and offset.
; Register usage:
; - y0: angle values. Output as the offset.
; - y1: weight (output).
; - y2: tmp.
%macro GEN_WEIGHT_OFF 2                     ; %1: 1 if vertical prediction, %2: 1 if we need to import neighbour
                                            ; from the other side.
    %assign IS_VERT %1
    %assign IS_INVERSED %2

    ; Generate weights.
    vpbroadcastd    y2, [pat_w_31]          ; Load weight mask.
    vpand           y1, y0, y2              ; Extract weight.
    vpbroadcastd    y2, [pat_w_32]          ; Load weight sum.
    vpsubw          y2, y2, y1              ; Get weight complement.

    %if (IS_VERT && IS_INVERSED) || (!IS_VERT && !IS_INVERSED)
    vpsllw          y2, y2, 8
    vpor            y1, y1, y2
    %elif (IS_VERT && !IS_INVERSED) || (!IS_VERT && IS_INVERSED)
    vpsllw          y1, y1, 8
    vpor            y1, y1, y2
    %endif

    ; Generate offset.
    vpsrlw          y0, y0, 5               ; Get the offset (word).

    %if !IS_VERT && IS_INVERSED
    vpaddw          y0, [intra_inv_ox_4]    ; Add the effect of rows.
    %elif !IS_VERT && !IS_INVERSED
    vpaddw          y0, [intra_ox_4]        ; Add the effect of rows.
    %elif IS_VERT && !IS_INVERSED
    vpbroadcastq    y2, [angle_inv_mul_hor]
    vpaddw          y0, y2                  ; Add the effect of columns.
    %elif IS_VERT && IS_INVERSED
    ; No processing in this case.
    %endif

    ; Double word value (once per byte).
    vpsllw          y2, y0, 8
    vpor            y0, y2, y0

    %if !IS_VERT && !IS_INVERSED
    vpbroadcastd    y2, [pat_b_14_15]       ; Generate offset pairs.
    vpsubw          y0, y2, y0              ; Final offset.

    %elif !IS_VERT && IS_INVERSED
    vpbroadcastd    y2, [pat_b_4_5]         ; Generate offset pairs.
    vpaddw          y0, y2, y0              ; Final offset.

    %elif IS_VERT && !IS_INVERSED
    vpbroadcastd    y2, [pat_b_0_1]         ; Generate offset pairs.
    vpaddw          y0, y2, y0              ; Final offset.

    %elif IS_VERT && IS_INVERSED
    vpbroadcastq    y2, [intra_tl_4]        ; Generate offset pairs.
    vpsubw          y0, y2, y0              ; Final offset.
    %endif
%endmacro


; Calculate prediction and save it.
; Register usage:
; - y0: offset.
; - y1: weight.
; - y%1: neighbours.
%macro DO_PRED 1
    vpshufb         y%1, y0                 ; Position neighbours pairs.
    vpmaddubsw      y%1, y1                 ; Multiply by weight.
    vpbroadcastw    y1, [pat_w_1024]
    vpmulhrsw       y%1, y1                 ; Round.

    vpackuswb       y%1, y%1, y%1           ; Word to byte.
    vmovq           [g0], x%1               ; Save the results.
    vextracti128    x%1, y%1, 1
    vmovq           [g0+8], x%1
%endmacro


; Calculate inverted neighbours offset.
%macro INV_RESHUF 3                         ; %1: 1 if we are doing vertical prediction, %2: output register, %3: tmp.
    %assign IS_VERT %1

    ; Load inversed angle.
    lea             g3, [inv_angle]
    %if !IS_VERT
    neg             g2
    vpbroadcastd    %2, [g3+g2*4+18*4]
    %else
    vpbroadcastd    %2, [g3+g2*4-18*4]      ; Load the inversed angle values.
    vmovq           %3, [angle_inv_mul_ver+8] ; Load the inversed weight values.
    %endif
    vpmullw         %2, %2, %3              ; Get the weight. Some neighbour will have an invalid offset.

    vpbroadcastd    %3, [pat_w_128]         ; Load offset rounding bias.
    %if IS_VERT
    vpaddw          %2, %3                  ; Add rounding.
    %else
    vpsubw          %2, %3                  ; Reuse an existing pattern that is "one off".
    %endif                                  ; Subtract bias to compensate.
    vpsraw          %2, 8                   ; Round.

    %if IS_VERT
    vpbroadcastd    %3, [pat_w_8]           ; Load inversed angle offset.
    vpsubb          %2, %3, %2              ; Invert the index.
    %endif

    vpackuswb       %2, %2, %2              ; Word to byte.
%endmacro


; Intra angular horizontal bottom 4x4.
DEFFUN f265_lbd_predict_intra_hor_bot_4_avx2, ia=4, at=8844, ti=0, tv=3, ym=1
    ; Get angle values.
    neg             g2
    lea             g3, [intra_angle]
    vpbroadcastd    y0, [g3+g2*4+9*4]

    vpbroadcastq    y2, [angle_mul_hor]     ; Load weight mask.
    vpmaddubsw      y0, y2                  ; Multiply to match lane offsets.

    GEN_WEIGHT_OFF  0, 0

    ; Calculate prediction.
    vbroadcasti128  y2, [g1+48]             ; Load neighbours.

    DO_PRED         2
    RET


; Intra angular horizontal top 4x4.
DEFFUN f265_lbd_predict_intra_hor_top_4_avx2, ia=4, at=8844, ti=0, tv=4, ym=1
    ; Get angle values.
    lea             g3, [intra_angle]
    vpbroadcastd    y0, [g3+g2*4-11*4]

    vpbroadcastq    y3, [angle_mul_hor]     ; Load weight mask.
    vpmaddubsw      y0, y3                  ; Multiply to match lane offsets.

    GEN_WEIGHT_OFF  0, 1

    ; Load inversed angle.
    INV_RESHUF      0, x2, x3

    vmovq           x3, [g1+64]             ; Load top data.
    vpshufb         x2, x3, x2              ; Re-order top neighbours.
    vpalignr        x3, x2, x2, 15

    ; Re-order neighbours.
    vmovq           x2, [g1+56]             ; Load left data.
    vpblendd        x2, x3, x2, 0b0011      ; Blend left neighbours with top neighbours.
    vpinsrb         x2, [g1+128], 8         ; Insert top-left neighbour.
    vinserti128     y2, y2, x2, 1

    DO_PRED         2
    RET


; Intra angular vertical left 4x4.
DEFFUN f265_lbd_predict_intra_ver_left_4_avx2, ia=4, at=8844, ti=0, tv=4, ym=1

    vmovq           x0, [g1+64-8]           ; Load left neighbours.
    vpinsrb         x0, [g1+128], 8         ; Load & insert top-left neighbour.

    ; Re-order the left neighbours.
    INV_RESHUF      1, x2, x1

    ; Blend re-ordered neighbours with the top neighbours.
    vpshufb         x2, x0, x2              ; Re-order left neighbours.
    vpbroadcastd    x3, [g1+64]             ; Load top neighbours with an offset in the register.
    vpblendd        x3, x2, x3, 0b1110      ; Blend with re-ordered left neighbours.
    vinserti128     y3, y3, x3, 1           ; Duplicate.

    ; Get angle values.
    neg             g2
    lea             g3, [intra_angle]
    vpbroadcastd    y0, [g3+g2*4+25*4]

    vpmaddubsw      y0, [intra_ver_4]       ; Multiply to match lane offsets.

    GEN_WEIGHT_OFF  1, 1
    DO_PRED         3
    RET


; Intra angular vertical right 4x4.
DEFFUN f265_lbd_predict_intra_ver_right_4_avx2, ia=4, at=8844, ti=0, tv=3, ym=1
    ; Get angle values.
    lea             g3, [intra_angle]
    vpbroadcastd    y0, [g3+g2*4-27*4]

    vpmaddubsw      y0, [intra_ver_4]       ; Multiply to match lane offsets.

    GEN_WEIGHT_OFF  1, 0

    ; Calculate prediction.
    vbroadcasti128  y2, [g1+64]             ; Load neighbours.

    DO_PRED         2
    RET

%unmacro GEN_WEIGHT_OFF 2
%unmacro DO_PRED 1
%unmacro INV_RESHUF 3


; Intra pure horizontal 4x4.
DEFFUN f265_lbd_predict_intra_hor_4_avx2, ia=4, at=8844, ti=0, tv=3, ym=0
    vmovd           x0, [g1+60]
    vpshufb         x0, [intra_hor_4]

    and             g3, 1
    jz              .SKIP_FILTER

    vpmovzxbw       x1, [g1+64]             ; Load top neighbours.
    vmovd           x2, [g1+128]            ; Load & broadcast top-left neighbour.
    vpbroadcastb    x2, x2
    vpmovzxbw       x2, x2                  ; Byte to word.
    vpsubw          x1, x2                  ; Top - top-left.
    vpsraw          x1, 1                   ; (top - top-left)/2.

    vmovd           x2, [g1+63]             ; Load & broadcast topmost left neighbour.
    vpbroadcastb    x2, x2
    vpmovzxbw       x2, x2                  ; Byte to word.
    vpaddw          x1, x2                  ; Left + (top - top-left)/2.

    vpxor           x2, x2                  ; Replace negative values by 0.
    vpmaxsw         x1, x2

    vpackuswb       x1, x1                  ; Word to byte with unsigned saturation.

    vpblendd        x0, x1, x0, 0xfe        ; Update the first row.

    .SKIP_FILTER:
    vmovdqu         [g0], x0                ; Save the results.
    RET


; Intra pure diagonal top-left 4x4.
DEFFUN f265_lbd_predict_intra_dia_top_left_4_avx2, ia=4, at=8844, ti=0, tv=1, ym=0
    vmovq           x0, [g1+56]             ; Load top row.
    vmovhps         x0, [g1+64]             ; Load left row.
    vpinsrb         x0, [g1+128], 0         ; Load & insert top-left.
    vpshufb         x0, [intra_dia_tl_4]    ; Shuffle.
    vmovdqu         [g0], x0                ; Save the results.
    RET


; Intra pure vertical 4x4.
DEFFUN f265_lbd_predict_intra_ver_4_avx2, ia=4, at=8844, ti=0, tv=3, ym=0
    vpbroadcastd    x0, [g1+64]             ; Copy the top neighbours 4 times.

    and             g3, 1
    jz              .SKIP_FILTER

    vpmovzxbw       x1, [g1+60]             ; Load left column.
    vmovd           x2, [g1+128]            ; Load & broadcast top-left.
    vpbroadcastb    x2, x2
    vpmovzxbw       x2, x2                  ; Byte to word.
    vpsubw          x1, x2                  ; Left - top-left.
    vpsraw          x1, 1                   ; (left - top-left)/2.

    vmovd           x2, [g1+64]             ; Load top.
    vpbroadcastb    x2, x2
    vpmovzxbw       x2, x2                  ; Byte to word.
    vpaddw          x1, x2                  ; Top + (left - top-left)/2.

    vpxor           x2, x2                  ; Replace negative values by 0.
    vpmaxsw         x1, x2

    vpackuswb       x1, x1                  ; Word to byte with unsigned saturation.
    vpshufb         x1, [intra_hor_4]       ; Re-order them.

    vpbroadcastd    x2, [pat_q_255]
    vpblendvb       x0, x0, x1, x2          ; Update the first byte of every rows.

    .SKIP_FILTER:
    vmovdqu         [g0], x0                ; Save the results.
    RET


; Intra angular top right 4x4.
DEFFUN f265_lbd_predict_intra_dia_top_right_4_avx2, ia=4, at=8844, ti=0, tv=1, ym=0
    vmovq           x0, [g1+64]             ; Load all data.
    vpshufb         x0, [intra_dia_tr_4]    ; Shuffle data.
    vmovdqu         [g0], x0                ; Save it.
    RET


; Extract and filter neighbours for intra prediction.
;
; Input format:
; EAB
; C
; D
;
; Output format:
;   padding   [56] [64]  padding [128]
; [ ...       DC   AB    ...     E]
;
; Input parameters:
; - g0: nbuf[2][160].
; - g1: pred.
; - g2: pred_stride.
; - g3: avail[2].
; - g4: filter_flag.
; - g5: packed (ignored).
DEFFUN f265_lbd_extract_intra_neigh_4_avx2, ia=6, at=884844, ti=1, tv=5, ym=1
    ; Get 4 left neighbours.
    ; Input:
    ; - %1: the xmm register in which to save the value.
    ; - %2: temp.
    ; - %3: temp.
    ; - ga: first row address. Must be aligned on the dword left of the row.
    ; - g2: pred_stride.
    ; - g3: 3*pred_stride.
    %macro load     3
    vpbroadcastd    %1, [ga]                ; Load & broadcast the left neighbour.
    vpbroadcastd    %2, [ga+g2]             ; Load & broadcast the next left neighbour.
    vpblendd        %1, %1, %2, 0b0101_0101 ; Mix even and odd row: result 1 0 1 0.

    vpbroadcastd    %2, [ga+g2*2]           ; Load & broadcast the next left neighbour.
    vpbroadcastd    %3, [ga+g3]             ; Load & broadcast the next left neighbour.
    vpblendd        %2, %2, %3, 0b0101_0101 ; Mix even and odd row: result 3 2 3 2.

    vpblendd        %1, %1, %2, 0b0011_0011 ; Mix 1 0 and 3 2. Result 3 2 1 0.
    vpshufb         %1, x4                  ; Keep the last byte of each dword.
    %endmacro


    ; Load availability.
    movzx           g5, byte [g3]           ; Load availx.
    movzx           g6, byte [g3+4]         ; Load availy.

    ; Test if left neighbours are available.
    cmp             g6, 0
    jz              .LEFT_NOT_AVAILABLE

    ; Left neighbours are available.
    vpbroadcastd    x4, [neigh_last_b_of_d] ; Load shuffle mask.

    lea             ga, [g1-4]              ; Align to load the left neighbours as a double.
    lea             g3, [g2*3]              ; 3* pred_stride.
    load            x0, x1, x2              ; Load C0 to C3.

    ; Test if bottom-left neighbours are available.
    cmp             g6, 4
    jg              .BOTTOM_AVAIL

    ; Bottom-left neighbours are unavailable. Broadcast the bottommost left neighbour.
    vpalignr        x1, x0, x0, 4
    vpbroadcastb    x1, x1
    vpblendd        x0, x0, x1, 0b0001

    ; Left present and loaded. Bottom-left loaded or emulated.
    .LEFT_LOADED:

    ; Test if the top neighbours are present.
    cmp             g5, 0
    jz              .TOP_NOT_AVAILABLE

    ; Load top and top-left neighbours.
    mov             ga, g2
    neg             ga
    vmovhps         x0, [g1+ga]             ; Load top neighbours. Also eagerly load top-left neighbour.
    vmovd           x2, [g1+ga-1]           ; Load top-left.

    .TOP_LOADED:

    ; Top-right neighbours might be missing.
    vpalignr        x1, x0, x0, 11          ; Broadcast rightmost top neighbours.
    vpbroadcastb    x1, x1

    sub             g5, 1                   ; Remove 1 from the number of available top neighbours (nb_availy).
    movd            x3, g5d                 ; Required because vpcmpgtb is signed and 128 is a possible value.
    vpbroadcastb    x3, x3                  ; Broadcast nb_availy.
    vmovdqu         x4, [neig_bl_unav_8]    ; Neighbour is present if nb_availy is at least the index of the neighbour.
    vpcmpgtb        x4,  x3                 ; Generate mask from availability.
    vpblendvb       x0, x0, x1, x4          ; Blend broadcasted neighbours if required.

    .LEFT_AND_TOP_LOADED:

    ; Save unfiltered neighbours.
    vmovq           [g0+56], x0
    vmovhps         [g0+64], x0
    vmovd           [g0+128], x2

    ; Test if filtering is required.
    cmp             g4, 0
    je              .SKIP_FILTER

    ; Filter.
    ; Pseudo code:
    ; Register ordering : D3, D2, D1, D0, C3, ..., C0, E, A0, ..., A3, B0, ... B3.
    ; V*[i] = (V[i-1] + 2*V[i] + V[i+1] + 2) >> 2.
    ; D*3 = D3, B*3 = B3.

    vpalignr        x3, x0, x0, 7           ; Offset top neighbours, keeping space for top-left.

    vmovd           g4d, x2                 ; Push top-left to register.
    vmovd           g3d, x0                 ; Keep a copy of D3.
    vpextrb         g2d, x0, 15             ; Keep a copy of B3.

    vpextrb         g1d, x0, 8              ; Extract A0.
    vpinsrb         x0, g1d, 9              ; Insert the A0 that will filter E.
    vpinsrb         x0, g4d, 8              ; Insert E.
    vpinsrb         x3, g4d, 0              ; Insert the E that will filter A0.
    vinserti128     y0, y0, x3, 1           ; Merge top row with left row.

    vpbroadcastd    y4, [pat_b_1]
    vpmaddubsw      y1, y0, y4              ; Add pairs of neighbours. Generate odd-even pairs (C3+C2, ...).
    vpalignr        y0, y0, 1               ; Offset pairs by one byte.
    vpmaddubsw      y0, y0, y4              ; Add pairs of neighbours. Generate even-odd pairs (C2+C1, ...).
    vpaddw          y2, y0, y1              ; Add both pairs together. Generate half of the sum (C3+2*C2+C1).

    vpalignr        y3, y0, y0, 14          ; Offset low pairs.
    vpblendd        y0, y0, y3, 0x0f        ; Keep high lane intact.

    vpalignr        y3, y1, y1, 2           ; Offset high pairs.
    vpblendd        y1, y1, y3, 0xf0        ; Keep low lane intact.

    vpaddw          y0, y0, y1              ; Generate remaining values (C2+2*C1+C0).

    ; Round.
    vpbroadcastd    y4, [pat_w_8192]
    vpmulhrsw       y0, y0, y4
    vpmulhrsw       y2, y2, y4

    ; Pack to byte.
    vpackuswb       y0, y0, y0
    vpackuswb       y2, y2, y2

    ; Get filtered left neighbours.
    vpunpcklbw      y3, y0, y2              ; Intermix result.
    vpinsrb         x3, g3d, 0              ; Insert not-filtered D3.
    vmovq           [g0+160+56], x3         ; Save filtered left neighbours.
    vmovhps         [g0+160+128], x3        ; Save filtered top-left.

    ; Get filtered top neighbours.
    vpunpcklbw      y3, y2, y0              ; Intermix result.
    vextracti128    x3, y3, 1               ; Extract high lane.
    vpinsrb         x3, g2d, 7              ; Insert not-filtered B3.
    vmovq           [g0+160+64], x3         ; Save filtered left neighbours.

    .SKIP_FILTER:
    RET


    .TOP_NOT_AVAILABLE:
    ; Left neighbours are present and loaded.
    vpalignr        x2, x0, x0, 7           ; C0 as E.
    vpbroadcastb    x2, x2                  ; Broadcast C0.
    vpblendd        x0, x0, x2, 0b1100      ; Blend broadcasted value as top neighbours.
    jmp             .LEFT_AND_TOP_LOADED


    .BOTTOM_AVAIL:
    ; Left neighbours are loaded.
    lea             ga, [ga+g2*4]
    load            x1, x2, x3              ; Load D0 to D3.
    vpblendd        x0, x0, x1, 0b0001      ; Blend bottom with left.
    jmp             .LEFT_LOADED


    .LEFT_NOT_AVAILABLE:
    ; Left is not available.

    ; Check if the top is available.
    cmp             g5, 0
    je              .NOTHING_AVAILABLE

    ; Top is available. Load and broadcast on left.
    mov             ga, g2
    neg             ga
    vmovq           x2, [g1+ga]             ; Load top neighbours. Preemptively load top-right.
    vpalignr        x0, x2, x2, 8           ; Place it within the register to the top neighbour offset.
    vpbroadcastb    x2, x2                  ; Broadcast C0.
    vpblendd        x0, x0, x2, 0b0011      ; Blend left with top.
    jmp             .TOP_LOADED


    .NOTHING_AVAILABLE:
    ; No neighbours present. Use the default value.
    vpbroadcastd    x0, [pat_b_128]         ; Broadcast default value.

    vmovq           [g0+56], x0             ; Save unfiltered.
    vmovq           [g0+64], x0
    vmovq           [g0+128], x0

    vmovq           [g0+160+56], x0         ; Save filtered.
    vmovq           [g0+160+64], x0
    vmovq           [g0+160+128], x0

    %unmacro load 3
    RET

