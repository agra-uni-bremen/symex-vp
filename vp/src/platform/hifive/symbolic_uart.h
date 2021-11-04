#ifndef RISCV_VP_SYMBOLIC_SLIP_H
#define RISCV_VP_SYMBOLIC_SLIP_H

#include <stdint.h>
#include <stddef.h>

#include <queue>
#include <systemc>
#include <tlm_utils/simple_target_socket.h>
#include <clover/clover.h>

#include "symbolic_format.h"
#include "symbolic_context.h"
#include "symbolic_extension.h"

#include "core/common/irq_if.h"
#include "util/tlm_map.h"
#include "platform/common/async_event.h"

class SymbolicUART : public sc_core::sc_module {
public:
	interrupt_gateway *plic;
	tlm_utils::simple_target_socket<SymbolicUART> tsock;

	SymbolicUART(sc_core::sc_module_name, uint32_t, SymbolicContext &_ctx, SymbolicFormat &_fmt);
	~SymbolicUART(void);

	SC_HAS_PROCESS(SymbolicUART);

private:
	clover::Solver &solver;
	clover::ExecutionContext &ctx;
	SymbolicFormat &fmt;

	uint32_t irq;

	// memory mapped configuration registers
	uint32_t txdata = 0;
	uint32_t rxdata = 0;
	uint32_t txctrl = 0;
	uint32_t rxctrl = 0;
	uint32_t ie = 0;
	uint32_t ip = 0;
	uint32_t div = 0;

	AsyncEvent asyncEvent;
	std::queue<std::shared_ptr<clover::ConcolicValue>> rx_fifo;

	vp::map::LocalRouter router = {"SymbolicUART"};

	void register_access_callback(const vp::map::register_access_t &);
	void transport(tlm::tlm_generic_payload &, sc_core::sc_time &);
	void interrupt(void);
};

#endif
