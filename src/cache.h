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

#ifndef __CACHE_H
#define __CACHE_H

#include <exception>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <deque>

#include "util.h"
#include "mutex.h"
#include "store.h"


extern void *default_cache;

namespace cm_cache {

enum token_type {
     input_end, error, comment, string, identifier, raw,
        tk_add, tk_read, tk_read_remove, tk_remove, tk_watch, tk_watch_remove, tk_tag
};

struct token_t {
    token_type id = input_end;
    std::string value;
};


struct cache_event {
    int fd = -1;         // response socket
    std::string request;
    std::string name;
    std::string tag;
    std::string value;
    std::string result;
    bool notify = false;    // notify watchers

    cache_event() {}
    ~cache_event() {}

    cache_event(int fd_, std::string request_): fd(fd_), request(request_) {}
    cache_event(const cache_event &r): fd(r.fd), request(r.request),
        name(r.name), tag(r.tag), value(r.value), result(r.result),
        notify(r.notify)  {}
    
    cache_event &operator = (const cache_event &r) {
        fd = r.fd;
        request = r.request;
        name = r.name;
        tag = r.tag;
        value = r.value;
        result = r.result;
        notify = r.notify;
        return *this;
    }
};

class scanner_processor {

public:
    virtual bool do_add(const std::string &name, const std::string &value, cache_event &event) = 0;
    virtual bool do_read(const std::string &name, cache_event &event) = 0;
    virtual bool do_read_remove(const std::string &name, cache_event &event) = 0;
    virtual bool do_remove(const std::string &name, cache_event &event) = 0;
    virtual bool do_watch(const std::string &name, const std::string &tag, cache_event &event) = 0;
    virtual bool do_watch_remove(const std::string &name, const std::string &tag, cache_event &event) = 0;
    virtual bool do_result(cache_event &event) = 0;
    virtual bool do_input(const std::string &in_str, cache_event &event) = 0;
    virtual bool do_error(const std::string &expr, const std::string &err, cache_event &event) = 0;
};

class scanner {

protected:
    const char *buffer;     // input buffer
    size_t buf_sz;          // buffer size
    token_t token;          // current token
    int index = 0;          // index into input buffer

    scanner_processor *processor = nullptr;

public:

    scanner(scanner_processor *processor_): processor(processor_),
        buffer(NULL), index(0), buf_sz(0) { }

    ~scanner() {}

    bool next_token();

    bool is_ident(char ch) {

        // includes =+/ symbols to pickup base64 encoded strings
        // as indentifiers

        return (isalnum(ch) || ch == '-' || ch == '_' || ch == '.' ||
                ch == '=' || ch == '+' || ch == '/' || ch == '$');
    }

    void accept(token_type id) { token.id = id; index++; }
    void skip_whitespace() { while(isspace( buffer[index] )) index++; }
    void skip_to_end() { while(buffer[index] != '\0') index++; }

    void scan_string(char quote_ch);
    void scan_identifier();
    void scan_raw();  

    void set_input(const char *p, size_t sz) { index = 0; buffer = p; buf_sz = sz;}
    
    std::string get_input() { return std::string(buffer, buf_sz); }
};


class cache: public scanner {


protected:
    std::string name;

    std::string lvalue;
    std::string rvalue;

public:
    cache(scanner_processor *processor):
        scanner(processor), name("memory-cache" ) { }

    ~cache() { }

    int load(const std::string &path);
    bool eval(const std::string &expr, cache_event &event);
    bool parse_identifier();

    bool parse_add(cache_event &event);
    bool parse_read(cache_event &event);
    bool parse_read_remove(cache_event &event);
    bool parse_remove(cache_event &event);
    bool parse_watch(cache_event &event);
    bool parse_watch_remove(cache_event &event);

    inline bool parse_error(const std::string &err, cache_event &event) {
        processor->do_error(get_input(), err, event);
    }
};


} // namespace cm_cache


#endif	// __CACHE_H

