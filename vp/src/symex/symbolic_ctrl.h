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

#ifndef RISCV_ISA_SYMBOLIC_CTRL_H
#define RISCV_ISA_SYMBOLIC_CTRL_H

#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#include <systemc>

#include <tlm_utils/simple_target_socket.h>
#include <util/memory_map.h>
#include <clover/clover.h>

#include "symbolic_if.h"

// This class implements a SystemC peripheral for interacting with the
// symbolic execution engine. Currently, this peripheral can be used to
// mark a specified memory range as symbolic, signal an error condition,
// or cause execution for the current path to terminate.
//
// For this purpose three 32 bit memory mapped registers are provided:
//
//   (1) Address register used to mark memory range symbolic.
//   (2) Size register used to mark memory range as symbolic.
//   (3) Control register for miscellaneous functionallity.
//
// In order to mark memory as symbolic the address register needs to be
// written and afterwards the size register needs to be written. To
// signal an error condition the 31th bit in the control register must
// be set, the 30th bit can be used to terminate execution of for the
// current path. All other bits are reserved for future use.
//
// In a sense, this is a specialized version of the syscall peripheral.
class SymbolicCTRL : public sc_core::sc_module {
private:
	RegisterRange reg_addr{0x0, sizeof(uint32_t)};
	ArrayView<uint32_t> make_symbolic_addr{reg_addr};

	RegisterRange reg_size{0x4, sizeof(uint32_t)};
	ArrayView<uint32_t> make_symbolic_size{reg_size};

	RegisterRange reg_ctrl{0x8, sizeof(uint32_t)};
	ArrayView<uint32_t> symbolic_ctrl{reg_ctrl};

	std::vector<RegisterRange *> register_ranges{&reg_addr, &reg_size, &reg_ctrl};
	symbolic_iss_if &symif;

	void transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay);

	void write_size(RegisterRange::WriteInfo t);
	void write_ctrl(RegisterRange::WriteInfo t);

public:
	tlm_utils::simple_target_socket<SymbolicCTRL> tsock;

	SymbolicCTRL(sc_core::sc_module_name, symbolic_iss_if &_symif);
};

#endif
