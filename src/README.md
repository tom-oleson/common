<pre>
       _   _ _
 _   _| |_(_) |
| | | | __| | |
| |_| | |_| | |
 \__,_|\__|_|_|
</pre>

Frequently used functions that perform general purpose tasks.


<pre>
 _   _                              _       _
| |_(_)_ __ ___   _____      ____ _| |_ ___| |__   ___ _ __
| __| | '_ ` _ \ / _ \ \ /\ / / _` | __/ __| '_ \ / _ \ '__|
| |_| | | | | | |  __/\ V  V / (_| | || (__| | | |  __/ |
 \__|_|_| |_| |_|\___| \_/\_/ \__,_|\__\___|_| |_|\___|_|
</pre>

Low impact time source thread for applications. Reads realtime clock ten times per second. Use in loggers to avoid excessive context switching in multi-threaded applications.


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

