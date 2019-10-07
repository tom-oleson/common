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
// * watch #tag   
// identifier/key
// string/value
// \0 eol

// key (identifier or string)
// value (string or raw)

bool cm_cache::scanner::next_token() {

    skip_whitespace();
    token.id = error;
    token.value.clear();

    if(index >= buf_sz) {
        token.id = input_end;
        return false;
    }

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
                    else {
                        token.id = raw;
                        scan_raw();
                    }
                    break;
    }
    return true;
} 

void cm_cache::scanner::scan_string(char quote_ch) { 
    char ch;
    token.value.append(1,quote_ch);
    while((ch = buffer[index]) && ch != quote_ch) {
        token.value.append(1,ch);
        index++;
    }
    if(ch == quote_ch) {
        token.value.append(1,quote_ch);
        index++;
    }
}

void cm_cache::scanner::scan_identifier() {
    char ch;
    while(is_ident(ch = buffer[index])) {
        token.value.append(1,ch);
        index++;
    } 
}

void cm_cache::scanner::scan_raw() {
    char ch;
    while(ch = buffer[index]) {
        token.value.append(1,ch);
        index++;
    } 
}

bool cm_cache::cache::parse_add(std::string &result) {

    next_token();

    std::string lvalue = std::move(token.value);

    if(token.id == cm_cache::string || token.id == cm_cache::identifier) {

        next_token();
        rvalue = std::move(token.value);

        if(token.id == cm_cache::string || token.id == cm_cache::raw ||
           token.id == cm_cache::identifier) {

            next_token();

            if(token.id == cm_cache::input_end) {
                return processor->do_add(lvalue, rvalue, result);
            }
            return parse_error("expected end after value", result);
        }
        return parse_error("expected string, raw or identifier for value", result);
    } 
    return parse_error("expected string or identifier for key", result);
}

bool cm_cache::cache::parse_read(std::string &result) {

    next_token();

    std::string lvalue = std::move(token.value);

    if(token.id == cm_cache::string || token.id == cm_cache::identifier) {

        next_token();

        if(token.id == cm_cache::input_end) {
            return processor->do_read(lvalue, result);
        }
        return parse_error("expected end after key", result);
    }
    return parse_error("expected string or identifier for key", result);
    
}

bool cm_cache::cache::parse_remove(std::string &result) {

    next_token();

    std::string lvalue = std::move(token.value);

    if(token.id == cm_cache::string || token.id == cm_cache::identifier) {

        next_token();

        if(token.id == cm_cache::input_end) {        
            return processor->do_remove(lvalue, result);
        }
        return parse_error("expected end after key", result);
    }
    return parse_error("expected string or identifier for key", result);
}

bool cm_cache::cache::parse_watch(std::string &result) {

    next_token();

    std::string lvalue = std::move(token.value);

    if(token.id == cm_cache::string || token.id == cm_cache::identifier) {

        next_token();

        if(token.id == cm_cache::tk_tag) {

            next_token();

            std::string rvalue = std::move(token.value);

            if(token.id == cm_cache::identifier) {

                next_token();

                if(token.id == cm_cache::input_end) {   
                    return processor->do_watch(lvalue, rvalue, result);
                }
                return parse_error("expected end after #tag", result);
            }
            return parse_error("expected identifier for #tag", result);
        }
        return parse_error("expected #tag", result);
    } 
    return parse_error("expected string or identifier for key", result);
}

bool cm_cache::cache::eval(const std::string &expr, std::string &result) {

    set_input(expr.c_str(), expr.size());

    while(next_token()) {

        if(token.id == cm_cache::comment) {
            continue;
        }

        if(token.id == cm_cache::tk_add) {
            if(!parse_add(result)) {
                return false;
            }
            continue;
        }

        if(token.id == cm_cache::tk_read) {
            if(!parse_read(result)) {
                return false;
            }
            continue;
        }

        if(token.id == cm_cache::tk_remove) {
            if(!parse_remove(result)) {
                return false;
            }
            continue;
        }

        if(token.id == cm_cache::tk_watch) {
            if(!parse_watch(result)) {
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
    char buf[2048] = {'\0'};

    fs.open(path.c_str(), std::fstream::in);
    if(!fs.is_open()) {
        return -1;
    }

    std::string expr;
    std::string result;
    int error_count = 0;


    while( fs.getline(buf, sizeof(buf)) ) {
        expr = std::move(std::string(buf, sizeof(buf)));
        if(!eval(expr, result)) {
            error_count++;
        }
    }

    if(fs.bad()) {
        fs.close();
        return -3;
    }

    fs.close();

    return error_count;
}
