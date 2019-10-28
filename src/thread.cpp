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
    tp->tid = 0;
    tp->done = true;
    tp->cleanup();
}

void *cm_thread::basic_thread::run_handler(void *p) {
    cm_thread::basic_thread *tp = (cm_thread::basic_thread*)p;

    pthread_cleanup_push(cleanup_handler, p);

    tp->sys_tid = cm_util::tid();

    if(tp->setup()) {
        tp->started = true;
        while(tp->process()) {
            nanosleep(&tp->delay, NULL);
        }
    }

    pthread_cleanup_pop(1); /* pop and call cleanup_handler */
    return NULL;
}

}

cm_thread::basic_thread::basic_thread():
 tid(0), started(false), done(false), rc(0) {
    
}

cm_thread::basic_thread::~basic_thread() {
    if(is_started() && !is_done()) stop();
}

void cm_thread::basic_thread::start() {
   
    if(tid == 0) {
        started = done = false;

        rc = pthread_attr_init(&attr);

        rc = pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE);

        rc = pthread_create(&tid, &attr, &run_handler, (void*) this);

        while(!is_started() && !is_done()) {
            nanosleep(&delay, NULL);
        }
    }
}

void cm_thread::basic_thread::stop() {

    if(tid != 0) {    
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
}

///////////////////// thread pool ////////////////////////////////

bool cm_thread::worker_thread::process() {
    
    bool do_work = thread_pool->next_task(thread_work_task);
    thread_pool->task_begin();
    if(do_work) {
        thread_work_task.function(thread_work_task.arg);
        thread_work_task.done = true;
        if(thread_work_task.arg && thread_work_task.dealloc) {
            thread_work_task.dealloc(thread_work_task.arg);
        }
        task_count++;
    }
    thread_pool->task_end();
    return do_work;
}

cm_thread::worker_thread::worker_thread(cm_thread::pool *p): thread_pool(p),
    task_count(0) {
    start();
}

cm_thread::worker_thread::~worker_thread() { stop(); }

cm_thread::pool::pool(int size): shutdown(false), running_count(0) {

    for(int n = 0; n < size; ++n) {
        worker_thread *p = new worker_thread(this);
        threads.push_back(p);
    }
}

cm_thread::pool::~pool() {

    shutdown = true;
    que_access.broadcast();

    for(auto p: threads) {
        while(!p->is_done()) {
            que_access.broadcast();
        }
    }

    for(auto it = threads.begin(); it != threads.end();) {
        delete (*it);
        it = threads.erase(it);
    }
}

void cm_thread::pool::add_task(cm_task_function(fn), void *arg, cm_task_dealloc(dealloc)) {

    task work_task(fn, arg, dealloc);

    que_mutex.lock();
    work_queue.push_back(work_task);
    que_mutex.unlock();

    que_access.signal();
}

bool cm_thread::pool::next_task(task &work_task) {

    if(shutdown) return false;

    que_mutex.lock();
    while(work_queue.empty()) {
        if(shutdown) {
            que_mutex.unlock();
            que_access.broadcast();
            return false;
        }
        que_access.wait(que_mutex);
    }

    work_task = work_queue.pop_front();

    que_mutex.unlock();
    que_access.signal();

    return true;
}

void cm_thread::pool::wait_all() {

    while(!work_queue.empty() || running_tasks() > 0) {
        timespec delay = {0, 100000000};   // 100 ms
        nanosleep(&delay, NULL);           
    }
}

void cm_thread::pool::log_counts() {

    size_t n = threads.size();
    cm_log::info(cm_util::format("Threads in pool: %lu", n));

    size_t total_count = 0;
    for(auto p: threads) {
        total_count += p->count();
    }

    for(auto p: threads) {
        size_t count = p->count();
        double percent = ((double) count / (double) total_count) * 100;

        cm_log::info(cm_util::format("Thread(%5d): %10lu:%7.2lf%%",
             p->thread_id(), count, percent));

        p->count_clear();
    }

    cm_log::info(cm_util::format("Total tasks completed: %lu", total_count));
}
