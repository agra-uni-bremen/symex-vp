#include "symbolic_extension.h"

SymbolicExtension::SymbolicExtension(std::shared_ptr<clover::ConcolicValue> _value)
{
	value = _value;
}

SymbolicExtension::~SymbolicExtension(void)
{
	return; // TODO: Decrement shared_ptr counter?
}

void
SymbolicExtension::copy_from(const tlm_extension_base &extension)
{
	value = static_cast<SymbolicExtension const &>(extension).value;
}

tlm::tlm_extension_base *
SymbolicExtension::clone(void) const
{
	return new SymbolicExtension(*this);
}

std::shared_ptr<clover::ConcolicValue>
SymbolicExtension::getValue(void)
{
	return value;
}
