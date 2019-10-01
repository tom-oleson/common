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

#include "cache.h"

// tokens:
// # comment
// // comment
// + add/update
// $ read
// - remove
// * watch    
// identifier/key
// string/value
// \0 eol

bool cm_cache::scanner::next_token() {

    skip_whitespace();
    token.id = error;
    token.value.clear();

    if(index >= buf_sz) return false;

    char ch = buffer[index];
    switch(ch) {
        case '+':   if(index == 0) {
                        accept(tk_add);
                    }
                    break; 

        case '$':   if(index == 0) {
                        accept(tk_read);
                    }
                    break; 

        case '*':   if(index == 0) {
                        accept(tk_watch);
                    }
                    break; 

        case '-':   if(index == 0) {
                        accept(tk_remove);
                    }
                    break; 

        case '#':   if(index == 0) {
                        accept(comment);
                        skip_to_end();
                    }
                    else {
                        accept(tk_tag);
                    } 
                    break;

        case '/':   if(index == 0 && buffer[index+1] == '/') {
                        accept(comment);
                        skip_to_end();
                    } /* else error */
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

void cm_cache::scanner::scan_string(char quote_ch) { 
    char ch;
    while((ch = buffer[index]) && ch != quote_ch) {
        token.value.append(1,ch);
        index++;
    }
    if(ch == quote_ch) index++;
}

void cm_cache::scanner::scan_identifier() {
    char ch;
    while(is_ident(ch = buffer[index])) {
        token.value.append(1,ch);
        index++;
    } 
}

bool cm_cache::cache::parse_add() {

    next_token();
    std::string lvalue = token.value;

    if(token.id == cm_cache::string || token.id == cm_cache::identifier) {

        next_token();
        std::string rvalue = token.value;

        if(token.id == cm_cache::string || token.id == cm_cache::identifier) {
            return processor->do_add(lvalue, rvalue);
        }
    } 

    return false;
}

bool cm_cache::cache::parse_read() {

    next_token();
    std::string lvalue = token.value;

    if(token.id == cm_cache::string || token.id == cm_cache::identifier) {
        return processor->do_read(lvalue);
    }

    return false;
}

bool cm_cache::cache::parse_remove() {

    next_token();
    std::string lvalue = token.value;

    if(token.id == cm_cache::string || token.id == cm_cache::identifier) {
        return processor->do_remove(lvalue);
    }

    return false;
}

bool cm_cache::cache::parse_watch() {

    next_token();
    std::string lvalue = token.value;

    if(token.id == cm_cache::string || token.id == cm_cache::identifier) {

        next_token();
        if(token.id == cm_cache::tk_tag) {

            next_token();
            std::string rvalue = token.value;

            if(token.id == cm_cache::string || token.id == cm_cache::identifier) {
                return processor->do_watch(lvalue, rvalue);
            }

        }
    } 

    return false;
}

bool cm_cache::cache::eval(const std::string &expr) {

    set_input(expr.c_str(), expr.size());

    while(next_token()) {

        if(token.id == cm_cache::comment) {
            continue;
        }

        if(token.id == cm_cache::tk_add) {
            if(!parse_add()) {
                return false;
            }
            continue;
        }

        if(token.id == cm_cache::tk_read) {
            if(!parse_read()) {
                return false;
            }
            continue;
        }

        if(token.id == cm_cache::tk_remove) {
            if(!parse_remove()) {
                return false;
            }
            continue;
        }

        if(token.id == cm_cache::tk_watch) {
            if(!parse_watch()) {
                return false;
            }
            continue;
        }

        if(token.id == cm_cache::input_end) {
            break;
        }

        if(token.id == cm_cache::error) {
            return false;
        }
    }

    return true;      
}

int cm_cache::cache::load(const std::string &path) {

    std::fstream fs;
    char buf[1024] = {'\0'};

    fs.open(path.c_str(), std::fstream::in);
    if(!fs.is_open()) {
        return -1;
    }

    std::string expr;

    while( fs.getline(buf, sizeof(buf)) ) {
        expr = std::move(std::string(buf, sizeof(buf)));
        if(!eval(expr)) {
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
