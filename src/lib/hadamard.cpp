// Copyright (C) 2016 Parabola Research Limited
//
// Use of this source code is governed by a BSD-style license that
// can be found in the COPYING file in the root of the source tree.


#include "hadamard.h"
#include "hevcasm_test.h"
#include "Jit.h"
#include <stdlib.h>
#include <assert.h>




static void hadamard_iteration(int m, int n, int *dst, int *src, ptrdiff_t stride)
{
	for (int i = 0; i < m; i += 2 * n)
	{
		for (int j = 0; j < n; ++j)
		{
			int a = src[(i + j)*stride];
			int b = src[(i + n + j)*stride];
			dst[i + j] = a + b;
			dst[i + n + j] = a - b;
		}
	}
}

static void hadamard_transform(int m, int n, int *dst, int *src, ptrdiff_t stride)
{
	if (n == 1)
	{
		hadamard_iteration(m, n, dst, src, stride);
	}
	else
	{
		assert(m <= 8);
		int temp[8];
		hadamard_iteration(m, n, temp, src, stride);
		hadamard_transform(m, n / 2, dst, temp, 1);
	}
}


static int compute_satd(int n, const uint8_t *pA, ptrdiff_t strideA, const uint8_t *pB, ptrdiff_t strideB)
{
	assert(n <= 8);

	int intermediate[8][8];

	// subtraction and horizontal transform
	for (int y = 0; y < n; ++y)
	{
		int diff[8];
		for (int x = 0; x < n; ++x)
		{
			diff[x] = pA[x] - pB[x];
		}

		hadamard_transform(n, n / 2, intermediate[y], diff, 1);

		pA += strideA;
		pB += strideB;
	}
	
	// vertical transform and sum of absolutes
	const int roundingOffset = n / 4;
	int sad = roundingOffset;
	for (int x = 0; x < n; ++x)
	{
		int transformed[8];

		hadamard_transform(n, n / 2, transformed, &intermediate[0][x], 8);

		for (int y = 0; y < n; ++y)
		{
			sad += abs(transformed[y]);
		}
	}
	return sad / (n / 2);
}


static int compute_satd_2x2(const uint8_t *pA, ptrdiff_t strideA, const uint8_t *pB, ptrdiff_t strideB)
{
	return compute_satd(2, pA, strideA, pB, strideB);
}


static int compute_satd_4x4(const uint8_t *pA, ptrdiff_t strideA, const uint8_t *pB, ptrdiff_t strideB)
{
	return compute_satd(4, pA, strideA, pB, strideB);
}


static int compute_satd_8x8(const uint8_t *pA, ptrdiff_t strideA, const uint8_t *pB, ptrdiff_t strideB)
{
	return compute_satd(8, pA, strideA, pB, strideB);
}

#define ORDER(a, b, c, d) ((a << 6) | (b << 4) | (c << 2) | d)


//; % 1 - destination
//; % 2 and %3 - sources
//; uses m5 and m7
#define DIFF_BW(a1, a2, a3) \
	movdqu(a1, ptr a2); \
	movdqu(m5, ptr a3); \
	punpcklbw(a1, m7); \
	punpcklbw(m5, m7);\
	psubw(a1, m5);

#define BUTTERFLY_HORIZONTAL_4(a1, a2) \
	pshufd(a2, a1, ORDER(1, 0, 3, 2)); \
	pxor(a1, m6); \
	psubw(a1, m6); \
	paddw(a1, a2);

#define BUTTERFLY_HORIZONTAL_2(a1, a2) \
	pshufd(a2, a1, ORDER(2, 3, 0, 1)); \
	pxor(a1, m6); \
	psubw(a1, m6); \
	paddw(a1, a2);

#define BUTTERFLY_HORIZONTAL_1(a1, a2) \
	movdqa(a2, a1); \
	phaddw(a2, a1); \
	phsubw(a1, a1); \
	punpcklwd(a1, a2);



struct Satd4
	:
	Jit::Function
{
	Satd4(Jit::Buffer *buffer)
		:
		Jit::Function(buffer, Jit::CountArguments<hevcasm_hadamard_satd>::value)
	{
		this->build();
	}

	Xbyak::Label constant_ffffffff00000000ffffffff00000000;

	void data()
	{
		align();

		L(constant_ffffffff00000000ffffffff00000000);
		dd({ -1, 0 }, 2);
	}

	void assemble()
	{
		auto &m0 = regXmm(0);
		auto &m1 = regXmm(1);
		auto &m2 = regXmm(2);
		auto &m3 = regXmm(3);
		auto &m4 = regXmm(4);
		auto &m5 = regXmm(5);
		auto &m6 = regXmm(6);
		auto &m7 = regXmm(7);

		auto &r0 = arg64(0);
		auto &r1 = arg64(1);
		auto &r2 = arg64(2);
		auto &r3 = arg64(3);

		pxor(m7, m7);

		DIFF_BW(m0, [r0], [r2])
		DIFF_BW(m1, [r0 + r1], [r2 + r3])
		lea(r0, ptr[r0 + r1 * 2]);
		lea(r2, ptr[r2 + r3 * 2]);
		DIFF_BW(m2, [r0], [r2])
		DIFF_BW(m3, [r0 + r1], [r2 + r3])

		punpcklqdq(m0, m2);
		punpcklqdq(m1, m3);

		// diff in m0, m1, m2, m3

		movaps(m6, ptr[rip + constant_ffffffff00000000ffffffff00000000]);
		BUTTERFLY_HORIZONTAL_2(m0, m4)
		BUTTERFLY_HORIZONTAL_2(m1, m4)

		BUTTERFLY_HORIZONTAL_1(m0, m4)
		BUTTERFLY_HORIZONTAL_1(m1, m4)

		// rows in m0, m1, m2, m3

		// vertical butterfly 2
		// mova m6, [constant_ffffffffffffffff0000000000000000]
		pshufd(m6, m6, ORDER(3, 1, 2, 0));

		BUTTERFLY_HORIZONTAL_4(m0, m4);
		BUTTERFLY_HORIZONTAL_4(m1, m4);

		// rows in m0, m1

		// vertical butterfly 1
		vpsubw(m2, m0, m1); // really is this SSE2?
		paddw(m0, m1);

		// rows in m0, m2

		pabsw(m0, m0);
		pabsw(m2, m2);

		paddw(m0, m2);
		phaddw(m0, m0);
		phaddw(m0, m0);
		phaddw(m0, m0);

		movd(eax, m0);
		and(eax, 0xffff);
		add(eax, 1);
		shr(eax, 1);
	}
	};

#define HADAMARD_8_H(a1, a2) \
	vmovq(xmm ## a1, ptr [r0]);  /*srcA(7..0, y) */ \
	vmovq(xmm ## a2, ptr [r2]);  /* srcB(7..0, y) */ \
	vmovq(xmm10, ptr [r0+r1]);  /* srcA(7..0, y+2) */ \
	vmovq(xmm11, ptr [r2+r3]);  /* srcB(7..0, y+2) */ \
	\
	lea(r0, ptr[r0+r1*2]); \
	lea(r2, ptr[r2+r3*2]); \
	\
	vinserti128(m ## a1, m ## a1, ptr[r0], 1); \
	vinserti128(m ## a2, m ## a2, ptr[r2], 1); \
	vinserti128(m10, m10, ptr[r0+r1], 1); \
	vinserti128(m11, m11, ptr[r2+r3], 1); \
	\
	vpunpcklqdq(m ## a1, m ## a1);  /* b  srcA(7..0, y+1),  srcA(7..0, y+1),  srcA(7..0, y+0),  srcA(7..0, y+0) */ \
	vpunpcklqdq(m ## a2, m ## a2);  /* b  srcB(7..0, y+1),  srcB(7..0, y+1),  srcB(7..0, y+0),  srcB(7..0, y+0) */ \
	vpunpcklqdq(m10, m10);  /* b  srcA(7..0, y+3),  srcA(7..0, y+3),  srcA(7..0, y+2),  srcA(7..0, y+2) */ \
	vpunpcklqdq(m11, m11);  /* b  srcB(7..0, y+3),  srcB(7..0, y+3),  srcB(7..0, y+2),  srcB(7..0, y+2) */ \
	\
	vpmaddubsw(m ## a1, m15); /* w  srcA(7,y+1)+srcA(6,y+1)..srcA(1,y+1)+srcA(0,y+1),  srcA(7,y+1)-srcA(6,y+1)..srcA(1,y+1)-srcA(0,y+1),  srcA(7,y+0)+srcA(6,y+0)..srcA(1,y+0)+srcA(0,y+0),  srcA(7,y+0)-srcA(6,y+0)..srcA(1,y+0)-srcA(0,y+0),  */ \
	vpmaddubsw(m ## a2, m15); /* w  srcB(7,y+1)+srcB(6,y+1)..srcB(1,y+1)+srcB(0,y+1),  srcB(7,y+1)-srcB(6,y+1)..srcB(1,y+1)-srcB(0,y+1),  srcB(7,y+0)+srcB(6,y+0)..srcB(1,y+0)+srcB(0,y+0),  srcB(7,y+0)-srcB(6,y+0)..srcB(1,y+0)-srcB(0,y+0), */ \
	vpmaddubsw(m10, m15); /* w  srcA(7,y+3)+srcA(6,y+3)..srcA(1,y+3)+srcA(0,y+3),  srcA(7,y+3)-srcA(6,y+3)..srcA(1,y+3)-srcA(0,y+3),  srcA(7,y+2)+srcA(6,y+2)..srcA(1,y+2)+srcA(0,y+2),  srcA(7,y+2)-srcA(6,y+2)..srcA(1,y+2)-srcA(0,y+2), */ \
	vpmaddubsw(m11, m15); /* w  srcB(7,y+3)+srcB(6,y+3)..srcB(1,y+3)+srcB(0,y+3),  srcB(7,y+3)-srcB(6,y+3)..srcB(1,y+3)-srcB(0,y+3),  srcB(7,y+2)+srcB(6,y+2)..srcB(1,y+2)+srcB(0,y+2),  srcB(7,y+2)-srcB(6,y+2)..srcB(1,y+2)-srcB(0,y+2), */ \
	\
	vpsubw(m ## a1, m ## a2); /* w src(7,y+1)+src(6,y+1)..src(1,y+1)+src(0,y+1), src(7,y+1)-src(6,y+1)..src(1,y+1)-src(0,y+1), src(7,y+0)+src(6,y+0)..src(1,y+0)+src(0,y+0), src(7,y+0)-src(6,y+0)..src(1,y+0)-src(0,y+0),  */ \
	vpsubw(m10, m11); /* w src(7,y+3)+src(6,y+3)..src(1,y+3)+src(0,y+3), src(7,y+3)-src(6,y+3)..src(1,y+3)-src(0,y+3), src(7,y+2)+src(6,y+2)..src(1,y+2)+src(0,y+2), src(7,y+2)-src(6,y+2)..src(1,y+2)-src(0,y+2),  */ \
	\
	vphsubw(m12, m ## a1, m10); \
	vphaddw(m13, m ## a1, m10); \
	\
	vpunpckldq(m4, m12, m13); \
	vpunpckhdq(m5, m12, m13); \
	\
	vphsubw(m6, m4, m4); \
	vphaddw(m7, m4, m4); \
	vphsubw(m8, m5, m5); \
	vphaddw(m9, m5, m5); \
	\
	vpunpcklwd(m ## a1, m6, m7); \
	vpunpcklwd(m ## a2, m8, m9); \


#define	PUNPCKHDQQQ(a1, a2, a3) \
	vperm2i128(m ## a1, m ## a2, m ## a3, 0x31);

#define	PUNPCKLDQQQ(a1, a2, a3) \
	vinserti128(m ## a1, m ## a2, xmm ## a3, 0x1);


struct Satd8
	:
	Jit::Function
{
	Satd8(Jit::Buffer *buffer)
		:
		Jit::Function(buffer, Jit::CountArguments<hevcasm_hadamard_satd>::value)
	{
		this->build();
	}

	Xbyak::Label constant_010101010101010101ff01ff01ff01ff010101010101010101ff01ff01ff01ff;
	Xbyak::Label constant_times_16_dw_1;

	void data()
	{
		align(32);
		L(constant_010101010101010101ff01ff01ff01ff010101010101010101ff01ff01ff01ff);
		db({ 1, 1 }, 4);
		db({ 1, -1 }, 4);
		db({ 1, 1 }, 4);
		db({ 1, -1 }, 4);

		L(constant_times_16_dw_1);
		dw({ 1 }, 16);
	}

	void assemble()
	{
		auto &m0 = ymm0;
		auto &m1 = ymm1;
		auto &m2 = ymm2;
		auto &m3 = ymm3;
		auto &m4 = ymm4;
		auto &m5 = ymm5;
		auto &m6 = ymm6;
		auto &m7 = ymm7;
		auto &m8 = ymm8;
		auto &m9 = ymm9;
		auto &m10 = ymm10;
		auto &m11 = ymm11;
		auto &m12 = ymm12;
		auto &m13 = ymm13;
		auto &m14 = ymm14;
		auto &m15 = ymm15;

		regXmm(15);

		auto &r0 = arg64(0);
		auto &r1 = arg64(1);
		auto &r2 = arg64(2);
		auto &r3 = arg64(3);

		vmovdqa(m15, ptr[rip + constant_010101010101010101ff01ff01ff01ff010101010101010101ff01ff01ff01ff]);

		HADAMARD_8_H(0, 1);
		lea(r0, ptr[r0 + r1 * 2]);
		lea(r2, ptr[r2 + r3 * 2]);
		HADAMARD_8_H(2, 3);

		// horizontal transform now done - output order is incorrect but same in each row so OK for SATD

		// (x, 0) in m0 low
		// (x, 1) in m1 low
		// (x, 2) in m0 high
		// (x, 3) in m1 high
		// (x, 4) in m2 low
		// (x, 5) in m3 low
		// (x, 6) in m2 high
		// (x, 7) in m3 high

		// m0  H = 00 + 00000 L = +0000000
		// m1  H = 000 + 0000 L = 0 + 000000
		// m2  H = 000000 + 0 L = 0000 + 000
		// m3  H = 0000000 + L = 00000 + 00

		vpaddw(m4, m0, m2); // H = 00 + 000 + 0 L = +000 + 000
		vpaddw(m5, m1, m3); // H = 000 + 000 + L = 0 + 000 + 00
		vpsubw(m6, m0, m2); // H = 00 + 000 - 0 L = +000 - 000
		vpsubw(m7, m1, m3); // H = 000 + 000 - L = 0 + 000 - 00

		vpaddw(m0, m4, m5); // H = 00++00++L = ++00++00
		vpsubw(m1, m4, m5); // H = 00 + -00 + -L = +-00 + -00
		vpaddw(m2, m6, m7); // H = 00++00--L = ++00--00
		vpsubw(m3, m6, m7); // H = 00 + -00 - +L = +-00 - +00

		PUNPCKHDQQQ(4, 0, 1); // H = 00 + -00 + -L = 00++00++
		PUNPCKLDQQQ (5, 0, 1); // H = +-00 + -00 L = ++00++00
		PUNPCKHDQQQ(6, 2, 3); // H = 00 + -00 - +L = 00++00--
		PUNPCKLDQQQ (7, 2, 3); // L = +-00 - +00 L = ++00--00

		vpaddw(m0, m4, m5); // H = +-+-+-+-L = ++++++++
		vpsubw(m1, m5, m4); // H = +--++-- + L = ++--++--
		vpaddw(m2, m6, m7); // H = +-+-- + -+L = ++++----
		vpsubw(m3, m7, m6); // H = +-- + -++ - L = ++----++

		// vertical transform now done too.

		vpabsw(m0, m0);
		vpabsw(m1, m1);
		vpabsw(m2, m2);
		vpabsw(m3, m3);

		// transformed absolute differences now computed, just need to sum them and return

		vpaddw(m0, m1);
		vpaddw(m2, m3);

		vpaddw(m0, m2); // 16 w

		vpmaddwd(m0, m0, ptr[rip + constant_times_16_dw_1]); // 8 d
		vextracti128(xmm1, m0, 1);
		
		vzeroupper();

		paddd(xmm0, xmm1);
		movhlps(xmm1, xmm0);
		paddd(xmm0, xmm1);
		pshuflw(xmm1, xmm0, 0xe);
		paddd(xmm0, xmm1);
		movd(eax, xmm0);
		add(eax, 2);
		shr(eax, 2);
	}
};

#ifdef HEVCASM_X64
hevcasm_hadamard_satd hevcasm_hadamard_satd_4x4_sse2;
hevcasm_hadamard_satd hevcasm_hadamard_satd_8x8_avx2;
#endif

int wrap(const uint8_t *srcA, ptrdiff_t stride_srcA, const uint8_t *srcB, ptrdiff_t stride_srcB)
{
	static Jit::Buffer buffer(8000);
	static Satd8 satd8(&buffer);
	static hevcasm_hadamard_satd *f = satd8;
	return f(srcA, stride_srcA, srcB, stride_srcB);
}

void HEVCASM_API hevcasm_populate_hadamard_satd(hevcasm_table_hadamard_satd *table, hevcasm_instruction_set mask)
{
	*hevcasm_get_hadamard_satd(table, 1) = 0;
	*hevcasm_get_hadamard_satd(table, 2) = 0;
	*hevcasm_get_hadamard_satd(table, 3) = 0;

	if (mask & (HEVCASM_C_REF | HEVCASM_C_OPT))
	{
		*hevcasm_get_hadamard_satd(table, 1) = compute_satd_2x2;
		*hevcasm_get_hadamard_satd(table, 2) = compute_satd_4x4;
		*hevcasm_get_hadamard_satd(table, 3) = compute_satd_8x8;
	}

#ifdef HEVCASM_X64
	if (mask & HEVCASM_SSE2)
	{
		static Jit::Buffer buffer(8000);
		static Satd4 satd4(&buffer);
		*hevcasm_get_hadamard_satd(table, 2) = satd4;
	}
	if (mask & HEVCASM_AVX2)
	{
		*hevcasm_get_hadamard_satd(table, 3) = &wrap;//satd8.function();;
	}
#endif
}


typedef struct
{
	uint8_t *srcA, *srcB;
	int log2TrafoSize;
	int satd;
	hevcasm_hadamard_satd *f;
}
bound_hadamard_satd;


int init_hadamard_satd(void *p, hevcasm_instruction_set mask)
{
	bound_hadamard_satd *s = (bound_hadamard_satd *)p;

	hevcasm_table_hadamard_satd table;

	hevcasm_populate_hadamard_satd(&table, mask);

	s->f = *hevcasm_get_hadamard_satd(&table, s->log2TrafoSize);

	if (mask == HEVCASM_C_REF)
	{
		const int nCbS = 1 << s->log2TrafoSize;
		printf("\t%dx%d : ", nCbS, nCbS);
	}

	return !!s->f;
}


void invoke_hadamard_satd(void *p, int n)
{
	bound_hadamard_satd *s = (bound_hadamard_satd *)p;
	const int nCbS = 1 << s->log2TrafoSize;
	while (n--)
	{
		s->satd = s->f(s->srcA, 2*nCbS, s->srcB, 2*nCbS);
	}
}


int mismatch_hadamard_satd(void *boundRef, void *boundTest)
{
	bound_hadamard_satd *ref = (bound_hadamard_satd *)boundRef;
	bound_hadamard_satd *test = (bound_hadamard_satd *)boundTest;

	return ref->satd != test->satd;
}


void HEVCASM_API hevcasm_test_hadamard_satd(int *error_count, hevcasm_instruction_set mask)
{
	printf("\nhevcasm_hadamard_satd - Hadamard Sum of Absolute Transformed Differences\n");

	uint8_t  srcA[16 * 8];
	uint8_t  srcB[16 * 8];

	for (int i = 0; i < 16 * 8; ++i)
	{
		srcA[i] = rand() & 0xff;
		srcB[i] = rand() & 0xff;
	}

	bound_hadamard_satd b[2];

	b[0].srcA = srcA;
	b[0].srcB = srcB;

	for (b[0].log2TrafoSize = 3; b[0].log2TrafoSize >= 1; --b[0].log2TrafoSize)
	{
		b[1] = b[0];
		*error_count += hevcasm_test(&b[0], &b[1], init_hadamard_satd, invoke_hadamard_satd, mismatch_hadamard_satd, mask, 100000);
	}
}
