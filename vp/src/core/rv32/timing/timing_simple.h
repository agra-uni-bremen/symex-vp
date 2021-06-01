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

#include "../iss.h"

struct SimpleTimingDecorator : public timing_if {
	std::array<sc_core::sc_time, Opcode::NUMBER_OF_INSTRUCTIONS> instr_cycles;
	sc_core::sc_time cycle_time = sc_core::sc_time(10, sc_core::SC_NS);

	SimpleTimingDecorator() {
		for (int i = 0; i < Opcode::NUMBER_OF_INSTRUCTIONS; ++i) instr_cycles[i] = cycle_time;

		const sc_core::sc_time memory_access_cycles = 4 * cycle_time;
		const sc_core::sc_time mul_div_cycles = 8 * cycle_time;

		instr_cycles[Opcode::LB] = memory_access_cycles;
		instr_cycles[Opcode::LBU] = memory_access_cycles;
		instr_cycles[Opcode::LH] = memory_access_cycles;
		instr_cycles[Opcode::LHU] = memory_access_cycles;
		instr_cycles[Opcode::LW] = memory_access_cycles;
		instr_cycles[Opcode::SB] = memory_access_cycles;
		instr_cycles[Opcode::SH] = memory_access_cycles;
		instr_cycles[Opcode::SW] = memory_access_cycles;
		instr_cycles[Opcode::MUL] = mul_div_cycles;
		instr_cycles[Opcode::MULH] = mul_div_cycles;
		instr_cycles[Opcode::MULHU] = mul_div_cycles;
		instr_cycles[Opcode::MULHSU] = mul_div_cycles;
		instr_cycles[Opcode::DIV] = mul_div_cycles;
		instr_cycles[Opcode::DIVU] = mul_div_cycles;
		instr_cycles[Opcode::REM] = mul_div_cycles;
		instr_cycles[Opcode::REMU] = mul_div_cycles;
	}

	void on_begin_exec_step(Instruction instr, Opcode::mapping op, ISS &iss) override {
		auto new_cycles = instr_cycles[op];

		iss.quantum_keeper.inc(new_cycles);
	}
};
