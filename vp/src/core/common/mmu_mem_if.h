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

#ifndef RISCV_VP_MMU_MEM_IF_H
#define RISCV_VP_MMU_MEM_IF_H

#include <stdint.h>

enum MemoryAccessType { FETCH, LOAD, STORE };

struct mmu_memory_if {
    virtual ~mmu_memory_if() {}

    virtual uint64_t v2p(uint64_t vaddr, MemoryAccessType type) = 0;
    virtual uint64_t mmu_load_pte64(uint64_t addr) = 0;
    virtual uint64_t mmu_load_pte32(uint64_t addr) = 0;
    virtual void mmu_store_pte32(uint64_t addr, uint32_t value) = 0;
};

#endif //RISCV_VP_MMU_MEM_IF_H
