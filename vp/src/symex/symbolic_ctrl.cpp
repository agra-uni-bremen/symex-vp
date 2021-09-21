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

#include "symbolic_ctrl.h"

#define CTRL_ERROR (1 << 31)

SymbolicCTRL::SymbolicCTRL(sc_core::sc_module_name, symbolic_iss_if &_symif)
  : symif(_symif)
{
	tsock.register_b_transport(this, &SymbolicCTRL::transport);

	reg_size.post_write_callback =
		std::bind(&SymbolicCTRL::write_size, this, std::placeholders::_1);
	reg_ctrl.post_write_callback =
		std::bind(&SymbolicCTRL::write_ctrl, this, std::placeholders::_1);
}

void
SymbolicCTRL::transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay)
{
	delay += sc_core::sc_time(10, sc_core::SC_NS);
	vp::mm::route("SymbolicCTRL", register_ranges, trans, delay);
}

void
SymbolicCTRL::write_size(RegisterRange::WriteInfo t)
{
	// Assumption: This is written *after* reg_addr was written.
	uint32_t *addr = &make_symbolic_addr[0];
	uint32_t *size = &make_symbolic_size[0];
	symif.make_symbolic(*addr, (size_t)*size);
}

void
SymbolicCTRL::write_ctrl(RegisterRange::WriteInfo t)
{
	// XXX: Only 31th bit is used currently, all other bits are
	// reserved for future use and currently entirely ignored.
	uint32_t *val = &symbolic_ctrl[0];
	if (*val & CTRL_ERROR)
		SC_REPORT_ERROR("/AGRA/riscv-vp/host-error", "SYS_host_error");

	*val = 0; // reset
}
