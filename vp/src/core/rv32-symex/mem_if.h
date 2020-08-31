#pragma once

#include <stdint.h>
#include <clover/clover.h>

namespace rv32 {

struct instr_memory_if {
	virtual ~instr_memory_if() {}

	virtual uint32_t load_instr(uint64_t pc) = 0;
};

//NOTE: load/store double is used for floating point D extension
struct data_memory_if {
	virtual ~data_memory_if() {}

#if 0
    virtual int64_t load_double(uint64_t addr) = 0;
	virtual int32_t load_word(uint64_t addr) = 0;
	virtual int32_t load_half(uint64_t addr) = 0;
	virtual int32_t load_byte(uint64_t addr) = 0;
	virtual uint32_t load_uhalf(uint64_t addr) = 0;
	virtual uint32_t load_ubyte(uint64_t addr) = 0;

    virtual void store_double(uint64_t addr, uint64_t value) = 0;
	virtual void store_word(uint64_t addr, uint32_t value) = 0;
	virtual void store_half(uint64_t addr, uint16_t value) = 0;
	virtual void store_byte(uint64_t addr, uint8_t value) = 0;

	virtual int32_t atomic_load_word(uint64_t addr) = 0;
	virtual void atomic_store_word(uint64_t addr, uint32_t value) = 0;
	virtual int32_t atomic_load_reserved_word(uint64_t addr) = 0;
	virtual bool atomic_store_conditional_word(uint64_t addr, uint32_t value) = 0;
	virtual void atomic_unlock() = 0;
#endif

	typedef std::shared_ptr<clover::ConcolicValue> Concolic;

	virtual void symbolic_store_data(Concolic addr, Concolic data, size_t num_bytes) = 0;
	virtual Concolic symbolic_load_data(Concolic addr, size_t num_bytes) = 0;

	virtual Concolic load_word(Concolic addr) = 0;
	virtual Concolic load_half(Concolic addr) = 0;
	virtual Concolic load_byte(Concolic addr) = 0;
	virtual Concolic load_uhalf(Concolic addr) = 0;
	virtual Concolic load_ubyte(Concolic addr) = 0;

	virtual void store_word(Concolic addr, Concolic value) = 0;
	virtual void store_half(Concolic addr, Concolic value) = 0;
	virtual void store_byte(Concolic addr, Concolic value) = 0;

    virtual void flush_tlb() = 0;
};

}  // namespace rv32
