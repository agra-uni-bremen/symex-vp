#ifndef RISCV_ISA_EXPLORATION_H
#define RISCV_ISA_EXPLORATION_H

#include <stddef.h>
#include <stdlib.h>

#include <iostream>
#include <systemc>

#include "symbolic_context.h"

int
main(int argc, char **argv)
{
	size_t paths_found = 0;

	clover::ExecutionContext &ctx = symbolic_context.ctx;
	clover::Trace &tracer = symbolic_context.trace;

	// Hide SystemC copyright message
	setenv("SYSTEMC_DISABLE_COPYRIGHT_MESSAGE", "1", 0);

	// use current time as seed for random generator
	std::srand(std::time(nullptr));

	do {
		std::cout << std::endl << "##" << std::endl << "# "
			<< ++paths_found << "th concolic execution" << std::endl
			<< "##" << std::endl;

		tracer.reset();

		// Reset SystemC simulation context
		// See also: https://github.com/accellera-official/systemc/issues/8
		if (sc_core::sc_curr_simcontext)
			delete sc_core::sc_curr_simcontext;
		sc_core::sc_curr_simcontext = NULL;

		int ret;
		if ((ret = sc_core::sc_elab_and_sim(argc, argv)))
			return ret;
	} while (ctx.setupNewValues(tracer));

	std::cout << std::endl << "---" << std::endl;
	std::cout << "Unique paths found: " << paths_found << std::endl;

	return 0;
}

#endif
