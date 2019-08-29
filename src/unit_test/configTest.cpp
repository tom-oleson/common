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

    cm_config::file_config cf("./config_test.cfg");
    cf.load();
    set_default_config(&cf);

    bool found = cm_config::check("section1.option1");
    CPPUNIT_ASSERT(found == true);

    std::string option1 = cm_config::get("section1.option1");
    CPPUNIT_ASSERT(option1 == "value1");

    std::string option2 = cm_config::get("section2.option2");
    CPPUNIT_ASSERT(option2 == "value2");

    std::string option3 = cm_config::get("section3.option3");
    CPPUNIT_ASSERT(option3 == "host.name");

    std::string option4 = cm_config::get("section4.option4");
    CPPUNIT_ASSERT(option4 == "12345678");

    std::string option = cm_config::get("option");
    CPPUNIT_ASSERT(option == "base level value");

    std::string wild_guess = cm_config::get("never_find", "not there");
    CPPUNIT_ASSERT(wild_guess == "not there");

    std::string optionA = cm_config::get("section4.sectionA.optionA");
    CPPUNIT_ASSERT(optionA == "123");
}
