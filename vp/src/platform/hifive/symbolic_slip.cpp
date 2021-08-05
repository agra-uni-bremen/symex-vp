#include <assert.h>
#include <stddef.h>

#include <iostream>

#include "symbolic_extension.h"
#include "symbolic_slip.h"

enum {
	TXDATA_REG_ADDR = 0x0,
	RXDATA_REG_ADDR = 0x4,
	TXCTRL_REG_ADDR = 0x8,
	RXCTRL_REG_ADDR = 0xC,
	IE_REG_ADDR = 0x10,
	IP_REG_ADDR = 0x14,
	DIV_REG_ADDR = 0x18,
};

#define UART_TXWM (1 << 0)
#define UART_RXWM (1 << 1)
#define UART_FULL (1 << 31)

#define SLIP_END 0300
#define SLIP_ESC 0333
#define SLIP_ESC_END 0334
#define SLIP_ESC_ESC 0335

/* Extracts the interrupt trigger threshold from a control register */
#define UART_CTRL_CNT(REG) ((REG) >> 16)

/* Value for version field of IP header */
#define IP_VERSION4 (0x40U)
#define IP_VERSION6 (0x60U)

/* Next header value for ICMPv6 */
#define PROTNUM_ICMPV6 (58)

SymbolicSLIP::SymbolicSLIP(sc_core::sc_module_name, uint32_t irqsrc, SymbolicContext &_ctx, size_t pktsiz)
  : solver(_ctx.solver), ctx(_ctx.ctx) {
	irq = irqsrc;

	end = solver.BVC(std::nullopt, (uint8_t)SLIP_END);
	esc = solver.BVC(std::nullopt, (uint8_t)SLIP_ESC);
	esc_end = solver.BVC(std::nullopt, (uint8_t)SLIP_ESC_END);
	esc_esc = solver.BVC(std::nullopt, (uint8_t)SLIP_ESC_ESC);

	this->create_input(pktsiz);
	tsock.register_b_transport(this, &SymbolicSLIP::transport);
}

SymbolicSLIP::~SymbolicSLIP(void) {
	return;
}

void SymbolicSLIP::push_byte(uint8_t byte) {
	switch (byte) {
	case SLIP_END:
		input.push(esc);
		input.push(esc_end);
		break;
	case SLIP_ESC:
		input.push(esc);
		input.push(esc_esc);
		break;
	default:
		input.push(solver.BVC(std::nullopt, byte));
		break;
	}
}

void SymbolicSLIP::create_input(size_t pktsiz) {
	/* Make sure created data is always treated as an IPv6 packet */
	input.push(solver.BVC(std::nullopt, (uint8_t)IP_VERSION6));

	for (size_t i = 0; i < pktsiz; i++) {
		auto byte = ctx.getSymbolicByte("slip_byte" + std::to_string(i));
		input.push(byte->urem(end));
	}

	/* Mark end of packet */
	input.push(end);
}

uint32_t *SymbolicSLIP::addr2register(uint64_t addr) {
	switch (addr) {
	case TXDATA_REG_ADDR:
		return &txdata;
	case RXDATA_REG_ADDR:
		return &rxdata;
	case TXCTRL_REG_ADDR:
		return &txctrl;
	case RXCTRL_REG_ADDR:
		return &rxctrl;
	case IE_REG_ADDR:
		return &ie;
	case IP_REG_ADDR:
		return &ip;
	case DIV_REG_ADDR:
		return &div;
	}

	return NULL;
}

void SymbolicSLIP::transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay) {
	auto addr = trans.get_address();
	auto cmd = trans.get_command();
	auto len = trans.get_data_length();
	auto ptr = trans.get_data_ptr();

	uint32_t *reg = addr2register(addr);
	if (!reg) {
		trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
		return;
	}

	if (cmd == tlm::TLM_READ_COMMAND) {
		switch (addr) {
		case TXDATA_REG_ADDR:
			txdata = 0; // Pretend to be always ready to transmit
			break;
		case RXDATA_REG_ADDR:
			/* Many UART drivers drain the UART before accepting interrupts,
			 * thus we only send data after receive interrupts have been enabled. */
			if (input.empty() || !(ie & UART_RXWM)) {
				rxdata = 1 << 31;
			} else {
				auto value = input.front()->zext(32);
				input.pop();

				rxdata = solver.getValue<uint32_t>(value->concrete);

				auto ext = new SymbolicExtension(value);
				trans.set_extension(ext);
			}
			break;
		case IP_REG_ADDR:
			ip = UART_TXWM; // Transmit is always ready
			if (input.size() > UART_CTRL_CNT(rxctrl))
				ip |= UART_RXWM;
			break;
		}
	}

	if (len != sizeof(uint32_t)) {
		trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
		return;
	}
	if (cmd == tlm::TLM_READ_COMMAND)
		memcpy(ptr, reg, sizeof(uint32_t));
	else if (cmd == tlm::TLM_WRITE_COMMAND)
		memcpy(reg, ptr, sizeof(uint32_t));

	if ((ie & UART_RXWM) && input.size() > UART_CTRL_CNT(rxctrl))
		plic->gateway_trigger_interrupt(irq);
	// Don't trigger transmit interrupts explicitly
}
