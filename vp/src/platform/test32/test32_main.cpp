#include <cstdlib>
#include <ctime>

#include "core/common/clint.h"
#include "elf_loader.h"
#include "debug_memory.h"
#include "iss.h"
#include "mem.h"
#include "memory.h"
#include "syscall.h"

#include "gdb-mc/gdb_server.h"
#include "gdb-mc/gdb_runner.h"

#include "htif.h"

#include <boost/io/ios_state.hpp>
#include <boost/program_options.hpp>
#include <iomanip>
#include <iostream>
#include <fstream>

using namespace rv32;

struct Options {
    typedef unsigned int addr_t;

    Options &check_and_post_process() {
        mem_end_addr = mem_start_addr + mem_size - 1;
        return *this;
    }

    std::string input_program;
    std::string test_signature;
    std::string isa;
    unsigned int max_test_instrs = 1000000;

    addr_t mem_size = 1024 * 1024 * 32;  // 32 MB ram, to place it before the CLINT and run the base examples (assume
    // memory start at zero) without modifications
    addr_t mem_start_addr = 0x00000000;
    addr_t mem_end_addr = mem_start_addr + mem_size - 1;
    addr_t clint_start_addr = 0x02000000;
    addr_t clint_end_addr = 0x0200ffff;
    addr_t sys_start_addr = 0x02010000;
    addr_t sys_end_addr = 0x020103ff;

    bool use_debug_runner = false;
    bool use_instr_dmi = false;
    bool use_data_dmi = false;
    bool trace_mode = false;
    bool intercept_syscalls = false;
    bool use_E_base_isa = false;
    unsigned int debug_port = 5005;

    unsigned int tlm_global_quantum = 10;
};

Options parse_command_line_arguments(int argc, char **argv) {
    // Note: first check for *help* argument then run *notify*, see:
    // https://stackoverflow.com/questions/5395503/required-and-optional-arguments-using-boost-library-program-options
    try {
        Options opt;

        namespace po = boost::program_options;

        po::options_description desc("Options");

        desc.add_options()
                ("help", "produce help message")
                ("memory-start", po::value<unsigned int>(&opt.mem_start_addr),"set memory start address")
                ("memory-size", po::value<unsigned int>(&opt.mem_size), "set memory size")
                ("use-E-base-isa", po::bool_switch(&opt.use_E_base_isa), "use the E instead of the I integer base ISA")
                ("intercept-syscalls", po::bool_switch(&opt.intercept_syscalls),"directly intercept and handle syscalls in the ISS")
                ("debug-mode", po::bool_switch(&opt.use_debug_runner),"start execution in debugger (using gdb rsp interface)")
                ("debug-port", po::value<unsigned int>(&opt.debug_port), "select port number to connect with GDB")
                ("trace-mode", po::bool_switch(&opt.trace_mode), "enable instruction tracing")
                ("tlm-global-quantum", po::value<unsigned int>(&opt.tlm_global_quantum), "set global tlm quantum (in NS)")
                ("max-instrs", po::value<unsigned int>(&opt.max_test_instrs), "maximum number of instructions to execute (soft limit, checked periodically)")
                ("use-instr-dmi", po::bool_switch(&opt.use_instr_dmi), "use dmi to fetch instructions")
                ("use-data-dmi", po::bool_switch(&opt.use_data_dmi), "use dmi to execute load/store operations")
                ("use-dmi", po::bool_switch(), "use instr and data dmi")
                ("signature", po::value<std::string>(&opt.test_signature)->default_value(""), "output filename for the test execution signature")
                ("isa", po::value<std::string>(&opt.isa)->default_value("imacfnus"), "output filename for the test execution signature")
                ("input-file", po::value<std::string>(&opt.input_program)->required(), "input file to use for execution");

        po::positional_options_description pos;
        pos.add("input-file", 1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(pos).run(), vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            exit(0);
        }

        po::notify(vm);

        if (vm["use-dmi"].as<bool>()) {
            opt.use_data_dmi = true;
            opt.use_instr_dmi = true;
        }

        return opt.check_and_post_process();
    } catch (boost::program_options::error &e) {
        std::cerr << "Error parsing command line options: " << e.what() << std::endl;
        exit(-1);
    }
}

void dump_test_signature(Options &opt, uint8_t *mem, ELFLoader &loader) {
    auto begin_sig = loader.get_begin_signature_address();
    auto end_sig = loader.get_end_signature_address();

    {
        boost::io::ios_flags_saver ifs(std::cout);
        std::cout << std::hex;
        std::cout << "begin_signature: " << begin_sig << std::endl;
        std::cout << "end_signature: " << end_sig << std::endl;
        std::cout << "signature output file: " << opt.test_signature << std::endl;
    }

    assert (end_sig >= begin_sig);
    assert (begin_sig >= opt.mem_start_addr);

    auto begin = begin_sig - opt.mem_start_addr;
    auto end = end_sig - opt.mem_start_addr;

    std::ofstream sigfile(opt.test_signature, std::ios::out);

    auto n = begin;
    assert (n % 4 == 0);
    while (n < end) {
        uint32_t *p = ((uint32_t*)&mem[n]);
        assert ((uintptr_t)p % 4 == 0);
        sigfile << std::hex << std::setw(8) << std::setfill('0') << *p << std::endl;
        n += 4;
    }
}

int sc_main(int argc, char **argv) {
    Options opt = parse_command_line_arguments(argc, argv);

    std::srand(std::time(nullptr));  // use current time as seed for random generator

    tlm::tlm_global_quantum::instance().set(sc_core::sc_time(opt.tlm_global_quantum, sc_core::SC_NS));

    ISS core(0, opt.use_E_base_isa);
    MMU mmu(core);
    CombinedMemoryInterface core_mem_if("MemoryInterface0", core, &mmu);
    SimpleMemory mem("SimpleMemory", opt.mem_size);
    ELFLoader loader(opt.input_program.c_str());
    SimpleBus<2, 3> bus("SimpleBus");
    SyscallHandler sys("SyscallHandler");
    CLINT<1> clint("CLINT");
    DebugMemoryInterface dbg_if("DebugMemoryInterface");

    MemoryDMI dmi = MemoryDMI::create_start_size_mapping(mem.data, opt.mem_start_addr, mem.size);
    InstrMemoryProxy instr_mem(dmi, core);

    std::shared_ptr<BusLock> bus_lock = std::make_shared<BusLock>();
    core_mem_if.bus_lock = bus_lock;

    instr_memory_if *instr_mem_if = &core_mem_if;
    data_memory_if *data_mem_if = &core_mem_if;
    if (opt.use_instr_dmi)
        instr_mem_if = &instr_mem;
    if (opt.use_data_dmi) {
        core_mem_if.dmi_ranges.emplace_back(dmi);
    }

    loader.load_executable_image(mem.data, mem.size, opt.mem_start_addr);
    core.init(instr_mem_if, data_mem_if, &clint, loader.get_entrypoint(), rv32_align_address(opt.mem_end_addr));
    sys.init(mem.data, opt.mem_start_addr, loader.get_heap_addr());
    sys.register_core(&core);

    if (opt.intercept_syscalls)
        core.sys = &sys;

    // setup port mapping
    bus.ports[0] = new PortMapping(opt.mem_start_addr, opt.mem_end_addr);
    bus.ports[1] = new PortMapping(opt.clint_start_addr, opt.clint_end_addr);
    bus.ports[2] = new PortMapping(opt.sys_start_addr, opt.sys_end_addr);

    // connect TLM sockets
    core_mem_if.isock.bind(bus.tsocks[0]);
    dbg_if.isock.bind(bus.tsocks[1]);
    bus.isocks[0].bind(mem.tsock);
    bus.isocks[1].bind(clint.tsock);
    bus.isocks[2].bind(sys.tsock);

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

    {
        auto addr = loader.get_to_host_address();
        uint8_t *p = &(mem.data[addr - opt.mem_start_addr]);
        assert (((uintptr_t)p) % 8 == 0);  // correct alignment for uint64_t
        auto to_host_callback = [&core](uint64_t x) {
            if (x == 1) {
                //NOTE: the "scall" benchmark (RISC-V compliance) still requires HTIF support for successful completion ...
                core.sys_exit();
            } else {
                if (x != 0) {
                    std::cout << "to-host: " << std::to_string(x) << std::endl;
                    core.sys_exit();
                }
                //throw std::runtime_error("to-host: " + std::to_string(x));
            }
        };
        new HTIF("HTIF", (uint64_t*)p, &core.total_num_instr, opt.max_test_instrs, to_host_callback);
    }

    {
        std::transform(opt.isa.begin(), opt.isa.end(), opt.isa.begin(), ::toupper);
        core.csrs.misa.extensions = core.csrs.misa.I;
        if (opt.isa.find('G') != std::string::npos)
            core.csrs.misa.extensions |= core.csrs.misa.M | core.csrs.misa.A | core.csrs.misa.F | core.csrs.misa.D;
        if (opt.isa.find('M') != std::string::npos)
            core.csrs.misa.extensions |= core.csrs.misa.M;
        if (opt.isa.find('A') != std::string::npos)
            core.csrs.misa.extensions |= core.csrs.misa.A;
        if (opt.isa.find('C') != std::string::npos)
            core.csrs.misa.extensions |= core.csrs.misa.C;
        if (opt.isa.find('F') != std::string::npos)
            core.csrs.misa.extensions |= core.csrs.misa.F;
        if (opt.isa.find('D') != std::string::npos)
            core.csrs.misa.extensions |= core.csrs.misa.D;
        if (opt.isa.find('N') != std::string::npos)
            core.csrs.misa.extensions |= core.csrs.misa.N;
        if (opt.isa.find('U') != std::string::npos)
            core.csrs.misa.extensions |= core.csrs.misa.U;
        if (opt.isa.find('S') != std::string::npos)
            core.csrs.misa.extensions |= core.csrs.misa.S | core.csrs.misa.U; // NOTE: S mode implies U mode
    }

    sc_core::sc_start();

    core.show();

    if (!opt.test_signature.empty()) {
        dump_test_signature(opt, mem.data, loader);
    }

    return 0;
}
