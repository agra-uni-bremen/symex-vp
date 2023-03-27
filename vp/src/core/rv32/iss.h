/*
 * Copyright (c) 2020,2021 Group of Computer Architecture, University of Bremen
 *
 *  This file is free software: you may copy, redistribute and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  This file is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and permission notice:
 *
 *  Copyright (c) 2017-2018 Group of Computer Architecture, University of Bremen <riscv@systemc-verification.org>
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#pragma once

#include "core/common/bus_lock_if.h"
#include "core/common/clint_if.h"
#include "core/common/instr.h"
#include "core/common/irq_if.h"
#include "core/common/trap.h"
#include "core/common/debug.h"
#include "csr.h"
#include "mem_if.h"
#include "syscall_if.h"
#include "symbolic_if.h"
#include "symbolic_context.h"
#include "util/common.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include <clover/clover.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/tlm_quantumkeeper.h>
#include <systemc>

namespace rv32 {

struct RegFile {
	clover::Solver &solver;
	clover::Trace &trace;
	static constexpr unsigned NUM_REGS = 32;

	typedef std::shared_ptr<clover::ConcolicValue> RegValue;
	std::array<RegValue, NUM_REGS> regs;

	RegFile(clover::Solver &_solver, clover::Trace &_trace);

	RegFile(clover::Solver &_solver, clover::Trace &_trace, const RegFile &other);

	void write(uint32_t index, RegValue value);

	RegFile::RegValue read(uint32_t index);

	RegFile::RegValue shamt(uint32_t index);

	const RegValue &operator[](const uint32_t idx);

	void show();

	enum e : uint16_t {
		x0 = 0,
		x1,
		x2,
		x3,
		x4,
		x5,
		x6,
		x7,
		x8,
		x9,
		x10,
		x11,
		x12,
		x13,
		x14,
		x15,
		x16,
		x17,
		x18,
		x19,
		x20,
		x21,
		x22,
		x23,
		x24,
		x25,
		x26,
		x27,
		x28,
		x29,
		x30,
		x31,

		zero = x0,
		ra = x1,
		sp = x2,
		gp = x3,
		tp = x4,
		t0 = x5,
		t1 = x6,
		t2 = x7,
		s0 = x8,
		fp = x8,
		s1 = x9,
		a0 = x10,
		a1 = x11,
		a2 = x12,
		a3 = x13,
		a4 = x14,
		a5 = x15,
		a6 = x16,
		a7 = x17,
		s2 = x18,
		s3 = x19,
		s4 = x20,
		s5 = x21,
		s6 = x22,
		s7 = x23,
		s8 = x24,
		s9 = x25,
		s10 = x26,
		s11 = x27,
		t3 = x28,
		t4 = x29,
		t5 = x30,
		t6 = x31,
	};
};

// NOTE: on this branch, currently the *simple-timing* model is still directly
// integrated in the ISS. Merge the *timedb* branch to use the timing_if.
struct ISS;

struct timing_if {
	virtual ~timing_if() {}

	virtual void update_timing(Instruction instr, Opcode::Mapping op, ISS &iss) = 0;
};

/* Buffer to be used between the ISS and instruction memory interface to cache compressed instructions.
 * In case the ISS does not support compressed instructions, then this buffer is not necessary and the ISS
 * can use the memory interface directly. */
struct InstructionBuffer {
	instr_memory_if *instr_mem = nullptr;
	uint32_t last_fetch_addr = 0;
	uint32_t buffer = 0;

	uint32_t load_instr(uint64_t addr) {
		if (addr == (last_fetch_addr + 2))
			return (buffer >> 16);

		last_fetch_addr = addr;
		buffer = instr_mem->load_instr(addr);
		return buffer;
	}
};

struct PendingInterrupts {
	PrivilegeLevel target_mode;
	uint32_t pending;
};

struct ISS : public external_interrupt_target, public clint_interrupt_target, public iss_syscall_if, public debug_target_if, public symbolic_iss_if {
	clover::Solver &solver;
	clover::ExecutionContext &ctx;
	clover::Trace &tracer;

	clint_if *clint = nullptr;
	instr_memory_if *instr_mem = nullptr;
	data_memory_if *mem = nullptr;
	syscall_emulator_if *sys = nullptr;  // optional, if provided, the iss will intercept and handle syscalls directly
	RegFile regs;
	uint32_t pc = 0;
	uint32_t last_pc = 0;
	bool trace = false;
	bool shall_exit = false;
    bool ignore_wfi = false;
	csr_table csrs;
	PrivilegeLevel prv = MachineMode;
	int64_t lr_sc_counter = 0;
	uint64_t total_num_instr = 0;

	// last decoded and executed instruction and opcode
	Instruction instr;
	Opcode::Mapping op;

	CoreExecStatus status = CoreExecStatus::Runnable;
	std::unordered_set<uint32_t> breakpoints;
	bool debug_mode = false;

	sc_core::sc_event wfi_event;

	std::string systemc_name;
	tlm_utils::tlm_quantumkeeper quantum_keeper;
	sc_core::sc_time cycle_time;
	sc_core::sc_time cycle_counter;  // use a separate cycle counter, since cycle count can be inhibited
	std::array<sc_core::sc_time, Opcode::NUMBER_OF_INSTRUCTIONS> instr_cycles;

	static constexpr int32_t REG_MIN = INT32_MIN;
    static constexpr unsigned xlen = 32;

	ISS(SymbolicContext &_ctx, uint32_t hart_id, bool use_E_base_isa = false);

	void exec_step();

	uint64_t _compute_and_get_current_cycles();

	void init(instr_memory_if *instr_mem, data_memory_if *data_mem, clint_if *clint, uint32_t entrypoint, uint32_t sp);

	void trigger_external_interrupt(PrivilegeLevel level) override;

	void clear_external_interrupt(PrivilegeLevel level) override;

	void trigger_timer_interrupt(bool status) override;

	void trigger_software_interrupt(bool status) override;


	void sys_exit() override;
	unsigned get_syscall_register_index() override;
	uint64_t read_register(unsigned idx) override;
	void write_register(unsigned idx, uint64_t value) override;

    std::vector<uint64_t> get_registers(void) override;

    bool eval(std::shared_ptr<clover::BitVector> bv) {
        auto q = tracer.getQuery(bv);
        return solver.eval(q);
    };

    void track_and_trace_branch(bool cond, std::shared_ptr<clover::ConcolicValue> expr) {
        if (expr->symbolic.has_value())
            tracer.add(cond, *expr->symbolic, last_pc);
    };

    void make_symbolic(uint32_t addr, size_t size) override {
        std::string name = std::string("memory") + "<" + std::to_string(addr) + ">";

        auto saddr = solver.BVC(std::nullopt, addr);
        auto value = ctx.getSymbolicBytes(name, size);

        mem->symbolic_store_data(saddr, value, size);
    }

    Architecture get_architecture(void) override {
        return RV32;
    }

    uint64_t get_progam_counter(void) override;
    void enable_debug(void) override;
    CoreExecStatus get_status(void) override;
    void set_status(CoreExecStatus) override;
    void block_on_wfi(bool) override;

    void insert_breakpoint(uint64_t) override;
    void remove_breakpoint(uint64_t) override;

	uint64_t get_hart_id() override;


	void release_lr_sc_reservation() {
		lr_sc_counter = 0;
		mem->atomic_unlock();
	}

#if 0
	void fp_prepare_instr();
	void fp_finish_instr();
	void fp_set_dirty();
	void fp_update_exception_flags();
	void fp_setup_rm();
	void fp_require_not_off();
#endif

	uint32_t get_csr_value(uint32_t addr);
	void set_csr_value(uint32_t addr, uint32_t value);

	bool is_invalid_csr_access(uint32_t csr_addr, bool is_write);
	void validate_csr_counter_read_access_rights(uint32_t addr);

	unsigned pc_alignment_mask() {
		if (csrs.misa.has_C_extension())
			return ~0x1;
		else
			return ~0x3;
	}

	inline void trap_check_pc_alignment() {
		assert(!(pc & 0x1) && "not possible due to immediate formats and jump execution");

		if (unlikely((pc & 0x3) && (!csrs.misa.has_C_extension()))) {
			// NOTE: misaligned instruction address not possible on machines supporting compressed instructions
			raise_trap(EXC_INSTR_ADDR_MISALIGNED, pc);
		}
	}

	template <unsigned Alignment, bool isLoad>
	inline void trap_check_addr_alignment(std::shared_ptr<clover::ConcolicValue> addr) {
		auto caddr = solver.getValue<uint32_t>(addr->concrete);
		if (unlikely(caddr % Alignment)) {
			raise_trap(isLoad ? EXC_LOAD_ADDR_MISALIGNED : EXC_STORE_AMO_ADDR_MISALIGNED, caddr);
		}
	}

	inline void execute_amo(Instruction &instr, std::function<RegFile::RegValue(RegFile::RegValue, RegFile::RegValue)> operation) {
		auto addr = regs[instr.rs1()];
		trap_check_addr_alignment<4, false>(addr);
		std::shared_ptr<clover::ConcolicValue> data;
		try {
			data = mem->atomic_load_word(addr);
		} catch (SimulationTrap &e) {
			if (e.reason == EXC_LOAD_ACCESS_FAULT)
				e.reason = EXC_STORE_AMO_ACCESS_FAULT;
			throw e;
		}
		auto val = operation(data, regs[instr.rs2()]);
		mem->atomic_store_word(addr, val);
		regs.write(instr.rd(), data);
	}

	inline bool m_mode() {
		return prv == MachineMode;
	}

	inline bool s_mode() {
		return prv == SupervisorMode;
	}

	inline bool u_mode() {
		return prv == UserMode;
	}

	PrivilegeLevel prepare_trap(SimulationTrap &e);

	void prepare_interrupt(const PendingInterrupts &x);

	PendingInterrupts compute_pending_interrupts();

	bool has_pending_enabled_interrupts() {
		return compute_pending_interrupts().target_mode != NoneMode;
	}

	bool has_local_pending_enabled_interrupts() {
		return csrs.mie.reg & csrs.mip.reg;
	}

	void return_from_trap_handler(PrivilegeLevel return_mode);

	void switch_to_trap_handler(PrivilegeLevel target_mode);

	void performance_and_sync_update(Opcode::Mapping executed_op);

	void run_step() override;

	void run() override;

	void show();
};

/* Do not call the run function of the ISS directly but use one of the Runner
 * wrappers. */
struct DirectCoreRunner : public sc_core::sc_module {
	ISS &core;
	std::string thread_name;

	SC_HAS_PROCESS(DirectCoreRunner);

	DirectCoreRunner(ISS &core) : sc_module(sc_core::sc_module_name(core.systemc_name.c_str())), core(core) {
		thread_name = "run" + std::to_string(core.get_hart_id());
		SC_NAMED_THREAD(run, thread_name.c_str());
	}

	void run() {
		core.run();

		if (core.status == CoreExecStatus::HitBreakpoint) {
			throw std::runtime_error(
			    "Breakpoints are not supported in the direct runner, use the debug "
			    "runner instead.");
		}
		assert(core.status == CoreExecStatus::Terminated);

		sc_core::sc_stop();
	}
};

}  // namespace rv32
