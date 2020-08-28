#ifndef RISCV_ISA_SYMBOLIC_MEMORY_H
#define RISCV_ISA_SYMBOLIC_MEMORY_H

#include <stddef.h>

#include <clover/clover.h>
#include <tlm_utils/simple_target_socket.h>

#include <systemc>
#include <memory>

#include "symbolic_extension.h"

class SymbolicMemory : public sc_core::sc_module {
private:
	clover::ConcolicMemory memory;
	size_t size;

public:
	typedef std::shared_ptr<clover::ConcolicValue> Data;
	tlm_utils::simple_target_socket<SymbolicMemory> tsock;

	SymbolicMemory(sc_core::sc_module_name, clover::Solver &solver, size_t _size);

private:
	unsigned read_data(tlm::tlm_generic_payload &trans, uint64_t addr, size_t size);
	unsigned write_data(tlm::tlm_generic_payload &trans, uint64_t addr, size_t size);

protected:
	void transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay);
	unsigned transport_dbg(tlm::tlm_generic_payload &trans);
};

#endif
