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

#include "xbyak/xbyak.h"
#include <vector>
#include <cstdint>
#include <cstddef>
#include <cassert>


namespace Jit {

struct Buffer
{
	Buffer(size_t n)
		:
		buffer(n)
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


template <class Derived, class FunctionType>
struct Function
	:
	Xbyak::CodeGenerator
{
	Function(Buffer &buffer)
		:
		buffer(&buffer),
		Xbyak::CodeGenerator(buffer.nRemainingBytes(), buffer.pointer())
	{
	}

	template<typename ... Types>
	Function(Function *secondPass, Types ... config)
		:
		buffer(0)
	{
		static_cast<Derived *>(this)->assemble(config...);
	}

	template<typename ... Types>
	Function(Buffer &buffer, Types ... config)
		:
		buffer(&buffer),
		Xbyak::CodeGenerator(buffer.nRemainingBytes(), buffer.pointer())
	{
		Derived firstPass(this, config...);
		if (firstPass.getSize() == 0) return;

		this->prologue(firstPass.variables, firstPass.mmRegisters);
		static_cast<Derived *>(this)->assemble(config...);
		this->epilogue(firstPass.variables, firstPass.mmRegisters);
		
		this->buffer->increment(this->getSize());
	}

	FunctionType *function() const
	{
		if (this->getSize() == 0) return 0;

		return reinterpret_cast<FunctionType *>(this->getCode());
	}

protected:
	int stackOffset;

	void prologue(int variables, int mmRegisters)
	{
#ifdef WIN32
		for (int i = 4; i < CountArguments<FunctionType>::value; ++i)
		{
			mov(arg64(i), ptr[rsp + 8 + 8 * i]);
		}

		this->stackOffset = 8;

		int registers = CountArguments<FunctionType>::value + variables;
		for (int i = 7; i < registers; ++i)
		{
			push(reg64(i));
			this->stackOffset = 8 - this->stackOffset;
		}

		if (mmRegisters >= 6)
		{
			this->stackOffset += 16 * (mmRegisters - 6);
			sub(rsp, this->stackOffset);

			for (int i = 6; i < mmRegisters; ++i)
			{
				vmovaps(ptr[rsp + (i - 6) * 16], regXmm(i));
			}
		}
#else
		for (int i = 6; i < CountArguments<FunctionType>::value; ++i)
		{
			mov(arg64(i), ptr[rsp + 8 * (i - 5)]);
		}
#endif
	}

	void epilogue(int variables, int mmRegisters)
	{
#ifdef WIN32
		if (mmRegisters >= 6)
		{
			for (int i = mmRegisters - 1; i >= 6; --i)
			{
				vmovaps(regXmm(i), ptr[rsp + (i - 6) * 16]);
			}

			add(rsp, this->stackOffset);
		}

		int registers = CountArguments<FunctionType>::value + variables;
		for (int i = registers - 1; i >= 7; --i)
		{
			pop(reg64(i));
		}
#else
#endif
		ret();
	}

	Xbyak::Reg64 const &reg64(size_t i)
	{
#ifdef WIN32
		Xbyak::Reg64 const *lookup[15] = { &rcx, &rdx, &r8, &r9, &r10, &r11, &rax, &rdi, &rsi, &rbx, &rbp, &r12, &r13, &r14, &r15 };
#else
		Xbyak::Reg64 const *lookup[15] = { &rdi, &rsi, &rdx, &rcx, &r8, &r9, &rax, &r10, &r11, &rbx, &rbp, &r12, &r13, &r14, &r15 };
#endif
		return *lookup[i];
	}

	Xbyak::Reg64 const &arg64(size_t i)
	{
		assert(i < CountArguments<FunctionType>::value);
		return reg64(i);
	}

	Xbyak::Reg64 const &var64(size_t i)
	{
		i += CountArguments<FunctionType>::value;
		return reg64(i);
	}

	Xbyak::Reg64 const &getVar64()
	{
		return reg64(CountArguments<FunctionType>::value + this->variables++);
	}

	Xbyak::Xmm const &regXmm(size_t i)
	{
		Xbyak::Xmm const *lookup[] = { &xmm0, &xmm1, &xmm2, &xmm3, &xmm4, &xmm5, &xmm6, &xmm7, &xmm8, &xmm9, &xmm10, &xmm11, &xmm12, &xmm13, &xmm14, &xmm15 };
		return *lookup[i];
	}

	Xbyak::Xmm const &getXmm()
	{
		return regXmm(this->mmRegisters++);
	}

private:
	Buffer *buffer;
	int increment = 0;
	int variables = 0;
	int mmRegisters = 0;
};

}
