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

#ifndef __THREAD_H
#define __THREAD_H

#pragma once


#include <pthread.h>

#include "util.h"
#include "mutex.h"
#include "queue.h"
#include "log.h"

#define THREAD_PAGE_SIZE 4096
#define THREAD_STACK_SIZE (8 * THREAD_PAGE_SIZE)

namespace cm_thread {


class basic_thread: public cm::mutex {

private:
    basic_thread(const basic_thread &r) { /* do not implement */ }

protected:

    pthread_t tid = 0;
    pid_t sys_tid = 0;
    pthread_attr_t attr;
    int rc = 0;

    timespec delay = {0, 1000000}; 
    bool started = false;
    bool done = false;

    static void cleanup_handler(void *);    
    static void *run_handler(void*);

    void set_delay(timespec &ts) { delay = ts; }
    timespec get_delay() { return delay; }

    virtual bool setup() { return true; }
    virtual void cleanup() { }
    virtual bool process() {  return true; }

public:
    basic_thread();
    ~basic_thread();

    pid_t thread_id() { return sys_tid; }

    bool is_started() { return started; }
    bool is_done() { return done; }
    bool is_valid() { return rc == 0; }

    // starts thread if not already started
    void start();

    // stops running thread
    void stop();

};

////////////////////// thread pool //////////////////////////////


#define cm_task_function(fn) void (*fn)(void *)
#define cm_task_dealloc(fn) void (*fn)(void *)

struct task {

    cm_task_function(function) = nullptr;
    void *arg = nullptr;
    cm_task_dealloc(dealloc) = nullptr;
    bool done = false;
    
    task() {}

    task(cm_task_function(fn), void *arg_, cm_task_dealloc(dealloc_)):
        function(fn), arg(arg_), done(false), dealloc(dealloc_) { }

    ~task() { }


    task(const task &r):
        function(r.function), arg(r.arg), done(r.done), dealloc(r.dealloc) { }

    task &operator = (const task &r) {
        function = r.function;
        arg = r.arg;
        done = r.done;
        dealloc = r.dealloc;
        return *this;
    }

};

class pool;

class worker_thread: public basic_thread {

    pool *thread_pool;  
    task thread_work_task;
    size_t task_count = 0;
  
    bool process();

public:
    worker_thread(pool *p);
    ~worker_thread();

    size_t count() { return task_count; }  
    void count_clear() { task_count = 0; }  
};


class pool {

    std::vector<worker_thread *> threads;
    cm_queue::double_queue<task> work_queue;

    cm::mutex   que_mutex;
    cm::cond    que_access;

    bool shutdown = false;
    size_t running_count = 0;

public:
    pool(int size);
    ~pool();

    void task_begin() { running_count++; }
    void task_end() { running_count--; }

    size_t work_queue_count() { return work_queue.size(); }
    size_t thread_count() { return threads.size(); }

    void add_task(cm_task_function(fn), void *arg, cm_task_dealloc(dealloc_) = nullptr);
    bool next_task(task &work_task);
    void wait_all();

    void log_counts();
    
};


} // namespace cm_thread


#endif
