#ifndef RISCV_ISA_SYMBOLIC_CTX_H
#define RISCV_ISA_SYMBOLIC_CTX_H

#include <clover/clover.h>

class SymbolicContext {
private:
	clover::Solver solver;
	clover::Trace trace;
	clover::ExecutionContext ctx;

public:
	SymbolicContext(clover::Solver &_solver, clover::Trace &_trace, clover::ExecutionContext &_ctx);

	clover::Solver &get_solver(void);
	clover::Trace &get_trace(void);
	clover::ExecutionContext &get_ctx(void);
};

#endif
