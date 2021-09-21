# assertion-failure

Example application illustrating replaying of error inputs.

## Usage

First, the application needs to be compiled. For doing so, a RV32I cross
toolchain is required. This toolchain can, for instance, be obtained
using [riscv-gnu-toolchain][riscv-gnu-toolchain github]. Afterwards,
run the following command:

	$ make

This will create an executable `./main` binary in the current directory.
This binary can then be executed using the `symex-vp` Virtual Prototype
(VP). If `symex-vp` is in your `$PATH`, simply run:

	$ make sim

## Results

Running `make sim` should result in the discovery of three paths through
the program. That is, the output will end with the following
information:

	<........>
	
	---
	Unique paths found: 3
	Errors found: 1
	Testcase directory: "/tmp/clover_testseIFaOe

On one of the executed paths, `symex-vp` found an error. An error
condition can, for instance, be signaled from the software to the VP on
assertion failures. A file in the testcase directory contains the
assignment of symbolic values, needed to reproduce the error.

## Debugging

In order to debug the error condition we can reply
the input which triggers this error and debug it further using the GDB
stub provided by `symex-vp` using:

	$ SYMEX_TESTCASE=/tmp/clover_testseIFaOe/error1 symex-vp --debug-mode main

In a separate terminal window we can then connect to the started GDB
server using a gdb version with RISC-V support (gdb-multiarch in this
example):

	$ gdb-multiarch main
	(gdb) target remote :5005
	Remote debugging using :5005
	_start () at bootstrap.S:14
	14      jal main
	(gdb) break main
	Breakpoint 1 at 0x10080: file main.c, line 15.
	(gdb) cont
	Continuing.

	Breakpoint 1, main () at main.c:15
	15              make_symbolic(&a, sizeof(a));
	(gdb) n
	16              if (a % 2 == 0)
	(gdb) n
	17                      MY_ASSERT(a != 42);
	(gdb) p a
	$1 = 42
	(gdb) n
	Remote connection closed

From the gdb output, we can deduce that this path constitute an error because
our assertion fails and a path through the program where `a` takes the value
`42` was found by the symbolic execution engine.

[riscv-gnu-toolchain github]: https://github.com/riscv-collab/riscv-gnu-toolchain
