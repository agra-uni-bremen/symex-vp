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

#include "platform/common/async_event.h"
#include "core/common/irq_if.h"
#include "gpio/gpio-server.hpp"
#include "util/tlm_map.h"

#include <tlm_utils/simple_target_socket.h>
#include <systemc>
#include <thread>

struct GPIO : public sc_core::sc_module {
	tlm_utils::simple_target_socket<GPIO> tsock;

	// memory mapped configuration registers
	uint32_t value = 0;
	uint32_t input_en = 0;
	uint32_t output_en = 0;
	uint32_t port = 0;
	uint32_t pullup_en = 0;
	uint32_t pin_drive_strength = 0;
	uint32_t rise_intr_en = 0;
	uint32_t rise_intr_pending = ~0l;
	uint32_t fall_intr_en = 0;
	uint32_t fall_intr_pending = ~0l;
	uint32_t high_intr_en = 0;
	uint32_t high_intr_pending = ~0l;
	uint32_t low_intr_en = 0;
	uint32_t low_intr_pending = ~0l;
	uint32_t iof_en = 0;
	uint32_t iof_sel = 0;
	uint32_t out_xor = 0;

	enum {
		PIN_VALUE_ADDR = 0x000,
		INPUT_EN_REG_ADDR = 0x004,
		OUTPUT_EN_REG_ADDR = 0x008,
		PORT_REG_ADDR = 0x00C,
		PULLUP_EN_ADDR = 0x010,
		PIN_DRIVE_STNGTH = 0x014,
		RISE_INTR_EN = 0x018,
		RISE_INTR_PEND = 0x01C,
		FALL_INTR_EN = 0x020,
		FALL_INTR_PEND = 0x024,
		HIGH_INTR_EN = 0x028,
		HIGH_INTR_PEND = 0x02C,
		LOW_INTR_EN = 0x030,
		LOW_INTR_PEND = 0x034,
		IOF_EN_REG_ADDR = 0x038,
		IOF_SEL_REG_ADDR = 0x03C,
		OUT_XOR_REG_ADDR = 0x040,
	};

	vp::map::LocalRouter router = {"GPIO"};
	interrupt_gateway *plic = nullptr;

	const unsigned int_gpio_base;
	GpioServer server;
	std::thread *serverThread;
	AsyncEvent asyncEvent;

	SC_HAS_PROCESS(GPIO);
	GPIO(sc_core::sc_module_name, unsigned int_gpio_base);
	~GPIO();

	void register_access_callback(const vp::map::register_access_t &r);

	void transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay);

	void asyncOnchange(uint8_t bit, GpioCommon::Tristate val);
	void synchronousChange();
};
