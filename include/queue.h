/*
 * Copyright (c) 2019, Tom Oleson <tom dot oleson at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * The names of its contributors may NOT be used to endorse or promote
 *     products derived from this software without specific prior written
 *     permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __QUEUE_H
#define __QUEUE_H

#include <string>
#include <deque>

#include "mutex.h"

namespace cm_queue {

// Implements a double sided FIFO queue that decouples input from output.
// In other words, publisher threads will not continuously contend with locks
// from consumer threads. The input deque can fill while the output deque is
// being consumed. When the output deque is empty, the input deque will be
// swaped to the output side.

template<class valueT>
class double_queue {

protected:
    std::deque<valueT> in;
    std::deque<valueT> out;

    cm::mutex in_mutex;
    cm::mutex out_mutex;

public:

    bool empty() const { return in.empty() && out.empty(); }
    size_t size() const { return (size_t) in.size() + (size_t) out.size(); }

    void push_back(const valueT &value) {
        in_mutex.lock();
        in.push_back(value);
        in_mutex.unlock();
    }

    void push_back(std::deque<valueT> &q) {
        in_mutex.lock();
        for(auto &value : q) {
            in.push_back(value);
        }               
        in_mutex.unlock();
    }

    valueT pop_front() {
        valueT value;
        out_mutex.lock();
        if(out.empty()) _swap();
        if(!out.empty()) {
            value = out.front();
            out.pop_front();
        }
        out_mutex.unlock();
        return value;
    }

    void swap_out(std::deque<valueT> &q) {
        out_mutex.lock();
        if(out.empty()) _swap();
        if(!out.empty()) {
            out.swap(q);
        }
        out_mutex.unlock();
    }

    void _swap() {
        in_mutex.lock();
        if(!in.empty()) in.swap(out);
        in_mutex.unlock();
    }
};


extern double_queue<std::string> mem_queue;

} // namespace cm_queue

extern void *default_queue;

inline void *get_default_queue() { return default_queue; }
inline void set_default_queue(void *_queue) {
    default_queue = _queue;
}

#endif	// __QUEUE_H

