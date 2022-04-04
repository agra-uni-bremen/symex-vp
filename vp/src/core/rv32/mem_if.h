/*
 * Copyright (c) 2020,2021 Group of Computer Architecture, University of Bremen
 *
 *  This file is free software: you may copy, redistribute and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  This file is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and permission notice:
 *
 *  Copyright (c) 2017-2018 Group of Computer Architecture, University of Bremen <riscv@systemc-verification.org>
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#pragma once

#include <stdint.h>
#include <clover/clover.h>

namespace rv32 {

struct instr_memory_if {
	virtual ~instr_memory_if() {}

	virtual uint32_t load_instr(uint64_t pc) = 0;
};

//NOTE: load/store double is used for floating point D extension
struct data_memory_if {
	virtual ~data_memory_if() {}

#if 0
    virtual int64_t load_double(uint64_t addr) = 0;
	virtual int32_t load_word(uint64_t addr) = 0;
	virtual int32_t load_half(uint64_t addr) = 0;
	virtual int32_t load_byte(uint64_t addr) = 0;
	virtual uint32_t load_uhalf(uint64_t addr) = 0;
	virtual uint32_t load_ubyte(uint64_t addr) = 0;

    virtual void store_double(uint64_t addr, uint64_t value) = 0;
	virtual void store_word(uint64_t addr, uint32_t value) = 0;
	virtual void store_half(uint64_t addr, uint16_t value) = 0;
	virtual void store_byte(uint64_t addr, uint8_t value) = 0;

	virtual int32_t atomic_load_word(uint64_t addr) = 0;
	virtual void atomic_store_word(uint64_t addr, uint32_t value) = 0;
	virtual int32_t atomic_load_reserved_word(uint64_t addr) = 0;
	virtual bool atomic_store_conditional_word(uint64_t addr, uint32_t value) = 0;
	virtual void atomic_unlock() = 0;
#endif

	typedef std::shared_ptr<clover::ConcolicValue> Concolic;

	virtual void symbolic_store_data(Concolic addr, Concolic data, size_t num_bytes) = 0;
	virtual Concolic symbolic_load_data(Concolic addr, size_t num_bytes) = 0;

	virtual Concolic load_word(Concolic addr) = 0;
	virtual Concolic load_half(Concolic addr) = 0;
	virtual Concolic load_byte(Concolic addr) = 0;
	virtual Concolic load_uhalf(Concolic addr) = 0;
	virtual Concolic load_ubyte(Concolic addr) = 0;

	virtual void store_word(Concolic addr, Concolic value) = 0;
	virtual void store_half(Concolic addr, Concolic value) = 0;
	virtual void store_byte(Concolic addr, Concolic value) = 0;

	virtual Concolic atomic_load_word(Concolic addr) = 0;
	virtual void atomic_store_word(Concolic addr, Concolic value) = 0;
	virtual Concolic atomic_load_reserved_word(Concolic addr) = 0;
	virtual bool atomic_store_conditional_word(Concolic addr, Concolic value) = 0;
	virtual void atomic_unlock() = 0;

    virtual void flush_tlb() = 0;
};

}  // namespace rv32
