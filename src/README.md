

<pre>
                  __ _
  ___ ___  _ __  / _(_) __ _
 / __/ _ \| '_ \| |_| |/ _` |
| (_| (_) | | | |  _| | (_| |
 \___\___/|_| |_|_| |_|\__, |
                       |___/
</pre>

Thread-safe general purpose application configuration component.


Set and get configuration information in memory_config example:
<pre>
#include "util.h"
#include "log.h"
#include "config.h"


void memory_config_example() {

    cm_config::set("host", cm_util::get_hostname());

    std::string host = cm_config::get("host");


    bool found = cm_config::check("host");
    if(found) {
        cm_log::info(cm_util::format("host: [%s]", host.c_str()));
    }

    // default value when key is not present
    std::string some_option = cm_config::get("some_option", "none" );
    cm_log::info(cm_util::format("some_option: [%s]", some_option.c_str()));

}
</pre>

Output:
<pre>
08/11/2019 12:21:46 [info]: host: [tom-dell]
08/11/2019 12:21:46 [info]: some_option: [none]
</pre>


Access configuration file information at different levels example:

example_config.cfg:
<pre>
# example configuration file

option = "base level"

section {

    option = "section level"

    subsection {

        option = "subsection level"
    }
}
</pre>
<pre>
void file_config_example() {

    std::string path = "./example_config.cfg";

    cm_config::file_config config_file(path);
    config_file.load();
    set_default_config(&config_file);

    // access base level option
    std::string option = cm_config::get("option", "none");

    // access section option
    std::string section_option = cm_config::get("section.option", "none");

    // access nested sub-section option
    std::string subsection_option = cm_config::get("section.subsection.option", "none");

    cm_log::info(cm_util::format("1: [%s], 2: [%s], 3: [%s]",
        option.c_str(), section_option.c_str(), subsection_option.c_str()));
}
</pre>
Output:
<pre>
08/11/2019 12:55:11 [info]: 1: [base level], 2: [section level], 3: [subsection level]
</pre>
 

<pre>
 _   _                              _       _
| |_(_)_ __ ___   _____      ____ _| |_ ___| |__   ___ _ __
| __| | '_ ` _ \ / _ \ \ /\ / / _` | __/ __| '_ \ / _ \ '__|
| |_| | | | | | |  __/\ V  V / (_| | || (__| | | |  __/ |
 \__|_|_| |_| |_|\___| \_/\_/ \__,_|\__\___|_| |_|\___|_|
</pre>

Low impact time source thread for applications. Reads realtime clock ten times per second. Used in loggers to avoid excessive context switching in multi-threaded applications.


<pre>
       _   _ _
 _   _| |_(_) |
| | | | __| | |
| |_| | |_| | |
 \__,_|\__|_|_|
</pre>

Frequently used functions that perform general purpose tasks.

Example of using cm_util::format() with log output.
<pre>
void format_example() {

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

        cm_log::info(cm_util::format("pass: %7.4lf secs   total: %7.4lf secs   delta: %7.4lf secs", diff, total, delta));

    }
}
</pre>


Output:
<pre>
08/01/2019 17:16:10 [info]: pass:  1.5002 secs   total:  1.5002 secs   delta:  0.0002 secs
08/01/2019 17:16:11 [info]: pass:  1.5004 secs   total:  3.0006 secs   delta:  0.0006 secs
08/01/2019 17:16:12 [info]: pass:  1.5003 secs   total:  4.5009 secs   delta:  0.0009 secs
08/01/2019 17:16:14 [info]: pass:  1.5002 secs   total:  6.0011 secs   delta:  0.0011 secs
08/01/2019 17:16:15 [info]: pass:  1.5002 secs   total:  7.5013 secs   delta:  0.0013 secs
08/01/2019 17:16:17 [info]: pass:  1.5002 secs   total:  9.0016 secs   delta:  0.0016 secs
08/01/2019 17:16:18 [info]: pass:  1.5002 secs   total: 10.5018 secs   delta:  0.0018 secs
08/01/2019 17:16:20 [info]: pass:  1.5002 secs   total: 12.0021 secs   delta:  0.0021 secs
08/01/2019 17:16:21 [info]: pass:  1.5002 secs   total: 13.5023 secs   delta:  0.0023 secs
08/01/2019 17:16:23 [info]: pass:  1.5002 secs   total: 15.0025 secs   delta:  0.0025 secs
</pre>

<pre>
 _
| | ___   __ _
| |/ _ \ / _` |
| | (_) | (_| |
|_|\___/ \__, |
         |___/
</pre>

Thread-safe general purpose application logger.

Output to stdout example:
<pre>
#include "log.h"

void stdout_default_logger_example() {

    get_default_logger().set_message_format("${date_time} [${lvl}]: ${msg}");

    cm_log::info("This log message will go to stdout.");

    cm_log::trace("This message should not be seen on stdout.");

    get_default_logger().set_log_level(cm_log::level::trace);
    cm_log::trace("This second trace message will now be allowed on stdout.");
}
</pre>
Output:
<pre>
07/31/2019 02:29:08 [info]: This log message will go to stdout.
07/31/2019 02:29:08 [trace]: This second trace message will now be allowed on stdout.
</pre>

Output to file logger (set as new default) example:
<pre>
void set_default_logger_example() {

    cm_log::file_logger log("./example.log");
    log.set_message_format("${date_time}${millis} [${lvl}] <${thread}>: ${msg}");
    set_default_logger(&log);

    cm_log::info("This log message will go to the file example.log.");
}
</pre>

Output:
<pre>
07/31/2019 02:34:00.306 [info] <30761>: This log message will go to the file example.log.
</pre>

Output data to log in hexidecimal format. Very handy for debugging issues related to
external data sources (such as outside servers).
<pre>
void hex_dump_to_log_example() {

    cm_log::file_logger log("./hex_dump_example.log");
    set_default_logger(&log);

    const char *str = "This string will be\n\rdumped to the log file in\n\r hexidecimal format.\n\r";
    cm_log::hex_dump(cm_log::level::info, str, strlen(str), 16);

}
</pre>
Output:

<pre>
09/10/2019 21:42:08 [info]: 54 68 69 73 20 73 74 72 69 6e 67 20 77 69 6c 6c   This string will
09/10/2019 21:42:08 [info]: 20 62 65 0a 0d 64 75 6d 70 65 64 20 74 6f 20 74    be..dumped to t
09/10/2019 21:42:08 [info]: 68 65 20 6c 6f 67 20 66 69 6c 65 20 69 6e 0a 0d   he log file in..
09/10/2019 21:42:08 [info]: 20 68 65 78 69 64 65 63 69 6d 61 6c 20 66 6f 72    hexidecimal for
09/10/2019 21:42:08 [info]: 6d 61 74 2e 0a 0d                                 mat...
</pre>

Output source code location where the log message was generated:
<pre>
void include_source_location_example() {

    cm_log::file_logger dlog("./example_debug.log");
    dlog.set_message_format("${date_time}${millis} [${lvl}] <${thread}> src=[${file}:${func}:${line}]: ${msg}");
    dlog.set_log_level(cm_log::level::debug);


    dlog.info("This is the debug log file: example_debug.log.");
    dlog.debug("This log message will go to the file example_debug.log.");
    dlog.debug("This second log message will also go there.");
}
</pre>

Output:
<pre>
07/31/2019 02:54:12.867 [info] <30893> src=[log_examples.cpp:include_source_location_example:37]: This is the debug log file: example_debug.log.
07/31/2019 02:54:12.867 [debug] <30893> src=[log_examples.cpp:include_source_location_example:38]: This log message will go to the file example_debug.log.
07/31/2019 02:54:12.867 [debug] <30893> src=[log_examples.cpp:include_source_location_example:39]: This second log message will also go there.
</pre>

Example date_time formats. See <strong>strftime()</strong> for documentation on specifiers. 
<pre>
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

</pre>

Output:
<pre>
07/31/2019 04:04:19 [info] <32103>: This is the default date_time format (%m/%d/%Y %H:%M:%S).
1564560259 [info] <32103>: The  number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC).
2019-07-31 04:04:19 [info] <32103>: Equivalent to %Y-%m-%d (the ISO 8601 date format) and %H:%M:%S (24-hour time).
2019-07-31 04:04:19 EDT [info] <32103>: Same as previous with time zone abbreviation.
2019-07-31 04:04:19 -0400 [info] <32103>: Same as previous but with +hhmm or -hhmm time zone offset.
2019-07-31 08:04:19 Z [info] <32103>: Same as previous but UTC time instead of local time.
</pre>


Example of a multiplex_logger. Each log message is processed by each
of the member loggers. Note that each logger can have a different log level,
date_time format and log message format.

<pre>
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

</pre>

Output:
<pre>
$ cat mx_app.log
07/31/2019 14:37:14 [info]: This message will go to app_log and trace_log.
07/31/2019 14:37:14 [error]: This message will go to all three logs.

$ cat mx_error.log
2019-07-31 18:37:14Z [error]: This message will go to all three logs.

$ cat mx_trace.log
1564598234.890 [info] <5379> [log_examples.cpp:multiplexed_logs_example:121]: This message will go to app_log and trace_log.
1564598234.890 [error] <5379> [log_examples.cpp:multiplexed_logs_example:122]: This message will go to all three logs.
1564598234.890 [debug] <5379> [log_examples.cpp:multiplexed_logs_example:123]: This message will go to trace_log only.
1564598234.890 [trace] <5379> [log_examples.cpp:multiplexed_logs_example:124]: This message will go to trace_log only.
</pre>

