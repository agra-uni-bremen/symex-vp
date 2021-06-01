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

#ifndef RISCV_ISA_IRQ_IF_H
#define RISCV_ISA_IRQ_IF_H

#include <stdint.h>

typedef uint32_t PrivilegeLevel;

constexpr uint32_t MachineMode = 0b11;
constexpr uint32_t HypervisorMode = 0b10;
constexpr uint32_t SupervisorMode = 0b01;
constexpr uint32_t UserMode = 0b00;
constexpr uint32_t NoneMode = -1;  // invalid sentinel to avoid passing a boolean alongside a privilege level

struct external_interrupt_target {
	virtual ~external_interrupt_target() {}

	virtual void trigger_external_interrupt(PrivilegeLevel level) = 0;
	virtual void clear_external_interrupt(PrivilegeLevel level) = 0;
};

struct clint_interrupt_target {
	virtual ~clint_interrupt_target() {}

	virtual void trigger_timer_interrupt(bool status) = 0;
	virtual void trigger_software_interrupt(bool status) = 0;
};

struct interrupt_gateway {
	virtual ~interrupt_gateway() {}

	virtual void gateway_trigger_interrupt(uint32_t irq_id) = 0;
};

#endif  // RISCV_ISA_IRQ_IF_H
