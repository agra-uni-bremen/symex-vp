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

#ifndef RISCV_VP_SYMBOLIC_SLIP_H
#define RISCV_VP_SYMBOLIC_SLIP_H

#include <stdint.h>
#include <stddef.h>

#include <queue>
#include <systemc>
#include <tlm_utils/simple_target_socket.h>
#include <clover/clover.h>

#include "symbolic_format.h"
#include "symbolic_context.h"
#include "symbolic_extension.h"

#include "core/common/irq_if.h"
#include "util/tlm_map.h"
#include "platform/common/async_event.h"

class SymbolicUART : public sc_core::sc_module {
public:
	interrupt_gateway *plic;
	tlm_utils::simple_target_socket<SymbolicUART> tsock;

	SymbolicUART(sc_core::sc_module_name, uint32_t, SymbolicContext &_ctx, SymbolicFormat &_fmt);
	~SymbolicUART(void);

	SC_HAS_PROCESS(SymbolicUART);

private:
	clover::Solver &solver;
	clover::ExecutionContext &ctx;
	SymbolicFormat &fmt;

	uint32_t irq;

	// memory mapped configuration registers
	uint32_t txdata = 0;
	uint32_t rxdata = 0;
	uint32_t txctrl = 0;
	uint32_t rxctrl = 0;
	uint32_t ie = 0;
	uint32_t ip = 0;
	uint32_t div = 0;

	AsyncEvent asyncEvent;
	std::queue<std::shared_ptr<clover::ConcolicValue>> rx_fifo;

	vp::map::LocalRouter router = {"SymbolicUART"};

	void register_access_callback(const vp::map::register_access_t &);
	void transport(tlm::tlm_generic_payload &, sc_core::sc_time &);
	void interrupt(void);
};

#endif
