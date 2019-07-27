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


CPPUNIT_TEST_SUITE_REGISTRATION( logTest );

//void logTest::setUp() { }
//void logTest::tearDown() { }


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
