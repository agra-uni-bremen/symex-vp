# symex-vp

A virtual prototype based on [riscv-vp][riscv-vp github] with [symbolic execution][wikipedia symex] support.

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

Consider the following assembly example program:

	.globl _start
	_start:

	/* Make word (4 bytes) at addr 0x0 symbolic */
	li a7, 96 /* SYS_sym_mem */
	li a0, 0  /* address 0x0 */
	li a1, 4  /* 4 bytes */
	ecall     /* execute make_symbolic ecall */

	/* Load symbolic memory word into register a1 */
	lw   a1, 0(t0)

	li a2, 23
	blt  a2, a1, true
	addi a3, a3, 1
	true:
	addi a3, a3, 2

	li a7, 93 /* SYS_exit */
	ecall     /* Signal end of execution */

This example program contains a single branch instruction (`BLT`). The
program can be compiled using:

	$ riscv32-unknown-elf-gcc -o main main.S -march=rv32i -mabi=ilp32 -nostartfiles

Afterwards, execute it using:

	$ symex-vp --intercept-syscalls main

The `symex-vp` will discover two paths through this program. One where
`BLT` jumps to `true` and one where it doesn't. The value of the `a3`
register will differ in these two executions.

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

## Design

This software allows symbolic execution of software compiled for RV32.
Registers or memory values can be marked symbolic explicitly using an
intercepted `ECALL` instruction. Branches based on symbolic values are
tracked and as soon as execution terminates a new assignment for
symbolic variables are determined which discovers new paths for the
program by negating encountered branches. For each new assignment, the
SystemC simulation is restarted from the beginning. After all
encountered branches have been negated, the virtual prototype
terminates.

Errors are signaled by the executed software through a custom `ECALL`.
This `ECALL` can, for instance, be used in software `panic` handlers.
For each path causing an invocation of this `ECALL` a test file with
concrete input values is created. This test file can be replayed by
pointing the `SYMEX_TESTCASE` environment variable to the corresponding
test case file. It is also possible to terminate execution upon
encountering the first error using the `SYMEX_ERREXIT` environment
variable.

## How To Cite

The concepts behind `symex-vp` are further described in the following
publication which is about to be published as part of the
[DATE 2021][date conference] proceedings:

	@inproceedings{tempel2021systemc,
		author    = {Tempel, Sören and Herdt, Vladimir and Drechsler, Rolf},
		title     = {{An Effective Methodology for Integrating Concolic Testing with SystemC-based Virtual Prototypes}},
		booktitle = {Design, Automation and Test in Europe Conference (DATE). Design, Automation \& Test in Europe (DATE-2021)},
		year      = {2021},
		month     = feb,
		location  = {Grenoble, France}
	}

Sense the proceedings have not been published yet, you can find a
[PDF](https://www.informatik.uni-bremen.de/agra/doc/konf/DATE-2021_Concolic_Testing_with_SystemC_based_VPs.pdf)
on our website in the meantime.

## Acknowledgements

This work was supported in part by the German Federal Ministry of
Education and Research (BMBF) within the project Scale4Edge under
contract no.~16ME0127 and within the project VerSys under contract
no.~01IW19001.

[riscv-vp github]: https://github.com/agra-uni-bremen/riscv-vp
[wikipedia symex]: https://en.wikipedia.org/wiki/Symbolic_execution
[z3 repo]: https://github.com/Z3Prover/z3
[llvm website]: https://llvm.org/
[cmake website]: https://cmake.org/
[boost website]: https://www.boost.org/
[sifive hifive1]: https://www.sifive.com/boards/hifive1
[riot website]: https://riot-os.org/
[zephyr website]: https://riot-os.org/
[riscv-compliance github]: https://github.com/riscv/riscv-compliance/
[date conference]: https://www.date-conference.com/
