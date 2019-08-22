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

#ifndef __STORE_H
#define __STORE_H

#include <string>
#include <unordered_map>

#include "mutex.h"

namespace cm {


template<class keyT, class valueT>
class info_store: protected cm::mutex {

protected:
    // unordered map for fast access using buckets
    std::unordered_map<keyT,valueT> _map;

public:

    bool check(const keyT &name) {
        lock();
        bool b = _map.find(name) != _map.end();
        unlock();
        return b;
    }

    bool set(const keyT &name, const valueT &value) {
        lock();
        _map[name] = value;
        unlock();
    }

    valueT find(const keyT &name) {
        valueT value;
        lock();
        if( _map.find(name) != _map.end() ) {
            value = _map[name];
        }
        unlock();
        return value;
    }

    valueT get(const keyT &name, const valueT &_default) {
        valueT value = _default;
        lock();
        if(_map.find(name) != _map.end()) {
           value = _map[name]; 
        }
        unlock();
        return value;
    }
    
    size_t remove(const keyT &name) {
        lock();
        size_t num_erased = _map.erase(name);
        unlock();
        return num_erased;   
    }

    size_t size() {
        lock();
        size_t size = _map.size();
        unlock();
        return size;
    }

};

extern info_store<std::string,std::string> mem_store;

} // namespace cm

extern void *default_store;

inline void *get_default_store() { return default_store; }
inline void set_default_store(void *_store) {
    default_store = _store;
}

#endif	// __STORE_H

