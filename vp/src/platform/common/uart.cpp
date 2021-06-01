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

#include "uart.h"
#include "core/common/rawmode.h"

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <systemc>

#include <sys/types.h>

/* character → control key */
#define CTRL(c) ((c) & 0x1f)

#define KEY_ESC  CTRL('a') /* Ctrl-a (character to enter command mode) */
#define KEY_EXIT CTRL('x') /* Ctrl-x (character to exit in command mode) */

UART::UART(const sc_core::sc_module_name& name, uint32_t irqsrc)
		: AbstractUART(name, irqsrc) {
	enableRawMode(STDIN_FILENO);
	start_threads(STDIN_FILENO);
}

UART::~UART(void) {
	disableRawMode(STDIN_FILENO);
}

void UART::handle_input(int fd) {
	uint8_t buf;
	ssize_t nread;

	nread = read(fd, &buf, sizeof(buf));
	if (nread == -1)
		throw std::system_error(errno, std::generic_category());
	else if (nread != sizeof(buf))
		throw std::runtime_error("short read");

	switch (state) {
	case STATE_NORMAL:
		rxpush(buf);
		break;
	case STATE_COMMAND:
		handle_cmd(buf);
		break;
	}

	/* update state of input state machine for next run */
	if (buf == KEY_ESC && state != STATE_COMMAND) {
		state = STATE_COMMAND;
	} else {
		state = STATE_NORMAL;
	}
}

void UART::handle_cmd(uint8_t cmd) {
	switch (cmd) {
	case KEY_ESC: /* double escape */
		rxpush(cmd);
		break;
	case KEY_EXIT:
		exit(EXIT_SUCCESS);
		break;
	default:
		return; /* unknown command → ignore */
	}
}

void UART::write_data(uint8_t data) {
	ssize_t nwritten;

	nwritten = write(STDOUT_FILENO, &data, sizeof(data));
	if (nwritten == -1)
		throw std::system_error(errno, std::generic_category());
	else if (nwritten != sizeof(data))
		throw std::runtime_error("short write");
}
