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

enum ExceptionCode {
	// interrupt exception codes (mcause)
	EXC_U_SOFTWARE_INTERRUPT = 0,
	EXC_S_SOFTWARE_INTERRUPT = 1,
	EXC_M_SOFTWARE_INTERRUPT = 3,

	EXC_U_TIMER_INTERRUPT = 4,
	EXC_S_TIMER_INTERRUPT = 5,
	EXC_M_TIMER_INTERRUPT = 7,

	EXC_U_EXTERNAL_INTERRUPT = 8,
	EXC_S_EXTERNAL_INTERRUPT = 9,
	EXC_M_EXTERNAL_INTERRUPT = 11,

	// non-interrupt exception codes (mcause)
	EXC_INSTR_ADDR_MISALIGNED = 0,
	EXC_INSTR_ACCESS_FAULT = 1,
	EXC_ILLEGAL_INSTR = 2,
	EXC_BREAKPOINT = 3,
	EXC_LOAD_ADDR_MISALIGNED = 4,
	EXC_LOAD_ACCESS_FAULT = 5,
	EXC_STORE_AMO_ADDR_MISALIGNED = 6,
	EXC_STORE_AMO_ACCESS_FAULT = 7,

	EXC_ECALL_U_MODE = 8,
	EXC_ECALL_S_MODE = 9,
	EXC_ECALL_M_MODE = 11,

	EXC_INSTR_PAGE_FAULT = 12,
	EXC_LOAD_PAGE_FAULT = 13,
	EXC_STORE_AMO_PAGE_FAULT = 15,
};

struct SimulationTrap {
	ExceptionCode reason;
	unsigned long mtval;
};

inline void raise_trap(ExceptionCode exc, unsigned long mtval) {
	throw SimulationTrap({exc, mtval});
}
