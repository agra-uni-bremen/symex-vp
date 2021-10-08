#ifndef RISCV_VP_SYMBOLIC_FMT_H
#define RISCV_VP_SYMBOLIC_FMT_H

#include <memory>

#include <symbolic_context.h>
#include <stdint.h>
#include <clover/clover.h>

class SymbolicFormat {
private:
	clover::ExecutionContext &ctx;
	clover::Solver &solver;
	int fd;
	size_t numSymField = 0;

	std::shared_ptr<clover::ConcolicValue> next_field(void);

public:
	SymbolicFormat(SymbolicContext &_ctx, std::string path);
	std::shared_ptr<clover::ConcolicValue> get_input(void);
};

#endif
