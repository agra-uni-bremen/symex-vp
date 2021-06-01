/*
 * Copyright (c) 2020,2021 Group of Computer Architecture, University of Bremen
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
