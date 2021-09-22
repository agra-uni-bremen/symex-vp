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

#ifndef RISCV_ISA_SYMBOLIC_SENSOR_H
#define RISCV_ISA_SYMBOLIC_SENSOR_H

#include <stdint.h>
#include <stddef.h>

#include <systemc>
#include <vector>

#include <symbolic_context.h>
#include <symbolic_extension.h>
#include <tlm_utils/simple_target_socket.h>
#include <clover/clover.h>

class SymbolicSensor : public sc_core::sc_module {
private:
	clover::Solver &solver;
	clover::ExecutionContext &ctx;

	size_t nread = 0;

	uint32_t lower_bound = 0;
	uint32_t upper_bound = 0;

	void transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay);
	uint32_t *addr2register(uint64_t);

public:
	tlm_utils::simple_target_socket<SymbolicSensor> tsock;

	SymbolicSensor(sc_core::sc_module_name, SymbolicContext &_ctx);
};

#endif
