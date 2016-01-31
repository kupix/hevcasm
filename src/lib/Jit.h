/*
The copyright in this software is being made available under the BSD
License, included below. This software may be subject to other third party
and contributor rights, including patent rights, and no such rights are
granted under this license.


Copyright(c) 2016, Parabola Research Limited
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met :

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and / or other materials provided with the distribution.
* Neither the name of the copyright holder nor the names of its contributors may
be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#define NOMINMAX 

#include "xbyak/xbyak.h"
#include "hevcasm.h"
#include <vector>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <iostream>
#include <algorithm>


namespace Jit {

struct Buffer
{
	Buffer(size_t n, hevcasm_instruction_set isa=0)
		:
		buffer(n),
		isa(isa)
	{
		this->update(&this->buffer[0]);
		Xbyak::CodeArray::protect(this->p, this->nRemainingBytes(), true);
	}

	void update(uint8_t *p)
	{
		this->p = Xbyak::CodeArray::getAlignedAddress(p);
		assert(this->nRemainingBytes() >= 0);
	}

	void increment(size_t n)
	{
		this->update(this->p + n);
	}

	ptrdiff_t nRemainingBytes() const
	{
		return &this->buffer[0] + this->buffer.size() - this->p;
	}

	uint8_t *pointer() const
	{
		return this->p;
	}

	hevcasm_instruction_set const isa;

private:
	std::vector<uint8_t> buffer;
	uint8_t *p;
};


template <typename FunctionType>
struct CountArguments;


template<typename R, typename ...Args>
struct CountArguments<R(Args...)>
{
	static const size_t value = sizeof...(Args);
};


struct Function
	:
	Xbyak::CodeGenerator
{
	int nArguments;

	Function(Buffer *buffer, int nArguments)
		:
		buffer(buffer),
		Xbyak::CodeGenerator(buffer ? buffer->nRemainingBytes() : 4096, buffer ? buffer->pointer() : 0),
		nArguments(nArguments)
	{
	}

	int pass = 1;

	virtual void assemble() = 0;
	virtual void data() { };

	void build()
	{
		assert(pass == 1);

		this->assemble();
		this->data();

		if (this->getSize() == 0) return;

		if (this->debug)
		{
			std::cerr << "function has "
				<< this->nArguments << " arguments, "
				<< this->variables << " variables and uses the first "
				<< this->mmRegisters << " XMM registers and requies " 
				<< this->stackSize << " stack bytes\n";
		}

		this->reset();
		this->pass = 2;

		this->prologue(this->variables, this->mmRegisters, this->stackSize);
		this->variables = 0;
		this->mmRegisters = 0;
		this->stackSize = 0;
		this->assemble();
		this->epilogue(this->variables, this->mmRegisters, this->stackSize);
		this->data();

		this->buffer->increment(this->getSize());
	}

	void buildSinglePass(int variables, int mmRegisters, int stackSize)
	{
		this->pass = 0; // indicate special mode
		this->prologue(variables, mmRegisters, stackSize);
		this->variables = 0;
		this->mmRegisters = 0;
		this->stackSize = 0;
		this->assemble();
		this->epilogue(variables, mmRegisters, stackSize);
		this->data();
		this->buffer->increment(this->getSize());
	}

	template <typename F>
	operator F *()
	{
		if (this->getSize() == 0) return 0;
		assert(CountArguments<F>::value == this->nArguments);
		return reinterpret_cast<F *>(this->getCode());
	}

protected:
	int frameSize;
	int stackOffset = 0;
	int stackSize = 0;

	void prologue(int variables, int mmRegisters, int stack)
	{
#ifdef WIN32
		this->stackOffset = 0x28;
		this->frameSize = 0;

		int registers = nArguments + variables;
		for (int i = 7; i < registers; ++i)
		{
			push(reg64(i));
			this->frameSize += 8;
		}

		if (mmRegisters > 8 || stack)
		{
			if (this->frameSize & 8) this->stackOffset += 8;
			if (mmRegisters > 8) this->stackOffset += (mmRegisters - 8) * 16;
			this->stackOffset += stack;
			this->frameSize += this->stackOffset;

			// chkstk implementation
			sub(rsp, this->stackOffset);
			for (int d = this->stackOffset - 4096; d >= 0; d -= 4096)
			{
				mov(ptr[rsp + d], rsp);
			}
		}

		// Store xmm6 and xmm7 in shadow space
		if (mmRegisters > 6) vmovaps(ptr[rsp + this->frameSize + 8], xmm6);
		if (mmRegisters > 7) vmovaps(ptr[rsp + this->frameSize + 24], xmm7);
		// Store xmm8 and up in allocated stack
		for (int i = 8; i < mmRegisters; ++i)
		{
			vmovaps(ptr[rsp + stack + (i - 6) * 0x10], regXmm(i));
		}

		for (int i = 4; i < nArguments; ++i)
		{
			mov(arg64(i), ptr[rsp + this->frameSize + 8 * (i + 1)]);
		}

#else
		// Registers RBP, RBX, and R12-R15 are callee-save registers; all others must be saved by the caller if it wishes to preserve their values.
		for (int i = 6; i < nArguments; ++i)
		{
			mov(arg64(i), ptr[rsp + 8 * (i - 5)]);
		}
#endif
	}

	void epilogue(int variables, int mmRegisters, int stack)
	{
#ifdef WIN32
		if (mmRegisters > 6) vmovaps(xmm6, ptr[rsp + this->frameSize + 8]);
		if (mmRegisters > 7) vmovaps(xmm7, ptr[rsp + this->frameSize + 24]);
		for (int i = 8; i < mmRegisters; ++i)
		{
			vmovaps(regXmm(i), ptr[rsp + stack + (i - 6) * 0x10]);
		}

		if (mmRegisters > 8 || stack)
		{
			add(rsp, this->stackOffset);
		}

		int registers = nArguments + variables;
		for (int i = registers - 1; i >= 7; --i)
		{
			pop(reg64(i));
		}
#else
#endif
		ret();
	}

	//virtual void appendix() { }

	Xbyak::Reg64 const &reg64(int i)
	{
		if (pass) this->variables = std::max(this->variables, i - nArguments + 1);
#ifdef WIN32
		Xbyak::Reg64 const *lookup[15] = { &rcx, &rdx, &r8, &r9, &r10, &r11, &rax, &rdi, &rsi, &rbx, &rbp, &r12, &r13, &r14, &r15 };
#else
		Xbyak::Reg64 const *lookup[15] = { &rdi, &rsi, &rdx, &rcx, &r8, &r9, &rax, &r10, &r11, &rbx, &rbp, &r12, &r13, &r14, &r15 };
#endif
		return *lookup[i];
	}

	Xbyak::Reg64 const &arg64(int i)
	{
		assert(i < nArguments);
		return reg64(i);
	}

	Xbyak::Xmm const &regXmm(int i)
	{
		if (pass) this->mmRegisters = std::max(this->mmRegisters, i + 1);
		Xbyak::Xmm const *lookup[] = { &xmm0, &xmm1, &xmm2, &xmm3, &xmm4, &xmm5, &xmm6, &xmm7, &xmm8, &xmm9, &xmm10, &xmm11, &xmm12, &xmm13, &xmm14, &xmm15 };
		return *lookup[i];
	}

	template <class T> void db(std::initializer_list<T> args, int times = 1)
	{
		for (int i = 0; i < times; ++i)
			for (auto arg : args)
				Xbyak::CodeGenerator::db(arg);
	}

	template <class T> void dw(std::initializer_list<T> args, int times = 1)
	{
		for (int i = 0; i < times; ++i)
			for (auto arg : args)
				Xbyak::CodeGenerator::dw(arg);
	}

	template <class T> void dd(std::initializer_list<T> args, int times = 1)
	{
		for (int i = 0; i < times; ++i)
			for (auto arg : args)
				Xbyak::CodeGenerator::dd(arg);
	}

	template <class T> void dq(std::initializer_list<T> args, int times = 1)
	{
		for (int i = 0; i < times; ++i)
			for (auto arg : args)
				Xbyak::CodeGenerator::dq(arg);
	}


	bool debug = false;

	hevcasm_instruction_set isa() const
	{
		return this->buffer->isa;
	}

private:
	Buffer *buffer;
	int increment = 0;
	int variables = 0;
	int mmRegisters = 0;
};

}

#define ORDER(a, b, c, d) ((a << 6) | (b << 4) | (c << 2) | d)


//
//namespace Jit2 {
//
//template <class T>
//struct Function;
//
//struct Buffer
//{
//	Buffer(hevcasm_instruction_set isa, size_t size) { }
//};
//
//
//template <typename T>
//struct Sad
//{
//	Sad(int bitDepth=8*sizeof(T), int width=0, int height=0) { }
//	typedef int Type(T *p, ptrdiff_t stride);
//	Type *make(Buffer &buffer)
//	{
//	}
//};
//
//
//struct Test
//{
//	Test()
//	{
//		Buffer buffer(HEVCASM_AVX2, 2000);
//		Sad<uint16_t>::Type *function = Sad<uint16_t>(10, 32, 32).make(buffer);
//	}
//};
//
//static Test test;
//
//}