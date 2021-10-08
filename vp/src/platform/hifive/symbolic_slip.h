#ifndef RISCV_VP_SYMBOLIC_SLIP_H
#define RISCV_VP_SYMBOLIC_SLIP_H

#include <stdint.h>
#include <stddef.h>

#include <systemc>
#include <fstream>
#include <tlm_utils/simple_target_socket.h>
#include <clover/clover.h>

#include "symbolic_format.h"
#include "symbolic_context.h"
#include "core/common/irq_if.h"

class SymbolicSLIP : public sc_core::sc_module {
public:
	clover::Solver &solver;
	clover::ExecutionContext &ctx;

	interrupt_gateway *plic;
	tlm_utils::simple_target_socket<SymbolicSLIP> tsock;

	SymbolicSLIP(sc_core::sc_module_name, uint32_t, SymbolicContext &_ctx, SymbolicFormat &fmt);
	~SymbolicSLIP(void);

private:
	uint32_t irq;

	std::shared_ptr<clover::ConcolicValue> pktfmt;
	unsigned off = 0;

	// memory mapped configuration registers
	uint32_t txdata = 0;
	uint32_t rxdata = 0;
	uint32_t txctrl = 0;
	uint32_t rxctrl = 0;
	uint32_t ie = 0;
	uint32_t ip = 0;
	uint32_t div = 0;

	std::shared_ptr<clover::ConcolicValue> end;
	std::shared_ptr<clover::ConcolicValue> esc;
	std::shared_ptr<clover::ConcolicValue> esc_end;
	std::shared_ptr<clover::ConcolicValue> esc_esc;

	unsigned rxrem(void);
	uint32_t *addr2register(uint64_t addr);
	void transport(tlm::tlm_generic_payload &, sc_core::sc_time &);
};

#endif
