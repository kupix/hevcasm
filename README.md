HEVCasm
=======

A collection of permissively licensed assembler optimisations for video codecs.

Currently focussed on HEVC Main Profile decode and encode.

**Assembler:** YASM
**Wrapper / function select / test:** C (for compatibility with all C and C++ codecs)
**License:** BSD (for compatibility with all GPL, LGPL, permissive and closed-source codecs)
**External dependencies:** none
**Included dependencies:** libvpx; x86inc.asm
**Contributors:** Parabola engineers; code gratefully used from libvpx, x264

Current status
--------------

Builds on MS Visual Studio 2013, x64 configuration. Test output is as below. 

```
HEVCasm test program

Detected instruction set support:
        SSE2 (SSE2)
        SSE3 (SSE3)
        SSSE3 (Supplementary SSE3)
        SSE41 (SSE4.1)
        POPCNT (POPCNT)
        SSE42 (SSE4.2)
        AVX (AVX)
        RDRAND (RDRAND)
        PCLMUL_AES (PCLMUL and AES)
        AVX2 (AVX2)


inverse_transform_add
        DCT 8x8 :  C:1076 SSSE3:382(x2.82)

sad
        16x16 :  C:342 SSE2:32(x10.69)
        16x8 :  C:182 SSE2:18(x10.11)
        8x16 :  C:239 SSE2:34(x7.03)
        8x8 :  C:132 SSE2:22(x6.00)
        8x4 :  C:71 SSE2:15(x4.73)
        4x8 :  C:100
```



