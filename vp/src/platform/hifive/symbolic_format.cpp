/*
 * Copyright (c) 2021 Group of Computer Architecture, University of Bremen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <exception>
#include <iostream>

#include <err.h>
#include <assert.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "symbolic_format.h"

typedef enum {
	SYMBOLIC_BYTES = 0x1,
	CONCRETE_BYTES = 0x2,
} FieldType;

FieldType
intToFtype(uint8_t t) {
	switch (t) {
	case SYMBOLIC_BYTES:
		return SYMBOLIC_BYTES;
	case CONCRETE_BYTES:
		return CONCRETE_BYTES;
	default:
		throw std::invalid_argument("invalid field type");
	}
}

class Field {
public:
	FieldType type;
	uint64_t bitlen;
	size_t bytelen;
	uint8_t *value;

	Field(FieldType _type, uint64_t _bitlen)
	  : type(_type), bitlen(_bitlen) {
		uint64_t rem;

		// Round to next byte boundary.
		if ((rem = bitlen % CHAR_BIT) == 0)
			bytelen = _bitlen / CHAR_BIT;
		else
			bytelen = ((_bitlen - rem) + CHAR_BIT) / CHAR_BIT;

		value = new uint8_t[bytelen]();
	}

	~Field(void) {
		delete[] value;
	}
};

SymbolicFormat::SymbolicFormat(SymbolicContext &_ctx, std::string path)
  : ctx(_ctx.ctx), solver(_ctx.solver)
{
	if (path == "") {
		fd = -1;
		return;
	} else if (path == "-") {
		fd = STDIN_FILENO;
	} else if ((fd = open(path.c_str(), O_RDONLY)) == -1) {
		throw std::system_error(errno, std::generic_category());
	}

	input = get_input();
	offset = input->getWidth();

	return;
}

SymbolicFormat::~SymbolicFormat(void)
{
	if (fd < 0)
		return;

	if (close(fd) == -1)
		err(EXIT_FAILURE, "close failed");
}

std::shared_ptr<clover::ConcolicValue>
SymbolicFormat::next_field(void)
{
	uint8_t type;
	FieldType ftype;
	uint64_t bitlen;
	ssize_t recv;

	recv = read(fd, &type, sizeof(type));
	if (recv != sizeof(type))
		return nullptr; // EOF
	ftype = intToFtype(type);

	recv = read(fd, &bitlen, sizeof(bitlen));
	if (recv != sizeof(bitlen))
		throw std::out_of_range("not length field");
	bitlen = le64toh(bitlen);

	Field field(ftype, bitlen);
	recv = read(fd, field.value, field.bytelen);
	if (recv == -1 || (size_t)recv != field.bytelen)
		throw std::out_of_range("not value field of given length");

	std::shared_ptr<clover::ConcolicValue> v;
	switch (ftype) {
	case SYMBOLIC_BYTES:
		v = ctx.getSymbolicBytes("input_field" + std::to_string(numSymField++), field.bytelen);
		break;
	case CONCRETE_BYTES:
		v = solver.BVC(field.value, field.bytelen, true);
		break;
	default:
		assert(0 && "unreachable");
	}

	if (field.bitlen * 8 == field.bytelen)
		return v;
	return v->extract(0, field.bitlen);
}

std::shared_ptr<clover::ConcolicValue>
SymbolicFormat::get_input(void)
{
	std::shared_ptr<clover::ConcolicValue> field, r = nullptr;

	while ((field = next_field())) {
		if (!r) {
			r = field;
			continue;
		}
		r = r->concat(field);
	}

	assert(r != nullptr);
	assert(r->getWidth() % CHAR_BIT == 0);

	return r;
}

std::shared_ptr<clover::ConcolicValue>
SymbolicFormat::next_byte(void)
{
	if (fd == -1 || offset == 0)
		return nullptr;

	assert(offset % CHAR_BIT == 0);
	offset -= CHAR_BIT;

	auto byte = input->extract(offset, CHAR_BIT);
	assert(byte->getWidth() == CHAR_BIT);

	return byte;
}

size_t
SymbolicFormat::remaning_bytes(void)
{
	if (fd == -1 || offset == 0)
		return 0; // empty

	auto width = input->getWidth();
	assert(width % CHAR_BIT == 0);

	return (width - (width - offset)) / CHAR_BIT;
}
