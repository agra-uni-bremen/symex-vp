#include "symbolic_context.h"

// We need to pass the SymbolicContext which includes the solver,
// tracer, … to the sc_main method somehow. This cannot be done using
// function paramaters, for this reason a global variable is used
// instead.
SymbolicContext symbolic_context = SymbolicContext();

SymbolicContext::SymbolicContext(void)
	: solver(), trace(solver), ctx(solver)
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
