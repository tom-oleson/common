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

#ifndef __TIMEWATCHER_H
#define __TIMEWATCHER_H


#include "util.h"

#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>


class timewatcher {

        pthread_t tid;
        timespec now;

        timewatcher();
        timewatcher(const timewatcher &r) { /* singleton: do not implement */ }
	~timewatcher();

	static void *handler(void*); /* thread handler */ 

	public:

	friend timewatcher& timeWatcher();

	// read most recently stored time and return a copy
	timespec readTime();

        // seconds from most recently stored time
        time_t getTime();

	// seconds from the specified timespec
	time_t timeSeconds(timespec &ts);

	// compute millis from nanos
	time_t timeMillis(timespec &ts);

	// compute total millis from seconds and nanos
	int64_t timeTotalMillis(timespec &ts);
};

timespec readTime();
time_t getTime();
time_t timeSeconds(timespec &ts);
time_t timeMillis(timespec &ts);
time_t timeNanos(timespec &ts);
time_t timeTotalMillis(timespec &ts);

timewatcher& timeWatcher();

#endif
