// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.


#include "sad.h"
#include "hevcasm_test.h"
#include "Jit.h"
#include <array>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


struct SadSse2
	:
	Jit::Function
{
	SadSse2(Jit::Buffer *buffer, int width, int height)
		:
		Jit::Function(buffer, Jit::CountArguments<hevcasm_sad>::value),
		width(width),
		height(height)
	{
		this->build();
	}

	int width, height;

	void assemble() override
	{
		if ((width % 16) && width != 8) return;

		auto &src = arg64(0);
		auto &stride_src = arg64(1);
		auto &ref = arg64(2);
		auto &stride_ref = arg64(3);

		auto &n = reg64(4);

		const Xbyak::Reg64 *stride_ref_x3 = width == 16 ? &reg64(5) : 0;
		const Xbyak::Reg64 *stride_src_x3 = width == 16 ? &reg64(6) : 0;

		if (width == 8)
		{
			mov(n, height / 2);
		}
		else if (width == 16)
		{
			mov(n, height / 4);
			lea(*stride_ref_x3, ptr[stride_ref + stride_ref * 2]);
			lea(*stride_src_x3, ptr[stride_src + stride_src * 2]);
		}
		else if (width == 32)
		{
			mov(n, height / 2);
		}
		else
		{
			mov(n, height);
		}

		auto &xmm0 = regXmm(0);
		auto &xmm1 = regXmm(1);
		auto &xmm2 = regXmm(2);
		auto &xmm3 = regXmm(3);
		auto &xmm4 = regXmm(4);

		pxor(xmm0, xmm0);
		
		L("loop");
		{
			if (width == 8)
			{
				movq(xmm1, ptr[ref]);
				movhps(xmm1, ptr[ref + stride_ref]);
				movq(xmm2, ptr[src]);
				movhps(xmm2, ptr[src + stride_src]);
				psadbw(xmm1, xmm2);

				lea(ref, ptr[ref + stride_ref * 2]);
				paddd(xmm0, xmm1);
				lea(src, ptr[src + stride_src * 2]);
			}
			else if (width == 16)
			{
				movdqu(xmm1, ptr[ref]);
				movdqu(xmm2, ptr[ref + stride_ref]);
				movdqu(xmm3, ptr[ref + stride_ref * 2]);
				movdqu(xmm4, ptr[ref + *stride_ref_x3]);

				psadbw(xmm1, ptr[src]);
				psadbw(xmm2, ptr[src + stride_src]);
				psadbw(xmm3, ptr[src + stride_src * 2]);
				psadbw(xmm4, ptr[src + *stride_src_x3]);

				lea(src, ptr[src + stride_src * 4]);
				paddd(xmm1, xmm2);
				lea(ref, ptr[ref + stride_ref * 4]);
				paddd(xmm3, xmm4);
				paddd(xmm0, xmm1);
				paddd(xmm0, xmm3);
			}
			else if (width == 32)
			{
				movdqu(xmm1, ptr[ref]);
				movdqu(xmm2, ptr[ref + 16]);
				movdqu(xmm3, ptr[ref + stride_ref]);
				movdqu(xmm4, ptr[ref + stride_ref + 16]);

				psadbw(xmm1, ptr[src]);
				psadbw(xmm2, ptr[src + 16]);
				psadbw(xmm3, ptr[src + stride_src]);
				psadbw(xmm4, ptr[src + stride_src + 16]);

				lea(src, ptr[src + stride_src * 2]);
				paddd(xmm1, xmm2);
				lea(ref, ptr[ref + stride_ref * 2]);
				paddd(xmm3, xmm4);
				paddd(xmm0, xmm1);
				paddd(xmm0, xmm3);
			}
			else
			{
				assert(width > 32);
				movdqu(xmm1, ptr[ref]);
				movdqu(xmm2, ptr[ref + 16]);
				movdqu(xmm3, ptr[ref + 32]);
				if (width > 48) movdqu(xmm4, ptr[ref + 48]);
				psadbw(xmm1, ptr[src + 0 ]);
				psadbw(xmm2, ptr[src + 16]);
				psadbw(xmm3, ptr[src + 32]);
				if (width > 48) psadbw(xmm4, ptr[src + 48]);
				paddd(xmm1, xmm2);

				add(src, stride_src);
				if (width > 48) paddd(xmm3, xmm4);
				paddd(xmm0, xmm1);
				add(ref, stride_ref);
				paddd(xmm0, xmm3);
			}
		}
		dec(n);
		jg("loop");

		movhlps(xmm1, xmm0);
		paddd(xmm0, xmm1);
		movd(eax, xmm0);
	}
};




template <typename Sample>
static int hevcasm_sad_c_ref(const Sample *src, ptrdiff_t stride_src, const Sample *ref, ptrdiff_t stride_ref, uint32_t rect)
{
	const int width = rect >> 8;
	const int height = rect & 0xff;
	int sad = 0;
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			sad += abs((int)src[x + y * stride_src] - (int)ref[x + y * stride_ref]);
		}
	}
	return sad;
}


hevcasm_sad* get_sad(int width, int height, hevcasm_code code)
{
	auto &buffer = *reinterpret_cast<Jit::Buffer *>(code.implementation);
	if (buffer.isa & HEVCASM_SSE2)
	{
#define X(w, h) \
		{ \
			SadSse2 sadSse2(&buffer, w, h); \
			if (w==width && h==height) \
			{ \
				hevcasm_sad *f = sadSse2; \
				if (f) return f; \
			} \
		}

		X_HEVC_PU_SIZES;
#undef X
	}

	if (buffer.isa & (HEVCASM_C_REF | HEVCASM_C_OPT))
	{
		return (hevcasm_sad*)&hevcasm_sad_c_ref<uint8_t>;
	}

	return 0;
}

hevcasm_sad16* get_sad16(int width, int height, hevcasm_code code)
{
	auto &buffer = *reinterpret_cast<Jit::Buffer *>(code.implementation);

	if (buffer.isa & (HEVCASM_C_REF | HEVCASM_C_OPT))
	{
		return (hevcasm_sad16*)&hevcasm_sad_c_ref<uint16_t>;
	}

	return 0;
}


void HEVCASM_API hevcasm_populate_sad(hevcasm_table_sad *table, hevcasm_code code)
{
	for (int height = 4; height <= 64; height += 4)
	{
		for (int width = 4; width <= 64; width += 4)
		{
			*hevcasm_get_sad(table, width, height) = get_sad(width, height, code);
		}
	}
}


void HEVCASM_API hevcasm_populate_sad16(hevcasm_table_sad16 *table, hevcasm_code code)
{
	for (int height = 4; height <= 64; height += 4)
	{
		for (int width = 4; width <= 64; width += 4)
		{
			*hevcasm_get_sad16(table, width, height) = get_sad16(width, height, code);
		}
	}
}


template <typename Sample>
static void hevcasm_sad_multiref_4_c_ref(const Sample *src, ptrdiff_t stride_src, const Sample *ref[], ptrdiff_t stride_ref, int sad[], uint32_t rect)
{
	const int width = rect >> 8;
	const int height = rect & 0xff;

	sad[0] = 0;
	sad[1] = 0;
	sad[2] = 0;
	sad[3] = 0;

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			for (int way = 0; way < 4; ++way)
			{
				sad[way] += abs((int)src[x + y * stride_src] - (int)ref[way][x + y * stride_ref]);
			}
		}
	}
}


#if USE_WEBM_DERIVED
struct Sad4Avx2
	:
	Jit::Function
{
	Sad4Avx2(Jit::Buffer *buffer, int width, int height)
		:
		Jit::Function(buffer, Jit::CountArguments<hevcasm_sad_multiref>::value),
		width(width),
		height(height)
	{
		this->build();
	}

	int width, height;

	Xbyak::Label mask_24_32;
	Xbyak::Label mask_12_16;

	void data() override
	{
		align();

		if (width == 24)
		{
			L(mask_24_32);
			db({ 0xff }, 24);
			db({ 0 }, 8);
		}

		if (width == 12)
		{
			L(mask_12_16);
			db({ 0xff }, 12);
			db({ 0 }, 4);			
			db({ 0xff }, 12);
			db({ 0 }, 4);
		}
	}
		
	void assemble() override
	{
		auto &src = arg64(0);
		auto &src_stride = arg64(1);
		auto &ref = arg64(2);
		auto &ref_stride = arg64(3);
		auto &sads = arg64(4);
		auto &rect = arg64(5);

		auto &xmm0 = regXmm(0);
		auto &xmm1 = regXmm(1);
		auto &xmm2 = regXmm(2);
		auto &xmm3 = regXmm(3);
		auto &xmm4 = regXmm(4);
		auto &xmm5 = regXmm(5);
		auto &xmm6 = regXmm(6);
		auto &xmm7 = regXmm(7);
		auto &xmm8 = regXmm(8);

		if (width == 8)
		{
			auto &ref0 = reg64(6);
			auto &ref1 = reg64(7);
			auto &ref2 = reg64(8);
			auto &ref3 = ref;

			mov(ref0, ptr[ref]);
			mov(ref1, ptr[ref + 0x8]);
			mov(ref2, ptr[ref + 0x10]);
			mov(ref3, ptr[ref + 0x18]);

			and(rect, 0xff);
			auto &n = rect;
			shr(n, 1);

			vmovq(xmm4, ptr[src]);
			vmovq(xmm0, ptr[ref0]);
			vmovq(xmm1, ptr[ref1]);
			vmovq(xmm2, ptr[ref2]);
			vmovq(xmm3, ptr[ref3]);

			sub(ref1, ref0);
			sub(ref2, ref0);
			sub(ref3, ref0);
			lea(ref0, ptr[ref0 + ref_stride]);

			vmovhps(xmm4, ptr[src + src_stride]);
			vmovhps(xmm0, ptr[ref0]);
			vmovhps(xmm1, ptr[ref1 + ref0]);
			vmovhps(xmm2, ptr[ref2 + ref0]);
			vmovhps(xmm3, ptr[ref3 + ref0]);

			lea(ref0, ptr[ref0 + ref_stride]);
			lea(src, ptr[src + src_stride * 2]);

			vpsadbw(xmm0, xmm4);
			vpsadbw(xmm1, xmm4);
			vpsadbw(xmm2, xmm4);
			vpsadbw(xmm3, xmm4);

			dec(n);

			L("loop");
			{
				vmovq(xmm4, ptr[src]);
				vmovq(xmm5, ptr[ref0]);
				vmovq(xmm6, ptr[ref1 + ref0]);
				vmovq(xmm7, ptr[ref2 + ref0]);
				vmovq(xmm8, ptr[ref3 + ref0]);

				lea(ref0, ptr[ref0 + ref_stride]);

				vmovhps(xmm4, ptr[src + src_stride]);
				vmovhps(xmm5, ptr[ref0]);
				vmovhps(xmm6, ptr[ref1 + ref0]);
				vmovhps(xmm7, ptr[ref2 + ref0]);
				vmovhps(xmm8, ptr[ref3 + ref0]);

				lea(ref0, ptr[ref0 + ref_stride]);
				lea(src, ptr[src + src_stride * 2]);

				vpsadbw(xmm5, xmm4);
				vpsadbw(xmm6, xmm4);
				vpsadbw(xmm7, xmm4);
				vpsadbw(xmm8, xmm4);

				vpaddd(xmm0, xmm5);
				vpaddd(xmm1, xmm6);
				vpaddd(xmm2, xmm7);
				vpaddd(xmm3, xmm8);
			}
			dec(n);
			jg("loop");

			vpslldq(xmm1, 4);
			vpslldq(xmm3, 4);
			vpor(xmm0, xmm1);
			vpor(xmm2, xmm3);
			vmovdqa(xmm1, xmm0);
			vmovdqa(xmm3, xmm2);
			vpunpcklqdq(xmm0, xmm2);
			vpunpckhqdq(xmm1, xmm3);
			vpaddd(xmm0, xmm1);
			vmovdqu(ptr[sads], xmm0);
		}
		else if (width == 4)
		{
			auto &ref0 = reg64(6);
			auto &ref1 = reg64(7);
			auto &ref2 = reg64(8);
			auto &ref3 = ref;

			mov(ref0, ptr[ref]);
			mov(ref1, ptr[ref + 0x8]);
			mov(ref2, ptr[ref + 0x10]);
			mov(ref3, ptr[ref + 0x18]);

			vpxor(xmm0, xmm0);
			vpxor(xmm1, xmm1);
			vpxor(xmm2, xmm2);
			vpxor(xmm3, xmm3);

			sub(ref1, ref0);
			sub(ref2, ref0);
			sub(ref3, ref0);

			and (rect, 0xFF);
			auto &n = rect;
			shr(n, 1);

			L("loop");
			{
				vmovd(xmm4, ptr[src]);
				vmovd(xmm5, ptr[ref0]);
				vmovd(xmm6, ptr[ref1 + ref0]);
				vmovd(xmm7, ptr[ref2 + ref0]);
				vmovd(xmm8, ptr[ref3 + ref0]);
				lea(ref0, ptr[ref0 + ref_stride]);
				vpunpckldq(xmm4, ptr[src + src_stride]);
				vpunpckldq(xmm5, ptr[ref0]);
				vpunpckldq(xmm6, ptr[ref1 + ref0]);
				vpunpckldq(xmm7, ptr[ref2 + ref0]);
				vpunpckldq(xmm8, ptr[ref3 + ref0]);
				lea(ref0, ptr[ref0 + ref_stride]);
				lea(src, ptr[src + src_stride * 2]);
				vpsadbw(xmm5, xmm4);
				vpsadbw(xmm6, xmm4);
				vpsadbw(xmm7, xmm4);
				vpsadbw(xmm8, xmm4);
				vpunpckldq(xmm5, xmm6);
				vpunpckldq(xmm7, xmm8);
				vpaddd(xmm0, xmm5);
				vpaddd(xmm2, xmm7);
			}
			dec(n);
			jg("loop");

			vpslldq(xmm1, xmm1, 4);
			vpslldq(xmm3, xmm3, 4);
			vpor(xmm0, xmm1);
			vpor(xmm2, xmm3);
			vmovdqa(xmm1, xmm0);
			vmovdqa(xmm3, xmm2);
			vpunpcklqdq(xmm0, xmm2);
			vpunpckhqdq(xmm1, xmm3);
			vpaddd(xmm0, xmm1);
			vmovdqu(ptr[sads], xmm0);
		}
		else/* if (width == 32 || width == 48 || width == 64)*/
		{
			auto &r0 = arg64(0);
			auto &r1 = arg64(1);
			auto &r2 = arg64(2);
			auto &r3 = arg64(3);
			auto &r4 = arg64(4);
			auto &r5 = arg64(5);

			auto &r6 = reg64(6);
			auto &r7 = reg64(7);
			auto &r8 = reg64(8);

			mov(r6, r1); //stride_src
			mov(r7, r3); // stride_ref

			and (rect, 0xFF);
			auto &n = rect;

			if (width <= 16)
			{
				shr(n, 1);
			}

			mov(r8, r4); // sad[]
			
			mov(r1, ptr[ref + 0 * 8]);
			mov(r3, ptr[ref + 2 * 8]);
			mov(r4, ptr[ref + 3 * 8]);
			mov(r2, ptr[ref + 1 * 8]);

			vpxor(ymm5, ymm5);
			vpxor(ymm6, ymm6);
			vpxor(ymm7, ymm7);
			vpxor(ymm8, ymm8);

			L("loop");
			{
				if (width <= 16)
				{
					vmovdqu(xmm0, ptr[r0]);
					vmovdqu(xmm1, ptr[r1]);
					vmovdqu(xmm2, ptr[r2]);
					vmovdqu(xmm3, ptr[r3]);
					vmovdqu(xmm4, ptr[r4]);

					vinserti128(ymm0, ymm0, ptr[r0 + r6], 1);
					vinserti128(ymm1, ymm1, ptr[r1 + r7], 1);
					vinserti128(ymm2, ymm2, ptr[r2 + r7], 1);
					vinserti128(ymm3, ymm3, ptr[r3 + r7], 1);
					vinserti128(ymm4, ymm4, ptr[r4 + r7], 1);
					
					if (width == 12)
					{
						vpand(ymm0, ptr[rip + mask_12_16]);
						vpand(ymm1, ptr[rip + mask_12_16]);
						vpand(ymm2, ptr[rip + mask_12_16]);
						vpand(ymm3, ptr[rip + mask_12_16]);
						vpand(ymm4, ptr[rip + mask_12_16]);
					}
				}
				else
				{
					vmovdqu(ymm0, ptr[r0]);
					vmovdqu(ymm1, ptr[r1]);
					vmovdqu(ymm2, ptr[r2]);
					vmovdqu(ymm3, ptr[r3]);
					vmovdqu(ymm4, ptr[r4]);
				}

				vpsadbw(ymm1, ymm0);
				vpsadbw(ymm2, ymm0);
				vpsadbw(ymm3, ymm0);
				vpsadbw(ymm4, ymm0);

				if (width == 24)
				{
					vpand(ymm1, ptr[rip + mask_24_32]);
					vpand(ymm2, ptr[rip + mask_24_32]);
					vpand(ymm3, ptr[rip + mask_24_32]);
					vpand(ymm4, ptr[rip + mask_24_32]);
				}
				
				vpaddd(ymm5, ymm1);
				vpaddd(ymm6, ymm2);
				vpaddd(ymm7, ymm3);
				vpaddd(ymm8, ymm4);

				if (width > 32)
				{
					vmovdqu(ymm0, ptr[r0 + 32]);
					vmovdqu(ymm1, ptr[r1 + 32]);
					vmovdqu(ymm2, ptr[r2 + 32]);
					vmovdqu(ymm3, ptr[r3 + 32]);
					vmovdqu(ymm4, ptr[r4 + 32]);

					if (width == 48)
					{
						vpsadbw(xmm1, xmm0);
						vpsadbw(xmm2, xmm0);
						vpsadbw(xmm3, xmm0);
						vpsadbw(xmm4, xmm0);
					}
					else
					{
						vpsadbw(ymm1, ymm0);
						vpsadbw(ymm2, ymm0);
						vpsadbw(ymm3, ymm0);
						vpsadbw(ymm4, ymm0);
					}
					vpaddd(ymm5, ymm1);
					vpaddd(ymm6, ymm2);
					vpaddd(ymm7, ymm3);
					vpaddd(ymm8, ymm4);
				}
				if (width <= 16)
				{
					lea(r0, ptr[r0 + r6 * 2]);
					lea(r1, ptr[r1 + r7 * 2]);
					lea(r2, ptr[r2 + r7 * 2]);
					lea(r3, ptr[r3 + r7 * 2]);
					lea(r4, ptr[r4 + r7 * 2]);
				}
				else
				{
					lea(r0, ptr[r0 + r6]);
					lea(r1, ptr[r1 + r7]);
					lea(r2, ptr[r2 + r7]);
					lea(r3, ptr[r3 + r7]);
					lea(r4, ptr[r4 + r7]);
				}
			}
			dec(n);
			jg("loop");

			vextracti128(xmm0, ymm5, 1);
			vextracti128(xmm1, ymm6, 1);
			vextracti128(xmm2, ymm7, 1);
			vextracti128(xmm3, ymm8, 1);
			
			vpaddd(xmm5, xmm0);
			vpaddd(xmm6, xmm1);
			vpaddd(xmm7, xmm2);
			vpaddd(xmm8, xmm3);
			
			//; two different ways of achieving the same thing - both seem to take similar number of cycles
			if (0)
			{
				//	pshufd xm0, xm5, ORDER(0, 0, 0, 2)
				//	pshufd xm1, xm6, ORDER(0, 0, 0, 2)
				//	pshufd xm2, xm7, ORDER(0, 0, 0, 2)
				//	pshufd xm3, xm8, ORDER(0, 0, 0, 2)
				//	paddd xm5, xm0
				//	paddd xm6, xm1
				//	paddd xm7, xm2
				//	paddd xm8, xm3
				//	movd[r8 + 0 * 4], xm5
				//	movd[r8 + 1 * 4], xm6
				//	movd[r8 + 2 * 4], xm7
				//	movd[r8 + 3 * 4], xm8
			}
			else
			{
				vpslldq(xmm6, 4);
				vpslldq(xmm8, 4);
				vpor(xmm5, xmm6);
				vpor(xmm7, xmm8);
				vmovdqa(xmm6, xmm5);
				vmovdqa(xmm8, xmm7);
				vpunpcklqdq(xmm5, xmm7);
				vpunpckhqdq(xmm6, xmm8);
				vpaddd(xmm5, xmm6);
				vmovdqu(ptr[r8], xmm5);
			}
		}
	}
};

#endif


hevcasm_sad_multiref* get_sad_multiref(int ways, int width, int height, hevcasm_code code)
{
	auto &buffer = *reinterpret_cast<Jit::Buffer *>(code.implementation);
	hevcasm_sad_multiref* f = 0;

	if (ways != 4) return 0;

#if USE_WEBM_DERIVED
	if (buffer.isa & HEVCASM_AVX2)
	{
#define X(w, h) \
		{ \
			Sad4Avx2 sad4Avx2(&buffer, w, h); \
			if (w==width && h==height) \
			{ \
				hevcasm_sad_multiref *f = sad4Avx2; \
				if (f) return f; \
			} \
		}

		X_HEVC_PU_SIZES;
#undef X
	}
#endif

	if (buffer.isa & (HEVCASM_C_REF | HEVCASM_C_OPT))
	{
		if (!f) f = &hevcasm_sad_multiref_4_c_ref<uint8_t>;
	}

	return f;
}


hevcasm_sad_multiref16* get_sad_multiref16(int ways, int width, int height, hevcasm_code code)
{
	auto &buffer = *reinterpret_cast<Jit::Buffer *>(code.implementation);
	hevcasm_sad_multiref16* f = 0;

	if (ways != 4) return 0;

	if (buffer.isa & (HEVCASM_C_REF | HEVCASM_C_OPT))
	{
		if (!f) f = &hevcasm_sad_multiref_4_c_ref<uint16_t>;
	}

	return f;
}


void HEVCASM_API hevcasm_populate_sad_multiref(hevcasm_table_sad_multiref *table, hevcasm_code code)
{
	for (int height = 4; height <= 64; height += 4)
	{
		for (int width = 4; width <= 64; width += 4)
		{
			*hevcasm_get_sad_multiref(table, 4, width, height) = get_sad_multiref(4, width, height, code);
		}
	}

}


void HEVCASM_API hevcasm_populate_sad_multiref16(hevcasm_table_sad_multiref16 *table, hevcasm_code code)
{
	for (int height = 4; height <= 64; height += 4)
	{
		for (int width = 4; width <= 64; width += 4)
		{
			*hevcasm_get_sad_multiref16(table, 4, width, height) = get_sad_multiref16(4, width, height, code);
		}
	}

}



typedef struct
{
	HEVCASM_ALIGN(32, uint8_t, src[128 * 128]);
	HEVCASM_ALIGN(32, uint8_t, ref[128 * 128]);
	hevcasm_sad *f;
	int width;
	int height;
	int sad;
} 
bound_sad;


int init_sad(void *p, hevcasm_code code)
{
	bound_sad *s = (bound_sad *)p;

	hevcasm_table_sad table;
	hevcasm_populate_sad(&table, code);

	s->f = *hevcasm_get_sad(&table, s->width, s->height);

	auto &buffer = *reinterpret_cast<Jit::Buffer *>(code.implementation);
	if (buffer.isa == HEVCASM_C_REF) printf("\t%dx%d:", s->width, s->height);

	return !!s->f;
}


void invoke_sad(void *p, int n)
{
	bound_sad *s = (bound_sad *)p;
	const uint8_t *unaligned_ref = &s->ref[1 + 1 * 128];
	while (n--)
	{
		s->sad = s->f(s->src, 64, unaligned_ref, 64, HEVCASM_RECT(s->width, s->height));
	}
}


int mismatch_sad(void *boundRef, void *boundTest)
{
	bound_sad *ref = (bound_sad *)boundRef;
	bound_sad *test = (bound_sad *)boundTest;

	return  ref->sad != test->sad;
}


static const int partitions[][2] = {
	{ 64, 64 },{ 64, 48 },{ 64, 32 },{ 64, 16 },
	{ 48, 64 },
	{ 32, 64 },{ 32, 32 },{ 32, 24 },{ 32, 16 }, {32, 8},
	{ 24, 32 },
	{ 16, 64 },{ 16, 32 },{ 16, 16 },{ 16, 12 },{ 16, 8 },{16, 4},
	{ 12, 16 },
	{ 8, 32 },{ 8, 16 },{ 8, 8 }, { 8, 4 },
	{ 4, 8 },
	{ 0, 0 } };


void HEVCASM_API hevcasm_test_sad(int *error_count, hevcasm_instruction_set mask)
{
	printf("\nhevcasm_sad - Sum of Absolute Differences\n");

	bound_sad b[2];

	for (int x = 0; x < 128 * 128; x++) b[0].src[x] = rand();
	for (int x = 0; x < 128 * 128; x++) b[0].ref[x] = rand();

	for (int i = 0; partitions[i][0]; ++i)
	{
		b[0].width = partitions[i][0];
		b[0].height = partitions[i][1];
		b[1] = b[0];
		*error_count += hevcasm_test(&b[0], &b[1], init_sad, invoke_sad, mismatch_sad, mask, 100000);
	}
}


typedef struct
{
	hevcasm_sad_multiref *f;
	int ways;
	int width;
	int height;
	HEVCASM_ALIGN(32, uint8_t, src[128 * 128]);
	HEVCASM_ALIGN(32, uint8_t, ref[128 * 128]);
	const uint8_t *ref_array[4];
	int sad[4];
} 
bound_sad_multiref;


int init_sad_multiref(void *p, hevcasm_code code)
{
	bound_sad_multiref *s = (bound_sad_multiref *)p;

	hevcasm_table_sad_multiref table;
	hevcasm_populate_sad_multiref(&table, code);
	s->f = *hevcasm_get_sad_multiref(&table, s->ways, s->width, s->height);

	auto &buffer = *reinterpret_cast<Jit::Buffer *>(code.implementation);
	if (s->f && buffer.isa == HEVCASM_C_REF)
	{
		printf("\t%d-way %dx%d : ", s->ways, s->width, s->height);
	}

	return !!s->f;
}


void invoke_sad_multiref(void *p, int n)
{
	bound_sad_multiref *s = (bound_sad_multiref *)p;

	s->sad[0] = 0;
	s->sad[1] = 0;
	s->sad[2] = 0;
	s->sad[3] = 0;

	while (n--)
	{
		s->f(s->src, 64, s->ref_array, 64, s->sad, HEVCASM_RECT(s->width, s->height));
	}
}


int mismatch_sad_multiref(void *boundRef, void *boundTest)
{
	bound_sad_multiref *ref = (bound_sad_multiref *)boundRef;
	bound_sad_multiref *test = (bound_sad_multiref *)boundTest;

	assert(ref->ways);

	for (int i = 0; i < ref->ways; ++i)
	{
		if (ref->sad[i] != test->sad[i]) return 1;
	}

	return 0;
}


void HEVCASM_API hevcasm_test_sad_multiref(int *error_count, hevcasm_instruction_set mask)
{
	bound_sad_multiref b[2];

	b[0].ways = 4;

	printf("\nhevcasm_sad_multiref - Sum Of Absolute Differences with multiple references (%d candidate references)\n", b[0].ways);

	for (int x = 0; x < 128 * 128; x++) b[0].src[x] = rand();
	for (int x = 0; x < 128 * 128; x++) b[0].ref[x] = rand();

	b[0].ref_array[0] = &b[0].ref[1 + 2 * 128];
	b[0].ref_array[1] = &b[0].ref[2 + 1 * 128];
	b[0].ref_array[2] = &b[0].ref[3 + 2 * 128];
	b[0].ref_array[3] = &b[0].ref[2 + 3 * 128];

	for (int i = 0; partitions[i][0]; ++i)
	{
		b[0].width = partitions[i][0];
		b[0].height = partitions[i][1];
		b[1] = b[0];
		*error_count += hevcasm_test(&b[0], &b[1], init_sad_multiref, invoke_sad_multiref, mismatch_sad_multiref, mask, 1000);
	}
}
