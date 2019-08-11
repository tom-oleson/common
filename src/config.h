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
#include <deque>

#include "util.h"
#include "mutex.h"


extern void *default_config;

namespace cm_config {

// file grammer:
// # comment
// // comment
// = equals
// string
// identifier
// { left_brace
// identifier = string
// } right brace
// identifier = string
// \0 eol

enum token_type {
     input_end, error, comment, string, identifier, left_brace, right_brace, equals
};

struct token_t {
    token_type id = input_end;
    std::string value;
};

class scanner {

protected:
    const char *buffer;     // input buffer
    size_t buf_sz;          // buffer size
    token_t token;          // current token
    int index = 0;          // index into input buffer

    int level = 0;
    std::vector<std::string> sections;
    

    scanner(): buffer(NULL), index(0), buf_sz(0) { }
    ~scanner() {}

    bool next_token();

    bool is_ident(char ch) {
        return (isalnum(ch) || ch == '-' || ch == '_' || ch == '.');
    }

    void accept(token_type id) { token.id = id; index++; }
    void skip_whitespace() { while(isspace( buffer[index] )) index++; }
    void skip_to_end() { while(buffer[index] != '\0') index++; }

    void scan_string(char quote_ch);
    void scan_identifier();  

    void set_input(const char *p, size_t sz) { index = 0; buffer = p; buf_sz = sz;}
};


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
    virtual std::string get(const std::string &name, const std::string &_default) = 0;
};


class memory_config: public config, protected cm::mutex {

protected:
    std::map<std::string, std::string> _map; 

public:
    memory_config() { name = "memory-config"; }
    ~memory_config() { }

    bool check(const std::string &name) {
        lock();
        bool b = _map.find(name) != _map.end();
        unlock();
        return b;
    }

    void set(const std::string &name, const std::string &value) {
        lock();
        _map[name] = value;
        unlock();
    }
    std::string get(const std::string &name) {
        lock();
        std::string &value = _map[name];
        unlock();
        return value;
    }

    std::string get(const std::string &name, const std::string &_default) {
        std::string value = _default;
        lock();
        if(_map.find(name) != _map.end()) {
           value = _map[name]; 
        }
        unlock();
        return value;
    }

};

class file_config: public memory_config, protected scanner,  protected std::ofstream {

protected:
    std::string path;
    std::deque<std::string> sections;

public:
    file_config() { name = "file-config"; }
    file_config(const std::string _path): path(_path) { }
    ~file_config() { }

    int load();
    bool parse();
    bool parse_identifier();
    bool parse_section();
    bool parse_assignment(std::string &lvalue);
    void do_assign(std::string &lvalue, std::string &rvalue);

};

// default config access
bool check(const std::string &name);
void set(const std::string &name, const std::string &value);
std::string get(const std::string &name);
std::string get(const std::string &name, const std::string &_default);


extern memory_config mem_config;


} // namespace cm_config

inline cm_config::config &get_default_config() { return *(cm_config::config*)default_config; }
inline void set_default_config(cm_config::config *_config) {
    default_config = _config;
}

#endif	// __CONFIG_H

