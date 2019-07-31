
#include "log.h"


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


