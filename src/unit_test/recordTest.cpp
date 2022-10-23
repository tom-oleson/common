
#include <cppunit/config/SourcePrefix.h>

#include <iostream>	// for cout
#include <cstdint>	// for uint32_t
#include <arpa/inet.h> 	// for htonl()

#include "recordTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( recordTest );

//void recordTest::setUp() { }
//void recordTest::tearDown() { }

void recordTest::test_record() {

    cm_log::file_logger log("./log/record_test.log");
    set_default_logger(&log);

    cm_record::field name("name", "string", 0);
    cm_record::field age("age", "int", 0);

    cm_log::info(name.to_string());
    cm_log::info(age.to_string());

    CPPUNIT_ASSERT( name.get_name() == "name");
    CPPUNIT_ASSERT( age.get_name() == "age");


    cm_record::record_spec person_spec("person", "1.0");

    CPPUNIT_ASSERT(person_spec.get_name() == "person");
    CPPUNIT_ASSERT(person_spec.get_version() == "1.0");

    cm_log::info(person_spec.to_string());
    
}

