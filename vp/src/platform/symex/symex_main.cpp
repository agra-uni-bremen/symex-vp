#include <cstdlib>
#include <ctime>

#include "core/common/clint.h"
#include "elf_loader.h"
#include "debug_memory.h"
#include "iss.h"
#include "mem.h"
#include "memory.h"
#include "symbolic_memory.h"
#include "syscall.h"
#include "platform/common/options.h"

#include "gdb-mc/gdb_server.h"
#include "gdb-mc/gdb_runner.h"

#include <clover/clover.h>
#include <boost/io/ios_state.hpp>
#include <boost/program_options.hpp>
#include <iomanip>
#include <iostream>

static clover::Solver *sim_solver = NULL;
static clover::Trace *sim_tracer = NULL;
static clover::ExecutionContext *sim_ctx = NULL;

using namespace rv32;
namespace po = boost::program_options;

struct SymexOptions : public Options {
public:
	typedef unsigned int addr_t;

	addr_t dram_size = 1024 * 16;  // 16 KB dram
	addr_t dram_start_addr = 0x80000000;
	addr_t dram_end_addr = dram_start_addr + dram_size - 1;

	addr_t flash_size = 1024 * 1024 * 512;  // 512 MB flash
	addr_t flash_start_addr = 0x00000000;
	addr_t flash_end_addr = flash_start_addr + flash_size - 1;

	addr_t clint_start_addr = 0x02000000;
	addr_t clint_end_addr = 0x0200ffff;
	addr_t sys_start_addr = 0x02010000;
	addr_t sys_end_addr = 0x020103ff;
};

int
run_simulation(clover::Solver *solver, clover::Trace *tracer, clover::ExecutionContext *ctx, int argc, char **argv)
{
	int ret;

	sim_solver = solver;
	sim_tracer = tracer;
	sim_ctx = ctx;

	size_t paths_found = 0;
	do {
		printf("\n##\n# %zuth concolic execution\n##\n", ++paths_found);

		// TODO: Reset memory too.
		sim_tracer->reset();

		// Reset SystemC simulation context
		if (sc_core::sc_curr_simcontext)
			delete sc_core::sc_curr_simcontext;
		sc_core::sc_curr_simcontext = NULL;

		if ((ret = sc_core::sc_elab_and_sim(argc, argv)))
			return ret;
	} while (ctx->hasNewPath(*sim_tracer));

	printf("\n---\nUnique paths found: %zu\n", paths_found);
	return 0;
}

int
main(int argc, char **argv)
{
	clover::Solver solver;
	clover::Trace tracer(solver);
	clover::ExecutionContext ctx(solver);

	// Hide SystemC copyright message
	setenv("SYSTEMC_DISABLE_COPYRIGHT_MESSAGE", "1", 0);

	return run_simulation(&solver, &tracer, &ctx, argc, argv);
}

int sc_main(int argc, char **argv) {
	SymexOptions opt;
	opt.parse(argc, argv);

	std::srand(std::time(nullptr));  // use current time as seed for random generator

	tlm::tlm_global_quantum::instance().set(sc_core::sc_time(opt.tlm_global_quantum, sc_core::SC_NS));

	ISS core(*sim_solver, *sim_ctx, *sim_tracer, 0);
	MMU mmu(core);
	CombinedMemoryInterface core_mem_if("MemoryInterface0", core, &mmu);
	SimpleMemory dram("DRAM", opt.dram_size);
	SimpleMemory flash("Flash", opt.flash_size);
	ELFLoader loader(opt.input_program.c_str());
	SimpleBus<2, 4> bus("SimpleBus");
	SyscallHandler sys("SyscallHandler");
	CLINT<1> clint("CLINT");
	DebugMemoryInterface dbg_if("DebugMemoryInterface");

	MemoryDMI dram_dmi = MemoryDMI::create_start_size_mapping(dram.data, opt.dram_start_addr, dram.size);
	MemoryDMI flash_dmi = MemoryDMI::create_start_size_mapping(flash.data, opt.flash_start_addr, flash.size);
	InstrMemoryProxy instr_mem(flash_dmi, core);

	std::shared_ptr<BusLock> bus_lock = std::make_shared<BusLock>();
	core_mem_if.bus_lock = bus_lock;

	instr_memory_if *instr_mem_if = &core_mem_if;
	data_memory_if *data_mem_if = &core_mem_if;
	if (opt.use_instr_dmi)
		instr_mem_if = &instr_mem;
	if (opt.use_data_dmi) {
		core_mem_if.dmi_ranges.emplace_back(dram_dmi);
	}

	loader.load_executable_image(flash.data, flash.size, opt.flash_start_addr);
	core.init(instr_mem_if, data_mem_if, &clint, loader.get_entrypoint(), rv32_align_address(opt.dram_end_addr));
	sys.init(dram.data, opt.dram_start_addr, loader.get_heap_addr());
	sys.register_core(&core);

	if (opt.intercept_syscalls)
		core.sys = &sys;

	// setup port mapping
	bus.ports[0] = new PortMapping(opt.flash_start_addr, opt.flash_end_addr);
	bus.ports[1] = new PortMapping(opt.dram_start_addr, opt.dram_end_addr);
	bus.ports[2] = new PortMapping(opt.clint_start_addr, opt.clint_end_addr);
	bus.ports[3] = new PortMapping(opt.sys_start_addr, opt.sys_end_addr);

	// connect TLM sockets
	core_mem_if.isock.bind(bus.tsocks[0]);
	dbg_if.isock.bind(bus.tsocks[1]);
	bus.isocks[0].bind(flash.tsock);
	bus.isocks[1].bind(dram.tsock);
	bus.isocks[2].bind(clint.tsock);
	bus.isocks[3].bind(sys.tsock);

	// connect interrupt signals/communication
	clint.target_harts[0] = &core;

	// switch for printing instructions
	core.trace = opt.trace_mode;

	std::vector<debug_target_if *> threads;
	threads.push_back(&core);

	if (opt.use_debug_runner) {
		auto server = new GDBServer("GDBServer", threads, &dbg_if, opt.debug_port);
		new GDBServerRunner("GDBRunner", server, &core);
	} else {
		new DirectCoreRunner(core);
	}

	sc_core::sc_start();
	core.show();

	return 0;
}
