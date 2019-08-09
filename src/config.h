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

#ifndef __CONFIG_H
#define __CONFIG_H

#include <exception>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include "util.h"
#include "mutex.h"


extern void *default_config;

namespace cm_config {


class config {

protected:
    std::string name;
    void *save_default_config;

public:
	config() {
        // save pointer to default config to be restored
        save_default_config = default_config;
	}

    ~config() {
        // restore default config if we became "the one"
        if(default_config == this) {
            default_config = save_default_config;
        }
    }
  
    virtual bool check(const std::string &name) = 0; 

    virtual void set(const std::string &name, const std::string &value) = 0;
    virtual std::string get(const std::string &name) = 0;
};


class memory_config: protected config, protected cm::mutex {

protected:
    std::map<std::string, std::string> _map; 

public:
    memory_config() { name = "memory-config"; }
    ~memory_config() { }

    bool check(const std::string &name) { return _map.find(name) != _map.end(); }

    void set(const std::string &name, const std::string &value) { _map[name] = value; }
    std::string get(const std::string &name) { return _map[name]; }

};

class file_config : protected memory_config, protected std::ofstream {

protected:
    std::string path;

public:
    file_config() { name = "file-logger"; }
    file_config(const std::string _path): path(_path) { }
    ~file_config() { }

};

// default config access
bool check(const std::string &name);
void set(const std::string &name, const std::string &value);
std::string get(const std::string &name);


extern memory_config mem_config;


} // namespace cm_config

inline cm_config::config &get_default_config() { return *(cm_config::config*)default_config; }
inline void set_default_config(cm_config::config *_config) {
    default_config = _config;
}

#endif	// __CONFIG_H

