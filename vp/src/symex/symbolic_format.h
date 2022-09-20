/*
 * Copyright (c) 2021,2022 Group of Computer Architecture, University of Bremen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef RISCV_VP_SYMBOLIC_FMT_H
#define RISCV_VP_SYMBOLIC_FMT_H

#include <memory>
#include <stdint.h>
#include <symbolic_context.h>
#include <clover/clover.h>
#include <istream>

#include "bencode/include/bencode.hpp"

class SymbolicFormat {
private:

	clover::ExecutionContext &ctx;
	clover::Solver &solver;
	std::fstream file;

	// Bencode data retrieved from the stream.
	bencode::data data;

	// True if the stream has been parsed already.
	bool parsed = false;

	// SISL fields contained in the bencode data stream (set by get_values).
	std::vector<std::shared_ptr<clover::ConcolicValue>> values;

	// Retrieve a concrete or symbolic field value from the Bencode data.
	std::shared_ptr<clover::ConcolicValue> get_value(bencode::list list, std::string name, uint64_t bitsize);

public:
	SymbolicFormat(SymbolicContext &_ctx, std::string path);

	std::vector<std::shared_ptr<clover::ConcolicValue>> get_values(void);
	std::vector<std::shared_ptr<clover::ConcolicValue>> get_bytes(void);
};

#endif
