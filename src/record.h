/*
 * Copyright (c) 2022, Tom Oleson <tom dot oleson at gmail dot com>
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

#ifndef __RECORD_H
#define __RECORD_H

#include <string>

#include "util.h"           // for cm_util::format

using namespace std;


namespace cm_record {

class field {
    string  name;
    string  type;       // string | int 
    size_t  length;     // 0-n (0 means any length, use for fixed-length fields)

public:
    field() {}

    field(const string &_name, const string &_type, size_t _length) :
        name(_name), type(_type), length(_length) {}

    // copy constructor
    field(const field &r) :
        name(r.name), type(r.type), length(r.length) {}

    // assignment operator
    field &operator = (const field &r) {
        name = r.name;
        type = r.type;
        length = r.length;
        return *this;
    }

    void set_name(const string &s) { name = s; }
    void set_type(const string &s) { type = s; }
    void set_length(size_t n) { length = n; }

    const string &get_name() { return name; }
    const string &get_type() { return type; }
    size_t get_length() { return length; }

    const string to_string();
};

class record_spec {

protected:
    string  name;
    string  version;
    string  delimiter;

    vector<field> fields;                   // fields in parse order
    map<string,field> field_names_map;      // names --> fields map

public:
    record_spec() {}      

    record_spec(const string &_name, const string &_version, const string &_delimiter = "|") :
        name(_name), version(_version), delimiter(_delimiter) {}

    // copy constructor
    record_spec(const record_spec &r) :
        name(r.name), version(r.version), delimiter(r.delimiter), fields(r.fields), field_names_map(r.field_names_map) {}

    // assignment operator
    record_spec &operator = (const record_spec &r) {
        name = r.name;
        version = r.version;
        delimiter = r.delimiter;
        fields = r.fields;
        field_names_map = r.field_names_map;
        return *this;
    }        

    size_t size() { return fields.size(); }
    
    void set_name(const string &s) { name = s; }
    void set_version(const string &s) { version = s; }
    void set_delimiter(const string &s) { delimiter = s; }

    const string &get_name() { return name; }
    const string &get_version() { return version; }
    const string &get_delimiter() { return delimiter; }

    void add_field(field &f) {
        fields.push_back(f);
        field_names_map.insert(pair<string,field>(f.get_name(), f));
    }

    // get field by index
    field &get_field(size_t index) {
        return fields[index];
    }

    // get field by name
    field &get_field(const string &_name) {
        map<string,field>::iterator p = field_names_map.find(_name);
        if(p != field_names_map.end()) return p->second;
        throw cm_util::format("Invalid field name: [%s] for %s:%s", _name.c_str(), name.c_str(), version.c_str());
    }

    const string to_string();

};

// find spec with matching name and version in specified xml (memory or file) and add fields to spec object 
bool xml_load_record_spec(const string spec, const string name, const string version, record_spec *spec_ptr);


}

#endif  // __RECORD_H
