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
