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


// receive function called by connection_threads
void receive(const char *buf, size_t sz) {


}

cm_log::file_logger server_log("./log/network_test.log");

void networkTest::test_network() {

    set_default_logger(&server_log);

    cm_log::info("test_network");

    cm_net::server_thread server(56000 /* port */, receive);


    CPPUNIT_ASSERT( server.is_started() == true );

}

