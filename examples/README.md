# Examples

This directory contains very basic examples for using `symex-vp`. These
examples are kept simple intentionally. The following three example
applications are currently provided:

1. `assertion-failure:` Demonstrate declaring a variable as symbolic
   using the `SymbolicCTRL` peripheral and finds a path, based on this
   variable, which causes an assertion failure.
2. `zig-out-of-bounds:` Demonstrate discovery of out-of-bounds accesses
   using a programming language with spatial memory safety features.
   This example also utilizes the SymbolicCTRL` peripheral for
   explicitly declaring a variable as symbolic.
3. `symbolic-sensor`: Demonstrates discovery of paths through a program
   without explicitly declaring a variable as symbolic. Instead,
   symbolic data is retrieved from a exemplary symbolic sensor
   peripheral.

Refer to the `README.md` file in these subdirectories for more information.
