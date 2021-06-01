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

#ifndef RISCV_VP_UART_H
#define RISCV_VP_UART_H

#include <stdint.h>
#include <systemc>
#include "abstract_uart.h"

class UART : public AbstractUART {
public:
	UART(const sc_core::sc_module_name&, uint32_t);
	~UART(void);

private:
	typedef enum {
		STATE_COMMAND,
		STATE_NORMAL,
	} uart_state;

	/**
	 * State of the input handling state machine. In normal mode
	 * (STATE_NORMAL) the next input character is forwarded to the
	 * guest. In command mode (STATE_COMMAND) the next input
	 * character is interpreted by ::handle_cmd.
	 */
	uart_state state = STATE_NORMAL;
	void handle_cmd(uint8_t);

	void handle_input(int fd) override;
	void write_data(uint8_t) override;
};

#endif  // RISCV_VP_UART_H
