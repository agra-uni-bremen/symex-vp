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

#include "symbolic_memory.h"

SymbolicMemory::SymbolicMemory(sc_core::sc_module_name, clover::Solver &_solver, size_t _size)
    : solver(_solver), size(_size), memory(solver)
{
	tsock.register_b_transport(this, &SymbolicMemory::transport);
	tsock.register_transport_dbg(this, &SymbolicMemory::transport_dbg);
}

void
SymbolicMemory::load_data(const char *src, uint64_t dst_addr, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		auto byte = solver.BVC(std::nullopt, (uint8_t)src[i]);
		memory.store(dst_addr + i, byte, 1);
	}
}

void
SymbolicMemory::load_zero(uint64_t dst_addr, size_t n)
{
	auto zero = solver.BVC(std::nullopt, (uint8_t)0);
	for (size_t i = 0; i < n; i++)
		memory.store(dst_addr + i, zero, 1);
}

unsigned
SymbolicMemory::read_data(tlm::tlm_generic_payload &trans)
{
	auto size = trans.get_data_length();

	auto data = memory.load(trans.get_address(), size);
	SymbolicExtension *extension = new SymbolicExtension(data);

	solver.BVCToBytes(data, trans.get_data_ptr(), trans.get_data_length());
	trans.set_extension(extension);

	return size;
}

unsigned
SymbolicMemory::write_data(tlm::tlm_generic_payload &trans)
{
	std::shared_ptr<clover::ConcolicValue> value;
	auto size = trans.get_data_length();

	SymbolicExtension *extension;
	trans.get_extension(extension);

	if (extension)
		value = extension->getValue();
	else
		value = solver.BVC(trans.get_data_ptr(), size);

	// ConcolicValue may have getWith() > size * 8, however,
	// the ConcolicMemory::store will only store size bytes.
	memory.store(trans.get_address(), value, size);

	return size;
}

void
SymbolicMemory::transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay)
{
	transport_dbg(trans);
	delay += sc_core::sc_time(10, sc_core::SC_NS);
}

unsigned
SymbolicMemory::transport_dbg(tlm::tlm_generic_payload &trans)
{
	uint64_t addr = trans.get_address();
	auto len = trans.get_data_length();

	if (addr + len > size) {
		trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
		return 0;
	}

	if (trans.is_read()) {
		return read_data(trans);
	} else if (trans.is_write()) {
		return write_data(trans);
	}

	trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
	return 0;
}
