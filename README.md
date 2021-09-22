# symex-vp

A RISC-V RV32 virtual prototype based on [riscv-vp][riscv-vp github] with [symbolic execution][wikipedia symex] support.

## About

This software allows symbolic execution (or more specifically [concolic
testing][wikipedia ct]) of RISC-V RV32 machine code. Symbolic variables
can be introduced through modeled hardware peripherals. Branches based
on introduced symbolic values are tracked and as soon as execution
terminates new assignments for symbolic variables are determined by
negating encountered branch conditions (Dynamic Symbolic Execution). For
each new assignment, the software [simulation is restarted][systemc restart]
from the beginning, thereby (ideally) enabling exploration of all paths
through the program based on the introduced symbolic variables.

## Cloning

This repository is a mirror of a private GitLab repository. Used
submodules point to other private repositories which are also mirrored
on GitHub. In order to rewrite the submodule URLs properly, clone the
repository as follows:

	$ git clone https://github.com/agra-uni-bremen/symex-vp
	$ git config --global url."https://github.com/agra-uni-bremen/".insteadOf "git@gitlab.informatik.uni-bremen.de:riscv/"
	$ git submodule update --init

## Installation

This software has the following dependencies:

* A C++ compiler toolchain with C++17 support
* [CMake][cmake website]
* A recent version of [Z3][z3 repo] (`4.8.X` is known to work)
* [LLVM][llvm website]
* [Boost][boost website]

After all dependencies have been installed run:

	$ make

Executable binaries are then available in `./vp/build/bin`.

## Dockerfile

To ease installation a `Dockerfile` is provided. To build a docker
image from this provided `Dockerfile` use the following command:

	$ docker build -t riscv-symex .

Afterwards, run the docker image using:

	$ docker run --rm -it riscv-symex

Executable binaries are available in `/home/riscv-vp/riscv-vp/vp/build/bin`.

## Usage

In regards to using `symex-vp` for software execution it behaves like a
normal virtual prototype and should be able to execute any `RV32IMC`
binaries. In order to utilize the symbolic execution features provided
by `symex-vp`, the following additional aspects have to be consider for
software testing. Communication between software and the virtual
prototype is achieved through memory-mapped IO with a provided
`SymbolicCTRL` peripheral.

1. **Symbolic Inputs:** In order to explore different paths through the
   program, inputs based on which path exploration should take place
   need to be marked as symbolic. Symbolic variables can be introduced
   through SystemC peripherals which return symbolic variables through
   a TLM 2.0 extension as part of memory-mapped I/O. For example, it is
   possible to model an Ethernet peripheral using SystemC which returns
   symbolic variables for network packets and thereby allows exploring
   paths through the network stack of the executed software.
   Alternatively, it is also possible to declare variables as symbolic
   manually through the aforementioned `SymbolicCTRL` peripheral.
2. **Termination Points:** Since `symex-vp` restarts the entire SystemC
   simulation for each new assignment of symbolic input variables,
   termination points need to be defined for the simulated software. For
   example, when exploring the network stack of the executed software,
   it may be sufficient to terminate software simulation as soon as the
   symbolic network packet was handled by the network stack. Termination
   points must presently be declared by the executed software by writing
   to a control register in the memory-mapped `SymbolicCTRL` peripheral.
3. **Path Analyzers:** In order to find errors using symbolic execution,
   a so-called path analyzer needs to be employed on each executed path.
   For each executed path, the employed path analyzer needs to decide if
   the path constitutes an error case. For example, errors may include
   violation of [spatial memory safety][dac checkedc],
   [stack overflows][fdl stack], et cetera. Presently, we mostly rely on
   the executed software to signal an error condition to the virtual
   prototype using the `SymbolicCTRL` peripheral. For example, this
   allows signaling errors from software-specific panic handlers.

## Usage Examples

Usage examples which demonstrate the three aspects mentioned in the
previous section can be found in the `./examples` subdirectory. For
instance, the `./examples/zig-out-of-bounds` demonstrates the discovery
of out-of-bounds array accesses using `symex-vp`. More information on
individual example applications is available in the `README.md` file in
the `./examples` subdirectory.

## Provided VPs

The following virtual prototypes are available:

* `symex-vp`: A very minimal virtual prototype, based on `tiny32-vp`
  from the original riscv-vp repository. This allows testing very basic
  bare-metal RV32 software.
* `hifive-vp`: A virtual prototype mostly compatible with the
  [SiFive HiFive1][sifive hifive1]. This allows executing software
  for embedded operating systems like [RIOT][riot website] or
  [zephyr][zephyr website] symbolically.
* `test32-vp`: This virtual prototype is intended to be used with
  the [riscv-compliance][riscv-compliance github] repository. This is
  primarily useful for development (e.g. during testing of new
  RISC-V extensions).

## Environment Variables

The following environment variables can be set:

* **SYMEX_ERREXIT:** If set, terminate after encountering the first
  error (see below) during symbolic execution of the software.
* **SYMEX_TIMEOUT:** This can be used to configure the solver timeout
  of the employed SMT solver, by default no timeout is used.
* **SYMEX_TESTCASE:** This environment variable can point to a test case
  file for replaying inputs causing an error. This is most useful in
  conjunction with `--debug-mode`.

## How To Cite

The concepts behind `symex-vp` are further described in the following
[publication][symex-vp paper] which was published as part of the
[DATE 2021][date conference] proceedings:

	@inproceedings{tempel2021systemc,
		author    = {Tempel, Sören and Herdt, Vladimir and Drechsler, Rolf},
		booktitle = {2021 Design, Automation \& Test in Europe Conference \& Exhibition (DATE)},
		title     = {An Effective Methodology for Integrating Concolic Testing with SystemC-based Virtual Prototypes},
		year      = {2021},
		pages     = {218-221},
		month     = feb,
		location  = {Grenoble, France},
		doi       = {10.23919/DATE51398.2021.9474149}
	}

## Acknowledgements

This work was supported in part by the German Federal Ministry of
Education and Research (BMBF) within the project Scale4Edge under
contract no. 16ME0127 and within the project VerSys under contract
no. 01IW19001.

## License

The original riscv-vp code is licensed under MIT (see `LICENSE.MIT`).
All modifications made for the integration of symbolic execution with
riscv-vp are licensed under GPLv3+ (see `LICENSE.GPL`). Consult the
copyright headers of individual files for more information.

[riscv-vp github]: https://github.com/agra-uni-bremen/riscv-vp
[wikipedia symex]: https://en.wikipedia.org/wiki/Symbolic_execution
[wikipedia ct]: https://en.wikipedia.org/wiki/Concolic_testing
[z3 repo]: https://github.com/Z3Prover/z3
[llvm website]: https://llvm.org/
[cmake website]: https://cmake.org/
[boost website]: https://www.boost.org/
[sifive hifive1]: https://www.sifive.com/boards/hifive1
[riot website]: https://riot-os.org/
[zephyr website]: https://riot-os.org/
[riscv-compliance github]: https://github.com/riscv/riscv-compliance/
[date conference]: https://www.date-conference.com/
[symex-vp paper]: https://ieeexplore.ieee.org/document/9474149
[systemc restart]: https://github.com/accellera-official/systemc/issues/8
[dac checkedc]: https://www.informatik.uni-bremen.de/agra/doc/konf/DAC-2021-CheckedC-Concolic-Testing.pdf
[fdl stack]: https://www.informatik.uni-bremen.de/agra/doc/konf/FDL21_VP_Stacksize.pdf
