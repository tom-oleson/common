/**********************************************************************
*
* configTest.cpp
*
**********************************************************************/

#include <cppunit/config/SourcePrefix.h>

#include <iostream>	// for cout
#include <cstdint>	// for uint32_t
#include <arpa/inet.h> 	// for htonl()

#include "configTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( configTest );

//void configTest::setUp() { }
//void configTest::tearDown() { }

void configTest::test_memory_config() {

    // test set and get for 'name'
    cm_config::set("host", cm_util::get_hostname());
    std::string host = cm_config::get("host");
    CPPUNIT_ASSERT(host == cm_util::get_hostname());

    // test pre-check function to see if 'name' is present
    bool found = cm_config::check("host");
    CPPUNIT_ASSERT(found == true);

    // test return of default value when 'name' is not present
    std::string some_option = cm_config::get("some_option", "none" );
    CPPUNIT_ASSERT(some_option == "none");
}


void configTest::test_file_config() {

    cm_config::file_config cf("./test.cfg");
    cf.load();
}
