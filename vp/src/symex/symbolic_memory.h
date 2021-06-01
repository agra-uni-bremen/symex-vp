/*
 * Copyright (c) 2020,2021 Group of Computer Architecture, University of Bremen
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

#ifndef RISCV_ISA_SYMBOLIC_MEMORY_H
#define RISCV_ISA_SYMBOLIC_MEMORY_H

#include <stddef.h>

#include <clover/clover.h>
#include <tlm_utils/simple_target_socket.h>
#include <load_if.h>

#include <systemc>
#include <memory>

#include "symbolic_extension.h"

class SymbolicMemory : public sc_core::sc_module, public load_if {
private:
	clover::Solver &solver;
	size_t size;

public:
	clover::ConcolicMemory memory;

	typedef std::shared_ptr<clover::ConcolicValue> Data;
	tlm_utils::simple_target_socket<SymbolicMemory> tsock;

	SymbolicMemory(sc_core::sc_module_name, clover::Solver &_solver, size_t _size);

	void load_data(const char *src, uint64_t dst_addr, size_t n) override;
	void load_zero(uint64_t dst_addr, size_t n) override;

private:
	unsigned read_data(tlm::tlm_generic_payload &trans);
	unsigned write_data(tlm::tlm_generic_payload &trans);

protected:
	void transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay);
	unsigned transport_dbg(tlm::tlm_generic_payload &trans);
};

#endif
