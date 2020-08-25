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
	typedef std::shared_ptr<clover::ConcolicValue> Address;
	typedef std::shared_ptr<clover::ConcolicValue> Value;

	virtual ~data_memory_if() {}

	virtual Value load_double(Address addr) = 0;
	virtual Value load_word(Address addr) = 0;
	virtual Value load_half(Address addr) = 0;
	virtual Value load_byte(Address addr) = 0;
	virtual Value load_uhalf(Address addr) = 0;
	virtual Value load_ubyte(Address addr) = 0;

	virtual void _store_data(Address addr, Value value, size_t size) = 0;

	virtual void store_double(Address addr, Value value) = 0;
	virtual void store_word(Address addr, Value value) = 0;
	virtual void store_half(Address addr, Value value) = 0;
	virtual void store_byte(Address addr, Value value) = 0;

	virtual int32_t atomic_load_word(uint64_t addr) = 0;
	virtual void atomic_store_word(uint64_t addr, uint32_t value) = 0;
	virtual int32_t atomic_load_reserved_word(uint64_t addr) = 0;
	virtual bool atomic_store_conditional_word(uint64_t addr, uint32_t value) = 0;
	virtual void atomic_unlock() = 0;

    virtual void flush_tlb() = 0;
};

}  // namespace rv32
