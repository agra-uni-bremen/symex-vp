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

#ifndef RISCV_VP_REAL_CLINT_H
#define RISCV_VP_REAL_CLINT_H

#include <stdint.h>

#include <chrono>
#include <systemc>
#include <map>
#include <tlm_utils/simple_target_socket.h>

#include "platform/common/async_event.h"
#include "util/memory_map.h"
#include "clint_if.h"
#include "irq_if.h"
#include "timer.h"

// This class implements a CLINT as specified in the FE310-G000 manual
// and the RISC-V Privileged Specification. As per the FE310-G000
// manual, this CLINT uses an input clock which runs at a frequency of
// 32.768 kHz. Currently, this frequency cannot be configured.
//
// Contrary to the CLINT class, also provided in this directory, this
// CLINT is based on "real time" instead of SystemC simulation time.
class RealCLINT : public clint_if, public sc_core::sc_module {
public:
	RealCLINT(sc_core::sc_module_name, std::vector<clint_interrupt_target*>&);
	~RealCLINT(void);

	tlm_utils::simple_target_socket<RealCLINT> tsock;
	uint64_t update_and_get_mtime(void) override;

	SC_HAS_PROCESS(RealCLINT);
public:
	typedef std::chrono::high_resolution_clock::time_point time_point;
	typedef Timer::usecs usecs;

	RegisterRange regs_msip;
	RegisterRange regs_mtimecmp;
	RegisterRange regs_mtime;

	ArrayView<uint32_t> msip;
	ArrayView<uint64_t> mtimecmp;
	IntegerView<uint64_t> mtime;

	std::vector<RegisterRange*> register_ranges;
	std::vector<clint_interrupt_target*> &harts;

	AsyncEvent event;
	std::vector<Timer*> timers;

	time_point first_mtime;

	void post_write_mtimecmp(RegisterRange::WriteInfo info);
	void post_write_msip(RegisterRange::WriteInfo info);
	void post_write_mtime(RegisterRange::WriteInfo info);
	bool pre_read_mtime(RegisterRange::ReadInfo info);

	uint64_t usec_to_ticks(usecs usec);
	usecs ticks_to_usec(uint64_t ticks);

	void interrupt(void);
	void transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay);
};

#endif
