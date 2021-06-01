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

#pragma once

#include <systemc>

struct HTIF : public sc_core::sc_module {
    uint64_t *to_host;
    uint64_t *num_instr;
    uint64_t max_instrs;
    std::function<void(uint64_t)> to_host_callback;

    SC_HAS_PROCESS(HTIF);
    HTIF(sc_core::sc_module_name, uint64_t *to_host, uint64_t *num_instr, uint64_t max_instrs, std::function<void(uint64_t)> to_host_callback) {
        this->to_host = to_host;
        this->num_instr = num_instr;
        this->max_instrs = max_instrs;
        this->to_host_callback = to_host_callback;
        SC_THREAD(run);
    }

    void run() {
        while (true) {
            sc_core::wait(10, sc_core::SC_US);
            auto x = *to_host;
            to_host_callback(x);
            x = *num_instr;
            if (x >= max_instrs)
                throw std::runtime_error("reached >= max #instrs: " + std::to_string(x));
        }
    }
};
