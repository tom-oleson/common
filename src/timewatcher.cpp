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
        timespec ts = {0, 100000000};
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

timespec readTime() {
	return timeWatcher().readTime();
}

time_t getTime(time_t *millis, time_t *nanos) {
	timespec ts = readTime();
	if(NULL != millis) {
		*millis = timeMillis(ts);
	}
	if(NULL != nanos) {
		*nanos = ts.tv_nsec;
	}
	return ts.tv_sec; 
}

time_t getTime() {
	return getTime(NULL, NULL);
}

time_t timeSeconds(timespec &ts) {
	return ts.tv_sec;
}

time_t timeMillis(timespec &ts) {
	// compute millis from nanoseconds
	return ts.tv_nsec / 1000000L;	
}

time_t  timeNanos(timespec &ts) {
        return ts.tv_nsec;
}


time_t timeTotalMillis(timespec &ts) {
        return (timeSeconds(ts) * 1000L) + timeMillis(ts);
}



// Call gets singleton instance 
timewatcher& timeWatcher() {
        static timewatcher tw;
        return tw;
}

