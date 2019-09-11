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

#include "config.h"


// globals
cm_config::memory_config cm_config::mem_config;

void *default_config = &cm_config::mem_config;

// default config access
bool cm_config::check(const std::string &name) { return get_default_config().check(name); }
void cm_config::set(const std::string &name, const std::string &value) {
    get_default_config().set(name, value);
}
std::string cm_config::get(const std::string &name) { return get_default_config().get(name); }
std::string cm_config::get(const std::string &name, const std::string &_default) {
    return get_default_config().check(name) ? get_default_config().get(name) : _default;
}

// file grammer:
// # comment
// // comment
// identifier
// { left_brace
// } right brace
// = equals
// string
// \0 eol

bool cm_config::scanner::next_token() {

    skip_whitespace();
    token.id = error;
    token.value.clear();

    if(index >= buf_sz) return false;

    char ch = buffer[index];
    switch(ch) {
        case '=':   accept(equals);
                    break; 

        case '#':   accept(comment);
                    skip_to_end();
                    break;

        case '/':   if(buffer[index+1] == '/') {
                        accept(comment);
                        skip_to_end();
                    } /* else error */
                    break;

        case '{':   accept(left_brace);
                    break;

        case '}':   accept(right_brace);
                    break;
       
        case '"':   accept(string);
                    scan_string('"');
                    break;

        case '\'':  accept(string);
                    scan_string('\'');
                    break; 

        case '\0':  token.id = input_end;
                    break;

        default:    if(is_ident(ch)) {
                        token.id = identifier;
                        scan_identifier();
                    }
                    break;
    }
    return true;
} 

void cm_config::scanner::scan_string(char quote_ch) { 
    char ch;
    while((ch = buffer[index]) && ch != quote_ch) {
        token.value.append(1,ch);
        index++;
    }
    if(ch == quote_ch) index++;
}

void cm_config::scanner::scan_identifier() {
    char ch;
    while(is_ident(ch = buffer[index])) {
        token.value.append(1,ch);
        index++;
    } 
}

bool cm_config::file_config::parse_identifier() {

    std::string lvalue = token.value;

    next_token();

    if(token.id == cm_config::left_brace || token.id == cm_config::input_end) {
        sections.push_back(lvalue);
        level++;
        return parse_section();
    } 
    if(token.id == cm_config::equals) {
        return parse_assignment(lvalue);
    } 

    return false;
}

bool cm_config::file_config::parse_section() {

    return parse();
}

bool cm_config::file_config::parse_assignment(std::string &lvalue) {

    next_token();

    std::string rvalue = token.value;

    if(token.id == cm_config::identifier || token.id == cm_config::string ) {
        do_assign(lvalue, rvalue);
        return true;
    }

    return false;
}

void cm_config::file_config::do_assign(std::string &lvalue, std::string &rvalue) {

    if(!sections.empty()) {

        std::string prefix;

        for(auto &section: sections) {
            prefix += section + ".";
        }

        set(prefix + lvalue, rvalue);
    }
    else {
        set(lvalue, rvalue);
    }

}

bool cm_config::file_config::parse() {

    while(next_token()) {

        if(token.id == cm_config::comment) {
            continue;
        }

        if(token.id == cm_config::identifier) {
            if(!parse_identifier()) {
                return false;
            }
            continue;
        }

        if(token.id == cm_config::right_brace) {
            // end of section
            sections.pop_back();
            if(level > 0) level--;
            continue;
        }

        if(token.id == cm_config::input_end) {
            break;
        }

        if(token.id == cm_config::error) {
            return false;
        }
    }

    return true;      
}

int cm_config::file_config::load() {

    std::fstream fs;
    char buf[1024] = {'\0'};

    fs.open(path.c_str(), std::fstream::in);
    if(!fs.is_open()) {
        return -1;
    }

    while( fs.getline(buf, sizeof(buf)) ) {

        set_input(buf, sizeof(buf));

        if(!parse()) {
            return -2;
        }
    }

    if(fs.bad()) {
        fs.close();
        return -3;
    }

    fs.close();

    return 0;
}
