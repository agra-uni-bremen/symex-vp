#ifndef RISCV_ISA_SYMBOLIC_CTX_H
#define RISCV_ISA_SYMBOLIC_CTX_H

#include <clover/clover.h>

class SymbolicContext {
private:
	clover::Solver solver;
	clover::Trace trace;
	clover::ExecutionContext ctx;

public:
	SymbolicContext(void);

	clover::Solver &get_solver(void);
	clover::Trace &get_trace(void);
	clover::ExecutionContext &get_ctx(void);
};

extern SymbolicContext symbolic_context;

#endif
