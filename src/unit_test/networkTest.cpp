/**********************************************************************
*
* networkTest.cpp
*
**********************************************************************/

#include <cppunit/config/SourcePrefix.h>

#include <iostream>	// for cout
#include <cstdint>	// for uint32_t
#include <arpa/inet.h> 	// for htonl()

#include "networkTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( networkTest );

//void networkTest::setUp() { }
//void networkTest::tearDown() { }


void networkTest::test_network() {

    cm_log::file_logger log("./log/network_test.log");
    set_default_logger(&log);


    CPPUNIT_ASSERT( true  == true );

}

