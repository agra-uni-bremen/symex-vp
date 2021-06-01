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
 * cli-client.cpp
 *
 *  Created on: 7 Nov 2018
 *      Author: dwd
 */

#include <unistd.h>
#include <iostream>

#include "gpio-client.hpp"

using namespace std;

int main(int argc, char* argv[]) {
	if (argc < 3) {
		cout << "usage: " << argv[0] << " host port (e.g. localhost 1339)" << endl;
		exit(-1);
	}

	GpioClient gpio;

	if (!gpio.setupConnection(argv[1], argv[2])) {
		cout << "cant setup connection" << endl;
		return -1;
	}

	while (true) {
		if (!gpio.update()) {
			cerr << "Error updating" << endl;
			return -1;
		}
		bitPrint(reinterpret_cast<unsigned char*>(&gpio.state), sizeof(GpioCommon::Reg));
		usleep(125000);
	}

	for (uint8_t i = 0; i < 64; i++) {
		if (!gpio.setBit(i, 1)) {
			cerr << "Error setting Bit " << i << endl;
			return -1;
		}
		if (!gpio.update()) {
			cerr << "Error updating" << endl;
			return -1;
		}
		bitPrint(reinterpret_cast<unsigned char*>(&gpio.state), sizeof(GpioCommon::Reg));
		usleep(750);
	}

	for (uint8_t i = 0; i < 64; i++) {
		if (!gpio.setBit(i, 0)) {
			cerr << "Error resetting Bit " << i << endl;
			return -1;
		}
		if (!gpio.update()) {
			cerr << "Error updating" << endl;
			return -1;
		}
		bitPrint(reinterpret_cast<unsigned char*>(&gpio.state), sizeof(GpioCommon::Reg));
		usleep(750);
	}
}
