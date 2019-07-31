
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

