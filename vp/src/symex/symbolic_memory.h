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
	clover::ConcolicMemory memory;
	size_t size;

public:
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
