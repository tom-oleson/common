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
#include "log.h"

// tokens:
// # comment
// // comment
// + add/update
// $ read
// ! read then remove
// - remove
// * watch #tag
// @ watch #tag (remove on notify)

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

        case '"':   accept(string);
                    scan_string('"');
                    break;

        case '\'':  accept(string);
                    scan_string('\'');
                    break; 

        case '\0':  token.id = input_end;
                    break;

        case '$':   if(index == 0) {
                        accept(tk_read);
                        break;
                    }
                    
        case '!':   if(index == 0) {
                        accept(tk_read_remove);
                        break;
                    }

        case '*':   if(index == 0) {
                        accept(tk_watch);
                        break;
                    }

        case '@':   if(index == 0) {
                        accept(tk_watch_remove);
                        break;
                    }
        
        case '+':   if(index == 0) {
                        accept(tk_add);
                        break;
                    }

        case '-':   if(index == 0) {
                        accept(tk_remove);
                        break;
                    }

        case '#':   if(index == 0) {
                        accept(comment);
                        skip_to_end();
                        break;
                    }
                    else if(ch == '#') {
                        accept(tk_tag);
                        break;
                    } 

        case '/':   if(index == 0 && buffer[index+1] == '/') {
                        accept(comment);
                        skip_to_end();
                        break;
                    } 

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
        if(ch == '\n') break;
        token.value.append(1,ch);
        index++;
    } 
}

bool cm_cache::cache::parse_add(cm_cache::cache_event &event) {

    next_token();

    std::string lvalue = std::move(token.value);

    if(token.id == cm_cache::string || token.id == cm_cache::identifier) {

        next_token();
        rvalue = std::move(token.value);

        if(token.id == cm_cache::string || token.id == cm_cache::raw ||
           token.id == cm_cache::identifier) {

            next_token();

            if(token.id == cm_cache::input_end) {
                return processor->do_add(lvalue, rvalue, event);
            }
            return parse_error("add: expected end after value", event);
        }
        return parse_error("add: expected string, raw or identifier for value", event);
    } 
    return parse_error("add: expected string or identifier for key", event);
}

bool cm_cache::cache::parse_read(cm_cache::cache_event &event) {

    next_token();

    std::string lvalue = std::move(token.value);

    if(token.id == cm_cache::string || token.id == cm_cache::identifier) {

        next_token();

        if(token.id == cm_cache::input_end) {
            return processor->do_read(lvalue, event);
        }
        return parse_error("read: expected end after key", event);
    }
    return parse_error("read: expected string or identifier for key", event);
    
}

bool cm_cache::cache::parse_read_remove(cm_cache::cache_event &event) {

    next_token();

    std::string lvalue = std::move(token.value);

    if(token.id == cm_cache::string || token.id == cm_cache::identifier) {

        next_token();

        if(token.id == cm_cache::input_end) {
            return processor->do_read_remove(lvalue, event);
        }
        return parse_error("read_remove: expected end after key", event);
    }
    return parse_error("read_remove: expected string or identifier for key", event);
    
}

bool cm_cache::cache::parse_remove(cm_cache::cache_event &event) {

    next_token();

    std::string lvalue = std::move(token.value);

    if(token.id == cm_cache::string || token.id == cm_cache::identifier) {

        next_token();

        if(token.id == cm_cache::input_end) {        
            return processor->do_remove(lvalue, event);
        }
        return parse_error("remove: expected end after key", event);
    }
    return parse_error("remove: expected string or identifier for key", event);
}

bool cm_cache::cache::parse_watch(cm_cache::cache_event &event) {

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
                    return processor->do_watch(lvalue, rvalue, event);
                }                

                if(token.id == cm_cache::identifier) {

                    // this is a re-pub event
                    event.pub_name = std::move(token.value);

                    if(token.id == cm_cache::identifier) {
                        return processor->do_watch(lvalue, rvalue, event);
                    }
                }
                return parse_error("watch: expected end or identifier after #tag", event);
            }
            return parse_error("watch: expected identifier for #tag", event);
        }
        return parse_error("watch: expected #tag", event);
    } 
    return parse_error("watch: expected string or identifier for key", event);
}

bool cm_cache::cache::parse_watch_remove(cm_cache::cache_event &event) {

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
                    return processor->do_watch(lvalue, rvalue, event);
                }                

                if(token.id == cm_cache::identifier) {

                    // this is a re-pub event
                    event.pub_name = std::move(token.value);

                    if(token.id == cm_cache::identifier) {
                        return processor->do_watch(lvalue, rvalue, event);
                    }
                }
                return parse_error("watch: expected end or identifier after #tag", event);
            }
            return parse_error("watch_remove: expected identifier for #tag", event);
        }
        return parse_error("watch_remove: expected #tag", event);
    } 
    return parse_error("watch_remove: expected string or identifier for key", event);
}

bool cm_cache::cache::eval(const std::string &expr, cm_cache::cache_event &event) {

    //event.clear();
    //event.result.assign(expr);

    // setup scanner input
    set_input(expr.c_str(), expr.size());

    while(next_token()) {

        if(token.id == cm_cache::comment) {
            continue;
        }

        if(token.id == cm_cache::tk_add) {
            if(!parse_add(event)) {
                return false;
            }
            continue;
        }

        if(token.id == cm_cache::tk_read) {
            if(!parse_read(event)) {
                return false;
            }
            continue;
        }

        if(token.id == cm_cache::tk_read_remove) {
            if(!parse_read_remove(event)) {
                return false;
            }
            continue;
        }

        if(token.id == cm_cache::tk_remove) {
            if(!parse_remove(event)) {
                return false;
            }
            continue;
        }

        if(token.id == cm_cache::tk_watch) {
            if(!parse_watch(event)) {
                return false;
            }
            continue;
        }

        if(token.id == cm_cache::tk_watch_remove) {
            if(!parse_watch_remove(event)) {
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

    cache_event event;
    std::string input;
    int rec_count = 0;

    while( fs.getline(buf, sizeof(buf)) ) {

        input = std::move(std::string(buf, sizeof(buf)));

        // extract expr from input
        if(processor->do_input(input, event)) {
            if(eval(event.request, event)) {
                rec_count++;
            }
        }
    }

    if(fs.bad()) {
        fs.close();
        return -3;
    }

    fs.close();

    return rec_count;
}
