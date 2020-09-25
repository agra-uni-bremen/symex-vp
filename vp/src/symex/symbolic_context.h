#ifndef RISCV_ISA_SYMBOLIC_CTX_H
#define RISCV_ISA_SYMBOLIC_CTX_H

#include <clover/clover.h>

class SymbolicContext {
public:
	clover::Solver solver;
	clover::Trace trace;
	clover::ExecutionContext ctx;

	SymbolicContext(void);
};

extern SymbolicContext symbolic_context;

#endif
