/*
 * Copyright (c) 2017-2018 Group of Computer Architecture, University of Bremen <riscv@systemc-verification.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef LIBGDB_RESPONSE_H
#define LIBGDB_RESPONSE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Copied from QEMU
 * See: https://github.com/qemu/qemu/blob/d37147997/gdbstub.c#L103-L113 */
typedef enum {
	GDB_SIGNAL_0 = 0,
	GDB_SIGNAL_INT = 2,
	GDB_SIGNAL_QUIT = 3,
	GDB_SIGNAL_TRAP = 5,
	GDB_SIGNAL_ABRT = 6,
	GDB_SIGNAL_ALRM = 14,
	GDB_SIGNAL_IO = 23,
	GDB_SIGNAL_XCPU = 24,
	GDB_SIGNAL_UNKNOWN = 143
} gdb_signal_t;

char *gdb_serialize(gdb_kind_t, const char *);

#ifdef __cplusplus
}
#endif

#endif
