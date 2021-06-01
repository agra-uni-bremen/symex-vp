/*
 * Copyright (c) 2017-2018 Group of Computer Architecture, University of Bremen <riscv@systemc-verification.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <stdint.h>

class MemoryDMI {
	uint8_t *mem;
	uint64_t start;
	uint64_t size;
	uint64_t end;

	MemoryDMI(uint8_t *mem, uint64_t start, uint64_t size) : mem(mem), start(start), size(size), end(start + size) {}

   public:
	static MemoryDMI create_start_end_mapping(uint8_t *mem, uint64_t start, uint64_t end) {
		assert(end > start);
		return create_start_size_mapping(mem, start, end - start);
	}

	static MemoryDMI create_start_size_mapping(uint8_t *mem, uint64_t start, uint64_t size) {
		assert(start + size > start);
		return MemoryDMI(mem, start, size);
	}

	uint8_t *get_raw_mem_ptr() {
		return mem;
	}

	template <typename T>
	T *get_mem_ptr_to_global_addr(uint64_t addr) {
		assert(contains(addr));
		assert((addr + sizeof(T)) <= end);
		// assert ((addr % sizeof(T)) == 0 && "unaligned access");   //NOTE: due to compressed instructions, fetching
		// can be unaligned
		return reinterpret_cast<T *>(mem + (addr - start));
	}

	template <typename T>
	T load(uint64_t addr) {
		static_assert(std::is_integral<T>::value, "integer type required");
		T ans;
		T *src = get_mem_ptr_to_global_addr<T>(addr);
		// use memcpy to avoid problems with unaligned loads into standard C++ data types
		// see: https://blog.quarkslab.com/unaligned-accesses-in-cc-what-why-and-solutions-to-do-it-properly.html
		memcpy(&ans, src, sizeof(T));
		return ans;
	}

	template <typename T>
	void store(uint64_t addr, T value) {
		static_assert(std::is_integral<T>::value, "integer type required");
		T *dst = get_mem_ptr_to_global_addr<T>(addr);
		memcpy(dst, &value, sizeof(value));
	}

	uint64_t get_start() {
		return start;
	}

	uint64_t get_end() {
		return start + size;
	}

	uint64_t get_size() {
		return size;
	}

	bool contains(uint64_t addr) {
		return addr >= start && addr < end;
	}
};