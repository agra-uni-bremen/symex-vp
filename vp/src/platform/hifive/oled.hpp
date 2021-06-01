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
 * oled.hpp
 *
 *  Created on: 20 Sep 2019
 *      Author: dwd
 *
 * This class models the SH1106 oled Display driver.
 */

#pragma once
#include "spi.h"
#include "oled/common.hpp"

#include <map>
#include <functional>

class SS1106 : public SpiInterface  {

	static const std::map<ss1106::Operator, uint8_t> opcode;

	struct Command
	{
		ss1106::Operator op;
		uint8_t payload;
	};

	enum class Mode : uint_fast8_t
	{
		normal,
		second_arg
	} mode = Mode::normal;

	void *sharedSegment = nullptr;
	ss1106::State* state;

	Command last_cmd = Command{ss1106::Operator::NOP, 0};

	std::function<bool()> getDCPin;


	uint8_t mask(ss1106::Operator op);
	Command match(uint8_t cmd);

public:
	SS1106(std::function<bool()> getDCPin);
	~SS1106();

	uint8_t write(uint8_t byte) override;
};
