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

#ifndef RISCV_ISA_SYMBOLIC_VALUE_H
#define RISCV_ISA_SYMBOLIC_VALUE_H

#include <tlm.h>
#include <clover/clover.h>

// This class implements a ignore extension for concolic values. Refer
// to Section 14.21.1.1 of IEEE Std 1666-2011 for more information.
//
// TLM components cannot rely on the presence of this extension in a payload.
class SymbolicExtension : public tlm::tlm_extension<SymbolicExtension> {
	std::shared_ptr<clover::ConcolicValue> value;

public:
	typedef tlm::tlm_base_protocol_types::tlm_payload_type tlm_payload_type;
	typedef tlm::tlm_base_protocol_types::tlm_phase_type tlm_phase_type;

	SymbolicExtension(std::shared_ptr<clover::ConcolicValue> _value);
	~SymbolicExtension(void);

	void copy_from(const tlm_extension_base &extension);
	tlm::tlm_extension_base *clone(void) const;
	std::shared_ptr<clover::ConcolicValue> getValue(void);
};

#endif
