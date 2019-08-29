/**********************************************************************
*
* threadTest.cpp
*
**********************************************************************/

#include <cppunit/config/SourcePrefix.h>

#include <iostream>	// for cout
#include <cstdint>	// for uint32_t
#include <arpa/inet.h> 	// for htonl()

#include "threadTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( threadTest );

//void threadTest::setUp() { }
//void threadTest::tearDown() { }

struct unit_thread: public cm_thread::basic_thread {

    int count = 0;

    bool process() {

        for(int x = 0; x < 10000; x++) {
            if(++count % 10000 == 0)
                cm_log::info(cm_util::format("count = [%d]", count));
        } 
        return count < 1000000;
    }  
};


void threadTest::test_thread() {

    cm_log::file_logger log("./log/thread_test.log");
    set_default_logger(&log);

    unit_thread thread;

    thread.start();

    CPPUNIT_ASSERT( thread.is_done() == false );

    sleep(2);

    CPPUNIT_ASSERT( thread.count == 1000000 );
    CPPUNIT_ASSERT( thread.is_done() == true );

}

