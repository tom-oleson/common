/**********************************************************************
*
* logTest.cpp
*
**********************************************************************/

#include <cppunit/config/SourcePrefix.h>

#include <iostream>	// for cout
#include <cstdint>	// for uint32_t
#include <arpa/inet.h> 	// for htonl()

#include "logTest.h"

#include "log.h"
#include "timewatcher.h"


CPPUNIT_TEST_SUITE_REGISTRATION( logTest );

//void logTest::setUp() { }
//void logTest::tearDown() { }

void logTest::test_default_logger() {

    // create a file logger 
    cm_log::file_logger my_log("./log/default_logger.log");
    my_log.set_log_level(cm_log::level::trace);

    // set it as the default logger to be used by package macros
    set_default_logger(&my_log);
    
    // see if our convience macros output to newly set default logger
    CM_LOG_ALWAYS { cm_log::always("This always message should be in the default log."); }
    CM_LOG_FATAL { cm_log::fatal("This fatal message should be in the default log."); }
    CM_LOG_CRITICAL { cm_log::critical("This critical message should be in the default log."); }
    CM_LOG_ERROR { cm_log::error("This error message should be in the default log."); }
    CM_LOG_WARNING { cm_log::warning("This warning message should be in the default log."); }
    CM_LOG_INFO { cm_log::info("This info message should be in the default log."); }
    CM_LOG_DEBUG { cm_log::debug("This debug message should be in the default log."); }
    CM_LOG_TRACE { cm_log::trace("This trace message should be in the default log."); }
}

void logTest::test_format_log_timestamp() {

	std::string fmt = "%m/%d/%Y %H:%M:%S";
	time_t seconds = 1564244978; 

	std::string gmt_result = cm_log::format_log_timestamp(fmt, seconds, /*gmt*/ true);
    CPPUNIT_ASSERT( gmt_result == "07/27/2019 16:29:38" );

    // output as local time
	std::string local_result = cm_log::format_log_timestamp(fmt, seconds, /*gmt*/ false);
    CPPUNIT_ASSERT( local_result != gmt_result );


    // test %z
    fmt = "%F %T %z";
    std::string tz_result = cm_log::format_log_timestamp(fmt, seconds, false);

}

void logTest::test_format_millis() {

	std::string result;

	result = cm_log::format_millis(0);
	CPPUNIT_ASSERT(result == ".000");

    result = cm_log::format_millis(50);
    CPPUNIT_ASSERT(result == ".050");

    result = cm_log::format_millis(999);
    CPPUNIT_ASSERT(result == ".999");
}

void logTest::test_file_logger() {

	cm_log::file_logger log("./log/file_logger.log");
	log.set_log_level(cm_log::level::trace);

	log.always("Hello, world!");
	log.critical("Test of critical log level.");
	log.fatal("Test of fatal log level.");
	log.error("Test of error log level.");
	log.warning("Test of the warning log level.");
	log.info("Test of info log level.");
	log.debug("Test of debug log level.");
	log.trace("Test of trace log level.");

	CPPUNIT_ASSERT( true == true );
}

void logTest::test_log_level_if_macros() {

    cm_log::file_logger log("./log/test_log_level_macros.log");
	log.set_log_level(cm_log::level::off);

	log_always(log) { log.always("This message must always be allowed in the log."); }
	log_critical(log) { log.critical("This message must not be in the log."); }
	log_fatal(log) { log.fatal("This message must not be in the log."); }
	log_error(log) { log.error("This message must not be in the log."); }
	log_warning(log) { log.warning("This message must not be in the log."); }
	log_info(log) { log.info("This message must not be in the log."); } 
	log_debug(log) { log.debug("This message must not be in the log."); }
	log_trace(log)  { log.trace("This message must not be in the log."); }
}

void logTest::test_get_part_index() {

	int index;

	CPPUNIT_ASSERT((index = cm_log::get_part_index("date_time")) == cm_log::part::date_time);
	CPPUNIT_ASSERT((index = cm_log::get_part_index("millis")) == cm_log::part::millis);
	CPPUNIT_ASSERT((index = cm_log::get_part_index("tz")) == cm_log::part::tz);
	CPPUNIT_ASSERT((index = cm_log::get_part_index("file")) == cm_log::part::file);
	CPPUNIT_ASSERT((index = cm_log::get_part_index("line")) == cm_log::part::line);
	CPPUNIT_ASSERT((index = cm_log::get_part_index("func")) == cm_log::part::func);
	CPPUNIT_ASSERT((index = cm_log::get_part_index("thread")) == cm_log::part::thread);
	CPPUNIT_ASSERT((index = cm_log::get_part_index("host")) == cm_log::part::host);
	CPPUNIT_ASSERT((index = cm_log::get_part_index("msg")) == cm_log::part::msg);	

	CPPUNIT_ASSERT((index = cm_log::get_part_index("deadbeef")) == -1);
}

void logTest::test_parse_message_format() {

    std::string fmt = "${date_time}${millis}${tz} ${lvl} <${file}:${line}:${func}> (${host})[${thread}]: ${msg}";
    std::vector<std::string> out_fmt;

    // parse the msg format
    cm_log::parse_message_format(fmt, out_fmt); 

    std::string date_time_fmt = "%m/%d/%Y %H:%M:%S";

    cm_log::extra ext = CM_LOG_EXTRA;

    std::string result = cm_log::format_log_message(ext, date_time_fmt, out_fmt, cm_log::level::info, "Test build_log_message.", true); 

    CPPUNIT_ASSERT( true == true );


}

void logTest::test_rotate() {

    cm_log::rolling_file_logger log("./log/", "rotate", ".log", 5 /* seconds */);
    log.set_log_level(cm_log::level::info);

    // set it as the default logger to be used by package macros
    set_default_logger(&log);

    int count = 1;
    std::string s;

    while(count <= 12) {
        cm_log::info(cm_util::format(s, "Message #%d in the log file.", count));
        count++;
        sleep(1);
    }
}
