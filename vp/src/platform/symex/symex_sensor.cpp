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

#include <assert.h>
#include <string.h>

#include "symex_sensor.h"

enum {
	LOWER_REG_ADDR = 0x0,
	UPPER_REG_ADDR = 0x4,
	VALUE_REG_ADDR = 0x8,
};

SymbolicSensor::SymbolicSensor(sc_core::sc_module_name, SymbolicContext &_ctx)
  : solver(_ctx.solver), ctx(_ctx.ctx)
{
	tsock.register_b_transport(this, &SymbolicSensor::transport);
}

uint32_t*
SymbolicSensor::addr2register(uint64_t addr)
{
	switch (addr) {
	case LOWER_REG_ADDR:
		return &lower_bound;
	case UPPER_REG_ADDR:
		return &upper_bound;
	}

	return NULL;
}

void
SymbolicSensor::transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay)
{
	auto addr = trans.get_address();
	auto cmd = trans.get_command();
	auto len = trans.get_data_length();
	auto ptr = trans.get_data_ptr();

	if (len != sizeof(uint32_t)) {
		trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
		return;
	}

	if (cmd == tlm::TLM_READ_COMMAND) {
		if (addr != VALUE_REG_ADDR || upper_bound <= lower_bound) {
			trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
			return;
		}

		auto sval = ctx.getSymbolicWord("SymbolicSensorValue" + std::to_string(nread++));
		sval = sval->urem(solver.BVC(std::nullopt, upper_bound - lower_bound));
		sval = sval->add(solver.BVC(std::nullopt, lower_bound));

		auto ext = new SymbolicExtension(sval);
		trans.set_extension(ext);

		uint32_t cval = solver.getValue<uint32_t>(sval->concrete);
		memcpy(ptr, &cval, sizeof(uint32_t));
	}

	if (cmd == tlm::TLM_WRITE_COMMAND) {
		uint32_t *reg = addr2register(addr);
		if (!reg) {
			trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
			return;
		}

		memcpy(reg, ptr, sizeof(uint32_t));
	}

	trans.set_response_status(tlm::TLM_OK_RESPONSE);
}
