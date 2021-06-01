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

/*
 * gpio-server.cpp
 *
 *  Created on: 5 Nov 2018
 *      Author: dwd
 */

#include <unistd.h>
#include <csignal>
#include <functional>
#include <iostream>
#include <thread>

#include "gpio-server.hpp"

using namespace std;

bool stop = false;

void signalHandler(int signum) {
	cout << "Interrupt signal (" << signum << ") received.\n";

	if (stop)
		exit(signum);
	stop = true;
	raise(SIGUSR1);  // this breaks wait in thread
}

void onChangeCallback(GpioServer* gpio, uint8_t bit, GpioCommon::Tristate val) {
	if (((gpio->state & (1l << bit)) >> bit) == val) {
		printf("Bit %d still at %d\n", bit, val);
		return;
	}
	if (val == 0) {
		gpio->state &= ~(1l << bit);
	} else if (val == 1) {
		gpio->state |= 1l << bit;
	} else {
		printf("Ignoring tristate for now\n");
		return;
	}
	printf("Bit %d changed to %d\n", bit, val);
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		cout << "usage: " << argv[0] << " port (e.g. 1339)" << endl;
		exit(-1);
	}

	GpioServer gpio;

	if (!gpio.setupConnection(argv[1])) {
		cerr << "cant set up server" << endl;
		exit(-1);
	}

	signal(SIGINT, signalHandler);

	gpio.registerOnChange(bind(onChangeCallback, &gpio, placeholders::_1, placeholders::_2));
	thread server(bind(&GpioServer::startListening, &gpio));

	while (!stop && !gpio.isStopped()) {
		usleep(100000);
		if (!(gpio.state & (1 << 11))) {
			gpio.state <<= 1;
			if (!(gpio.state & 0xFF)) {
				gpio.state = 1;
			}
		}
	}
	gpio.quit();
	server.join();
}
