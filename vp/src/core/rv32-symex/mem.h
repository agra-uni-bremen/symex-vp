#pragma once

#include "core/common/dmi.h"
#include "iss.h"
#include "mmu.h"

#include <clover/clover.h>

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
	clover::Solver &solver;
	clover::ConcolicMemory sym_mem;
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
	    : iss(owner), solver(owner.solver), sym_mem(owner.solver), quantum_keeper(iss.quantum_keeper), mmu(mmu) {
	}

    uint64_t v2p(uint64_t vaddr, MemoryAccessType type) {
	    if (mmu == nullptr)
	        return vaddr;
        return mmu->translate_virtual_to_physical_addr(vaddr, type);
    }

	inline void _do_transaction(tlm::tlm_command cmd, uint64_t addr, uint8_t *data, unsigned num_bytes) {
		tlm::tlm_generic_payload trans;
		trans.set_command(cmd);
		trans.set_address(addr);
		trans.set_data_ptr(data);
		trans.set_data_length(num_bytes);
		trans.set_response_status(tlm::TLM_OK_RESPONSE);

		sc_core::sc_time local_delay = quantum_keeper.get_local_time();

		isock->b_transport(trans, local_delay);

		assert(local_delay >= quantum_keeper.get_local_time());
		quantum_keeper.set(local_delay);

		if (trans.is_response_error()) {
			if (iss.trace)
				std::cout << "WARNING: core memory transaction failed -> raise trap" << std::endl;
			if (cmd == tlm::TLM_READ_COMMAND)
				raise_trap(EXC_LOAD_PAGE_FAULT, addr);
			else if (cmd == tlm::TLM_WRITE_COMMAND)
				raise_trap(EXC_STORE_AMO_PAGE_FAULT, addr);
			else
				throw std::runtime_error("TLM command must be read or write");
		}
	}

	template <typename T>
	inline T _raw_load_data(uint64_t addr) {
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
	inline void _raw_store_data(uint64_t addr, T value) {
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
		atomic_unlock();
	}


    inline Value _load_data(Address addr, size_t size) {
        auto caddr = solver.evalValue<uint32_t>(addr->concrete);
        return sym_mem.load(v2p(caddr, LOAD), size);
    }

    template <typename T>
    inline void _store_data(uint64_t addr, T value) {
        _raw_store_data(v2p(addr, STORE), value);
    }

    uint64_t mmu_load_pte64(uint64_t addr) override {
        return _raw_load_data<uint64_t>(addr);
    }
    uint64_t mmu_load_pte32(uint64_t addr) override {
        return _raw_load_data<uint32_t>(addr);
    }
    void mmu_store_pte32(uint64_t addr, uint32_t value) override {
        _raw_store_data(addr, value);
    }

    void flush_tlb() override {
        mmu->flush_tlb();
    }

    uint32_t load_instr(uint64_t addr) override {
        return _raw_load_data<uint32_t>(v2p(addr, FETCH));
    }

	Value load_double(Address addr) override {
		return _load_data(addr, sizeof(int64_t));
	}
	Value load_word(Address addr) override {
		return _load_data(addr, sizeof(int32_t));
	}
	Value load_half(Address addr) override {
		return _load_data(addr, sizeof(int16_t));
	}
	Value load_byte(Address addr) override {
		return _load_data(addr, sizeof(int8_t));
	}
	Value load_uhalf(Address addr) override {
		return _load_data(addr, sizeof(uint16_t));
	}
	Value load_ubyte(Address addr) override {
		return _load_data(addr, sizeof(uint8_t));
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

	virtual int32_t atomic_load_word(uint64_t addr) override {
		throw std::string(__func__) + " not implemented";
	}
	virtual void atomic_store_word(uint64_t addr, uint32_t value) override {
		throw std::string(__func__) + " not implemented";
	}
	virtual int32_t atomic_load_reserved_word(uint64_t addr) override {
		throw std::string(__func__) + " not implemented";
	}
	virtual bool atomic_store_conditional_word(uint64_t addr, uint32_t value) override {
		throw std::string(__func__) + " not implemented";
	}
	virtual void atomic_unlock() override {
		throw std::string(__func__) + " not implemented";
	}
};

}  // namespace rv32
