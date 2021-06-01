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

#include "gdb_runner.h"

GDBServerRunner::GDBServerRunner(sc_core::sc_module_name name, GDBServer *server, debug_target_if *hart) {
	(void)name;

	this->hart = hart;
	this->server = server;

	this->stop_event = server->get_stop_event(hart);
	server->set_run_event(hart, &this->run_event);

	hart->enable_debug();
	SC_THREAD(run);
}

void GDBServerRunner::run(void) {
	for (;;) {
		sc_core::wait(run_event);
		if (server->single_run) {
			hart->run_step();
			if (hart->get_status() == CoreExecStatus::Runnable)
				hart->set_status(CoreExecStatus::HitBreakpoint);
		} else {
			hart->run();
		}
		stop_event->notify();

		if (hart->get_status() == CoreExecStatus::Terminated)
			break;
	}
}
