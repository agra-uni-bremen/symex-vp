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

#ifndef RISCV_VP_TIMER_H
#define RISCV_VP_TIMER_H

#include <chrono>
#include <system_error>

#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>

class Timer {
public:
	typedef std::chrono::duration<uint64_t, std::micro> usecs;

	typedef void (*Callback) (void*);
	class Context {
	public:
		Timer::usecs duration;
		Callback fn;
		void *arg;

		Context(usecs _duration, Callback _fn, void *_arg)
			: duration(_duration), fn(_fn), arg(_arg) {};
	};

	Timer(Callback fn, void *arg);
	~Timer(void);

	void pause(void);
	void start(usecs duration);

private:
	Context ctx;
	pthread_t thread;
	bool running;

	void stop_thread(void);
};

#endif
