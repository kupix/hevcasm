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

	template <class Function>
	void assemble(Function &function)
	{
		Assembler(*this)
	}

private:
	std::vector<uint8_t> buffer;
	uint8_t *p;
};


template <class Base, class FunctionType>
struct Function
	:
	Xbyak::CodeGenerator
{
	Function(Buffer &buffer)
		:
		buffer(buffer),
		Xbyak::CodeGenerator(buffer.nRemainingBytes(), buffer.pointer())
	{
	}

	FunctionType *function() const
	{
		assert(this->committed);
		return reinterpret_cast<FunctionType *>(this->getCode());
	}

protected:

#ifdef WIN32
	Xbyak::Reg64 const &arg0() { return rcx; }
	Xbyak::Reg64 const &arg1() { return rdx; }
	Xbyak::Reg64 const &arg2() { return r8; }
	Xbyak::Reg64 const &arg3() { return r9; }
#else
	Xbyak::Reg64 const &arg0() { return rdi; }
	Xbyak::Reg64 const &arg1() { return rsi; }
	Xbyak::Reg64 const &arg2() { return rdx; }
	Xbyak::Reg64 const &arg3() { return rcx; }
#endif

	void commit()
	{
		this->buffer.increment(this->getSize());
		this->committed = true;
	}

private:
	Buffer &buffer;
	bool committed = false;
	int increment = 0;
};

}
