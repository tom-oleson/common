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

#include "log.h"
#include "examples.h"

void stdout_default_logger_example() {

    get_default_logger().set_message_format("${date_time} [${lvl}]: ${msg}");

    cm_log::info("This log message will go to stdout.");
   
    cm_log::trace("This message should not be seen on stdout.");


    get_default_logger().set_log_level(cm_log::level::trace);
    cm_log::trace("This second trace message will now be allowed on stdout.");

}


void set_default_logger_example() {

    cm_log::file_logger log("./example.log");
    log.set_message_format("${date_time}${millis} [${lvl}] <${thread}>: ${msg}"); 
    set_default_logger(&log);


    cm_log::info("This log message will go to the file example.log.");
}

void include_source_location_example() {

    cm_log::file_logger dlog("./example_debug.log");
    dlog.set_message_format("${date_time}${millis} [${lvl}] <${thread}> src=[${file}:${func}:${line}]: ${msg}");
    dlog.set_log_level(cm_log::level::debug);


    dlog.info("This is the debug log file: example_debug.log.");
    dlog.debug("This log message will go to the file example_debug.log.");
    dlog.debug("This second log message will also go there.");
}

void date_time_format_example() {

    cm_log::file_logger log("./date_time_example.log");
    log.set_message_format("${date_time} [${lvl}] <${thread}>: ${msg}");
    set_default_logger(&log);

    cm_log::info("This is the default date_time format (%m/%d/%Y %H:%M:%S).");

    log.set_date_time_format("%s");
    cm_log::info("The  number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC).");

    log.set_date_time_format("%F %T");
    cm_log::info("Equivalent to %Y-%m-%d (the ISO 8601 date format) and %H:%M:%S (24-hour time)."); 

    log.set_date_time_format("%F %T %Z");
    cm_log::info("Same as previous with time zone abbreviation.");

    log.set_date_time_format("%F %T %z");
    cm_log::info("Same as previous but with +hhmm or -hhmm time zone offset.");

    log.set_date_time_format("%F %T Z");
    log.set_gmt(true);  // output UTC time
    cm_log::info("Same as previous but UTC time instead of local time.");
}


void multiplexed_logs_example() {

    cm_log::file_logger err_log("./mx_error.log");
    err_log.set_log_level(cm_log::level::error);
    err_log.set_date_time_format("%F %T");
    err_log.set_gmt(true); // output UTC time 
    err_log.set_message_format("${date_time}Z [${lvl}]: ${msg}");

    cm_log::file_logger app_log("./mx_app.log");
    app_log.set_log_level(cm_log::level::info);
    app_log.set_date_time_format("%m/%d/%Y %H:%M:%S");
    app_log.set_message_format("${date_time} [${lvl}]: ${msg}");

    cm_log::file_logger trace_log("./mx_trace.log");
    trace_log.set_log_level(cm_log::level::trace);
    trace_log.set_date_time_format("%s");
    trace_log.set_message_format("${date_time}${millis} [${lvl}] <${thread}> [${file}:${func}:${line}]: ${msg}");

    cm_log::multiplex_logger mx;
    mx.add(err_log);
    mx.add(app_log);
    mx.add(trace_log);

    set_default_logger(&mx);

    cm_log::info("This message will go to app_log and trace_log.");
    cm_log::error("This message will go to all three logs.");
    cm_log::debug("This message will go to trace_log only.");
    cm_log::trace("This message will go to trace_log only.");
}

void set_message_format_example() {

    cm_log::file_logger log("./message_format_example.log");
    set_default_logger(&log);

    cm_log::info("This message has the default message format given to newly created loggers.");

    // make sure we use local time for proper time-zone offset
    log.set_gmt(false); 
    log.set_message_format("${date_time}${millis}${tz} [${lvl}] <${thread}>: ${msg}");
    cm_log::info("This log message adds milliseconds, time-zone offset (+|-hh:mm) and thread id.");

    log.set_message_format("${date_time} [${lvl}] <${host}>: ${msg}");
    cm_log::info("This log message adds host name.");

    log.set_message_format("[${lvl}] ${msg}");
    cm_log::info("This log message is minimal with level and message only.");

}


void hex_dump_to_log_example() {

    cm_log::file_logger log("./hex_dump_example.log");
    set_default_logger(&log);

    const char *str = "This string will be\n\rdumped to the log file in\n\r hexidecimal format.\n\r";
    cm_log::hex_dump(cm_log::level::info, str, strlen(str), 16);

}

