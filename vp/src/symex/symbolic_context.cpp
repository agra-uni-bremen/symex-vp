#include "symbolic_context.h"

SymbolicContext::SymbolicContext(clover::Solver &_solver,
                                 clover::Trace &_trace,
                                 clover::ExecutionContext &_ctx)
	: solver(_solver), trace(_trace), ctx(_ctx)
{
	return;
}

clover::Solver &
SymbolicContext::get_solver(void)
{
	return solver;
}

clover::Trace &
SymbolicContext::get_trace(void)
{
	return trace;
}

clover::ExecutionContext &
SymbolicContext::get_ctx(void)
{
	return ctx;
}
