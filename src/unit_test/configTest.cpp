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

    cm_config::set("host", cm_util::get_hostname());

    std::string host = cm_config::get("host");
    CPPUNIT_ASSERT(host == cm_util::get_hostname());

    bool found = cm_config::check("host");
    CPPUNIT_ASSERT(found == true);
}
