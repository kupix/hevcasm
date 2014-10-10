; Copyright (c) 2014, VANTRIX CORPORATION. All rights reserved. See LICENSE.txt
; for the full license text.

%include "x86inc.asm"

section .data

align 4
pat_pmaddubsw_sub:  times 2 db 0x01, 0xff   ; 1, -1 in every two bytes.
pat_dw_1:           dd 1
pat_dw_128:         dd 128
pat_dw_2:           dd 2
pat_dw_256:         dd 256
pat_dw_4:           dd 4
pat_dw_64:          dd 64
pat_dw_512:         dd 512
pat_dw_2048:        dd 2048
pat_dw_8:           dd 8
pat_dw_1024:        dd 1024
pat_idct16_sign:    dd 1, -1
pat_idct32_sign:    dw -1, -1

align 8
pat_idct16_shuf2:   db 0, 4, 2, 6, 7, 3, 5, 1

align 16
pat_dct4_shuf:      db 0,1, 6,7, 2,3, 4,5, 8,9, 14,15, 10,11, 12,13
pat_dct4_pass1:     dw 64,64, 64,64, 64,64, 64,64, 64,64, 64,64, 64,64, 64,64
                    dw 83,-83, 36,-36, 83,-83, 36,-36, 83,-83, 36,-36, 83,-83, 36,-36
                    dw 36,-36, 83,-83, 36,-36, 83,-83, 36,-36, 83,-83, 36,-36, 83,-83
pat_dct4_pass2:     dw 64,-64, 64,-64, 64,-64, 64,-64, -64,64, -64,64, -64,64, -64,64
                    dw 83,36, 83,36, 83,36, 83,36, -36,-83, -36,-83, -36,-83, -36,-83
                    dw 36,-83, 36,-83, 36,-83, 36,-83, 83,-36, 83,-36, 83,-36, 83,-36

pat_idct4_shuf1:    db 0,1, 8,9, 2,3, 10,11, 4,5, 12,13, 6,7, 14,15
; For IDCT4 Pass 1, the multiplication by 64 is done by a left shift operation. Hence, we need only the coefficients
; for 'add_1' and 'sub_1' (the notation is from the C code). The first 8 values are for calculating 'add_1' and the
; remaining 8 for 'sub_1'.
pat_idct4_pass1:    dw 83, 36, 83, 36, 83, 36, 83, 36,  36, -83, 36, -83, 36, -83, 36, -83
pat_idct4_pass2:    dw 64, 83, 64, 36, 64, 36, -64, -83, 64, -36, -64, 83, 64, -83, 64, -36
pat_idct4_sign:     dd 1, 1, 1, 1, -1, -1, -1, -1

pat_dst_pass2:      dw 29, 55, 29, 55, 29, 55, 29, 55,  74, 84, 74, 84, 74, 84, 74, 84
                    dw 84, -29, 84, -29, 84, -29, 84, -29,  -74, 55, -74, 55, -74, 55, -74, 55
                    dw 74, 74, 74, 74, 74, 74, 74, 74,  0, -74, 0, -74, 0, -74, 0, -74
                    dw 55, -84, 55, -84, 55, -84, 55, -84,  74, -29, 74, -29, 74, -29, 74, -29

pat_idst_shuf:      dd 0, 4, 1, 5, 2, 6, 3, 7
pat_idst_pass2:     dw 29, 74, 29, 74, 29, 74, 29, 74, 84, 55, 84, 55, 84, 55, 84, 55
                    dw 74, 0, 74, 0, 74, 0, 74, 0, -74, 74,  -74, 74, -74, 74,  -74, 74
                    dw 55, 74, 55, 74, 55, 74, 55, 74, -29, -84, -29, -84, -29, -84, -29, -84
                    dw 84, -74, 84, -74, 84, -74, 84, -74, 55, -29, 55, -29, 55, -29, 55, -29

pat_dct8_sign:      dw 1, 1, 1, 1, -1, -1, -1, -1
pat_dct8_shuf:      db 0x0e,0x0f,0x0c,0x0d,0x0a,0x0b,0x08,0x09,0x06,0x07,0x04,0x05,0x02,0x03,0x00,0x01
pat_dct8_pass1:     dw 64, 64, 64, 64, 18, 50, 75, 89, 83, 36, -36, -83, -50, -89, -18, 75
                    dw 64, -64, -64, 64, 75, 18, -89, 50, 36, -83, 83, -36, -89, 75, -50, 18
pat_dct8_pass2:     dw 64, 64, 64, 64, 83, 83, 36, 36, 64, 64, -64, -64, 36, 36, -83, -83
                    dw 89, -89, 75, -75, 75, -75, -18, 18, 50, -50, -89, 89, 18, -18, -50, 50
                    dw 64, 64, 64, 64, -36, -36, -83, -83, -64, -64, 64, 64, 83, 83, -36, -36
                    dw 50, -50, 18, -18, -89, 89, -50, 50, 18, -18, 75, -75, 75, -75, -89, 89

pat_idct8_pass1:    dw 64, 64, 83, 36, 64, -64, 36, -83, 64, -64, -36, 83, 64, 64, -83, -36
                    dw 89, 50, 75, 18, 75, -89, -18, -50, 50, 18, -89, 75, 18, 75, -50, -89
pat_idct8_pass2:    dw 64, 89, 83, 75, 64, 50, 36, 18, 64, 75, 36, -18, -64, -89, -83, -50
                    dw 64, 50, -36, -89, -64, 18, 83, 75, 64, 18, -83, -50, 64, 75, -36, -89
                    dw 64, -18, -83, 50, 64, -75, -36, 89, 64, -50, -36, 89, -64, -18, 83, -75
                    dw 64, -75, 36, 18, -64, 89, -83, 50, 64, -89, 83, -75, 64, -50, 36, -18

pat_dct16_pass1:    db 64, 64, 64, 64, 70, 80, 87, 90, 64, 64, 64, 64, 9, 25, 43, 57
                    db 89, 75, 50, 18, -43, 9, 57, 87, -18, -50, -75, -89, -25, -70, -90, -80
                    db 83, 36, -36, -83, -87, -70, 9, 80, -83, -36, 36, 83, 43, 90, 57, -25
                    db 75, -18, -89, -50, 9, -87, -43, 70, 50, 89, 18, -75, -57, -80 ,25, 90
                    db 64, -64, -64, 64, 90, -25, -80, 57, 64, -64, -64, 64, 70, 43, -87, -9
                    db 50, -89, 18, 75, 25, 57, -90, 43, -75, -18, 89, -50, -80, 9, 70, -87
                    db 36, -83, 83, -36, -80, 90, -70, 25, -36, 83, -83, 36, 87, -57, 9, 43
                    db 18, -50, 75, -89, -57, 43, -25, 9, 89, -75, 50, -18, -90, 87, -80, 70
pat_dct16_pass2:    db 64, 64, 64, 64, 64, 64, 64, 64, 90, -90, 87, -87, 80, -80, 70, -70,
                    db 89, 89, 75, 75, 50, 50, 18, 18, 87, -87, 57, -57, 9, -9, -43, 43,
                    db 83, 83, 36, 36, -36, -36, -83, -83, 80, -80, 9, -9, -70, 70, -87, 87,
                    db 75, 75, -18, -18, -89, -89, -50, -50, 70, -70, -43, 43, -87, 87, 9, -9,
                    db 64, 64, -64, -64, -64, -64, 64, 64, 57, -57, -80, 80, -25, 25, 90, -90,
                    db 50, 50, -89, -89, 18, 18, 75, 75, 43, -43, -90, 90, 57, -57, 25, -25,
                    db 36, 36, -83, -83, 83, 83, -36, -36, 25, -25, -70, 70, 90, -90, -80, 80,
                    db 18, 18, -50, -50, 75, 75, -89, -89, 9, -9, -25, 25, 43, -43, -57, 57,
                    db 64, 64, 64, 64, 64, 64, 64, 64, 57, -57, 43, -43, 25, -25, 9, -9,
                    db -18, -18, -50, -50, -75, -75, -89, -89, -80, 80, -90, 90, -70, 70, -25, 25,
                    db -83, -83, -36, -36, 36, 36, 83, 83, -25, 25, 57, -57, 90, -90, 43, -43,
                    db 50, 50, 89, 89, 18, 18, -75, -75, 90, -90, 25, -25, -80, 80, -57, 57,
                    db 64, 64, -64, -64, -64, -64, 64, 64, -9, 9, -87, 87, 43, -43, 70, -70,
                    db -75, -75, -18, -18, 89, 89, -50, -50, -87, 87, 70, -70, 9, -9, -80, 80,
                    db -36, -36, 83, 83, -83, -83, 36, 36, 43, -43, 9, -9, -57, 57, 87, -87,
                    db 89, 89, -75, -75, 50, 50, -18, -18, 70, -70, -80, 80, 87, -87, -90, 90

pat_idct16_pass1:   db 64, 89, 83, 75, 64, 50, 36, 18, 64, 75, 36, -18, -64, -89, -83, -50
                    db 64, 50, -36, -89, -64, 18, 83, 75, 64, 18, -83, -50, 64, 75, -36, -89
                    db 64, -18, -83, 50, 64, -75, -36, 89, 64, -50, -36, 89, -64, -18, 83, -75
                    db 64, -75, 36, 18, -64, 89, -83, 50, 64, -89, 83, -75, 64, -50, 36, -18
                    db 90, 87, 80, 70, 57, 43, 25, 9, 87, 57, 9, -43, -80, -90, -70, -25
                    db 80, 9, -70, -87, -25, 57, 90, 43, 70, -43, -87, 9, 90, 25, -80, -57
                    db 57, -80, -25, 90, -9, -87, 43, 70, 43, -90, 57, 25, -87, 70, 9, -80
                    db 25, -70, 90, -80, 43, 9, -57, 87, 9, -25, 43, -57, 70, -80, 87, -90
pat_idct16_pass2:   db 64, 89, 83, 75, 90, 87, 80, 70, 64, 50, 36, 18, 57, 43, 25, 9,
                    db 64, 75, 36, -18, 87, 57, 9, -43, -64, -89, -83, -50, -80, -90, -70, -25,
                    db 64, 50, -36, -89, 80, 9, -70, -87, -64, 18, 83, 75, -25, 57, 90, 43,
                    db 64, 18, -83, -50, 70, -43, -87, 9, 64, 75, -36, -89, 90, 25, -80, -57,
                    db 64, -18, -83, 50, 57, -80, -25, 90, 64, -75, -36, 89, -9, -87, 43, 70,
                    db 64, -50, -36, 89, 43, -90, 57, 25, -64, -18, 83, -75, -87, 70, 9, -80,
                    db 64, -75, 36, 18, 25, -70, 90, -80, -64, 89, -83, 50, 43, 9, -57, 87,
                    db 64, -89, 83, -75, 9, -25, 43, -57, 64, -50, 36, -18, 70, -80, 87, -90,
pat_idct16_shuf1:   db 0,1, 4,5, 8,9, 12,13, 2,3, 6,7, 10,11, 14,15
pat_idct16_shuf3:   db 0,1, 2,3, 4,5, 6,7, 8,9, 10,11, 12,13, 14,15
                    db 2,3, 0,1, 6,7, 4,5, 10,11, 8,9, 14,15, 12,13

pat_dct32_pass1:    db 64, 64, 64, 64, 70, 80, 87, 90, 64, 64, 64, 64, 9, 25, 43, 57
                    db 89, 75, 50, 18, -43, 9, 57, 87, -18, -50, -75, -89, -25, -70, -90, -80
                    db 83, 36, -36, -83, -87, -70, 9, 80, -83, -36, 36, 83, 43, 90, 57, -25
                    db 75, -18, -89, -50, 9, -87, -43, 70, 50, 89, 18, -75, -57, -80 ,25, 90
                    db 64, -64, -64, 64, 90, -25, -80, 57, 64, -64, -64, 64, 70, 43, -87, -9
                    db 50, -89, 18, 75, 25, 57, -90, 43, -75, -18, 89, -50, -80, 9, 70, -87
                    db 36, -83, 83, -36, -80, 90, -70, 25, -36, 83, -83, 36, 87, -57, 9, 43
                    db 18, -50, 75, -89, -57, 43, -25, 9, 89, -75, 50, -18, -90, 87, -80, 70
                    db 90, 90, 88, 85, 82, 78, 73, 67, 61, 54, 46, 38, 31, 22, 13, 4
                    db 90, 82, 67, 46, 22, -4, -31, -54, -73, -85, -90, -88, -78, -61, -38, -13
                    db 88, 67, 31, -13, -54, -82, -90, -78, -46, -4, 38, 73, 90, 85, 61, 22
                    db 85, 46, -13, -67, -90, -73, -22, 38, 82, 88, 54, -4, -61, -90, -78, -31
                    db 82, 22, -54, -90, -61, 13, 78, 85, 31, -46, -90, -67, 4, 73, 88, 38
                    db 78, -4, -82, -73, 13, 85, 67, -22, -88, -61, 31, 90, 54, -38, -90, -46
                    db 73, -31, -90, -22, 78, 67, -38, -90, -13, 82, 61, -46, -88, -4, 85, 54
                    db 67, -54, -78, 38, 85, -22, -90, 4, 90, 13, -88, -31, 82, 46, -73, -61
                    db 61, -73, -46, 82, 31, -88, -13, 90, -4, -90, 22, 85, -38, -78, 54, 67
                    db 54, -85, -4, 88, -46, -61, 82, 13, -90, 38, 67, -78, -22, 90, -31, -73
                    db 46, -90, 38, 54, -90, 31, 61, -88, 22, 67, -85, 13, 73, -82, 4, 78
                    db 38, -88, 73, -4, -67, 90, -46, -31, 85, -78, 13, 61, -90, 54, 22, -82
                    db 31, -78, 90, -61, 4, 54, -88, 82, -38, -22, 73, -90, 67, -13, -46, 85
                    db 22, -61, 85, -90, 73, -38, -4, 46, -78, 90, -82, 54, -13, -31, 67, -88
                    db 13, -38, 61, -78, 88, -90, 85, -73, 54, -31, 4, 22, -46, 67, -82, 90
                    db 4, -13, 22, -31, 38, -46, 54, -61, 67, -73, 78, -82, 85, -88, 90, -90
pat_dct32_pass2:    ; Coefficients for the 32 rows of output. Since we process 4 columns at a time, 4 consecutive
                    ; values are used for each row of output. This helps reuse the factors for different column sets.
                    ; - Note: The explanation below documents the values used by the columns as seen in the C code. It
                    ; does not explain whether the columns are added or subtracted to get the output (e.g. to
                    ; calculate dst[96], we have "-13*s_15_16" which actually means dst[96] = x - 13*(col 15 - col 16),
                    ; but in the explanation we only say that col 16 uses "-13". We do not talk about the math.
                    ; - Note: Output rows start from 0, hence dst[0] is output row 0, which is considered even, dst[32]
                    ; is output row 1, which is considered to be an odd row.

                    ; - The first 8 rows are the factors for columns 0, 1, 2, 3 to calculate the 32 output rows.
                    ; (16 elements/row => 4 sets of factors each with 4 elements. Hence, 8 rows => 8x4 = 32 sets, each
                    ; set corresponding to one row of output).
                    ; - The 8 rows (32 sets) are used by cols 31, 30, 29, 28 to calculate the 32 rows of output (note
                    ; the descending order of columns !).
                    ; - The first 8 rows are also used by cols 16, 17, 18, 19. Alternate sets (of 4 values) are used to
                    ; obtain the even output rows (e.g. <64, 64, 64, 64> is used to calulate dst[0]; <90, 87, 80, 70>
                    ; is used to calculate dst[64]..).
                    ; Alternate sets of 4 values from the end of the eigth row are used to calculate the odd rows.
                    ; However, the values are alternately negated (e.g. <4, 13, 22, 31> and not <4, -13, 22, -31> is
                    ; used to calculate dst[32]; <-13,-38,-61,-78> and not <13,-38,61,-78> is used to obtain dst[96]).
                    ; - The rows are also used by cols 15, 14, 13, 12 exactly like how they were used by cols
                    ; 16, 17, 18, 19 respectively (note the descending order of columns).
                    dw 64, 64, 64, 64, 90, 90, 88, 85, 90, 87, 80, 70, 90, 82, 67, 46
                    dw 89, 75, 50, 18, 88, 67, 31, -13, 87, 57, 9, -43, 85, 46, -13, -67
                    dw 83, 36, -36, -83, 82, 22, -54, -90, 80, 9, -70, -87, 78, -4, -82, -73
                    dw 75, -18, -89, -50, 73, -31, -90, -22, 70, -43, -87, 9, 67, -54, -78, 38
                    dw 64, -64, -64, 64, 61, -73, -46, 82, 57, -80, -25, 90, 54, -85, -4, 88
                    dw 50, -89, 18, 75, 46, -90, 38, 54, 43, -90, 57, 25, 38, -88, 73, -4
                    dw 36, -83, 83, -36, 31, -78, 90, -61, 25, -70, 90, -80, 22, -61, 85, -90
                    dw 18, -50, 75, -89, 13, -38, 61, -78, 9, -25, 43, -57, 4, -13, 22, -31
                    ; - Following 8 rows are the coefficients for  cols 4, 5, 6, 7 respectively to calculate the 32
                    ; rows of output.
                    ; - The 8 rows are used by cols 27, 26, 25, 24 to calculate the 32 rows of output (note the
                    ; descending order of columns).
                    ; - Alternate sets of values are used by cols 20, 21, 22, 23 to calculate even rows of output
                    ; (e.g. <64, 64, 64, 64> is used to get dst[0], <57, 43, 25, 9> for dst[64] ..).
                    ; Alternate sets of values starting from the end of the last row are used to calculate the odd rows
                    ; of the output. Note that alternate values in the sets are negated (e.g. <38, 46, 54, 61> and not
                    ; <38, -46, 54, -61> is used to calculate dst[32]; <-88, -90, -85, -73> and not the
                    ; <88, -90, 85, -73> is used to calculate dst[96]).
                    ; - The rows are also used by cols 11, 10, 9, 8 similar to cols 20, 21, 22, 23 (above). Note the
                    ; descending order of columns.
                    dw 64, 64, 64, 64, 82, 78, 73, 67, 57, 43, 25, 9, 22, -4, -31, -54
                    dw -18, -50, -75, -89, -54, -82, -90, -78, -80, -90, -70, -25, -90, -73, -22, 38
                    dw -83, -36, 36, 83, -61, 13, 78, 85, -25, 57, 90, 43, 13, 85, 67, -22
                    dw 50, 89, 18, -75, 78, 67, -38, -90, 90, 25, -80, -57, 85, -22, -90, 4
                    dw 64, -64, -64, 64, 31, -88, -13, 90, -9, -87, 43, 70, -46, -61, 82, 13
                    dw -75, -18, 89, -50, -90, 31, 61, -88, -87, 70, 9, -80, -67, 90, -46, -31
                    dw -36, 83, -83, 36, 4, 54, -88, 82, 43, 9, -57, 87, 73, -38, -4, 46
                    dw  89, -75, 50, -18, 88, -90, 85, -73, 70, -80, 87, -90, 38, -46, 54, -61
pat_dct32_sign_1:   times 8 dw 1, -1
pat_dct32_sign_2:   times 8 dw -1, 1

pat_idct32_pass1:   ; Since we process 4 rows at a time, 4 consecutive values are required to obtain one output row.
                    ; Each row below can be divided into sets of 4 consecutive values and are reused by the inputs.
                    ; - Note: The notation in the explanation below is from the C code.

                    ; The first 8 rows are used to calculate the odd terms (viz, s0, s1, .. s15).
                    ; - The first 4 rows (i.e. 16 sets) are used sequentially by input rows 1, 3, 5, 7 (i.e. src[32],
                    ; src[96], src[160] and src[224]) to calculate s0, s1, .. s15 (e.g. <90, 90, 88, 85> is used to
                    ; calculate s0; <90, 82, 67, 46> for s2 ...).
                    ; - The values in the first 4 rows are alternately negated and used by input rows 31, 29, 28, 27
                    ; (i.e. src[992], src[928], src[864] and src[800]) to obtain s15, s14, ... s0 respectively
                    ; (e.g. <-90, 90, -88, 85> is used to obtain s15; <90, -82, 67, -46> is used for s14). Note the
                    ; descending order of input rows, as well as the descending order of outputs (i.e. <s15, s14 .. s0>
                    ; instead of <s0, s1, ... s15>).
                    ; - The second 4 rows are used by input rows 9, 11, 13, 15 (i.e. src[288], src[352], src[416] and
                    ; src[480]) to calculate s0, s1 .. s15 (e.g. <82, 78, 73, 67> is used to obtain s0;
                    ; <22, -4, -31, -54> is used to obtain s1 ..).
                    ; - The second 4 row values are alternately negated and used by input rows 23, 21, 19, 17 (i.e.
                    ; src[736], src[672], src[608] and src[544]) to obtain s15, s14 .. s0 (e.g. <-82, 78, -73, 67>
                    ; is used to calculate s15; <22, 4, -31, 54> for s14). Note the descending order of input rows,
                    ; as well as the descending order of outputs.
                    dw 90, 90, 88, 85, 90, 82, 67, 46, 88, 67, 31, -13, 85, 46, -13, -67
                    dw 82, 22, -54, -90, 78, -4, -82, -73, 73, -31, -90, -22, 67, -54, -78, 38
                    dw 61, -73, -46, 82, 54, -85, -4, 88, 46, -90, 38, 54, 38, -88, 73, -4
                    dw 31, -78, 90, -61, 22, -61, 85, -90, 13, -38, 61, -78, 4, -13, 22, -31
                    dw 82, 78, 73, 67, 22, -4, -31, -54, -54, -82, -90, -78, -90, -73, -22, 38
                    dw -61, 13, 78, 85, 13, 85, 67, -22, 78, 67, -38, -90, 85, -22, -90, 4
                    dw 31, -88, -13, 90, -46, -61, 82, 13, -90, 31, 61, -88, -67, 90, -46, -31
                    dw 4, 54, -88, 82, 73, -38, -4, 46, 88, -90, 85, -73, 38, -46, 54, -61
                    ; The following rows are used to calculate the even terms (viz, a0, a1, .. a15).
                    ; The following first two rows are used to calculate e_0, e_1 .. e_7.
                    ; - The 8 sets of values in the following 2 rows are used sequentially by input rows 2, 6, 10, 14
                    ; (i.e. src[64], src[192], src[320] and src[448]) to obtain e_0, e_1, .. e_7 (e.g. <90, 87, 80, 70>
                    ;  is used to obtain e_0; <87, 57, 9, -43> for e_1 ..).
                    ; - The values are alternately negated and used by input rows 30, 26, 22, 18 (i.e. src[960],
                    ; src[832], src[704] and src[576]) to calculate e_7, e_6, .. e_0 (e.g. <-90, 87, -80, 70> is used
                    ; for e_7; <87, -57, 9, 43> for e_6). Note the descending order of input rows, and the outputs.
                    dw 90, 87, 80, 70, 87, 57, 9, -43, 80, 9, -70, -87, 70, -43, -87, 9
                    dw 57, -80, -25, 90, 43, -90, 57, 25, 25, -70, 90, -80, 9, -25, 43, -57
                    ; The following row is used by input rows 4, 12, 20, 28 (i.e. src[128], src[384], src[640] and
                    ; src[896]) to calculate a_4, a_5, a_6 and a_7 (e.g. <89, 75, 50, 18> is used to obtain a_4).
                    dw 89, 75, 50, 18, 75, -18, -89, -50, 50, -89, 18, 75, 18, -50, 75, -89
                    ; The following row is used to get a_0, a_1, a_2 and a_3 by input rows 0, 16, 8, 24 (i.e. src[0],
                    ; src[512], src[256] and src[768]). Note the order of input rows. <64, 64, 83, 36> is used to
                    ; obtain a_0 and a_3, and <64, -64, 36, -83> for a_1 and a_2.
                    dw 64, 64, 83, 36, 64, -64, 36, -83
pat_idct32_pass2:   ; The first 8 rows are used to calculate the even terms (viz, a0, a1 .. a15). The symmetry of the
                    ; even terms helps reduce multiplications, and thereby reduce the number of IDCT factors required.
                    ; While combining the odd and even terms in IDCT Pass 1, the terms used to calculate a_0 .. a_7
                    ; (i.e. columns 0, 4, 8, 12, 16, 20, 24 and 28) in IDCT Pass 2 are grouped together and the terms
                    ; for e_0, ..e_7 (i.e. columns 2, 6, 10, 14, 18, 22, 26 and 30) are grouped together. In IDCT
                    ; Pass 2, just one set of multiplications gives 2 sets of outputs (e.g. a0 = a_0 + e_0 + x,
                    ; a15 = a_0 - e_0 + x). Likewise, IDCT factors in the following 8 rows are also grouped together
                    ; (e.g. <64, 89, 83, 75, 64, 50, 36, 18> is used for a_0; <90, 87, 80, 70, 57, 43, 25, 9> for e_0).
                    db 64, 89, 83, 75, 64, 50, 36, 18, 90, 87, 80, 70, 57, 43, 25, 9
                    db 64, 75, 36, -18, -64, -89, -83, -50, 87, 57, 9, -43, -80, -90, -70, -25
                    db 64, 50, -36, -89, -64, 18, 83, 75, 80, 9, -70, -87, -25, 57, 90, 43
                    db 64, 18, -83, -50, 64, 75, -36, -89, 70, -43, -87, 9, 90, 25, -80, -57
                    db 64, -18, -83, 50, 64, -75, -36, 89, 57, -80, -25, 90, -9, -87, 43, 70
                    db 64, -50, -36, 89, -64, -18, 83, -75, 43, -90, 57, 25, -87, 70, 9, -80
                    db 64, -75, 36, 18, -64, 89, -83, 50, 25, -70, 90, -80, 43, 9, -57, 87
                    db 64, -89, 83, -75, 64, -50, 36, -18, 9, -25, 43, -57, 70, -80, 87, -90
                    ; The following 16 rows are used to calculate the 16 odd terms (s0, s1 .. s15).
                    ; Each row below gives a term (e.g. row 0 => s0, row 1 => s1, ... row 15 => s15).
                    db 90, 90, 88, 85, 82, 78, 73, 67, 61, 54, 46, 38, 31, 22, 13, 4
                    db 90, 82, 67, 46, 22, -4, -31, -54, -73, -85, -90, -88, -78, -61, -38, -13
                    db 88, 67, 31, -13, -54, -82, -90, -78, -46, -4, 38, 73, 90, 85, 61, 22
                    db 85, 46, -13, -67, -90, -73, -22, 38, 82, 88, 54, -4, -61, -90, -78, -31
                    db 82, 22, -54, -90, -61, 13, 78, 85, 31, -46, -90, -67, 4, 73, 88, 38
                    db 78, -4, -82, -73, 13, 85, 67, -22, -88, -61, 31, 90, 54, -38, -90, -46
                    db 73, -31, -90, -22, 78, 67, -38, -90, -13, 82, 61, -46, -88, -4, 85, 54
                    db 67, -54, -78, 38, 85, -22, -90, 4, 90, 13, -88, -31, 82, 46, -73, -61
                    db 61, -73, -46, 82, 31, -88, -13, 90, -4, -90, 22, 85, -38, -78, 54, 67
                    db 54, -85, -4, 88, -46, -61, 82, 13, -90, 38, 67, -78, -22, 90, -31, -73
                    db 46, -90, 38, 54, -90, 31, 61, -88, 22, 67, -85, 13, 73, -82, 4, 78
                    db 38, -88, 73, -4, -67, 90, -46, -31, 85, -78, 13, 61, -90, 54, 22, -82
                    db 31, -78, 90, -61, 4, 54, -88, 82, -38, -22, 73, -90, 67, -13, -46, 85
                    db 22, -61, 85, -90, 73, -38, -4, 46, -78, 90, -82, 54, -13, -31, 67, -88
                    db 13, -38, 61, -78, 88, -90, 85, -73, 54, -31, 4, 22, -46, 67, -82, 90
                    db 4, -13, 22, -31, 38, -46, 54, -61, 67, -73, 78, -82, 85, -88, 90, -90
pat_idct32_shuf1:   dd 7, 6, 5, 4, 3, 2, 1, 0
pat_idct32_shuf2:   db 6,7, 4,5, 2,3, 0,1, 14,15, 12,13, 10,11, 8,9

align 32
pat_dst_pass1:      dw 29, 55, 74, 84, 74, 74, 0, -74, 84, -29, -74, 55, 55, -84, 74, -29
pat_idst_pass1:     dw 29, 74, 84, 55, 55, 74, -29, -84, 74, 0, -74, 74, 84, -74, 55, -29

section .text

; Note: the DCT factor expansion slow things down when using an explicit loop.
; Eventually reconsider the decision to expand the factors.

; ---------------------- DST/IDST/DCT/IDCT 4 macros ---------------------
%macro LOAD_64BIT 3                         ; %1: out 0, %2: out 1, %3: memory location to load from.
vpbroadcastq    %1, [%3]
vpbroadcastq    %2, [%3 + 8]
%endmacro

%macro MULT_4 5                             ; %1: in/mult 0, %2: in/mult 1, %3: in/mult 2, %4: in/mult 3, %5: input.
vpmaddwd        %1, %5, %1
vpmaddwd        %2, %5, %2
vpmaddwd        %3, %5, %3
vpmaddwd        %4, %5, %4
%endmacro

%macro PERM_4 6                             ; %1: out 0, %2: out 1, %3: in 0/out 2, %4: in 1, %5: in 2/out 3, %6: in 3.
vperm2i128      %1, %3, %4, 0x20
vperm2i128      %3, %3, %4, 0x31
vperm2i128      %2, %5, %6, 0x20
vperm2i128      %5, %5, %6, 0x31
%endmacro

%macro DCT_PASS2 0                          ; Could not be called as a function since cycles increased in DCT4x4.
PERM_4          y1, y3, y0, y5, y2, y4
vpaddd          y1, y1, y7
vpaddd          y3, y3, y7
vpaddd          y0, y0, y1
vpaddd          y2, y2, y3
vpsrad          y0, y0, 8
vpsrad          y2, y2, 8
vpackssdw       y0, y0, y2
vmovdqu         [g0], y0
%endmacro

; DCT 4x4.
;
; Input parameters:
; - g0:     destination.
; - g1:     source.
; - g2:     source stride.
; - g3:     prediction.
; - g4:     prediction stride.
; - g5:     spill buffer.
DEFFUN f265_lbd_dct_4_avx2, ia=6, at=884848, fa=0, ti=0, tv=8, ym=1

    ; Compute the residual.
    call            res_4x4                 ; Load the source and prediction values.
    lea             g1, [pat_dct4_pass1]
    vbroadcasti128  y3, [pat_dct4_shuf]
    vpbroadcastd    y6, [pat_dw_1]
    vpsubw          y0, y0, y7              ; Residual.

    ; Now for the DCT Pass 1.
    vpshufb         y0, y0, y3
    vpmaddwd        y1, y0, [g1]
    vpmaddwd        y2, y0, [g1 + 32]
    vpmaddwd        y3, y0, [g1 + 64]
    lea             g2, [pat_dct4_pass2]    ; Multiplication factors for pass 2.
    vmovdqu         y0, [g1]
    vmovdqu         y5, [g2]
    vphaddd         y7, y1, y2
    vphsubd         y3, y1, y3
    vmovdqu         y2, [g2 + 32]
    vmovdqu         y4, [g2 + 64]
    vpaddd          y1, y7, y6
    vpaddd          y3, y3, y6
    vpsrad          y1, y1, 1
    vpsrad          y3, y3, 1
    vpackssdw       y1, y1, y3              ; Output: 15.14.11.10  7.6.3.2 | 13.12.9.8  5.4.1.0.

    ; DCT Pass 2.
    MULT_4          y0, y5, y2, y4, y1
    vpbroadcastd    y7, [pat_dw_128]
    DCT_PASS2                               ; Sadly, rearrangement to call the macro as a function increased cycles.
    RET

; IDCT 4x4.
;
; Input parameters:
; - g0:     destination.
; - g1:     destination stride.
; - g2:     prediction.
; - g3:     prediction stride.
; - g4:     coefficients.
; - g5:     spill buffer.
DEFFUN f265_lbd_idct_4_avx2, ia=6, at=848488, fa=0, ti=0, tv=8, ym=1

    ; Do IDCT4 Pass 1.
    vmovdqu         y0, [g4]
    vmovdqu         y4, [pat_idct4_pass1]   ; Load multiplication factors.
    vpermq          y1, y0, 0x00            ;     3.2.1.0  3.2.1.0 | 3.2.1.0  3.2.1.0.
    vpermq          y2, y0, 0xAA            ; 11.10.9.8  11.10.9.8 | 11.10.9.8  11.10.9.8.
    vpermq          y0, y0, 0xDD            ; 15.14.13.12  7.6.5.4 | 15.14.13.12  7.6.5.4.
    vbroadcasti128  y3, [pat_idct4_shuf1]   ; Load shuffle pattern.
    vpbroadcastd    y6, [pat_dw_64]         ; Bias.
    vpmovsxwd       y1, x1
    vpmovsxwd       y2, x2
    vpsignd         y2, y2, [pat_idct4_sign]
    vpshufb         y0, y0, y3
    vpmaddwd        y0, y0, y4              ; Obtain sub_1 | add_1.
    vpaddd          y1, y1, y2
    vpslld          y1, y1, 6               ; Obtain sub_0 | add_0.
    vpaddd          y1, y1, y6              ; Add bias.
    vpaddd          y4, y1, y0              ; sub_0 + sub_1 | add_0 + add_1.
    vpsubd          y5, y1, y0              ; sub_0 - sub_1 | add_0 - add_1.
    vpsrad          y4, y4, 7               ; Shift.
    vpsrad          y5, y5, 7
    lea             g5, [pat_idct4_pass2]
    vpackssdw       y0, y4, y5              ; Convert to 16-bit and pack.
    lea             g4, [g2 + 2*g3]

    ; Do IDCT4 Pass 2.
    LOAD_64BIT      y2, y3, g5
    vpermq          y0, y0, 0x78            ; Shuffle input: 15.11.7.3  14.10.6.2 | 13.9.5.1  12.8.4.0.
    LOAD_64BIT      y1, y4, g5 + 16
    MULT_4          y2, y3, y1, y4, y0
    vpbroadcastd    y6, [pat_dw_2048]       ; Bias.
    vmovd           x5, [g2]                ; Load line 0 of prediction.
    vphaddd         y3, y2, y3
    vbroadcasti128  y7, [pat_idct16_shuf1]
    vphaddd         y1, y1, y4
    vpunpckldq      x5, x5, [g2 + g3]       ; Load line 1 of prediction.
    vmovd           x0, [g4]                ; Load line 2 of prediction.
    vpaddd          y3, y3, y6
    vpaddd          y1, y1, y6
    vpunpckldq      x0, x0, [g4 + g3]       ; Load line 3 of prediction.
    vpsrad          y1, y1, 12
    vpunpcklqdq     y0, y5, y0
    vpsrad          y3, y3, 12
    vpmovzxbw       y0, x0
    vpackssdw       y4, y3, y1
    vpshufb         y1, y4, y7
    call            recon_4x4               ; Reconstruction.
    RET

; DST.
;
; Input parameters:
; - g0:     destination.
; - g1:     source.
; - g2:     source stride.
; - g3:     prediction.
; - g4:     prediction stride.
; - g5:     spill buffer.
DEFFUN f265_lbd_dct_dst_avx2, ia=6, at=884848, fa=0, ti=0, tv=8, ym=1

    ; Compute the residual.
    call            res_4x4                 ; Load the source and prediction values.
    lea             g1, [pat_dst_pass1]
    LOAD_64BIT      y1, y2, g1
    vpsubw          y0, y0, y7              ; Residual.
    LOAD_64BIT      y3, y4, g1 + 16

    ; Now for the DST Pass 1.
    MULT_4          y1, y2, y3, y4, y0
    lea             g2, [pat_dst_pass2]
    vpbroadcastd    y6, [pat_dw_1]          ; Bias.
    vpbroadcastd    y7, [pat_dw_128]        ; Bias.
    vmovdqu         y0, [g2]
    vmovdqu         y5, [g2 + 32]
    vphaddd         y1, y1, y2
    vphaddd         y3, y3, y4
    vmovdqu         y2, [g2 + 64]
    vmovdqu         y4, [g2 + 96]
    vpaddd          y1, y1, y6              ; Add bias.
    vpaddd          y3, y3, y6
    vpsrad          y1, y1, 1
    vpsrad          y3, y3, 1
    vpackssdw       y1, y1, y3              ; Output: 15.14.11.10  7.6.3.2 | 13.12.9.8  5.4.1.0.

    ; Now for DST Pass 2.
    MULT_4          y0, y5, y2, y4, y1
    DCT_PASS2
    RET

; IDST.
;
; Input parameters:
; - g0:     destination.
; - g1:     destination stride.
; - g2:     prediction.
; - g3:     prediction stride.
; - g4:     coefficients.
; - g5:     spill buffer.
DEFFUN f265_lbd_idct_dst_avx2, ia=6, at=848488, fa=0, ti=0, tv=9, ym=1

    ; Do IDST Pass 1.
    vmovdqu         y0, [g4]
    vbroadcasti128  y3, [pat_idct4_shuf1]   ; Load shuffle pattern.
    lea             g5, [pat_idst_pass1]
    vmovdqu         y8, [pat_idst_shuf]
    vpbroadcastd    y6, [pat_dw_64]
    vpshufb         y0, y0, y3
    LOAD_64BIT      y1, y2, g5
    vpermd          y0, y8, y0
    LOAD_64BIT      y5, y4, g5 + 16
    MULT_4          y1, y2, y5, y4, y0
    lea             g5, [pat_idst_pass2]
    vmovdqu         y0, [g5]
    vmovdqu         y7, [g5 + 32]
    vphaddd         y1, y1, y2
    vphaddd         y5, y5, y4
    vmovdqu         y2, [g5 + 64]
    vmovdqu         y4, [g5 + 96]
    vpaddd          y1, y1, y6
    vpaddd          y5, y5, y6
    vpsrad          y1, y1, 7
    vpsrad          y5, y5, 7
    vpackssdw       y1, y1, y5              ; Output: 15.11.14.10  13.9.12.8 | 7.3.6.2  5.1.4.0.
    lea             g4, [g2 + 2*g3]

    ; Do IDST Pass 2.
    MULT_4          y0, y7, y2, y4, y1
    vpbroadcastd    y6, [pat_dw_2048]
    vmovd           x5, [g2]                ; Load line 0 of prediction.
    PERM_4          y1, y7, y0, y7, y2, y4
    vpaddd          y1, y1, y6
    vpaddd          y7, y7, y6
    vpunpckldq      x5, x5, [g2 + g3]       ; Load line 1 of prediction.
    vmovd           x6, [g4]                ; Load line 2 of prediction.
    vpaddd          y0, y1, y0
    vpaddd          y2, y2, y7
    vpunpckldq      x6, x6, [g4 + g3]       ; Load line 3 of prediction.
    vpunpcklqdq     y1, y5, y6
    vpsrad          y0, y0, 12
    vpsrad          y2, y2, 12
    vpmovzxbw       y1, x1
    vpackssdw       y0, y0, y2              ; Pack output to 16-bit.
    vpshufb         y0, y0, y3
    vpermd          y0, y8, y0
    call            recon_4x4               ; Reconstruction.
    RET

    ; IDCT4/IDST helper function - Reconstruction.
recon_4x4:
    vpaddw          y0, y1, y0
    vpackuswb       y1, y0, y0
    vextracti128    x0, y1, 1
    vpsrldq         y3, y1, 4
    vmovd           [g0], x1
    vmovd           [g0 + g1], x3
    lea             g0, [g0 + 2*g1]
    vpsrldq         y4, y0, 4
    vmovd           [g0], x0
    vmovd           [g0 + g1], x4
    ret

    ; DCT4/DST helper function - Residual calculation.
res_4x4:
    %macro LOAD_2ROWS 6                     ; %1: source register, %2: pred register, %3: source location
                                            ; %4: pred location, %5: source offset, %6: prediction offset.
    vmovd           %1, [%3]                ; Load source line "n".
    vmovd           %2, [%4]                ; Load prediction line "n".
    vpunpckldq      %1, %1, [%3 + %5]       ; Load source line "n+1".
    vpunpckldq      %2, %2, [%4 + %6]       ; Load prediction line "n+1".
    %endmacro
    LOAD_2ROWS      x0, x2, g1, g3, g2, g4  ; Load source & prediction lines 0 and 1.
    lea             g1, [g1 + 2*g2]
    lea             g3, [g3 + 2*g4]
    LOAD_2ROWS      x1, x3, g1, g3, g2, g4  ; Load source & prediction line 2 and 3.
    vpunpcklqdq     x0, x0, x1
    vpunpcklqdq     x2, x2, x3
    vpmovzxbw       y0, x0                  ; Source values.
    vpmovzxbw       y7, x2                  ; Prediction values.
    %unmacro LOAD_2ROWS 6
    ret

%unmacro LOAD_64BIT 3
%unmacro MULT_4 5
%unmacro DCT_PASS2 0

; DCT 8x8.
;
; Input parameters:
; - g0:     destination.
; - g1:     source.
; - g2:     source stride.
; - g3:     prediction.
; - g4:     prediction stride.
; - g5:     spill buffer.
DEFFUN f265_lbd_dct_8_avx2, ia=6, at=884848, fa=0, ti=0, tv=10, ym=1

    ; Compute the residual.
    vpbroadcastd    y2, [pat_pmaddubsw_sub] ; Residual computation pattern (src-pred).

    ; Process 2 rows at a time.
    xor             g5, g5                  ; Loop counter.
    .loop_res:
    vmovdqu         y0, [g1]                ; Load source line 0.
    vinserti128     y0, y0, [g1 + g2], 1    ; Load source line 1.
    vmovdqu         y1, [g3]                ; Load prediction line 0.
    vinserti128     y1, y1, [g3 + g4], 1    ; Load prediction line 1.

    lea             g1, [g1 + 2*g2]         ; Update the source and the prediction pointers.
    lea             g3, [g3 + 2*g4]

    vpunpcklbw      y0, y0, y1              ; Interleave the source and the prediction.
    vpmaddubsw      y0, y0, y2              ; Compute and store the residual.
    vmovdqu         [g0 + g5], y0

    add             g5, 32                  ; Pass to the next rows.
    cmp             g5, 128
    jne             .loop_res

    ; The DCT consists of two passes, one on the rows, one on the columns.
    ;
    ; The residual fits in 9 bits. Thus the addition/subtraction of 2 input
    ; values can be held in 16 bits. It is advantageous to perform the
    ; additions before the multiplications to reduce the total number of
    ; multiplications. However, this technique is not applicable in the second
    ; pass since the input values use 16 bits.
    ;
    ; All multiplications are 16-bit x 16-bit resulting in 32-bit values.

    ; First DCT pass.
    vbroadcasti128  y3, [pat_dct8_shuf]     ; Shuffle mask.
    vbroadcasti128  y4, [pat_dct8_sign]     ; Sign mask.
    vbroadcasti128  y5, [pat_dct8_pass1]    ; Multiplication factors for each row pair.
    vbroadcasti128  y6, [pat_dct8_pass1 + 16]
    vbroadcasti128  y7, [pat_dct8_pass1 + 32]
    vbroadcasti128  y8, [pat_dct8_pass1 + 48]
    vpbroadcastd    y9, [pat_dw_2]          ; Bias.

    ; Process 2 rows at a time.
    xor             g5, g5                  ; Loop counter.
    .loop_pass1:
    vmovdqu         y0, [g0 + g5]           ; Load a row in each lane. Pixel positions 7654 3210.
    vpshufb         y1, y0, y3              ; Reorder positions                        0123 4567.
    vpsignw         y0, y0, y4              ; Invert the signs of the 4 last positions.
    vpaddw          y0, y0, y1              ; Results: sub(0,7)..sub(3,4), add(3,4)..add(0,7)

    vpmaddwd        y1, y5, y0              ; Process the first 4 elements of the output column.
    vpmaddwd        y2, y6, y0
    vphaddd         y2, y1, y2

    vpmaddwd        y1, y7, y0              ; Process the last 4 elements of the output column.
    vpmaddwd        y0, y8, y0
    vphaddd         y0, y1, y0

    vpaddd          y2, y9, y2              ; Add the bias and shift.
    vpaddd          y0, y9, y0
    vpsrad          y2, y2, 2
    vpsrad          y0, y0, 2

    vpackssdw       y0, y2, y0              ; Pack to 16-bit.
    vpermq          y0, y0, 0xd8            ; Interleave columns A and B: b7..b4 a7..a4 | b3..b0 a3..a0.
    vmovdqu         [g0 + g5], y0           ; Store the column.

    add             g5, 32                  ; Pass to the next rows.
    cmp             g5, 128
    jne             .loop_pass1

    ; Second DCT pass.

    ; The multiplication is performed before the addition/subtraction and
    ; therefore the inputs have to be rearranged accordingly.
    vmovdqu         y4, [g0]                ; Rows 0 & 1.
    vmovdqu         y5, [g0 + 32]           ; Rows 2 & 3.
    vpermq          y6, y0, 0xb1            ; Rows 7 and 6 (swapped).
    vpermq          y7, [g0 + 64], 0xb1     ; Rows 5 & 4 (swapped).

    vpunpcklwd      y0, y4, y6              ; Columns 0-7.
    vpunpckhwd      y1, y4, y6              ; Columns 1-6.
    vpunpcklwd      y2, y5, y7              ; Columns 2-5.
    vpunpckhwd      y3, y5, y7              ; Columns 3-4.

    lea g1,         [pat_dct8_pass2]        ; Table addresses.
    vpbroadcastd    y7, [pat_dw_256]        ; Bias.

    ; Process 2 rows at a time. 4 pairs of constants are loaded for each row.
    ; The first row is composed of the sum of the elements (0 & 7), (1 & 6),
    ; (2 & 5) and (3 & 4), while the second row is composed of the differences.
    ; The negative sign has been absorbed in the constants as there is no
    ; 'vpmsubwd' instruction.
    xor             g2, g2                  ; Loop counter.
    .loop_pass2:

    %macro PROCESS_ROW 3                    ; %1: output register, %2: DCT factor 0, %3: DCT factor 1.
    vpbroadcastd    y4, [g1 + %2]           ; Add/sub terms and multiply by factor. Columns 0-7.
    vpmaddwd        %1, y4, y0

    vpbroadcastd    y4, [g1 + %2 + 4]       ; Columns 1-6.
    vpmaddwd        y4, y4, y1
    vpaddd          %1, y4                  ; Sum the columns.

    vpbroadcastd    y4, [g1 + %3]           ; Columns 2-5.
    vpmaddwd        y4, y4, y2
    vpaddd          %1, y4

    vpbroadcastd    y4, [g1 + %3 + 4]       ; Columns 3-4.
    vpmaddwd        y4, y4, y3
    vpaddd          %1, y4

    vpaddd          %1, %1, y7              ; Add the bias and shift.
    vpsrad          %1, %1, 9
    %endmacro
    PROCESS_ROW     y5, 0, 64               ; First row.
    PROCESS_ROW     y6, 32, 96              ; Second row.
    %unmacro PROCESS_ROW 3

    vpackssdw       y5, y5, y6              ; Convert to 16-bit and pack the rows together.
    vpermq          y5, y5, 0xd8            ; Merge the row pixels together and store.
    vmovdqu         [g0 + g2], y5

    add             g2, 32                  ; Pass to the next row.
    add             g1, 8
    cmp             g2, 128
    jne             .loop_pass2
    RET

; IDCT 8x8.
;
; Input parameters:
; - g0:     destination.
; - g1:     destination stride.
; - g2:     prediction.
; - g3:     prediction stride.
; - g4:     coefficients.
; - g5:     spill buffer.
DEFFUN f265_lbd_idct_8_avx2, ia=6, at=848488, fa=0, ti=1, tv=13, ym=1

    ; I. Load the 16-bit input data.

    ; Load and interleave adjacent rows.
    ;   Load: [ H1 G1 F1 E1 D1 C1 B1 A1 | H0 G0 F0 E0 D0 C0 B0 A0 ].
    ;   Perm: [ H1 G1 F1 E1 H0 G0 F0 E0 | D1 C1 B1 A1 D0 C0 B0 A0 ].
    vpermq          y4, [g4], 0xd8          ; rows 0 & 1.
    vpermq          y5, [g4 + 64], 0xd8     ; rows 4 & 5.
    vpermq          y6, [g4 + 32], 0xd8     ; rows 2 & 3.
    vpermq          y7, [g4 + 96], 0xd8     ; rows 6 & 7.

    ; Prepare the linear combination factors.
    ;   Perm: [ H4 H0 G4 G0 F0 F4 E4 E0 | D4 D0 C4 C0 B4 B0 A4 A0 ].
    vpunpcklwd      y0, y4, y5              ; rows 0 & 4.
    vpunpcklwd      y2, y6, y7              ; rows 2 & 6.
    vpunpckhwd      y1, y4, y5              ; rows 1 & 5.
    vpunpckhwd      y3, y6, y7              ; rows 3 & 7.

    lea             g6, [pat_idct8_pass1]   ; Table address.
    xor             g4, g4                  ; Loop counter.
    vpbroadcastd    y8, [pat_dw_64]         ; Load the bias (64).

    ; II. Perform the 1st pass of IDCT8x8.
    .loop_idct8_pass1:

    %macro PROCESS_COLUMN 4                 ; %1: output register, %2: 1st input, %3: 2nd input, %4: IDCT factor index.
    vpbroadcastd    y7, [g6 + %4]
    vpbroadcastd    y6, [g6 + %4 + 4]
    vpmaddwd        y7, y7, %2
    vpmaddwd        %1, y6, %3
    vpaddd          %1, y7, %1
    %endmacro

    PROCESS_COLUMN  y4, y0, y2, 0           ; Combine rows 0, 2, 4, 6.
    PROCESS_COLUMN  y5, y1, y3, 32          ; Combine rows 1, 3, 5, 7.
    %unmacro PROCESS_COLUMN 4

    vpaddd          y6, y4, y5              ; Do two linear combinations per itereration: 0-7, 1-6, 2-5, 3-4.
    vpsubd          y7, y4, y5

    vpaddd          y6, y8, y6              ; Add constant factor and shift.
    vpaddd          y7, y8, y7
    vpsrad          y6, y6, 7
    vpsrad          y7, y7, 7

    vpackssdw       y5, y6, y7              ; Pack.
    vpermq          y5, y5, 0xd8            ; Put back the column values together.
    vmovdqu         [g5 + g4], y5           ; Store.

    add             g4, 32
    add             g6, 8

    cmp             g4, 128
    jne             .loop_idct8_pass1

    ; III. Rearrange the rows of data.
    vmovdqu         y0, [g5]                ; Columns 0 & 7.
    vmovdqu         y1, [g5 + 32]           ; Columns 1 & 6.
    vmovdqu         y2, [g5 + 64]           ; Columns 2 & 5.
                                            ; Columns 3 & 4 are already in y5.

    vinserti128     y3, y0, x1, 1           ; Columns 0 & 1.
    vinserti128     y4, y2, x5, 1           ; Columns 2 & 3.
    vperm2i128      y5, y5, y2, 0x31        ; Columns 4 & 5.
    vperm2i128      y0, y0, y1, 0x13        ; Columns 6 & 7.

    vmovdqu         [g5], y3                ; Store in order.
    vmovdqu         [g5 + 32], y4
    vmovdqu         [g5 + 64], y5
    vmovdqu         [g5 + 96], y0

    ; IV. Perform the 2nd pass of IDCT8x8.
    ; FIXME: might want to do like DCT 32x32 here.
    lea             g4, [pat_idct8_pass2]   ; Load the multiplication factors.
    vbroadcasti128  y12, [g4]
    vbroadcasti128  y11, [g4 + 16]
    vbroadcasti128  y10, [g4 + 32]
    vbroadcasti128  y9, [g4 + 48]
    vbroadcasti128  y8, [g4 + 64]
    vbroadcasti128  y7, [g4 + 80]
    vbroadcasti128  y6, [g4 + 96]
    vbroadcasti128  y5, [g4 + 112]

    vpbroadcastd    y4, [pat_dw_2048]       ; Load the bias (2048).

    xor             g4, g4
    .loop_idct8_pass2:

    vmovdqu         y0, [g5 + g4]           ; Load the next column.
    %macro PROCESS_COLUMN 3                 ; %1: output register, %2: IDCT factors 0, %3: IDCT factors 1.
    vpmaddwd        y2, %3, y0
    vpmaddwd        %1, %2, y0              ; Multiply by the factors and add the terms once.
    vphaddd         %1, %1, y2
    %endmacro

    PROCESS_COLUMN  y1, y12, y11            ; Do the 8 linear combinations & add the terms.
    PROCESS_COLUMN  y3, y10, y9
    vphaddd         y1, y1, y3

    PROCESS_COLUMN  y3, y8, y7
    PROCESS_COLUMN  y0, y6, y5
    vphaddd         y0, y3, y0
    %unmacro PROCESS_COLUMN 3

    vpaddd          y1, y4, y1              ; Add the bias and shift.
    vpaddd          y0, y4, y0
    vpsrad          y1, y1, 12
    vpsrad          y0, y0, 12

    vpackssdw       y1, y1, y0              ; Pack.

    ; Perform the reconstruction.
    vmovq           x0, [g2]                ; Load the prediction.
    vmovhps         x0, [g2 + g3]
    lea             g2, [g2 + 2*g3]
    vpmovzxbw       y2, x0                  ; Convert the prediction to 16-bit.
    vpaddw          y1, y1, y2              ; Add the residual.
    vpackuswb       y0, y1, y1              ; Convert the reconstruction to 8-bit.
    vpermq          y0, y0, 0xd8            ; Merge the data in the low lane.
    vmovq           [g0], x0                ; Store.
    vmovhps         [g0 + g1], x0
    lea             g0, [g0 + 2*g1]

    add             g4, 32
    cmp             g4, 128
    jne             .loop_idct8_pass2
    RET


; ---------------------- DCT/IDCT 16 macros ---------------------

; Interleave two registers.
%macro INTERLEAVE_COL 4                 ; %1: lower output, %2: higher output, %3: input 0, %4: input 1.
vpunpcklwd      %1, %3, %4
vpunpckhwd      %2, %3, %4
%endmacro

; Expand the DCT factors from bytes to words and store them in spill memory. We
; could store them expanded in the data segment, but that would impact the
; cache. We could also expand them on the fly, but that kills performance. This
; operation has a limited impact on performance, so it's probably for the best.
%macro EXPAND_FACTOR 5                  ; %1: destination, %2: source, %3: loop counter, %4: tmp, %5: factor size.
xor             %3, %3
.loop_expand:
vpmovsxbw       %4, [%2 + %3]           ; Load and expand the factors.
vmovdqu         [%1 + 2*%3], %4         ; Store.
add             %3, 16
cmp             %3, %5
jne             .loop_expand
%endmacro

; DCT 16x16.
DEFFUN f265_lbd_dct_16_avx2, ia=6, at=884848, fa=0, ti=1, tv=15, ym=1

    ; Compute the residual and reorder the elements of each row before storing.
    vpbroadcastd    y2, [pat_pmaddubsw_sub] ; Residual computation pattern (src-pred).
    vbroadcasti128  y7, [pat_dct8_shuf]     ; Shuffling pattern before DCT 1.
    vbroadcasti128  y6, [pat_dct8_sign]     ; Sign pattern before DCT 1.

    ; Process 2 rows at a time.
    xor             g6, g6                  ; Loop counter.
    .loop_res:
    vmovdqu         y0, [g1]                ; Load source line 0.
    vmovdqu         y1, [g3]                ; Load prediction line 0.
    vinserti128     y0, y0, [g1 + g2], 1    ; Load source line 1.
    vinserti128     y1, y1, [g3 + g4], 1    ; Load prediction line 1.

    lea             g1, [g1 + 2*g2]         ; Update the source and the prediction pointers.
    lea             g3, [g3 + 2*g4]

    vpunpcklbw      y3, y0, y1              ; Interleave the source and the prediction.
    vpunpckhbw      y0, y0, y1

    vpmaddubsw      y3, y3, y2              ; Compute the residual.
    vpmaddubsw      y1, y0, y2

    vinserti128     y0, y3, x1, 1           ; Put each line in its own register.
    vperm2i128      y4, y1, y3, 0x13

    vpermq          y0, y0, 0x9c            ; Reorder: 11.10.9.8  7.6.5.4 | 15.14.13.12  3.2.1.0.
    vpermq          y4, y4, 0x9c
    vpshufb         y1, y0, y7              ;          4.5.6.7  8.9.10.11 | 0.1.2.3  12.13.14.15.
    vpshufb         y3, y4, y7
    vpsignw         y0, y0, y6              ; Invert the signs of the 4 last positions.
    vpsignw         y4, y4, y6
    vpaddw          y0, y0, y1              ; Result: S(4,11)..S(7,8) A(7,8)..A(4,11)|S(0,15)..S(3,12) A(3,12)..A(0,15).
    vpaddw          y4, y3, y4

    vmovdqu         [g5 + g6], y0           ; Store the rows.
    vmovdqu         [g5 + g6 + 32], y4

    add             g6, 64                  ; Pass to the next rows.
    cmp             g6, 512
    jne             .loop_res

    ; First DCT pass.
    lea             g6, [pat_dct16_pass1]
    vpmovsxbw       y5, [g6 + 0*16]          ; Load the multiplication factors.
    vpmovsxbw       y6, [g6 + 1*16]          ; The expansion hurts performance a little.
    vpmovsxbw       y7, [g6 + 2*16]
    vpmovsxbw       y8, [g6 + 3*16]
    vpmovsxbw       y9, [g6 + 4*16]
    vpmovsxbw       y10, [g6 + 5*16]
    vpmovsxbw       y11, [g6 + 6*16]
    vpmovsxbw       y12, [g6 + 7*16]

    vpbroadcastd    y13, [pat_dw_4]         ; Bias.
    xor             g3, g3                  ; Loop counter.

    .loop_pass1:
    vmovdqu          y0, [g5 + g3]          ; Load the row.

    ; Multiply and add in the same lanes.
    %macro PROCESS_COLUMN 4                 ; %1: result, %2: tmp, %3: DCT factors 0, %4: DCT factors 1.
    vpmaddwd        %2, %3, y0
    vpmaddwd        %1, %4, y0
    vphaddd         %1, %2, %1
    %endmacro

    ; Combine the lanes and get the final combinations.
    %macro COMBINE 4                        ; %1 result, %2: tmp, %3: src 0, %4: src 1.
    vperm2i128      %1, %3, %4, 0x31        ; Align the lanes and finish adding up the terms.
    vperm2i128      %2, %3, %4, 0x20
    vpaddd          %1, %2, %1
    vpaddd          %1, y13, %1             ; Add the bias and shift.
    vpsrad          %1, %1, 3
    %endmacro

    PROCESS_COLUMN  y1, y14, y5, y6
    PROCESS_COLUMN  y2, y14, y7, y8
    PROCESS_COLUMN  y3, y14, y9, y10
    PROCESS_COLUMN  y4, y14, y11, y12

    ; Delaying the combination helps performance, even though
    ; it increases the register pressure.
    COMBINE         y0, y14, y1, y2
    COMBINE         y1, y14, y3, y4
    %unmacro PROCESS_COLUMN 4
    %unmacro COMBINE 4

    vpackssdw       y0, y0, y1              ; Pack and store.
    vmovdqu         [g0 + g3], y0
    add             g3, 32
    cmp             g3, 32*16
    jne             .loop_pass1

    ; Second DCT pass.

    ; Expand the factors for pass 2.
    lea             g1, [g5 + 16*64]        ; Factor destination.
    lea             g2, [pat_dct16_pass2]   ; Factor source.
    EXPAND_FACTOR   g1, g2, g6, y0, 16*16

    ; Load the bias.
    vpbroadcastd    y6, [pat_dw_512]

    ; Since there are not enough registers to load all 16 columns (of 16-bit data),
    ; split the DCT16x16 second pass into loops. In the first loop, work on 8 columns
    ; and in the next loop, work on the remaining 8 columns.

    ; Load half of the columns in 8 registers.
    %macro LOAD_COL 1                       ; %1: column offset.
    vmovdqu         y0, [g0 + (0+%1)*32]    ; Interleave 0 and 15, 1 and 14, etc.
    vmovdqu         y1, [g0 + (15-%1)*32]
    vmovdqu         y2, [g0 + (1+%1)*32]
    vmovdqu         y3, [g0 + (14-%1)*32]
    vmovdqu         y4, [g0 + (2+%1)*32]
    vmovdqu         y5, [g0 + (13-%1)*32]
    INTERLEAVE_COL  y14, y13, y0, y1
    INTERLEAVE_COL  y12, y11, y2, y3
    INTERLEAVE_COL  y10, y9, y4, y5
    vmovdqu         y0, [g0 + (3+%1)*32]
    vmovdqu         y1, [g0 + (12-%1)*32]
    INTERLEAVE_COL  y8, y7, y0, y1
    %endmacro

    ; Load and process the first 8 columns, store temporary sums.
    LOAD_COL        0                       ; Load the columns.
    xor             g6, g6                  ; Temporary store offset.
    .loop_pass2a:
    call            .pass2_mult             ; Multipy and sum.
    vmovdqu         [g5 + g6], y4           ; Store the temporary sums.
    vmovdqu         [g5 + g6 + 32], y5
    add             g1, 16
    add             g6, 64
    cmp             g6, 64*16
    jne             .loop_pass2a

    ; Load and process the last 8 columns, store the final results.
    LOAD_COL        4                       ; Load the columns.
    xor             g6, g6                  ; Final store offset.
    .loop_pass2b:
    call            .pass2_mult             ; Multipy and sum.
    vpaddd          y4, y4, [g5 + 2*g6]     ; Add the sums from the first loop.
    vpaddd          y5, y5, [g5 + 2*g6 + 32]
    vpaddd          y4, y4, y6              ; Add the bias.
    vpaddd          y5, y5, y6
    vpsrad          y4, y4, 10              ; Shift.
    vpsrad          y5, y5, 10
    vpackssdw       y0, y4, y5              ; Convert to 16-bit and pack the rows together.
    vpermq          y0, y0, 0xd8            ; Merge the row pixels together and store.
    vmovdqu         [g0 + g6], y0
    add             g1, 16
    add             g6, 32
    cmp             g6, 32*16
    jne             .loop_pass2b
    RET

    ; Do the multiplications and partial sums of the second DCT pass.
    .pass2_mult:

    ; Broadcast the factors and multiply. There are two full registers per
    ; column, containing 4-byte results.
    %macro PROCESS_COL 5                    ; %1: out 0, %2: out 1, %3: in 0, %4: in 1, %5: DCT mult.
    vpbroadcastd    %2, [%5]
    vpmaddwd        %1, %3, %2
    vpmaddwd        %2, %4, %2
    %endmacro

    PROCESS_COL     y0, y1, y14, y13, g1    ; Do the multiplications on the first/last column.
    PROCESS_COL     y2, y3, y12, y11, g1 + 4; Same for the next column pair.
    vpaddd          y4, y0, y2              ; Sum.
    vpaddd          y5, y1, y3

    PROCESS_COL     y0, y1, y10, y9, g1 + 8
    PROCESS_COL     y2, y3, y8, y7, g1 + 12
    vpaddd          y4, y4, y0
    vpaddd          y5, y5, y1
    vpaddd          y4, y4, y2
    vpaddd          y5, y5, y3
    ret
    %unmacro LOAD_COL 1
    %unmacro PROCESS_COL 5

; IDCT 16x16.
DEFFUN f265_lbd_idct_16_avx2, ia=6, at=848488, fa=0, ti=3, tv=16, ym=1

    ; Spill buffer layout:
    ; - 512 bytes for pass 1 even rows, and pass 2 results.
    ; - 512 bytes for pass 1 results.
    ; - 256 bytes for factor expansion.

    ; DCT pass 1.

    ; Expand the factors for pass 1.
    lea             g8, [g5 + 1024]         ; Factor destination.
    lea             g7, [pat_idct16_pass1]  ; Factor source.
    EXPAND_FACTOR   g8, g7, g6, y0, 16*16

    ; Load half of the rows in 8 registers.
    %macro LOAD_COL 1                       ; %1: row offset.
    vmovdqu         y0, [g4 + 0*64 + %1]
    vmovdqu         y1, [g4 + 1*64 + %1]
    vmovdqu         y2, [g4 + 2*64 + %1]
    vmovdqu         y3, [g4 + 3*64 + %1]
    vmovdqu         y4, [g4 + 4*64 + %1]
    vmovdqu         y5, [g4 + 5*64 + %1]
    INTERLEAVE_COL  y14, y13, y0, y1
    INTERLEAVE_COL  y12, y11, y2, y3
    INTERLEAVE_COL  y10, y9, y4, y5
    vmovdqu         y0, [g4 + 6*64 + %1]
    vmovdqu         y1, [g4 + 7*64 + %1]
    INTERLEAVE_COL  y8, y7, y0, y1
    %endmacro

    ; Process the even rows.
    LOAD_COL        0
    xor             g6, g6                  ; Loop counter.
    .loop_pass1a:
    call            .pass1_process          ; Process all the rows.
    vmovdqu         [g5 + g6], y0           ; Store the temporary sums.
    vmovdqu         [g5 + g6 + 32], y1
    add             g6, 64
    cmp             g6, 8*64
    jne             .loop_pass1a

    ; Process the odd rows.
    LOAD_COL        32
    xor             g6, g6                  ; Loop counter.
    mov             g7, 480                 ; Column location 15-n.
    vpbroadcastd    y6, [pat_dw_64]         ; Bias.
    .loop_pass1b:
    call            .pass1_process          ; Process all the rows.

    vmovdqu         y4, [g5 + 2*g6]         ; Load the temporary sums.
    vmovdqu         y5, [g5 + 2*g6 + 32]
    vpaddd          y4, y4, y6              ; Add the bias.
    vpaddd          y5, y5, y6

    vpsubd          y2, y4, y0              ; Subtract the odd terms.
    vpsubd          y3, y5, y1
    vpaddd          y0, y0, y4              ; Add the odd terms.
    vpaddd          y1, y1, y5

    vpsrad          y0, y0, 7               ; Shift.
    vpsrad          y1, y1, 7
    vpsrad          y2, y2, 7
    vpsrad          y3, y3, 7

    vpackssdw       y0, y0, y1              ; Pack.
    vpackssdw       y1, y2, y3

    vmovdqu         [g5 + g6 + 512], y0     ; Column "n".
    vmovdqu         [g5 + g7 + 512], y1     ; Column "15 - n".

    add             g6, 32
    sub             g7, 32
    cmp             g6, 8*32
    jne             .loop_pass1b
    %unmacro LOAD_COL 1

    ; DCT pass 2.
    lea             g6, [pat_idct16_pass2]  ; Load the multiplication factors.
    vpmovsxbw       y15, [g6 + 0*16]
    vpmovsxbw       y14, [g6 + 1*16]
    vpmovsxbw       y13, [g6 + 2*16]
    vpmovsxbw       y12, [g6 + 3*16]
    vpmovsxbw       y11, [g6 + 4*16]
    vpmovsxbw       y10, [g6 + 5*16]
    vpmovsxbw       y9, [g6 + 6*16]
    vpmovsxbw       y8, [g6 + 7*16]
    vbroadcasti128  y7, [pat_idct16_shuf1]  ; Shuffle.
    vpbroadcastq    y6, [pat_idct16_sign]   ; Sign.
    vpbroadcastd    y5, [pat_dw_2048]       ; Bias.

    xor             g6, g6                  ; Loop counter.
    .loop_pass2:

    vmovdqu         y0, [g5 + g6 + 512]     ; Load the column.
    vpshufb         y0, y0, y7              ; Put the even and odd terms together.

    ; Multiply and add.
    %macro MULTIPLY_ADD 4                   ; %1: result, %2: tmp, %3: DCT factors 0, %4: DCT factors 1.
    vpmaddwd        %2, %3, y0
    vpmaddwd        %1, %4, y0
    vphaddd         %1, %2, %1
    %endmacro

    ; Process the column.
    %macro PROCESS_COLUMN 7                 ; %1: result, %2-3: tmp, %4-7: DCT factors.
    MULTIPLY_ADD    %1, %3, %4, %5
    MULTIPLY_ADD    %2, %3, %6, %7
    vperm2i128      %3, %1, %2, 0x31        ; Align the lanes and add up the terms.
    vperm2i128      %2, %1, %2, 0x20
    vpaddd          %1, %3, %2
    %endmacro

    ; Finish adding up the terms.
    %macro FINISH 2                         ; %1: result, %2: tmp.
    vpsignd         %2, %1, y6              ; Change the sign of the odd terms.
    vphaddd         %1, %1, %2              ; Finish summing up the terms.
    vpaddd          %1, %1, y5              ; Add the bias and shift.
    vpsrad          %1, %1, 12
    %endmacro FINISH

    PROCESS_COLUMN  y1, y3, y4, y15, y14, y13, y12
    PROCESS_COLUMN  y2, y3, y4, y11, y10, y9, y8
    FINISH          y1, y3
    FINISH          y2, y3
    %unmacro MULTIPLY_ADD 4
    %unmacro PROCESS_COLUMN 7
    %unmacro FINISH 2

    vpackssdw       y0, y1, y2              ; Pack and store without reordering.
    vmovdqu         [g5 + g6], y0

    add             g6, 32
    cmp             g6, 512
    jne             .loop_pass2

    ; Reconstruct.
    vpmovzxbd       y2, [pat_idct16_shuf2]  ; Load the shuffle patterns.
    vmovdqu         y3, [pat_idct16_shuf3]
    xor             g6, g6                  ; Loop counter.
    .loop_rec:
    vpermd          y0, y2, [g5 + g6]       ; Load and reorder the residual.
    vpshufb         y0, y0, y3
    vpmovzxbw       y1, [g2]                ; Load the prediction.
    add             g2, g3
    vpaddw          y0, y0, y1              ; Add the residual and pack.
    vpackuswb       y0, y0, y0
    vpermq          y0, y0, 0xd8            ; Combine and store.
    vmovdqu         [g0], x0
    add             g0, g1
    add             g6, 32
    cmp             g6, 512
    jne             .loop_rec
    RET

    ; Process all rows in pass 1.
    .pass1_process:

    ; Process a row.
    %macro PROCESS_ROW 5                    ; %1-2: out, %3-4: in, %5: DCT factors.
    vpbroadcastd    %2, [%5]
    vpmaddwd        %1, %3, %2
    vpmaddwd        %2, %4, %2
    %endmacro

    PROCESS_ROW     y0, y1, y14, y13, g8
    PROCESS_ROW     y2, y3, y12, y11, g8 + 4
    vpaddd          y0, y0, y2
    vpaddd          y1, y1, y3

    PROCESS_ROW     y2, y3, y10, y9, g8 + 8
    vpaddd          y0, y0, y2
    vpaddd          y1, y1, y3

    PROCESS_ROW     y2, y3, y8, y7, g8 + 12
    vpaddd          y0, y0, y2
    vpaddd          y1, y1, y3

    add             g8, 16
    ret
%unmacro PROCESS_ROW 5
%unmacro EXPAND_FACTOR 5

; ---------------------- DCT/IDCT 32 macros ---------------------
; Load 4 rows of 32-bit data from consecutive memory locations.
%macro LOAD_DATA 2                          ; %1: base address, %2: data location index.
vmovdqu         y0, [g5 + %1 + %2]
vmovdqu         y1, [g5 + %1 + %2 + 32]
vmovdqu         y2, [g5 + %1 + %2 + 64]
vmovdqu         y3, [g5 + %1 + %2 + 96]
%endmacro

; Store 4 rows of 32-bit data to consecutive memory locations.
%macro STORE_DATA 2                         ; %1: base address, %2: data location index.
vmovdqu         [g5 + %1 + %2], y0
vmovdqu         [g5 + %1 + %2 + 32], y1
vmovdqu         [g5 + %1 + %2 + 64], y2
vmovdqu         [g5 + %1 + %2 + 96], y3
%endmacro

; Multiply 2 rows of data and add to the destination.
%macro MULT_ADD 3                           ; %1: destination, %2: input 0, %3: input 1.
vpmaddwd        y4, %2, y6
vpmaddwd        y5, %3, y7
vpaddd          %1, %1, y4
vpaddd          %1, %1, y5
%endmacro

; Multiply 2 rows of data and subtract from the destination.
%macro MULT_SUB 3                           ; %1: destination, %2: input 0, %3: input 1.
vpmaddwd        y4, %2, y6
vpmaddwd        y5, %3, y7
vpsubd          %1, %1, y4
vpsubd          %1, %1, y5
%endmacro

; Multiply 2 rows of data and perform a horizontal addition of terms.
%macro MULT_HADD 4
vpmaddwd        %1, %3, y0                  ; %1: result, %2: tmp, %3: DCT/IDCT factor 0, %4: DCT/IDCT factor 1.
vpmaddwd        %2, %4, y0
vphaddd         %1, %1, %2
%endmacro

; DCT 32x32.
;
; Input parameters:
; - g0:     destination.
; - g1:     source.
; - g2:     source stride.
; - g3:     prediction.
; - g4:     prediction stride.
; - g5:     spill buffer.
DEFFUN f265_lbd_dct_32_avx2, ia=6, at=884848, fa=0, ti=1, tv=16, ym=1
    ; Compute the residual and reorder the elements of each row before storing.
    vpbroadcastd    y2, [pat_pmaddubsw_sub] ; Residual computation pattern (src-pred).
    vbroadcasti128  y7, [pat_dct8_shuf]     ; Shuffling pattern before DCT 1.
    vbroadcasti128  y6, [pat_dct8_sign]     ; Sign pattern before DCT 1.

    ; Process 1 row at a time.
    xor             g6, g6                  ; Loop counter.
    .loop_res:
    vmovdqu         y0, [g1]                ; Load source line 0.
    vmovdqu         y1, [g3]                ; Load prediction line 0.
    lea             g1, [g1 + g2]           ; Update the source and the prediction pointers.
    lea             g3, [g3 + g4]
    vpunpcklbw      y3, y0, y1              ; Interleave the source and the prediction.
    vpunpckhbw      y0, y0, y1
    vpmaddubsw      y3, y3, y2              ; Compute the residual.
    vpmaddubsw      y1, y0, y2
    vinserti128     y0, y3, x1, 1           ; 15, 14, .. 9, 8 | 7, 6, ... 1, 0.
    vperm2i128      y4, y1, y3, 0x31        ; 23, 22, .... 16 | 31, 30 .... 24.
    vpshufb         y4, y4, y7              ; 16, 15 ..... 23 | 24, 25, ... 31.
    vpaddw          y1, y0, y4              ; Add:      15 + 16, 14 + 17, .... 8 + 23  | 7 + 24, .... 1 + 30, 0 + 31.
    vpsubw          y3, y0, y4              ; Subtract: 15 - 16, 14 - 17, .... 8 - 23  | 7 - 24, .... 1 - 30, 0 - 31.
    ; Compute values/columns to process the even rows of DCT32 Pass1.
    vpermq          y0, y1, 0x9c            ; Reorder to add/subtract: [(0+31) +/- (15+16)], [(1+30) +/- (14+17)] ...
    vpshufb         y1, y0, y7
    vpsignw         y0, y0, y6
    vpaddw          y0, y0, y1
    vmovdqu         [g5 + g6], y0           ; Store the rows.
    vmovdqu         [g5 + g6 + 1024], y3
    add             g6, 32                  ; Pass to the next rows.
    cmp             g6, 32*32
    jne             .loop_res

    ; Perform DCT32x32 Pass 1.
    ; Since it is a 32x32 DCT, we need 16 ymm registers for just the DCT multiplication factors (in the first pass).
    ; Since this is not possible, we split the DCT Pass 1 into 2: 1 for the even rows and the other for the odd rows.

    ; First, work on the odd rows. We have 16 odd rows, each with 32 16-bit values. However, from the above loop,
    ; each value is the difference of 2 values (e.g. [0 - 31], [1 - 30] etc.], and thereby we have 16 odd rows with
    ; 16 16-bit values. For the 16 rows, we need 16 rows of multiplication factors, requiring 16 ymm registers.
    ; Since this is not possible, we split the processing into two to get 8 output rows after each.

    ; Next, perform DCT on the even rows. Each even row has 16 16-bit values. The first 4 values are the sum of
    ; 4 residuals ( [(0+31)+(15+16)] .. [(3+28)+(12+19)] ) while the next 4 values are the difference of the sum of 2
    ; values ( [(3+28)-(12+19)] ... [(0+31)-(15+16)] ). Similarly, the higher lane has
    ; ( [(4+27)+(11+20)] .. [(7+24)+(8+23)] ) and ( [(4+27)-(11+20)] .. [(7+24)-(8+23)] ). The lower and higher lanes
    ; together form 2 input rows. Hence, each execution of the loop will give 2 rows of outputs.

    %macro LOAD_DCT_FACT 3                  ; %1: base address for the DCT factors, %2: g2 value, %3: g3 value.
    vpmovsxbw       y5, [%1 + 0*16]         ; Load the multiplication factors.
    vpmovsxbw       y6, [%1 + 1*16]         ; The expansion hurts performance a little.
    vpmovsxbw       y7, [%1 + 2*16]
    vpmovsxbw       y8, [%1 + 3*16]
    vpmovsxbw       y9, [%1 + 4*16]
    vpmovsxbw       y10, [%1 + 5*16]
    vpmovsxbw       y11, [%1 + 6*16]
    vpmovsxbw       y12, [%1 + 7*16]
    vpbroadcastd    y13, [pat_dw_8]         ; Bias.
    mov             g2, %2                  ; Destination offset.
    mov             g3, %3                  ; Source offset.
    %endmacro

    ; Processing to get the odd rows of the DCT Pass1 output.
    lea             g6, [pat_dct32_pass1 + 128]
    lea             g4, [pat_dct32_pass1 + 256]
    lea             g1, [pat_dct32_pass1]
    LOAD_DCT_FACT   g6, 2048, 1024
    call            .loop_dct32_pass1
    LOAD_DCT_FACT   g4, 2048 + 1024, 1024
    call            .loop_dct32_pass1

    ; Processing to get the even rows of the DCT Pass1 output.
    LOAD_DCT_FACT   g1, 0, 0
    %unmacro LOAD_DCT_FACT 3
    .loop_pass1a:
    ; Combine the lanes and get the final combinations.
    %macro COMBINE 4                        ; %1: result, %2: tmp, %3: src 0, %4: src 1.
    vperm2i128      %1, %3, %4, 0x31        ; Align the lanes and finish adding up the terms.
    vperm2i128      %2, %3, %4, 0x20
    vpaddd          %1, %2, %1
    vpaddd          %1, y13, %1             ; Add the bias and shift.
    vpsrad          %1, %1, 4
    %endmacro
    vmovdqu         y0, [g5 + g3]           ; Load the row.
    MULT_HADD       y1, y14, y5, y6
    MULT_HADD       y2, y15, y7, y8
    MULT_HADD       y3, y14, y9, y10
    MULT_HADD       y4, y15, y11, y12
    COMBINE         y0, y14, y1, y2
    COMBINE         y1, y15, y3, y4
    %unmacro COMBINE 4
    vpackssdw       y0, y0, y1              ; Pack.
    vmovdqu         y3, [g5 + g3 + 2048]    ; Load the odd rows of the DCT 1 output.
    vmovdqu         y4, [g5 + g3 + 2048 + 1024]
    vpackssdw       y3, y3, y4              ; The odd rows were stored as 32-bits. Convert to 16-bit.
    INTERLEAVE_COL  y1, y2, y0, y3          ; Interleave the odd and even rows.
    vmovdqu         [g0 + g2], y1           ; Store.
    vmovdqu         [g0 + g2 + 32], y2
    add             g2, 64
    add             g3, 32
    cmp             g3, 32*32
    jne             .loop_pass1a

    ; DCT Pass 2.
    ; Each column to be processed takes 2 ymm registers (32 values each of 16 bit). Hence, this pass is broken down
    ; into 8 smaller sub processes. Each process takes as input 4 consecutive columns either in ascending or descending
    ; order. This helps reduce the number of multiplication factors from 1024 to 256.
    ; From columns 8-11 onwards, the processing for odd and even output rows are split to 2 function calls. Tried
    ; merging the calls, but profiling indicated higher number of cycles.

    ; Define the macros.
    ; Load 8 consecutive rows of 256 bits. This corresponds to 4 columns in ascending order.
    %macro LOAD_COL 1                       ; %1: column offset.
    vmovdqu         y0, [g0 + (0 + %1)*32]
    vmovdqu         y1, [g0 + (1 + %1)*32]
    vmovdqu         y2, [g0 + (2 + %1)*32]
    vmovdqu         y3, [g0 + (3 + %1)*32]
    vmovdqu         y4, [g0 + (4 + %1)*32]
    vmovdqu         y5, [g0 + (5 + %1)*32]
    vmovdqu         y6, [g0 + (6 + %1)*32]
    vmovdqu         y7, [g0 + (7 + %1)*32]
    INTERLEAVE_COL  y15, y14, y0, y2
    INTERLEAVE_COL  y13, y12, y1, y3
    INTERLEAVE_COL  y11, y10, y4, y6
    INTERLEAVE_COL  y9, y8, y5, y7
    %endmacro
    ; Set values of the General Purpose Registers before processing the loops.
    %macro SET_GPR 5                        ; %1: g1 value, %2: g2 value, %3: g3 value, %4: g4 value, %5: g6 value.
    lea             g1, [pat_dct32_pass2 + %1]
    mov             g2, %2                  ; Loop counter, and also starting index of memory location for load/store.
    mov             g3, %3                  ; Step for the loop counter (signifying memory increment in bytes).
    mov             g4, %4                  ; Step for incrementing the DCT mult factor address (viz, g1).
    mov             g6, %5                  ; Maximum value of loop counter.
    %endmacro
    ; Load 8 consecutive rows of 256 bits. This corresponds to 4 columns in descending order.
    %macro LOAD_COL_REV 1                   ; %1: column offset.
    vmovdqu         y0, [g0 + (%1 - 0)*32]
    vmovdqu         y1, [g0 + (%1 - 1)*32]
    vmovdqu         y2, [g0 + (%1 - 2)*32]
    vmovdqu         y3, [g0 + (%1 - 3)*32]
    vmovdqu         y4, [g0 + (%1 - 4)*32]
    vmovdqu         y5, [g0 + (%1 - 5)*32]
    vmovdqu         y6, [g0 + (%1 - 6)*32]
    vmovdqu         y7, [g0 + (%1 - 7)*32]
    INTERLEAVE_COL  y15, y14, y1, y3
    INTERLEAVE_COL  y13, y12, y0, y2
    INTERLEAVE_COL  y11, y10, y5, y7
    INTERLEAVE_COL  y9, y8, y4, y6
    %endmacro
    ; Negate the rows of 16-bit data according to the specified pattern.
    %macro NEG_SIGN 1                       ; %1: register containing the sign pattern.
    vpsignw         y15, y15, %1
    vpsignw         y14, y14, %1
    vpsignw         y13, y13, %1
    vpsignw         y12, y12, %1
    vpsignw         y11, y11, %1
    vpsignw         y10, y10, %1
    vpsignw         y9, y9, %1
    vpsignw         y8, y8, %1
    %endmacro

    ; 1. First process columns 0 - 3 and obtain the 32 rows of output. Each output row has 32 values of 32 bits each.
    LOAD_COL        0
    lea             g1, [pat_dct32_pass2]
    xor             g2, g2
    vpbroadcastd    y6, [pat_dw_1024]       ; Bias. The bias is added initially so to keep registers free in the last 2
                                            ; loops. Helped in optimization.
    .loop_pass2a:
    vpbroadcastd    y0, [g1]                ; Load the DCT multiplication factors.
    vpbroadcastd    y1, [g1 + 4]
    %macro PROCESS_COL 3                    ; %1: input 0, %2: input 1, %3: store location 1.
    vpmaddwd        y2, %1, y0
    vpmaddwd        y3, %2, y1
    vpaddd          y2, y2, y6
    vpaddd          y2, y2, y3
    vmovdqu         [g5 + g2 + %3], y2
    %endmacro
    PROCESS_COL     y15, y11, 0
    PROCESS_COL     y14, y10, 32
    PROCESS_COL     y13, y9, 64
    PROCESS_COL     y12, y8, 96
    %unmacro PROCESS_COL 3
    add             g1, 8
    add             g2, 128
    cmp             g2, 128*32
    jne             .loop_pass2a

    ; 2. Now process columns 4 - 7 and add to the above rows of output.
    LOAD_COL        8
    SET_GPR         256, 0, 128, 8, 128*32
    call            .loop_dct32_pass2b      ; For (a0*src[0] + b0*src[4]) ... (a3*src[3] + b3*src[7]).

    ; 3. Now process columns 8 - 11.
    LOAD_COL_REV    23
    ; The first loop uses the same mult factors as the above processing (col. 4 - 7), albeit in reverse order
    ; (i.e. col 8 uses same factors as col 7, .. col 11 uses same factors as col 4). After processing, the result is
    ; added/subtracted to the rows of the output above. This is to do X*(src[7] + src[8]) and Y*(src[7] - src[8])
    ; for destination 'r' and 'r+64' respectively where r (index in doubleword) takes values 0 to 896 in steps of 128.
    SET_GPR         256, 0, 512, 16, 512*8  ; 512 = 128*4 bytes, where 128 = increment of 'r' in 32-bit terms.
    call            .loop_dct32_pass2
    ; Do (x*src[4] + y*src[11]) to (x*src[7] + y*src[8]) for destination 'r' where r is from 32 to 992 in steps of 64.
    ; The mult factors for cols 11 - 8 for dest 'r' are the same as those used by col 4 - 7 in dest '1024 - r'
    ; respectively, albeit negated at alternate values. Instead of negating the factors, we negate the inputs.
    vpbroadcastd    y4, [pat_dct32_sign_1]
    SET_GPR         504, 128, 512, -16, 128 + 512*8
    NEG_SIGN        y4
    call            .loop_dct32_pass2       ; Destination 'r' where r is from 32 to 992 in steps of 64.

    ; 4. Process columns 15, 14, 13, 12.
    LOAD_COL_REV    31
    SET_GPR         0, 0, 512, 16, 512*8
    call            .loop_dct32_pass2
    vpbroadcastd    y4, [pat_dct32_sign_1]
    SET_GPR         248, 128, 512, -16, 128 + 512*8
    NEG_SIGN        y4
    call            .loop_dct32_pass2       ; Destination 'r' where r is from 32 to 992 in steps of 64.

    ; 5. Now, process columns 16 - 19.
    LOAD_COL        32
    ; The first loop uses the same mult factors as in the '.loop_pass2a', and is added/subtracted to columns 0 - 3
    ; (respectively) of the output. This is to do X*(src[0] + src[16]) and Y*(src[0] - src[16]) for destination 'r' and
    ; 'r+64' respectively where r takes values 0 to 896 in steps of 128.
    SET_GPR         0, 0, 512, 16, 512*8
    call            .loop_dct32_pass2
    ; Do (x*src[0] - y*src[16]) to (x*src[3] - y*src[19]) for destination 'r' where r is from 32 to 992 in steps of 64.
    ; Instead of negating the factors, we negate the inputs (negate cols 17 and 19).
    vpbroadcastd    y4, [pat_dct32_sign_2]
    SET_GPR         248, 128, 512, -16, 128 + 512*8
    NEG_SIGN        y4
    call            .loop_dct32_pass2       ; Destination 'r' where r is from 96 to 992 in steps of 128.

    ; 6. Process columns 20 - 23.
    LOAD_COL        40
    SET_GPR         256, 0, 512, 16, 512*8
    call            .loop_dct32_pass2
    vpbroadcastd    y4, [pat_dct32_sign_2]
    SET_GPR         504, 128, 512, -16, 128 + 512*8
    NEG_SIGN        y4
    call            .loop_dct32_pass2       ; Destination 'r' where r is from 96 to 992 in steps of 128.

    ; 7. Process columns 27, 26, 25, 24.
    LOAD_COL_REV    55
    lea             g1, [pat_dct32_pass2 + 256]
    SET_GPR         256, 0, 256, 16, 256*16
    call            .loop_dct32_pass2b      ; For X*(src[4] + src[27]).
    lea             g1, [pat_dct32_pass2 + 264]
    mov             g2, 128
    .loop_2c:
    vpbroadcastd    y6, [g1]
    vpbroadcastd    y7, [g1 + 4]
    LOAD_DATA       g2, 0
    MULT_SUB        y0, y15, y11
    MULT_SUB        y1, y14, y10
    MULT_SUB        y2, y13, y9
    MULT_SUB        y3, y12, y8
    STORE_DATA      g2, 0
    add             g2, 256
    add             g1, 16
    cmp             g2, 128 + 256*16
    jne             .loop_2c

    ; 8. Now process columns 31, 30, 29, 28.
    ; This is the last stage of processing. The odd and even rows are separately calculated (as even rows involve
    ; addition of columns, while odd involve subtraction) in 2 different loops and stored.
    ; Tried to integrate the 2 loops. But, profiling indicated that the merging increased cycles.
    LOAD_COL_REV    63
    lea             g1, [pat_dct32_pass2]
    xor             g2, g2
    .loop_pass2_even:                       ; Process and store the even rows of the DCT32x32 output.
    LOAD_DATA       2*g2, 0
    vpbroadcastd    y6, [g1]
    vpbroadcastd    y7, [g1 + 4]
    %macro MULT_COMBINE 7                   ; %1-2: output, %3-6: input, %7: destination memory index.
    MULT_ADD        %1, %3, %5
    MULT_ADD        %2, %4, %6
    vpsrad          %1, %1, 11
    vpsrad          %2, %2, 11
    vpackssdw       %1, %1, %2              ; Convert to 16-bit and pack the rows together.
    vmovdqu         [g0 + g2 + %7], %1
    %endmacro
    MULT_COMBINE    y0, y1, y15, y14, y11, y10, 0
    MULT_COMBINE    y2, y3, y13, y12, y9, y8, 32
    %unmacro MULT_COMBINE 7
    add             g2, 128
    add             g1, 16
    cmp             g2, 128*16
    jne             .loop_pass2_even

    lea             g1, [pat_dct32_pass2 + 8]
    mov             g2, 64
    .loop_pass2_odd:                        ; Process and store the odd rows of the DCT32x32 output.
    LOAD_DATA       2*g2, 0
    vpbroadcastd    y6, [g1]
    vpbroadcastd    y7, [g1 + 4]
    %macro MULT_COMBINE 7
    MULT_SUB        %1, %3, %5
    MULT_SUB        %2, %4, %6
    vpsrad          %1, %1, 11
    vpsrad          %2, %2, 11
    vpackssdw       %1, %1, %2              ; Convert to 16-bit and pack the rows together.
    vmovdqu         [g0 + g2 + %7], %1
    %endmacro
    MULT_COMBINE    y0, y1, y15, y14, y11, y10, 0
    MULT_COMBINE    y2, y3, y13, y12, y9, y8, 32
    %unmacro MULT_COMBINE 7
    add             g2, 128
    add             g1, 16
    cmp             g2, 64 + 128*16
    jne             .loop_pass2_odd

    %unmacro NEG_SIGN 1
    %unmacro LOAD_COL_REV 1
    %unmacro SET_GPR 5
    %unmacro LOAD_COL 1
    RET

    ; Helper functions for DCT 32.
    ; For DCT Pass 1 - to calculate the odd rows.
    .loop_dct32_pass1:
    vmovdqu         y0, [g5 + g3]          ; Load the row.
    MULT_HADD       y1, y2, y5, y6
    MULT_HADD       y3, y4, y7, y8
    MULT_HADD       y2, y4, y9, y10
    MULT_HADD       y4, y0, y11, y12
    vphaddd         y1, y1, y3
    vphaddd         y2, y2, y4
    vperm2i128      y3, y1, y2, 0x31        ; Align the lanes and finish adding up the terms.
    vperm2i128      y4, y1, y2, 0x20
    vpaddd          y1, y3, y4
    vpaddd          y1, y13, y1             ; Add the bias and shift.
    vpsrad          y1, y1, 4
    vmovdqu         [g5 + g2], y1
    add             g2, 32
    add             g3, 32
    cmp             g3, 32*32*2
    jne             .loop_dct32_pass1
    ret

    ; For DCT Pass 2.
    .loop_dct32_pass2:
    vpbroadcastd    y6, [g1]
    vpbroadcastd    y7, [g1 + 4]
    LOAD_DATA       g2, 0
    MULT_ADD        y0, y15, y11
    MULT_ADD        y1, y14, y10
    MULT_ADD        y2, y13, y9
    MULT_ADD        y3, y12, y8
    STORE_DATA      g2, 0
    add             g1, g4
    vpbroadcastd    y6, [g1]
    vpbroadcastd    y7, [g1 + 4]
    LOAD_DATA       g2, 256
    MULT_SUB        y0, y15, y11
    MULT_SUB        y1, y14, y10
    MULT_SUB        y2, y13, y9
    MULT_SUB        y3, y12, y8
    STORE_DATA      g2, 256
    add             g2, 512
    add             g1, g4
    cmp             g2, g6
    jne             .loop_dct32_pass2
    ret

    ; For the addition only loops in DCT32 Pass2.
    .loop_dct32_pass2b:
    LOAD_DATA       g2, 0
    vpbroadcastd    y6, [g1]
    vpbroadcastd    y7, [g1 + 4]
    MULT_ADD        y0, y15, y11
    MULT_ADD        y1, y14, y10
    MULT_ADD        y2, y13, y9
    MULT_ADD        y3, y12, y8
    STORE_DATA      g2, 0
    add             g2, g3
    add             g1, g4
    cmp             g2, g6
    jne             .loop_dct32_pass2b
    ret

; IDCT 32x32.
;
; Input parameters:
; - g0:     destination.
; - g1:     destination stride.
; - g2:     prediction.
; - g3:     prediction stride.
; - g4:     coefficients.
; - g5:     spill buffer.
DEFFUN f265_lbd_idct_32_avx2, ia=6, at=848488, fa=0, ti=3, tv=16, ym=1

    ; Note : To make it easier, the notation used in the C
    ; code is used to describe the output terms in each code section.

    ; Load 4 consecutive, ascending rows of input.
    %macro LOAD_COL 4                       ; %1: row offset, %2: row spacing, %3: g7 value, %4: g6 value.
    vmovdqu         y0, [g4 + (0 + %1)*32]
    vmovdqu         y1, [g4 + (1 + %1)*32]
    vmovdqu         y2, [g4 + (0 + %2 + %1)*32]
    vmovdqu         y3, [g4 + (1 + %2 + %1)*32]
    lea             g7, [pat_idct32_pass1 + %3]
    mov             g6, %4
    INTERLEAVE_COL  y15, y14, y0, y2
    INTERLEAVE_COL  y13, y12, y1, y3
    vmovdqu         y4, [g4 + (0 + 2*%2 + %1)*32]
    vmovdqu         y5, [g4 + (1 + 2*%2 + %1)*32]
    vmovdqu         y6, [g4 + (0 + 3*%2 + %1)*32]
    vmovdqu         y7, [g4 + (1 + 3*%2 + %1)*32]
    INTERLEAVE_COL  y11, y10, y4, y6
    INTERLEAVE_COL  y9, y8, y5, y7
    %endmacro
    ; Load 4 consecutive, descending rows of input, and negate alternate rows. The alternating is done to include
    ; the sign of the IDCT factors. This helps reuse the IDCT factors.
    %macro LOAD_COL_REV 4                       ; %1: row offset, %2: row spacing, %3: g7 value, %4: g6 value.
    vpbroadcastd    y4, [pat_idct32_sign]
    vmovdqu         y0, [g4 + (1 + 3*%2 + %1)*32]
    vmovdqu         y1, [g4 + (0 + 3*%2 + %1)*32]
    vmovdqu         y2, [g4 + (1 + 2*%2 + %1)*32]
    vmovdqu         y3, [g4 + (0 + 2*%2 + %1)*32]
    vpsignw         y2, y2, y4
    vpsignw         y3, y3, y4
    INTERLEAVE_COL  y15, y14, y1, y3
    INTERLEAVE_COL  y13, y12, y0, y2
    vmovdqu         y0, [g4 + (1 + %2 + %1)*32]
    vmovdqu         y1, [g4 + (0 + %2 + %1)*32]
    vmovdqu         y2, [g4 + (1 + %1)*32]
    vmovdqu         y3, [g4 + (0 + %1)*32]
    vpsignw         y2, y2, y4
    vpsignw         y3, y3, y4
    INTERLEAVE_COL  y11, y10, y1, y3
    INTERLEAVE_COL  y9, y8, y0, y2
    lea             g7, [pat_idct32_pass1 + %3]
    mov             g6, %4
    %endmacro

    ; IDCT Pass 1.
    ; Each row of input requires 2 ymm registers => 8 ymm registers for 4 rows of inputs.

    ; First, process the odd rows to obtain s0, s1 .. s15.
    ; Since we can process only 4 rows at a time, and we have 16 odd rows, we do the processing 4 times.
    ; 1. Process rows 1, 3, 5 and 7 (i.e. src[32], src[96], src[160] and src[224]).
    LOAD_COL        2, 4, 0, 0
    mov             g8, 128*16
    call            .loop_idct32_pass1

    ; 2. Process rows 9, 11, 13, 15 (i.e. src[288], src[352], src[416] and src[480]).
    LOAD_COL        18, 4, 128, 0
    align 16                                ; Added for speed optimization: Aligns the loops '.loop_pass1_odd',
                                            ; '.loop_pass1_even1a' and the subroutines '.loop_idct32_pass1' and
                                            ; '.loop_idct32_pass2_odd' at 16-byte memory boundaries.
    .loop_pass1_odd:
    LOAD_DATA       g6, 0
    vpbroadcastd    y6, [g7]
    vpbroadcastd    y7, [g7 + 4]
    MULT_ADD        y0, y15, y11
    MULT_ADD        y1, y14, y10
    MULT_ADD        y2, y13, y9
    MULT_ADD        y3, y12, y8
    STORE_DATA      g6, 0
    add             g6, 128
    add             g7, 8
    cmp             g6, 128*16
    jne             .loop_pass1_odd

    ; 3. Process rows 23, 21, 19, 17 (i.e. src[736], src[672], src[608] and src[544]).
    LOAD_COL_REV    34, 4, 248, 0
    call            .loop_idct32_pass1_odd

    ; 4. Process rows 31, 29, 27, 25 (i.e. src[992], src[928], src[864] and src[800]).
    LOAD_COL_REV    50, 4, 120, 0
    call            .loop_idct32_pass1_odd

    ; Now process the even rows. Split this into 4.
    ; 1. Process rows 0, 16, 8, 24 (src[0], src[512], src[256] and src[768]) to calculate a_0, a_1, a_2, a_3.
    vmovdqu         y0, [g4]                ; Row 0.
    vmovdqu         y1, [g4 + 32]
    vmovdqu         y2, [g4 + 1024]         ; Row 16.
    vmovdqu         y3, [g4 + 1056]
    xor             g6, g6
    mov             g8, 384
    INTERLEAVE_COL  y15, y14, y0, y2
    INTERLEAVE_COL  y13, y12, y1, y3
    vmovdqu         y4, [g4 + 512]          ; Row 8.
    vmovdqu         y5, [g4 + 544]
    vmovdqu         y6, [g4 + 1536]         ; Row 24.
    vmovdqu         y7, [g4 + 1568]
    lea             g7, [pat_idct32_pass1 + 352]
    INTERLEAVE_COL  y11, y10, y4, y6
    INTERLEAVE_COL  y9, y8, y5, y7
    vpbroadcastd    y6, [pat_dw_64]         ; Bias.
     .loop_pass1_even1a:
    vpbroadcastd    y0, [g7]                ; Load the DCT multiplication factors.
    vpbroadcastd    y1, [g7 + 4]
    %macro PROCESS_ROW 3                    ; %1: input 0, %2: input 1, %3: store location's index.
    vpmaddwd        y2, %1, y0
    vpmaddwd        y3, %2, y1
    vpaddd          y4, y2, y3
    vpsubd          y5, y2, y3
    vpaddd          y4, y4, y6              ; Add the bias.
    vpaddd          y5, y5, y6
    vmovdqu         [g5 + 2048 + g6 + %3], y4
    vmovdqu         [g5 + 2048 + g8 + %3], y5
    %endmacro
    PROCESS_ROW     y15, y11, 0
    PROCESS_ROW     y14, y10, 32
    PROCESS_ROW     y13, y9, 64
    PROCESS_ROW     y12, y8, 96
    %unmacro PROCESS_ROW 3
    add             g6, 128
    sub             g8, 128
    add             g7, 8
    cmp             g6, 128*2
    jne             .loop_pass1_even1a

    ; 2. Process rows 4, 12, 20, 28 (src[128], src[384], src[640] and src[896]) to get a_4, a_5, a_6 and a_7.
    ; Also calculate (a_0 +/- a_4), (a_1 +/- a_5), (a_2 +/- a_6), (a_3 +/- a_7).
    LOAD_COL        8, 16, 320, 0
    mov             g8, 896
    .loop_pass1_even1b:
    vpbroadcastd    y6, [g7]
    vpbroadcastd    y7, [g7 + 4]
    %macro PROCESS_ROW 3                    ; %1: input 0, %2: input 1, %3: store location's index.
    vpmaddwd        y2, %1, y6
    vpmaddwd        y3, %2, y7
    vpaddd          y4, y2, y3
    vmovdqu         y5, [g5 + 2048 + g6 + %3]
    vpaddd          y2, y4, y5
    vpsubd          y4, y5, y4
    vmovdqu         [g5 + 2048 + g6 + %3], y2
    vmovdqu         [g5 + 2048 + g8 + %3], y4
    %endmacro
    PROCESS_ROW     y15, y11, 0
    PROCESS_ROW     y14, y10, 32
    PROCESS_ROW     y13, y9, 64
    PROCESS_ROW     y12, y8, 96
    %unmacro PROCESS_ROW 3
    add             g6, 128
    sub             g8, 128
    add             g7, 8
    cmp             g6, 128*4
    jne             .loop_pass1_even1b

    ; 3. Process rows 2, 6, 10, 14 to calculate the 4 terms of e_0, e_1, e_2, e_3, e_4, e_5, e_6, and e_7.
    LOAD_COL        4, 8, 256, 2048 + 1024
    mov             g8, 2048 + 1024 + 128*8
    call            .loop_idct32_pass1

    ; 4. Process rows 30, 26, 22, 18 to get e_0, e_1, e_2, e_3, e_4, e_5, e_6, and e_7. Also obtain a0 .. a15.
    LOAD_COL_REV    36, 8, 312, 2048
    ;xor g8, g8
    .loop_pass1_even1d:
    %macro LOAD_ADD_STORE 3                 ; %1: input 0, %2: input 1, %3: memory index for storing and loading.
    vmovdqu         y4, [g5 + g6 + %3]
    vmovdqu         y5, [g5 + g6 + %3 + 32]
    vpaddd          y6, %1, y4
    vpaddd          y7, %2, y5
    vmovdqu         [g5 + g6 + %3], y6
    vmovdqu         [g5 + g6 + %3 + 32], y7
    vpsubd          y4, y4, %1
    vpsubd          y5, y5, %2
    vmovdqu         [g5 + g6 + 1024 + %3], y4
    vmovdqu         [g5 + g6 + 1024 + %3 + 32], y5
    %endmacro
    LOAD_DATA       g6, 1024                ; To obtain e_0, e_2, e_4, e_6.
    vpbroadcastd    y6, [g7]
    vpbroadcastd    y7, [g7 + 4]
    MULT_ADD        y0, y15, y11
    MULT_ADD        y1, y14, y10
    MULT_ADD        y2, y13, y9
    MULT_ADD        y3, y12, y8
    LOAD_ADD_STORE  y0, y1, 0               ; Calculate a0 & a15, a2 & a13 ...
    LOAD_ADD_STORE  y2, y3, 64
    add             g6, 128
    sub             g7, 8
    vpbroadcastd    y6, [g7]
    vpbroadcastd    y7, [g7 + 4]
    LOAD_DATA       g6, 1024                ; To obtain e_1, e_3, e_5, e_7.
    MULT_SUB        y0, y15, y11
    MULT_SUB        y1, y14, y10
    MULT_SUB        y2, y13, y9
    MULT_SUB        y3, y12, y8
    LOAD_ADD_STORE  y0, y1, 0               ; Calculate a1 & a14, a3 & a12 ...
    LOAD_ADD_STORE  y2, y3, 64
    %unmacro LOAD_ADD_STORE 3
    add             g6, 128
    sub             g7, 8
    cmp             g6, 2048 + 128*8
    jne             .loop_pass1_even1d

    ; Combine the odd (s0, s1 .. s15) and even (a0, a1, .. a15) terms.
    %macro SET_GPR 4                        ; %1: g4 value, %2: g6 value, %3: g7 value, %4: g8 value.
    mov             g4, %1
    mov             g6, %2
    mov             g7, %3
    mov             g8, %4
    %endmacro
    vbroadcasti128  y6, [pat_idct16_shuf1]
    SET_GPR         128, 0, 2048, 1024
    call            .loop_idct32_pass1_combine
    SET_GPR         -128, 1024, 3968, 1024 + 1024
    call            .loop_idct32_pass1_combine
    %unmacro SET_GPR 4
    %unmacro LOAD_COL 4
    %unmacro LOAD_COL_REV 4

    ; Now, perform IDCT Pass 2.
    ; In the previous pass (IDCT32 Pass 1), after combining, the even terms were written at
    ; even locations (i.e. [g5 + 0], [g5 + 64], .. [g5 + 960]), while the odd terms were written at odd locations (i.e.
    ; [g5 + 32], [g5 + 96], .. [g5 + 992]). This division helps reduce the number of multiplications.
    ; Further note that the even terms were split so that all terms that enable calculation of a_x were grouped together
    ; and those that enable calculation of e_x (x = 0, 1 .. 7) were grouped together.

    ; First process the even terms.
    %macro LOAD_IDCT_FACT 2                 ; %1: g7 value, %2: g8 value.
    xor             g6, g6
    lea             g7, [pat_idct32_pass2 + %1]
    lea             g8, [g5 + %2]
    vpmovsxbw       y15, [g7]
    vpmovsxbw       y14, [g7 + 16]
    vpmovsxbw       y13, [g7 + 32]
    vpmovsxbw       y12, [g7 + 48]
    vpmovsxbw       y11, [g7 + 64]
    vpmovsxbw       y10, [g7 + 80]
    vpmovsxbw       y9, [g7 + 96]
    vpmovsxbw       y8, [g7 + 112]
    %endmacro

    LOAD_IDCT_FACT  0, 2048
    vmovdqu         y2, [pat_idct32_shuf1]
    vpbroadcastd    y1, [pat_dw_2048]       ; Bias.
    .loop_pass2_even:
    vpermq          y0, [g5 + g6], 0xD8
    MULT_HADD       y7, y6, y15, y14
    MULT_HADD       y5, y4, y13, y12
    MULT_HADD       y6, y4, y11, y10
    MULT_HADD       y4, y3, y9, y8
    vphaddd         y7, y7, y5
    vphaddd         y6, y6, y4
    vperm2i128      y5, y7, y6, 0x20
    vperm2i128      y4, y7, y6, 0x31
    vpaddd          y0, y5, y4              ; Calculate a0, a1, a2, ... a7.
    vpsubd          y3, y5, y4              ; Calculate a15, a14, a13 ... a8.
    vpermd          y3, y2, y3              ; Rearrange to give: a8, a9, ... a15.
    vpaddd          y0, y0, y1              ; Add the bias.
    vpaddd          y3, y3, y1
    vmovdqu         [g8 + g6], y0
    vmovdqu         [g8 + g6 + 32], y3
    add             g6, 64
    cmp             g6, 64*32
    jne             .loop_pass2_even

    ; Calculate the odd terms.
    LOAD_IDCT_FACT  128, 0
    call            .loop_idct32_pass2_odd
    LOAD_IDCT_FACT  256, 32
    call            .loop_idct32_pass2_odd

    ; Combine the even and odd terms.
    %macro SET_GPR 3                        ; %1: g4 value, %2: g6 value, %3: g7 value.
    mov             g4, %1
    mov             g6, %2
    mov             g7, %3
    %endmacro
    vbroadcasti128  y10, [pat_idct32_shuf2]
    SET_GPR         0, 128, 128*16
    call            .loop_idct32_pass2_combine2
    SET_GPR         1984, -128, -64
    call            .loop_idct32_pass2_combine2
    %unmacro SET_GPR 3
    %unmacro LOAD_IDCT_FACT 2
    RET

    ; IDCT32 Helper functions

    ; Perform IDCT32 Pass 1 related operations.
    .loop_idct32_pass1:
    vpbroadcastd    y0, [g7]                ; Load the DCT multiplication factors.
    vpbroadcastd    y1, [g7 + 4]
    %macro PROCESS_ROW 3                    ; %1: in 0, %2: in 1, %3: store location 1.
    vpmaddwd        y2, %1, y0
    vpmaddwd        y3, %2, y1
    vpaddd          y2, y2, y3
    vmovdqu         [g5 + g6 + %3], y2
    %endmacro
    PROCESS_ROW     y15, y11, 0
    PROCESS_ROW     y14, y10, 32
    PROCESS_ROW     y13, y9, 64
    PROCESS_ROW     y12, y8, 96
    %unmacro PROCESS_COL 3
    add             g6, 128
    add             g7, 8
    cmp             g6, g8
    jne             .loop_idct32_pass1
    ret

    ; Perform IDCT32 Pass 1 related operations for odd rows alone.
    .loop_idct32_pass1_odd:
    LOAD_DATA       g6, 0
    vpbroadcastd    y6, [g7]
    vpbroadcastd    y7, [g7 + 4]
    MULT_ADD        y0, y15, y11
    MULT_ADD        y1, y14, y10
    MULT_ADD        y2, y13, y9
    MULT_ADD        y3, y12, y8
    STORE_DATA      g6, 0
    sub             g7, 8
    vpbroadcastd    y6, [g7]
    vpbroadcastd    y7, [g7 + 4]
    LOAD_DATA       g6, 128
    MULT_SUB        y0, y15, y11
    MULT_SUB        y1, y14, y10
    MULT_SUB        y2, y13, y9
    MULT_SUB        y3, y12, y8
    STORE_DATA      g6, 128
    add             g6, 256
    sub             g7, 8
    cmp             g6, 256*16
    jne            .loop_idct32_pass1_odd
    ret

    ; Perform IDCT32 Pass 2 related operations for odd columns alone.
    .loop_idct32_pass2_odd:
    vmovdqu         y0, [g5 + g6 + 32]
    MULT_HADD       y7, y6, y15, y14
    MULT_HADD       y5, y4, y13, y12
    MULT_HADD       y6, y3, y11, y10
    MULT_HADD       y4, y2, y9, y8
    vphaddd         y7, y7, y5
    vphaddd         y6, y6, y4
    vperm2i128      y3, y7, y6, 0x20
    vperm2i128      y2, y7, y6, 0x31
    vpaddd          y0, y3, y2
    vmovdqu         [g8 + g6], y0
    add             g6, 64
    cmp             g6, 64*32
    jne            .loop_idct32_pass2_odd
    ret

    ; Combine odd and even terms for IDCT32 Pass 1.
    .loop_idct32_pass1_combine:
    %macro ADD_SUB 3                        ; %1: in/out 0, %2: in 1, %3: out 1/in 2.
    vpsubd          %1, %2, %3
    vpaddd          %3, %2, %3
    vpsrad          %1, %1, 7
    vpsrad          %3, %3, 7
    %endmacro
    %macro COMBINE 4                        ; %1-2: tmp, %3: in/out 0, %4: in/out 1.
    ADD_SUB         y4, %1, %3
    ADD_SUB         y5, %2, %4
    vpackssdw       y4, y4, y5
    vpackssdw       %3, %3, %4
    vpshufb         %3, %3, y6              ; Rearrange all the odd & even terms together : odd, even | odd, even.
    vpshufb         %4, y4, y6
    vpermq          %3, %3, 0xD8
    vpermq          %4, %4, 0xD8
    %endmacro
    %macro SHUFFLE 3                        ; %1-2: in 0-1, %3: memory index to store the separated even and odd terms.
    vperm2i128      y4, %1, %2, 0x20        ; All even terms.
    vperm2i128      y5, %1, %2, 0x31        ; All odd terms.
    vpshufb         y4, y4, y6
    vmovdqu         [g5 + g6 + %3], y4
    vmovdqu         [g5 + g6 + %3 + 32], y5
    %endmacro
    LOAD_DATA       g6, 0
    vmovdqu         y15, [g5 + g7 + 0]
    vmovdqu         y14, [g5 + g7 + 32]
    vmovdqu         y13, [g5 + g7 + 64]
    vmovdqu         y12, [g5 + g7 + 96]
    COMBINE         y15, y14, y0, y1
    COMBINE         y13, y12, y2, y3
    ; Shuffle the even terms to reduce multiplications for the next pass (i.e. Pass 2). Notation below is from C code.
    ; Group src[0], src[128], src[256], src[384], src[512], src[640], src[768], src[896] in the lower lane and
    ; src[64], src[192], src[320], src[448], src[576], src[704], src[832], src[960] in the higher lane.
    SHUFFLE          y0, y2, 0              ; Separate even and odd terms of IDCT32 Pass1 row "n".
    SHUFFLE          y1, y3, 64             ; Separate even and odd terms of IDCT32 Pass1 row "31 - n".
    %unmacro SHUFFLE 3
    %unmacro ADD_SUB 3
    %unmacro COMBINE 4
    add             g6, 128
    add             g7, g4
    cmp             g6, g8
    jne             .loop_idct32_pass1_combine
    ret

    ; Combine odd and even terms for IDCT32 Pass 2.
    .loop_idct32_pass2_combine2:
    %macro COMBINE 4                        ; %1-2: out 0-1 (to hold the sum and difference of the odd and even terms
                                            ; respectively), %3-4: memory index of the even and odd terms respectively.
    vmovdqu         y0, [g5 + g4 + 2048 + %3]
    vmovdqu         y2, [g5 + g4 + %4]      ; Odd terms.
    vpaddd          %1, y0, y2
    vpsubd          %2, y0, y2
    vpsrad          %1, %1, 12
    vpsrad          %2, %2, 12
    %endmacro
    COMBINE          y4, y6, 0, 0
    COMBINE          y5, y7, 32, 32
    %unmacro COMBINE 4
    vpackssdw       y0, y4, y5
    vpackssdw       y1, y6, y7
    vpermq          y0, y0, 0xD8
    vpshufb         y1, y1, y10
    vpermq          y1, y1, 0x27
    ; Reconstruction.
    vpmovzxbw       y2, [g2]
    vpmovzxbw       y3, [g2 + 16]
    add             g2, g3
    vpaddw          y0, y0, y2
    vpaddw          y1, y1, y3
    vpackuswb       y0, y0, y1
    vpermq          y0, y0, 0xd8
    vmovdqu         [g0], y0
    add             g0, g1
    add             g4, g6
    cmp             g4, g7
    jne             .loop_idct32_pass2_combine2
    ret

%unmacro MULT_HADD 4
%unmacro LOAD_DATA 2
%unmacro STORE_DATA 2
%unmacro MULT_ADD 3
%unmacro MULT_SUB 3
%unmacro INTERLEAVE_COL 4
