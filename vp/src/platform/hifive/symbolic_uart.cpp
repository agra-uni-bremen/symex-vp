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

#include "symbolic_uart.h"

#include <err.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#define UART_TXWM (1 << 0)
#define UART_RXWM (1 << 1)
#define UART_FULL (1 << 31)

/* Extracts the interrupt trigger threshold from a control register */
#define UART_CTRL_CNT(REG) ((REG) >> 16)

enum {
	TXDATA_REG_ADDR = 0x0,
	RXDATA_REG_ADDR = 0x4,
	TXCTRL_REG_ADDR = 0x8,
	RXCTRL_REG_ADDR = 0xC,
	IE_REG_ADDR = 0x10,
	IP_REG_ADDR = 0x14,
	DIV_REG_ADDR = 0x18,
};

SymbolicUART::SymbolicUART(sc_core::sc_module_name, uint32_t irqsrc, SymbolicContext &_ctx, SymbolicFormat &_fmt)
  : solver(_ctx.solver), ctx(_ctx.ctx), fmt(_fmt) {
	irq = irqsrc;
	tsock.register_b_transport(this, &SymbolicUART::transport);

	std::shared_ptr<clover::ConcolicValue> v;
	while ((v = fmt.next_byte()))
		rx_fifo.push(v);

	router
	    .add_register_bank({
		{TXDATA_REG_ADDR, &txdata},
		{RXDATA_REG_ADDR, &rxdata},
		{TXCTRL_REG_ADDR, &txctrl},
		{RXCTRL_REG_ADDR, &rxctrl},
		{IE_REG_ADDR, &ie},
		{IP_REG_ADDR, &ip},
		{DIV_REG_ADDR, &div},
	    })
	    .register_handler(this, &SymbolicUART::register_access_callback);

	SC_METHOD(interrupt);
	sensitive << asyncEvent;
	dont_initialize();
}

SymbolicUART::~SymbolicUART(void) {
	return;
}

void SymbolicUART::register_access_callback(const vp::map::register_access_t &r) {
	if (r.read) {
		if (r.vptr == &txdata) {
			txdata = 0; // Pretend to be always ready to transmit
		} else if (r.vptr == &rxdata) {
			// Check if RX interrupt is enabled since many
			// UART drivers drain rxdata during initialization.
			if (rx_fifo.empty() || !(ie & UART_RXWM)) {
				rxdata = 1 << 31;
			} else {
				auto symbolic_byte = rx_fifo.front();
				rx_fifo.pop();

				auto reg = symbolic_byte->zext(32);
				rxdata = solver.getValue<uint32_t>(reg->concrete);

				auto ext = new SymbolicExtension(reg);
				r.trans.set_extension(ext);
			}
		} else if (r.vptr == &ip) {
			uint32_t ret = UART_TXWM; // Transmit always ready
			if (rx_fifo.size() > UART_CTRL_CNT(rxctrl))
				ret |= UART_RXWM;
			ip = ret;
		} else if (r.vptr == &ie) {
			// do nothing
		} else if (r.vptr == &div) {
			// just return the last set value
		} else {
			std::cerr << "invalid offset for UART " << std::endl;
		}
	}

	bool notify = false;
	if (r.write) {
		if (r.vptr == &txctrl && UART_CTRL_CNT(r.nv) < UART_CTRL_CNT(txctrl))
			notify = true;
		else if (r.vptr == &rxctrl && UART_CTRL_CNT(r.nv) < UART_CTRL_CNT(rxctrl))
			notify = true;
	}

	// We are always ready to transmit thus, if TX_WM is enabled
	// signal a transmit interrupt.
	if (ie & UART_TXWM)
		notify = true;

	r.fn();

	// If the interrupt has just been enabled (i.e. IE register was
	// modified) then delay raising of the interrupt by a few ms.
	// This is necessary as RIOT uses the same stack for
	// initialization and interrupt handling. If the interrupt is
	// received directly after enabling it, then the initialization
	// hasn't finished yet and the interrupt handler will overwrite
	// its stack space.
	if (r.write && r.vptr == &ie)
		asyncEvent.notify(sc_core::sc_time(100, sc_core::SC_MS));
	else if (notify)
		asyncEvent.notify();
}

void SymbolicUART::transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay) {
	router.transport(trans, delay);
}

void SymbolicUART::interrupt(void) {
	bool trigger = false;
	if ((ie & UART_RXWM) && rx_fifo.size() > UART_CTRL_CNT(rxctrl))
		trigger = true;
	if (ie & UART_TXWM)
		trigger = true;

	if (trigger)
		plic->gateway_trigger_interrupt(irq);
}
