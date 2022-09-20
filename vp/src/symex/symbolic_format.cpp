/*
 * Copyright (c) 2021,2022 Group of Computer Architecture, University of Bremen
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

#include <vector>
#include <exception>
#include <iostream>

#include <err.h>
#include <limits.h>
#include <assert.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/stat.h>

#include "symbolic_format.h"
#include "symbolic_context.h"

// True if the given bit size is NOT aligned on a byte boundary.
#define HAS_PADDING(BITSIZE) (BITSIZE % CHAR_BIT != 0)

static size_t
to_byte_size(uint64_t bitsize)
{
	uint64_t rem;
	size_t bytesize;

	// Round to next byte boundary.
	if ((rem = bitsize % CHAR_BIT) == 0)
		bytesize = bitsize / CHAR_BIT;
	else
		bytesize = ((bitsize - rem) + CHAR_BIT) / CHAR_BIT;

	return bytesize;
}

SymbolicFormat::SymbolicFormat(SymbolicContext &_ctx, std::string path)
  : ctx(_ctx.ctx), solver(_ctx.solver), file(path)
{
	if (path == "") {
		parsed = true;
		return;
	}

	data = bencode::decode(file, bencode::no_check_eof);
	if (file.bad())
		throw std::runtime_error("failed to read bencode data from socket");
}

std::shared_ptr<clover::ConcolicValue>
SymbolicFormat::get_value(bencode::list list, std::string name, uint64_t bitsize)
{
	bool has_elems;
	size_t bytesize;
	std::vector<uint8_t> concrete_value;
	std::shared_ptr<clover::ConcolicValue> symbolic_value;

	bytesize = to_byte_size(bitsize);

	// Value is either:
	//
	//   1. A symbolic value identified by a list of (possibly empty)
	//      constraints expressed as a string in KLEE KQuery format.
	//   2. A concrete value identified by a list of (non-empty)
	//      integers each representing a single byte of a concrete
	//      bytevector.
	//
	// For concrete values, the specified bitsize must match the
	// length of the specified bytevector. Constrained symbolic
	// values are presently not supported as they require a modified
	// version of KLEE and would thus induce a maintenance burden.

	has_elems = false;
	for (auto elem : list) {
		has_elems = true;

		if (std::get_if<bencode::string>(&elem)) {
			throw std::logic_error("constrained symbolic fields not supported at the moment");
		} else {
			bencode::integer intval;
			try {
				intval = std::get<bencode::integer>(elem);
			} catch (const std::bad_variant_access&) {
				return nullptr;
			}
			if (intval < 0 || intval > UINT8_MAX)
				return nullptr;
			concrete_value.push_back((uint8_t)intval);
		}
	}

	if (!has_elems) { // unconstrained symbolic value
		auto symbolic_value = ctx.getSymbolicBytes(name, bytesize);
		if (HAS_PADDING(bitsize))
			symbolic_value = symbolic_value->extract(0, bitsize);
		return symbolic_value;
	} else {
		if (concrete_value.size() != bytesize)
			return nullptr;

		auto bvc = solver.BVC(concrete_value.data(), concrete_value.size(), true);
		if (HAS_PADDING(bitsize))
			bvc = bvc->extract(0, bitsize);
		return bvc;
	}
}

// TODO: Make this a singleton
std::vector<std::shared_ptr<clover::ConcolicValue>>
SymbolicFormat::get_values(void)
{
	std::shared_ptr<clover::ConcolicValue> prev_value = nullptr;

	// Only iterate over the bencode data stream once.
	if (parsed)
		return values;

	auto list = std::get<bencode::list>(data);
	for (auto &elem : list) {
		auto field = std::get<bencode::list>(elem);
		if (field.size() != 3)
			throw std::invalid_argument("invalid bencode field");

		auto name = std::get<bencode::string>(field[0]);
		auto size = std::get<bencode::integer>(field[1]);
		auto list = std::get<bencode::list>(field[2]);

		auto v = get_value(list, name, (uint64_t)size);
		if (!v)
			throw std::invalid_argument("invalid bencode value format");

		values.push_back(v);
	}

	parsed = true;
	return values;
}

std::vector<std::shared_ptr<clover::ConcolicValue>>
SymbolicFormat::get_bytes(void)
{
	unsigned off;
	std::vector<std::shared_ptr<clover::ConcolicValue>> vec;
	std::shared_ptr<clover::ConcolicValue> concated = nullptr;

	// TODO: KLEE isn't very good at optimizing concat and extract
	// expressions. Hence, this implementation is horribly
	// inefficient and could/should be optimized further.

	auto values = get_values();
	if (values.empty())
		return vec;

	for (auto value : values) {
		if (!concated) {
			concated = value;
			continue;
		}
		concated = concated->concat(value);
	}

	assert(concated != nullptr);
	assert(concated->getWidth() % CHAR_BIT == 0);

	off = concated->getWidth();
	assert(off >= CHAR_BIT);
	assert(off % CHAR_BIT == 0);
	for (;;) {
		if (off == 0)
			break;

		off -= CHAR_BIT;
		vec.push_back(concated->extract(off, CHAR_BIT));
	}

	return vec;
}
