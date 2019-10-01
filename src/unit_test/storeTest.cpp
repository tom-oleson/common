/**********************************************************************
*
* storeTest.cpp
*
**********************************************************************/

#include <cppunit/config/SourcePrefix.h>

#include <iostream>	// for cout
#include <cstdint>	// for uint32_t
#include <arpa/inet.h> 	// for htonl()

#include "storeTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( storeTest );

//void storeTest::setUp() { }
//void storeTest::tearDown() { }

void storeTest::test_memory_store() {

    // test set and get for 'name'
    cm_store::mem_store.set("host", cm_util::get_hostname());
    std::string host = cm_store::mem_store.find("host");
    CPPUNIT_ASSERT(host == cm_util::get_hostname());

    // test pre-check function to see if 'name' is present
    bool found = cm_store::mem_store.check("host");
    CPPUNIT_ASSERT(found == true);

    // test return of default value when 'name' is not present
    std::string some_option = cm_store::mem_store.get("some_option", "none" );
    CPPUNIT_ASSERT(some_option == "none");

    cm_store::mem_store.set("deadbeef", "deadbeef");
    std::string bin = cm_store::mem_store.find("deadbeef");
    CPPUNIT_ASSERT(bin == "deadbeef");

    cm_store::mem_store.remove("deadbeef");

    // test for something not there
    std::string value = cm_store::mem_store.find("deadbeef");
    CPPUNIT_ASSERT(value == "");

    CPPUNIT_ASSERT( cm_store::mem_store.size() == 1);
}

