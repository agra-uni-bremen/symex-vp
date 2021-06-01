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

#include <dlfcn.h>
#include <boost/io/ios_state.hpp>
#include <iomanip>

std::string RISCV_TIMING_SIM_LIB = "riscv-timing-sim.so";
std::string RISCV_TIMING_DB = "riscv-timing-db.xml";

// NOTE: the interface inside the library has to match this one exactly, don't
// forget the *virtual* attribute
struct SimTimingInterface {
	virtual uint64_t get_cycles_for_instruction(uint64_t pc);

	// only to detect errors in loading the shared library
	virtual uint64_t get_magic_number();
};

struct ExternalTimingDecorator : public timing_if {
	SimTimingInterface *timing_sim = 0;
	void *lib_handle = 0;
	SimTimingInterface *(*create)(const char *) = 0;
	void (*destroy)(SimTimingInterface *) = 0;

	void initialize() {
		lib_handle = dlopen(RISCV_TIMING_SIM_LIB.c_str(), RTLD_LAZY);
		if (!lib_handle)
			throw std::runtime_error("unable to open shared library '" + RISCV_TIMING_SIM_LIB + "'");

		create = (SimTimingInterface * (*)(const char *)) dlsym(lib_handle, "create_riscv_vp_timing_interface");
		if (!create)
			throw std::runtime_error("unable to load 'create_riscv_vp_timing_interface' function");

		destroy = (void (*)(SimTimingInterface *))dlsym(lib_handle, "destroy_riscv_vp_timing_interface");
		if (!destroy)
			throw std::runtime_error("unable to load 'destroy_riscv_vp_timing_interface' function");

		timing_sim = (SimTimingInterface *)create(RISCV_TIMING_DB.c_str());
	}

	ExternalTimingDecorator() {
		initialize();
	}

	~ExternalTimingDecorator() {
		assert(timing_sim != 0);
		destroy(timing_sim);
		assert(lib_handle != 0);
		dlclose(lib_handle);
	}

	void on_begin_exec_step(Instruction instr, Opcode::mapping op, ISS &iss) override {
		uint64_t cycles = timing_sim->get_cycles_for_instruction(iss.last_pc);

		assert(timing_sim->get_magic_number() == 0x5E5E5E5E5E5E5E5E);

		sc_core::sc_time delay = iss.cycle_time * cycles;

		iss.quantum_keeper.inc(delay);
	}
};
