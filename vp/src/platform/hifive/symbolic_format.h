/*
 * Copyright (c) 2021 Group of Computer Architecture, University of Bremen
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

#include <symbolic_context.h>
#include <stdint.h>
#include <clover/clover.h>

class SymbolicFormat {
private:
	clover::ExecutionContext &ctx;
	clover::Solver &solver;
	int fd;
	size_t numSymField = 0;
	unsigned offset;
	std::shared_ptr<clover::ConcolicValue> input;

	std::shared_ptr<clover::ConcolicValue> next_field(void);
	std::shared_ptr<clover::ConcolicValue> get_input(void);

public:
	SymbolicFormat(SymbolicContext &_ctx, std::string path);
	~SymbolicFormat(void);

	/* XXX: Could be implemented as an Iterator.
	 *
	 * Also: KLEE Array Type would be useful here ReadLSB, ReadMSB, …
	 * See:  https://gitlab.informatik.uni-bremen.de/riscv/clover/-/issues/7 */
	std::shared_ptr<clover::ConcolicValue> next_byte(void);
	size_t remaning_bytes(void);
};

#endif
