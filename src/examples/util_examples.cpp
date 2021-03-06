/*
 * Copyright (C)2019, Tom Oleson <tom dot oleson at gmail dot com>
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
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
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
#include "util.h"
#include "log.h"
#include "color.h"


void format_example() {

    // show millis in logger output
    cm_log::console.set_message_format("${date_time}${millis} [${lvl}]: ${msg}");
    bool color_save_enabled = cm_log::console.get_color_enable();
    cm_log::console.set_color_enable(true);

    timespec delay = {1, 500000000};   // 1.5 seconds

    timespec start, last, now;
    clock_gettime(CLOCK_REALTIME, &start);
    last = start;

    for(int n = 1; n <= 10; n++) {

        nanosleep(&delay, NULL);  // interruptable
        clock_gettime(CLOCK_REALTIME, &now);

        double diff = cm_time::duration(last, now);
        double total = cm_time::duration(start, now);
        double delta = total - ((double) n * 1.5);
        last = now;

        //cm_color::put(CM_COLOR_BOLD);
        //cm_color::put(CM_FG_GREEN);
        cm_log::info(cm_util::format("pass: " CM_FG_CYAN "%7.4lf" CM_FG_GREEN " secs   total: " CM_FG_YELLOW "%7.4lf" CM_FG_GREEN " secs   delta: " CM_FG_RED "%7.4lf" CM_FG_GREEN " secs", diff, total, delta));
        //cm_color::put(CM_COLOR_RESET);
    }

    cm_log::console.set_color_enable(color_save_enabled);
}

