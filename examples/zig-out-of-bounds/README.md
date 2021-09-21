# zig-out-of-bounds

This example is very similar to `assertion-failure`, but it is written
in a safer programming language called [Zig][zig web] and demonstrates
how safety features of the programming language make it easy to find
errors using `symex-vp`. Furthermore, it illustrates that `symex-vp` is
also able to execute programs written in different programming languages
since it operates on the machine code level.

In this example, `symex-vp` is able to detect an out-of-bounds array
access due to an off-by-one in the performed bounds check. Zig is
presently not a ready for production use, this example application is
known to compile with Zig `0.7.1+e4213c4b08`.

## Usage

Compiling this example requires installing a Zig toolchain. This process
is well documented on the [Zig website][zig install]. Afterwards, simply
run:

	$ zig build

This will generate an executable ELF file in `./zig-cache/bin/main`.
This file can then be executed with `symex-vp` using the following
command:

	$ symex-vp zig-cache/bin/main

## Debugging

Due to the fact that Zig is ABI compatible with C, the code can also be
debugged using the provided GDB stub. Refer to the `assertion-failure`
example for more information on this topic.

[zig web]: https://ziglang.org/
[zig install]: https://ziglang.org/learn/getting-started/#installing-zig
