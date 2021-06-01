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

/*****************************************************************************

  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
  more contributor license agreements.  See the NOTICE file distributed
  with this work for additional information regarding copyright ownership.
  Accellera licenses this file to you under the Apache License, Version 2.0
  (the "License"); you may not use this file except in compliance with the
  License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
  implied.  See the License for the specific language governing
  permissions and limitations under the License.

 *****************************************************************************/
/*
 * thread_safe_event.h
 *
 * Copyright (C) 2017, GreenSocs Ltd.
 *
 * Developed by Mark Burton mark@greensocs.com
 */

#pragma once
#include <systemc>

class AsyncEvent : public sc_core::sc_prim_channel {
	sc_core::sc_time m_delay;
	sc_core::sc_event m_event;

   public:
	AsyncEvent(const char* name = sc_core::sc_gen_unique_name("async_event"))
	    : sc_core::sc_prim_channel(name), m_event((std::string(this->basename()) + "_event").c_str()) {
		// register this channel as "suspending", to not end the simulation
		// when we're running out of internal events
		async_attach_suspending();
	}

	// THREADSAFE METHOD:
	void notify(sc_core::sc_time delay = sc_core::SC_ZERO_TIME) {
		m_delay = delay;
		async_request_update();
	}

	// only allow waiting for the event
	operator const sc_core::sc_event&() const {
		return m_event;
	}

   protected:
	void update(void) {
		// we're in the update phase of the SystemC kernel (thread)
		m_event.notify(m_delay);
	}
};
