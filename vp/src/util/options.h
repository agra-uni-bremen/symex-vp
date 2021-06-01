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

#include <functional>
#include <stdexcept>
#include <string>

template <typename T>
struct OptionValue {
	bool available = false;
	T value{};
	std::string option;

	bool finalize(std::function<T(const std::string &)> parser) {
		if (!option.empty()) {
			value = parser(option);
			available = true;
		}
		return available;
	}
};

unsigned long parse_ulong_option(const std::string &s) {
	bool is_hex = false;
	if (s.size() >= 2) {
		if ((s[0] == '0') && ((s[1] == 'x') || (s[1] == 'X')))
			is_hex = true;
	}

	try {
		if (is_hex)
			return stoul(s, 0, 16);
		return stoul(s);
	} catch (std::exception &e) {
		throw std::runtime_error(std::string("unable to parse option '") + s + "' into a number");
	}
}