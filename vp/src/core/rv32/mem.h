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

#include "core/common/dmi.h"
#include "iss.h"
#include "mmu.h"
#include "symbolic_extension.h"

namespace rv32 {

/* For optimization, use DMI to fetch instructions */
struct InstrMemoryProxy : public instr_memory_if {
	MemoryDMI dmi;

	tlm_utils::tlm_quantumkeeper &quantum_keeper;
	sc_core::sc_time clock_cycle = sc_core::sc_time(10, sc_core::SC_NS);
	sc_core::sc_time access_delay = clock_cycle * 2;

	InstrMemoryProxy(const MemoryDMI &dmi, ISS &owner) : dmi(dmi), quantum_keeper(owner.quantum_keeper) {}

	virtual uint32_t load_instr(uint64_t pc) override {
		quantum_keeper.inc(access_delay);
		return dmi.load<uint32_t>(pc);
	}
};

struct CombinedMemoryInterface : public sc_core::sc_module,
                                 public instr_memory_if,
                                 public data_memory_if,
                                 public mmu_memory_if  {
	ISS &iss;
	std::shared_ptr<bus_lock_if> bus_lock;
	uint64_t lr_addr = 0;

	tlm_utils::simple_initiator_socket<CombinedMemoryInterface> isock;
	tlm_utils::tlm_quantumkeeper &quantum_keeper;

	// optionally add DMI ranges for optimization
	sc_core::sc_time clock_cycle = sc_core::sc_time(10, sc_core::SC_NS);
	sc_core::sc_time dmi_access_delay = clock_cycle * 4;
	std::vector<MemoryDMI> dmi_ranges;

    MMU *mmu;

	CombinedMemoryInterface(sc_core::sc_module_name, ISS &owner, MMU *mmu = nullptr)
	    : iss(owner), quantum_keeper(iss.quantum_keeper), mmu(mmu) {
	}

    uint64_t v2p(uint64_t vaddr, MemoryAccessType type) override {
	    if (mmu == nullptr)
	        return vaddr;
        return mmu->translate_virtual_to_physical_addr(vaddr, type);
    }

	inline void _do_transaction(tlm::tlm_generic_payload &trans) {
		sc_core::sc_time local_delay = quantum_keeper.get_local_time();
		isock->b_transport(trans, local_delay);

		assert(local_delay >= quantum_keeper.get_local_time());
		quantum_keeper.set(local_delay);

		if (trans.is_response_error()) {
			auto addr = trans.get_address();
			if (iss.trace)
				std::cout << "WARNING: core memory transaction failed -> raise trap" << std::endl;
			if (trans.is_read())
				raise_trap(EXC_LOAD_PAGE_FAULT, addr);
			else if (trans.is_write())
				raise_trap(EXC_STORE_AMO_PAGE_FAULT, addr);
			else
				throw std::runtime_error("TLM command must be read or write");
		}

	}

	void _do_transaction(tlm::tlm_command cmd, uint64_t addr, Concolic &data, size_t num_bytes) {
		uint8_t buf[num_bytes];
		if (cmd == tlm::TLM_WRITE_COMMAND)
			iss.solver.BVCToBytes(data, &buf[0], num_bytes);
		else
			memset(&buf, 0, num_bytes);

		tlm::tlm_generic_payload trans;
		trans.set_command(cmd);
		trans.set_address(addr);
		trans.set_data_ptr(&buf[0]);
		trans.set_data_length(num_bytes);
		trans.set_response_status(tlm::TLM_OK_RESPONSE);

		if (cmd == tlm::TLM_WRITE_COMMAND) {
			SymbolicExtension *extension = new SymbolicExtension(data);
			trans.set_extension(extension);
		}

		_do_transaction(trans);
		if (cmd == tlm::TLM_WRITE_COMMAND)
			return;

		SymbolicExtension *extension;
		trans.get_extension(extension);
		if (extension) {
			data = extension->getValue(); // XXX: free extension?!
		} else {
			data = iss.solver.BVC(&buf[0], num_bytes);
		}
	}

	inline void _do_transaction(tlm::tlm_command cmd, uint64_t addr, uint8_t *data, size_t num_bytes) {
		tlm::tlm_generic_payload trans;
		trans.set_command(cmd);
		trans.set_address(addr);
		trans.set_data_ptr(data);
		trans.set_data_length(num_bytes);
		trans.set_response_status(tlm::TLM_OK_RESPONSE);

		_do_transaction(trans);
	}

	template <typename T>
	inline T concrete_load_data(uint64_t addr) {
		// NOTE: a DMI load will not context switch (SystemC) and not modify the memory, hence should be able to
		// postpone the lock after the dmi access
		bus_lock->wait_for_access_rights(iss.get_hart_id());

		for (auto &e : dmi_ranges) {
			if (e.contains(addr)) {
				quantum_keeper.inc(dmi_access_delay);
				return e.load<T>(addr);
			}
		}

		T ans;
		_do_transaction(tlm::TLM_READ_COMMAND, addr, (uint8_t *)&ans, sizeof(T));
		return ans;
	}

	template <typename T>
	inline void concrete_store_data(uint64_t addr, T value) {
		bus_lock->wait_for_access_rights(iss.get_hart_id());

		bool done = false;
		for (auto &e : dmi_ranges) {
			if (e.contains(addr)) {
				quantum_keeper.inc(dmi_access_delay);
				e.store(addr, value);
				done = true;
			}
		}

		if (!done)
			_do_transaction(tlm::TLM_WRITE_COMMAND, addr, (uint8_t *)&value, sizeof(T));
#if 0
		atomic_unlock();
#endif
	}

	void symbolic_store_data(Concolic addr, Concolic data, size_t num_bytes) override {
		bus_lock->wait_for_access_rights(iss.get_hart_id());
		// XXX: DMI is currently not supported for symbolic values.

		auto caddr = iss.solver.getValue<uint32_t>(addr->concrete);
		auto vaddr = v2p(caddr, STORE);

		_do_transaction(tlm::TLM_WRITE_COMMAND, vaddr, data, num_bytes);
	}

	Concolic symbolic_load_data(Concolic addr, size_t num_bytes) override {
		bus_lock->wait_for_access_rights(iss.get_hart_id());
		// XXX: DMI is currently not supported for symbolic values.

		auto caddr = iss.solver.getValue<uint32_t>(addr->concrete);
		auto vaddr = v2p(caddr, STORE);

		Concolic data;
		_do_transaction(tlm::TLM_READ_COMMAND, vaddr, data, num_bytes);
		return data;
	}

    template <typename T>
    inline T _load_data(uint64_t addr) {
        return concrete_load_data<T>(v2p(addr, LOAD));
    }

    template <typename T>
    inline void _store_data(uint64_t addr, T value) {
        concrete_store_data(v2p(addr, STORE), value);
    }

    uint64_t mmu_load_pte64(uint64_t addr) override {
        return concrete_load_data<uint64_t>(addr);
    }
    uint64_t mmu_load_pte32(uint64_t addr) override {
        return concrete_load_data<uint32_t>(addr);
    }
    void mmu_store_pte32(uint64_t addr, uint32_t value) override {
        concrete_store_data(addr, value);
    }

    void flush_tlb() override {
        mmu->flush_tlb();
    }

    uint32_t load_instr(uint64_t addr) override {
        return concrete_load_data<uint32_t>(v2p(addr, FETCH));
    }

#if 0
    int64_t load_double(uint64_t addr) override {
        return _load_data<int64_t>(addr);
    }
	int32_t load_word(uint64_t addr) override {
		return _load_data<int32_t>(addr);
	}
	int32_t load_half(uint64_t addr) override {
		return _load_data<int16_t>(addr);
	}
	int32_t load_byte(uint64_t addr) override {
		return _load_data<int8_t>(addr);
	}
	uint32_t load_uhalf(uint64_t addr) override {
		return _load_data<uint16_t>(addr);
	}
	uint32_t load_ubyte(uint64_t addr) override {
		return _load_data<uint8_t>(addr);
	}

    void store_double(uint64_t addr, uint64_t value) override {
        _store_data(addr, value);
    }
	void store_word(uint64_t addr, uint32_t value) override {
		_store_data(addr, value);
	}
	void store_half(uint64_t addr, uint16_t value) override {
		_store_data(addr, value);
	}
	void store_byte(uint64_t addr, uint8_t value) override {
		_store_data(addr, value);
	}
#endif

	Concolic load_word(Concolic addr) override {
		return symbolic_load_data(addr, sizeof(int32_t))->sext(32);
	}

	Concolic load_half(Concolic addr) override {
		return symbolic_load_data(addr, sizeof(int16_t))->sext(32);
	}

	Concolic load_byte(Concolic addr) override {
		return symbolic_load_data(addr, sizeof(int8_t))->sext(32);
	}

	Concolic load_uhalf(Concolic addr) override {
		auto r = symbolic_load_data(addr, sizeof(uint16_t));
		return (r->getWidth() < 32) ? r->zext(32) : r;
	}

	Concolic load_ubyte(Concolic addr) override {
		auto r = symbolic_load_data(addr, sizeof(uint8_t));
		return (r->getWidth() < 32) ? r->zext(32): r ;
	}

	void store_word(Concolic addr, Concolic value) override {
		symbolic_store_data(addr, value, sizeof(uint32_t));
	}

	void store_half(Concolic addr, Concolic value) override {
		symbolic_store_data(addr, value, sizeof(uint16_t));
	}

	void store_byte(Concolic addr, Concolic value) override {
		symbolic_store_data(addr, value, sizeof(uint8_t));
	}

#if 0
	virtual int32_t atomic_load_word(uint64_t addr) override {
		bus_lock->lock(iss.get_hart_id());
		return load_word(addr);
	}
	virtual void atomic_store_word(uint64_t addr, uint32_t value) override {
		assert(bus_lock->is_locked(iss.get_hart_id()));
		store_word(addr, value);
	}
	virtual int32_t atomic_load_reserved_word(uint64_t addr) override {
		bus_lock->lock(iss.get_hart_id());
		lr_addr = addr;
		return load_word(addr);
	}
	virtual bool atomic_store_conditional_word(uint64_t addr, uint32_t value) override {
		/* According to the RISC-V ISA, an implementation can fail each LR/SC sequence that does not satisfy the forward
		 * progress semantic.
		 * The lock is established by the LR instruction and the lock is kept while forward progress is maintained. */
		if (bus_lock->is_locked(iss.get_hart_id())) {
			if (addr == lr_addr) {
				store_word(addr, value);
				return true;
			}
			atomic_unlock();
		}
		return false;
	}
	virtual void atomic_unlock() override {
		bus_lock->unlock(iss.get_hart_id());
	}
#endif
};

}  // namespace rv32
