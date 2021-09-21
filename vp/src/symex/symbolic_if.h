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

#ifndef RISCV_ISA_SYMBOLIC_IF_H
#define RISCV_ISA_SYMBOLIC_IF_H

struct symbolic_iss_if {
	virtual ~symbolic_iss_if(void) {}
	virtual void make_symbolic(uint32_t addr, size_t size) = 0;
	virtual void sys_exit(void) = 0;
};

#endif
