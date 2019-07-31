
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
