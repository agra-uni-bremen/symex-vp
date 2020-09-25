#ifndef RISCV_ISA_EXPLORATION_H
#define RISCV_ISA_EXPLORATION_H

#include <stddef.h>
#include <stdlib.h>

#include <iostream>
#include <systemc>

#include "symbolic_context.h"

static int main_argc;
static char **main_argv;

static void
reset_systemc(void)
{
	// Reset SystemC simulation context
	// See also: https://github.com/accellera-official/systemc/issues/8
	if (sc_core::sc_curr_simcontext)
		delete sc_core::sc_curr_simcontext;
	sc_core::sc_curr_simcontext = NULL;
}

static void
report_handler(const sc_core::sc_report& report, const sc_core::sc_actions& actions)
{
	static bool debugging = false;

	auto mtype = report.get_msg_type();
	if (strcmp(mtype, "/AGRA/riscv-vp/host-error") || debugging) {
		sc_core::sc_report_handler::default_handler(report, actions);
		return;
	}

	clover::ExecutionContext &ctx = symbolic_context.ctx;
	std::cout << "Error encountered, restarting in debug mode..." << std::endl;

	reset_systemc();
	ctx.useOldValues();

	// Make sure the debug runner is used in the next run
	setenv("SYMEX_DEBUG", "1", 0);

	// Prevent nested debugging sessions.
	debugging = true;

	// If the ISS would not modify the tracer it would be possible
	// to continue epxloration after debugging. However, since the
	// tracer is modified this doesn't work currently.
	//
	// Furthermore, I believe the best approach would be to store
	// inputs in a file (like klee) thereby allowing debugging.
	std::cout << "Debug interface enabled, attach with GDB" << std::endl;
	exit(sc_core::sc_elab_and_sim(main_argc, main_argv));
}

int
main(int argc, char **argv)
{
	main_argc = argc;
	main_argv = argv;

	clover::ExecutionContext &ctx = symbolic_context.ctx;
	clover::Trace &tracer = symbolic_context.trace;

	// Hide SystemC copyright message
	setenv("SYSTEMC_DISABLE_COPYRIGHT_MESSAGE", "1", 0);

	// Set report handler for detecting errors
	sc_core::sc_report_handler::set_handler(report_handler);

	// Use current time as seed for random generator
	std::srand(std::time(nullptr));

	size_t paths_found = 0;
	do {
		std::cout << std::endl << "##" << std::endl << "# "
			<< ++paths_found << "th concolic execution" << std::endl
			<< "##" << std::endl;

		tracer.reset();
		reset_systemc();

		int ret;
		if ((ret = sc_core::sc_elab_and_sim(argc, argv)))
			return ret;
	} while (ctx.setupNewValues(tracer));

	std::cout << std::endl << "---" << std::endl;
	std::cout << "Unique paths found: " << paths_found << std::endl;

	return 0;
}

#endif
