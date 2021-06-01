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

#ifndef GDB_PROTOCOL_FNS
#define GDB_PROTOCOL_FNS

#include <assert.h>
#include <stddef.h>

#include <libgdb/parser2.h>

#ifdef NDEBUG
#define xassert(X) ((void)(X)) /* prevent -Wunused-parameter warning */
#else
#define xassert(X) (assert(X))
#endif

void *xrealloc(void *, size_t);
void *xmalloc(size_t);
char *xstrdup(char *);

gdb_command_t *gdb_new_cmd(char *, gdb_argument_t);

int calc_csum(const char *);
bool gdb_is_valid(gdb_packet_t *);
char *gdb_unescape(char *);
char *gdb_decode_runlen(char *);

#endif
