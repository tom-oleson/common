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

#define THREAD_PAGE_SIZE 4096
#define THREAD_STACK_SIZE (8 * THREAD_PAGE_SIZE)

namespace cm_thread {


class basic_thread: public cm::mutex {

private:
    basic_thread(const basic_thread &r) { /* do not implement */ }

protected:

    pthread_t tid = 0;
    pthread_attr_t attr;
    int rc = 0;

    timespec delay = {0, 1000000}; /* 1 ms delay between process calls */
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
    basic_thread(bool auto_start = false);
    ~basic_thread();

    bool is_started() { return started; }
    bool is_done() { return done; }
    bool is_valid() { return rc == 0; }

    // starts thread if not already started
    void start();

    // stops running thread
    void stop();

};

////////////////////// thread pool //////////////////////////////


#define CM_TASK_FP(fn) void (*fn)(void *data)


struct task {

    task() {}

    task(CM_TASK_FP(fn), void *data):
        task_fn(fn), task_data(data), done(false) { }
    ~task() { }


    task(const task &r):
        task_fn(r.task_fn), task_data(r.task_data), done(r.done) { }
    task &operator = (const task &r) {
        task_fn = r.task_fn;
        task_data = r.task_data;
        done = r.done;
        return *this;
    }

    CM_TASK_FP(task_fn) = nullptr;
    void *task_data = nullptr;
    bool done = false;

};

class pool;

class worker_thread: public basic_thread {

    pool *_pool;    // pointer to thread pool
    task _task;     // current task

    bool process();

public:
    worker_thread(pool *p);
    ~worker_thread();    
};


class pool {

    std::vector<worker_thread *> threads;
    cm_queue::double_queue<task> work_queue;

    cm::mutex   que_mutex;
    cm::cond    que_access;

    bool shutdown = false;

public:
    pool(int size);
    ~pool();

    size_t work_queue_count() { return work_queue.size(); }
    size_t thread_count() { return threads.size(); }

    void add_task(CM_TASK_FP(_fn), void *_data) {
        task t(_fn, _data);

        que_mutex.lock();
        work_queue.push_back(t);
        que_mutex.unlock();

        que_access.signal();
    }

    bool next_task(task &t) {

        if(shutdown) return false;

        que_mutex.lock();
        while(work_queue.empty()) {
            if(shutdown) {
                que_mutex.unlock();
                return false;
            }
            que_access.wait(que_mutex);
        }

        t = work_queue.pop_front();

        que_mutex.unlock();
        que_access.signal();

        return true;
    }

};


} // namespace cm_thread


#endif
