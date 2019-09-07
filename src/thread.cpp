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

#include "thread.h"

extern "C" {

void cm_thread::basic_thread::cleanup_handler(void *p) {
    cm_thread::basic_thread *tp = (cm_thread::basic_thread*)p;
    tp->cleanup();
    tp->done = true;
}

void *cm_thread::basic_thread::run_handler(void *p) {
    cm_thread::basic_thread *tp = (cm_thread::basic_thread*)p;

    pthread_cleanup_push(cleanup_handler, p);

    if(tp->setup()) {
        tp->started = true;
        while(tp->process()) {
            nanosleep(&tp->delay, NULL);    /* cancellation point */
        }
    }

    pthread_cleanup_pop(1); /* pop and call cleanup_handler */
    return NULL;
}

}


cm_thread::basic_thread::basic_thread(bool auto_start) {
    if(auto_start) start();
}

cm_thread::basic_thread::~basic_thread() {
    if(is_started() && !is_done()) stop();
}

void cm_thread::basic_thread::start() {
    pthread_create(&tid, NULL, &run_handler, (void*) this);

    while(!is_started()) {
        nanosleep(&delay, NULL);
    }
}

void cm_thread::basic_thread::stop() {

    if(pthread_self() == tid) {
        // self terminating
        pthread_exit(NULL);
    }
    else if(is_started()) {
        // being terminated by another thread
        pthread_cancel(tid);        /* request thread cancel */
        pthread_join(tid, NULL);/* wait here until its done */
    }
}


