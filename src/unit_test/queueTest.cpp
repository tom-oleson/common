/**********************************************************************
*
* queueTest.cpp
*
**********************************************************************/

#include <cppunit/config/SourcePrefix.h>

#include <iostream>	// for cout
#include <cstdint>	// for uint32_t
#include <arpa/inet.h> 	// for htonl()

#include "queueTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( queueTest );

//void queueTest::setUp() { }
//void queueTest::tearDown() { }

void queueTest::test_memory_queue() {

    // test push_back and pop_front
    cm_queue::mem_queue.push_back( cm_util::get_hostname() );
    std::string host = cm_queue::mem_queue.pop_front();
    CPPUNIT_ASSERT(host == cm_util::get_hostname());
    CPPUNIT_ASSERT(cm_queue::mem_queue.size() == 0);

    // test swap_out: load up with 10,000 strings then get them all out
    for(int x = 0; x < 10000; x++) {
       cm_queue::mem_queue.push_back( "deadbeef" ); 
    }
    std::deque<std::string> q;
    cm_queue::mem_queue.swap_out(q);

    CPPUNIT_ASSERT(q.size() == 10000);
    CPPUNIT_ASSERT(cm_queue::mem_queue.size() == 0);

    // push the 10,000 strings back into mem_queue
    cm_queue::mem_queue.push_back(q);
    CPPUNIT_ASSERT(cm_queue::mem_queue.size() == 10000);
    CPPUNIT_ASSERT(q.size() == 10000);
}

