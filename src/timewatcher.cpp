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

#include "timewatcher.h"

extern "C" {
// read realtime clock 10x per second
void *timewatcher::handler(void *p) {
        timewatcher *tw = (timewatcher*)p;
        timespec ts = {0, 100000000};  // 100ms
        while (true) {
        	clock_gettime(CLOCK_REALTIME, &tw->now);
		nanosleep(&ts, NULL);
        }
        return NULL;
}}


timewatcher::timewatcher() {
        clock_gettime(CLOCK_REALTIME, &now);
        pthread_create(&tid, NULL, &handler, (void*) this);
}

timewatcher::~timewatcher() {
        pthread_cancel(tid);	/* request thread cancel */
	pthread_join(tid, NULL);/* wait here until its done */
}

timespec timewatcher::readTime() {
	return now;
}

timespec cm_time::clock_res() {
    timespec ts; 
    clock_getres(CLOCK_REALTIME, &ts);
    return ts;
}

timespec cm_time::clock_time() {
	return timeWatcher().readTime();
}

// from the clock time, return seconds and optional millis and nanos
time_t cm_time::clock_seconds(time_t *millis, time_t *nanos) {
	timespec ts = cm_time::clock_time();
	if(NULL != millis) {
		*millis = cm_time::millis(ts);
	}
	if(NULL != nanos) {
		*nanos = ts.tv_nsec;
	}
	return ts.tv_sec; 
}

// from the clock time, return seconds
time_t cm_time::clock_seconds() {
	return cm_time::clock_seconds(NULL, NULL);
}

// seconds from timespec
time_t cm_time::seconds(timespec &ts) {
	return ts.tv_sec;
}

// compute milliseconds from timespec
time_t cm_time::millis(timespec &ts) {
	return ts.tv_nsec / 1000000L;	
}

// nanoseconds from timespec
time_t  cm_time::nanos(timespec &ts) {
        return ts.tv_nsec;
}

// compute total milliseconds from timespec
time_t cm_time::total_millis(timespec &ts) {
        return (cm_time::seconds(ts) * 1000L) + cm_time::millis(ts);
}

double cm_time::duration(const timespec &start, const timespec &finish) {

    time_t start_secs, finish_secs;
    long start_ns, finish_ns;

    if(finish.tv_sec >= start.tv_sec) {  // expected order
        start_secs = start.tv_sec;
        start_ns = start.tv_nsec;
        finish_secs = finish.tv_sec;
        finish_ns = finish.tv_nsec;
    } else {                        // order reversed
        start_secs = finish.tv_sec;
        start_ns = finish.tv_nsec;
        finish_secs = start.tv_sec;
        finish_ns = start.tv_nsec;
    }

    time_t seconds = finish_secs - start_secs;
    long ns = finish_ns - start_ns;

    if(start_ns > finish_ns) { // clock underflow
        --seconds;
        ns += 1000000000;
    }

    return ((double) seconds + (double) ns / (double) 1000000000);

}

std::string cm_time::clock_gmt_timestamp() {
    time_t seconds = cm_time::clock_seconds();
    return cm_util::format_field_timestamp(seconds, true);
}

// Call gets singleton instance 
timewatcher& timeWatcher() {
        static timewatcher tw;
        return tw;
}
