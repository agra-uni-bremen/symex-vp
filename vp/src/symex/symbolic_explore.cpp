/*
 * Copyright (c) 2020,2021 Group of Computer Architecture, University of Bremen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef RISCV_ISA_EXPLORATION_H
#define RISCV_ISA_EXPLORATION_H

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

/* Debug leaks with valgrind --leak-check=full --undef-value-errors=no
 * Also: Define valgrind here to prevent spurious Z3 memory leaks. */
#ifdef VALGRIND
#include <z3.h>
#endif

#include <iostream>
#include <systemc>
#include <filesystem>
#include <systemc>

#include <clover/clover.h>
#include "symbolic_explore.h"
#include "symbolic_context.h"

#define TESTCASE_ENV "SYMEX_TESTCASE"
#define TIMEBUDGET_ENV "SYMEX_TIMEBUDGET"
#define ERR_EXIT_ENV "SYMEX_ERREXIT"

static std::filesystem::path *testcase_path = nullptr;
static size_t errors_found = 0;
static size_t paths_found = 0;

static std::chrono::duration<double, std::milli> solver_time;

static void
dump_stats(void)
{
	auto stime = std::chrono::duration_cast<std::chrono::seconds>(solver_time);

	std::cout << std::endl << "---" << std::endl;
	std::cout << "Unique paths found: " << paths_found << std::endl;
	std::cout << "Solver Time: " << stime.count() << " seconds" << std::endl;
	// TODO: Also dump instruction branch coverage here.
	if (errors_found > 0) {
		std::cout << "Errors found: " << errors_found << std::endl;
		std::cout << "Testcase directory: " << *testcase_path << std::endl;
	}
}

static std::optional<std::string>
dump_input(std::string fn)
{
	clover::ExecutionContext &ctx = symbolic_context.ctx;
	clover::ConcreteStore store = ctx.getPrevStore();
	if (store.empty())
		return std::nullopt; // Execution does not depend on symbolic values

	assert(testcase_path);
	auto path = *testcase_path / fn;

	std::ofstream file(path);
	if (!file.is_open())
		throw std::runtime_error("failed to open " + path.string());

	clover::TestCase::toFile(store, file);
	return path;
}

static void
report_handler(const sc_core::sc_report& report, const sc_core::sc_actions& actions)
{
	auto nactions = actions;
	auto mtype = report.get_msg_type();

	if (!strcmp(mtype, "/AGRA/riscv-vp/host-error") || !testcase_path) {
		auto path = dump_input("error" + std::to_string(++errors_found));
		if (!path.has_value())
			return;

		std::cerr << "Found error, use " << *path << " to reproduce." << std::endl;
		if (getenv(ERR_EXIT_ENV)) {
			std::cerr << "Exit on first error set, terminating..." << std::endl;
			exit(EXIT_FAILURE);
		}

		nactions &= ~sc_core::SC_DISPLAY; // Prevent SystemC output
		nactions &= sc_core::SC_STOP;     // Stop SystemC simulation
	}

	// Invoke default handler, even for host-error, to ensure that
	// SC_REPORT_ERROR is handled properly (i.e. execution is stopped).
	sc_core::sc_report_handler::default_handler(report, nactions);
}

static void
sigalrm_handler(int signum)
{
	(void)signum;

	std::cout << "Time budget exceeded, terminating..." << std::endl;

	dump_stats();
	exit(EXIT_SUCCESS);
}

static void
remove_testdir(void)
{
	assert(testcase_path != nullptr);
	if (errors_found > 0)
		return;

	// Remove test directory if no errors were found
	if (rmdir(testcase_path->c_str()) == -1)
		throw std::system_error(errno, std::generic_category());

	delete testcase_path;
	testcase_path = nullptr;
}

static void
create_testdir(void)
{
	char *dirpath;
	char tmpl[] = "/tmp/clover_testsXXXXXX";

	if (!(dirpath = mkdtemp(tmpl)))
		throw std::system_error(errno, std::generic_category());
	testcase_path = new std::filesystem::path(dirpath);

	if (std::atexit(remove_testdir))
		throw std::runtime_error("std::atexit failed");
}

static bool
setupNewValues(clover::ExecutionContext &ctx, clover::Trace &tracer)
{
	auto start = std::chrono::steady_clock::now();
	auto r = ctx.setupNewValues(tracer);
	auto end = std::chrono::steady_clock::now();

	solver_time += end - start;
	return r;
}

static int
run_test(const char *path, int argc, char **argv)
{
	std::string fp(path);
	std::ifstream file(fp);
	if (!file.is_open())
		throw std::runtime_error("failed to open " + fp);

	clover::ExecutionContext &ctx = symbolic_context.ctx;
	clover::ConcreteStore store = clover::TestCase::fromFile(fp, file);

	ctx.setupNewValues(store);
	return sc_core::sc_elab_and_sim(argc, argv);
}

static int
explore_paths(int argc, char **argv)
{
	clover::ExecutionContext &ctx = symbolic_context.ctx;
	clover::Trace &tracer = symbolic_context.trace;

	do {
		std::cout << std::endl << "##" << std::endl << "# "
			<< paths_found + 1 << "th concolic execution" << std::endl
			<< "##" << std::endl;

		tracer.reset();

		// Reset SystemC simulation context
		// See also: https://github.com/accellera-official/systemc/issues/8
		if (sc_core::sc_curr_simcontext) {
			sc_core::sc_report_handler::release();
			delete sc_core::sc_curr_simcontext;
		}
		sc_core::sc_curr_simcontext = NULL;

		int ret;
		if ((ret = sc_core::sc_elab_and_sim(argc, argv)))
			return ret;

		++paths_found;
	} while (setupNewValues(ctx, tracer));

	sc_core::sc_report_handler::release();
	delete sc_core::sc_curr_simcontext;

	return 0;
}

static void
setup_timeout(void)
{
	struct sigaction sa;

	char *timebudget = getenv(TIMEBUDGET_ENV);
	if (!timebudget)
		return;

	errno = 0;
	auto seconds = strtoul(timebudget, NULL, 10);
	if (!seconds && errno)
		throw std::system_error(errno, std::generic_category());

	sa.sa_flags = SA_RESTART;
	sa.sa_handler = sigalrm_handler;
	if (sigemptyset(&sa.sa_mask) == -1)
		throw std::system_error(errno, std::generic_category());
	if (sigaction(SIGALRM, &sa, NULL) == -1)
		throw std::system_error(errno, std::generic_category());

	int r = alarm(seconds);
	assert(r == 0);
	(void)r;
}

int
symbolic_explore(int argc, char **argv)
{
	// Hide SystemC copyright message
	setenv("SYSTEMC_DISABLE_COPYRIGHT_MESSAGE", "1", 0);

	// Mempool does not seem to free all memory, disable it.
	setenv("SYSTEMC_MEMPOOL_DONT_USE", "1", 0);

	// Use current time as seed for random generator
	std::srand(std::time(nullptr));

	char *testcase = getenv(TESTCASE_ENV);
	if (testcase)
		return run_test(testcase, argc, argv);
	create_testdir();

	// Set report handler for detecting errors
	sc_core::sc_report_handler::set_handler(report_handler);

	setup_timeout();
	int ret = explore_paths(argc, argv);
	dump_stats();

#ifdef VALGRIND
	Z3_finalize_memory();
#endif

	return ret;
}

#endif
