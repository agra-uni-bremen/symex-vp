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
 * common.hpp
 *
 *  Created on: 24 Sep 2019
 *      Author: dwd
 */

#pragma once
#include <inttypes.h>
#include "util/elegantEnums.hpp"

namespace ss1106
{

static constexpr uint8_t width  = 132;
static constexpr uint8_t padding_lr  = 2;
static constexpr uint8_t height = 64;
static_assert(height%8 == 0, "invalid height");
static constexpr uint16_t shm_key = 1339;

struct State
{
	uint8_t changed:1;	//beh, this is for syncing

	uint8_t column;
	uint8_t page;
	uint8_t pump_voltage:2;
	uint8_t display_startline:6;
	uint8_t contrast;
	uint8_t segment_remap:1;
	uint8_t entire_disp_on:1;
	uint8_t invert_color:1;
	uint8_t multiplex_whatever:6;
	uint8_t display_on:1;
	uint8_t frame[(height / 8)][width];
};

State* getSharedState();

DECLARE_ENUM(Operator,
	COL_LOW,
	COL_HIGH,
	PUMP_VOLTAGE,
	DISPLAY_START_LINE,
	CONTRAST_MODE_SET,		//Double Command
	SEGMENT_REMAP,
	ENTIRE_DISPLAY,
	NORMAL_INVERSE,
	MULTIPLEX_RATIO,		//Double command
	DC_DC_VOLTAGE,
	DISPLAY_ON,
	PAGE_ADDR,
	COMMON_OUTPUT_DIR,
	DISPLAY_OFFSET,			//Double command
	DISPLAY_DIVIDE_RATIO,	//Double command
	DIS_PRE_CHARGE_PERIOD,	//Double command
	COMMON_PADS_STUFF,		//DC
	VCOM_DESELECT,			//DC
	RMW,
	RMW_END,
	NOP
);

}

