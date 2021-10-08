#include <exception>
#include <iostream>

#include <assert.h>
#include <endian.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "symbolic_format.h"

typedef enum {
	CONCRETE_UINT = 0x1,
	CONCRETE_SINT = 0x2,
	CONCRETE_BYTES = 0xfe,
	SYMBOLIC_BYTES = 0xff,
} FieldType;

FieldType
intToFtype(uint8_t t) {
	switch (t) {
	case CONCRETE_UINT:
		return CONCRETE_UINT;
	case CONCRETE_SINT:
		return CONCRETE_SINT;
	case CONCRETE_BYTES:
		return CONCRETE_BYTES;
	case SYMBOLIC_BYTES:
		return SYMBOLIC_BYTES;
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
	if ((fd = open(path.c_str(), O_RDONLY)) == -1)
		throw std::system_error(errno, std::generic_category());

	return;
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
	if (ftype == SYMBOLIC_BYTES) {
		v = ctx.getSymbolicBytes("input_field" + std::to_string(numSymField++), field.bytelen);
	} else {
		v = solver.BVC(field.value, field.bytelen);
	}

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
		r = field->concat(r);
	}

	assert(r != nullptr);
	assert(r->getWidth() % CHAR_BIT == 0);

	return r;
}
