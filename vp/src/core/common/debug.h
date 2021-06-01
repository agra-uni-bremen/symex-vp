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

#ifndef RISCV_DEBUG
#define RISCV_DEBUG

#include <stdint.h>

#include <unordered_set>
#include <vector>

#include "core_defs.h"


struct debug_target_if {
	virtual ~debug_target_if() {}

	virtual void enable_debug(void) = 0;
	virtual CoreExecStatus get_status(void) = 0;
	virtual void set_status(CoreExecStatus) = 0;
	virtual void block_on_wfi(bool) = 0;

	virtual void insert_breakpoint(uint64_t) = 0;
	virtual void remove_breakpoint(uint64_t) = 0;

	virtual Architecture get_architecture(void) = 0;
	virtual uint64_t get_hart_id(void) = 0;

	virtual uint64_t get_progam_counter(void) = 0;
	virtual std::vector<uint64_t> get_registers(void) = 0;
	virtual uint64_t read_register(unsigned) = 0;

	virtual void run(void) = 0;
	virtual void run_step(void) = 0;
};

#endif
