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

#ifndef __BUFFER_H
#define __BUFFER_H

#include <stdlib.h>
#include <string.h>

namespace cm_buf {

struct buffer {

    size_t len;             /* bytes written to buffer */
    size_t sz;              /* buffer size */
    char *buf = nullptr;    /* buffer address */

    buffer(char *_buf, size_t _sz): buf(_buf), sz(_sz) { clear(); }

    inline size_t available() { return sz - len; }
    inline size_t size() { return sz; }
    inline size_t length() { return len; }


    inline void clear() { len = 0; buf[0] = '\0'; }
    
    inline void append(char ch) { 
        if(len < sz) {
            buf[len++] = ch;
            buf[len] = '\0';
        }
    }

    inline void append(const char *s, size_t s_sz) {

        size_t n = ( s_sz < available() ? s_sz : available()) - 1;
        memcpy(&buf[len], s, n);
        len += n;
        buf[len] = '\0';

    }

    // append C-string to buffer
    inline void append(const char *s) {
        append(s, strlen(s));
    }

    inline void append(int i) {
        size_t n = available();
        int ss = snprintf(&buf[len], n, "%d", i);
        len += (ss >= n ? n : ss );
    }

    inline void append(long l) {
        size_t n = available();
        int ss = snprintf(&buf[len], n, "%ld", l);
        len += (ss >= n ? n : ss );
    }

    inline void append(double f) {
        size_t n = available();
        int ss = snprintf(&buf[len], n, "%f", f);
        len += (ss >= n ? n : ss);
    }

};
 
} // namespace cm_util


#endif
