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

#ifndef RISCV_VP_SLIP_H
#define RISCV_VP_SLIP_H

#include <stdint.h>
#include <systemc>
#include "abstract_uart.h"

class SLIP : public AbstractUART {
public:
	SLIP(const sc_core::sc_module_name &, uint32_t, std::string);
	~SLIP(void);

private:
	int get_mtu(const char *);
	void send_packet(void);
	void write_data(uint8_t) override;
	void handle_input(int fd) override;

	int tunfd;

	uint8_t *sndbuf = NULL, *rcvbuf = NULL;
	size_t sndsiz, rcvsiz;

};

#endif  // RISCV_VP_SLIP_H
