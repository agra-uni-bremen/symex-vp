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

#include <err.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <libgdb/parser1.h>
#include <libgdb/response.h>

#include "internal.h"

static char
kind_to_char(gdb_kind_t kind)
{
	switch (kind) {
	case GDB_KIND_NOTIFY:
		return '#';
	case GDB_KIND_PACKET:
		return '$';
	case GDB_KIND_NACK:
		return '-';
	case GDB_KIND_ACK:
		return '+';
	default:
		xassert(0);
		return -1;
	}
}

char *
gdb_serialize(gdb_kind_t kind, const char *data)
{
	size_t pktlen;
	char *serialized;
	char pktkind;
	int csum, ret;

	pktkind = kind_to_char(kind);
	if (kind == GDB_KIND_NACK || kind == GDB_KIND_ACK) {
		xassert(data == NULL);
		serialized = xmalloc(2); /* kind + nullbyte */

		serialized[0] = pktkind;
		serialized[1] = '\0';

		return serialized;
	}

	csum = calc_csum(data);

	/* + 3 → nullbyte, checksum delimiter, kind */
	pktlen = strlen(data) + GDB_CSUM_LEN + 3;
	serialized = xmalloc(pktlen);

	ret = snprintf(serialized, pktlen, "%c%s#%.2x", pktkind, data, csum);
	if (ret < 0)
		err(EXIT_FAILURE, "snprintf failed");
	else if ((size_t)ret >= pktlen)
		errx(EXIT_FAILURE, "insufficient snprintf buffer size");

	return serialized;
}
