#ifndef RISCV_VP_SYMBOLIC_SLIP_H
#define RISCV_VP_SYMBOLIC_SLIP_H

#include <stdint.h>
#include <stddef.h>

#include <queue>
#include <systemc>
#include <fstream>
#include <tlm_utils/simple_target_socket.h>
#include <clover/clover.h>

#include "symbolic_context.h"
#include "core/common/irq_if.h"

class SymbolicSLIP : public sc_core::sc_module {
public:
	enum {
		/* The IPv6 header itself has a size of 360 bits */
		DEFAULT_PKT_SIZE = 45,
	};

	clover::Solver &solver;
	clover::ExecutionContext &ctx;

	interrupt_gateway *plic;
	tlm_utils::simple_target_socket<SymbolicSLIP> tsock;

	SymbolicSLIP(sc_core::sc_module_name, uint32_t, SymbolicContext &_ctx,
	            size_t pktsiz = DEFAULT_PKT_SIZE);
	~SymbolicSLIP(void);

private:
	uint32_t irq;
	std::queue<std::shared_ptr<clover::ConcolicValue>> input;

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

	void push_byte(uint8_t byte);
	void create_input(size_t pktsiz);

	uint32_t *addr2register(uint64_t addr);
	void transport(tlm::tlm_generic_payload &, sc_core::sc_time &);
};

#endif
