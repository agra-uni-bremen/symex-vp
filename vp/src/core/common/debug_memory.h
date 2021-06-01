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

#ifndef RISCV_ISA_DEBUG_MEMORY_H
#define RISCV_ISA_DEBUG_MEMORY_H

#include <string>
#include <type_traits>

#include <tlm_utils/simple_initiator_socket.h>
#include <systemc>

#include "core_defs.h"
#include "trap.h"

struct DebugMemoryInterface : public sc_core::sc_module {
	tlm_utils::simple_initiator_socket<DebugMemoryInterface> isock;

	DebugMemoryInterface(sc_core::sc_module_name) {}

	unsigned _do_dbg_transaction(tlm::tlm_command cmd, uint64_t addr, uint8_t *data, unsigned num_bytes);

	std::string read_memory(uint64_t start, unsigned nbytes);

	void write_memory(uint64_t start, unsigned nbytes, const std::string &data);
};

#endif  // RISCV_ISA_GDB_STUB_H
