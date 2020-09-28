#ifndef RISCV_ISA_EXPLORATION_H
#define RISCV_ISA_EXPLORATION_H

#include <stddef.h>
#include <stdlib.h>

#include <iostream>
#include <systemc>
#include <filesystem>
#include <systemc>

#include <clover/clover.h>
#include "symbolic_explore.h"
#include "symbolic_context.h"

static std::filesystem::path testcase_path;
static size_t errors_found = 0;

static void
report_handler(const sc_core::sc_report& report, const sc_core::sc_actions& actions)
{
	auto mtype = report.get_msg_type();
	if (strcmp(mtype, "/AGRA/riscv-vp/host-error")) {
		sc_core::sc_report_handler::default_handler(report, actions);
		return;
	}

	clover::ExecutionContext &ctx = symbolic_context.ctx;
	clover::ConcreteStore store = ctx.getPrevStore();

	auto path = testcase_path / ("error" + std::to_string(++errors_found));
	std::ofstream file(path);
	if (!file.is_open())
		throw std::runtime_error("failed to open " + path.string());

	clover::TestCase::toFile(store, file);
}

static void
create_testdir(void)
{
	char *dirpath;
	char tmpl[] = "/tmp/clover_testsXXXXXX";

	if (!(dirpath = mkdtemp(tmpl)))
		throw std::system_error(errno, std::generic_category());
	testcase_path = std::filesystem::path(dirpath);
}

int
symbolic_explore(int argc, char **argv)
{
	create_testdir();

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

	if (errors_found > 0) {
		std::cout << "Errors found: " << errors_found << std::endl;
		std::cout << "Testcase directory: " << testcase_path << std::endl;
	}

	return 0;
}

#endif
