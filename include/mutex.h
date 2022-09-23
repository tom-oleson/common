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

#ifndef __CM_MUTEX_H
#define __CM_MUTEX_H

#include <pthread.h>

namespace cm {

class mutex {
public:
    mutex() { pthread_mutex_init(&_mutex, NULL); }
    ~mutex() { pthread_mutex_destroy(&_mutex); }

    int lock() { return pthread_mutex_lock(&_mutex); } 
    int unlock(){ return pthread_mutex_unlock(&_mutex);  }

    bool try_lock() {
        return pthread_mutex_trylock(&_mutex) == 0 ? true : false;
    }

    pthread_mutex_t _mutex;

private:

    // do not implement these constructors
    mutex(const mutex&);
    const mutex& operator=(const mutex&);

};


class cond {
public:

    cond() { pthread_cond_init(&_cond, NULL); }
    ~cond() { pthread_cond_destroy(&_cond); }

    int wait(mutex &m) { return pthread_cond_wait(&_cond, &m._mutex); }

    int timed_wait(mutex &m, const struct timespec &ts) {
        return pthread_cond_timedwait(&_cond, &m._mutex, &ts);} 

    int signal(){ return pthread_cond_signal(&_cond);  }
    int broadcast(){ return pthread_cond_broadcast(&_cond); }

    pthread_cond_t _cond;

private:
   
    // do not implement these constructors
    cond(const cond&);
    const cond& operator=(const cond&);

};


} // namespace cm

#endif
