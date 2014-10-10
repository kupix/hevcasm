# HEVCasm

A collection of permissively licensed assembler optimisations for video codecs.

* **Assemblers:** YASM and NASM 2.10
* **Wrapper / function select / test:** C (for compatibility with all C and C++ codecs)
* **License:** BSD (for compatibility with all GPL, LGPL, permissive and closed-source codecs)
* **External dependencies:** none
* **Included dependencies:** f265; libvpx; x86inc.asm
* **Contributors:** Parabola Research; code gratefully reused from f265, libvpx, x264

Please contact us at Parabola if you're interested in working with us to make use of or to improve this library. We can arrange a commercial license (in the unlikely event that corporate lawyers are unhappy with HM-like BSD terms) and can offer professional support for the project. If you don't see a function you need optimised here, we are happy to develop it on your behalf.

contact@parabolaresearch.com


## Current status

Builds on MS Visual Studio 2013, Win32 and x64 configurations.

Builds on 64-bit Linux.


### Supported functions

#### Generic:

* SAD functions 
* SSD function (e.g. for PSNR computation)
 
#### HEVC Main Profile (8-bit):

* Forward transform (8x8 cosine)
* Inverse transform and add to predicted (some sizes)
* Non-weighted inter prediction
* Inverse quantization
* Simple forward quantization
